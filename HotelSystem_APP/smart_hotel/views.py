import os
import json
import datetime
import paho.mqtt.publish as publish
from django.shortcuts import render
from django.http import JsonResponse
from django.core.management import call_command
from django.contrib.auth.decorators import login_required, user_passes_test
from django.views.decorators.csrf import csrf_exempt
from django.conf import settings
from .models import RoomStatus, SystemBackup, SystemLog

# === MQTT 配置 (用于发送指令) ===
# 请确保 IP 与您的 Qt 上位机配置一致
MQTT_BROKER = "192.168.137.1"
MQTT_PORT = 1883
MQTT_AUTH = {'username': 'wy', 'password': 'wy123'}
MQTT_CMD_TOPIC = "hotel/admin/commands"


def send_mqtt_command(cmd_dict):
    """辅助函数：发送 JSON 指令到 Qt"""
    try:
        payload = json.dumps(cmd_dict)
        publish.single(
            MQTT_CMD_TOPIC,
            payload=payload,
            hostname=MQTT_BROKER,
            port=MQTT_PORT,
            auth=MQTT_AUTH
        )
        print(f"📡 [Django] 发送指令成功: {payload}")
        return True
    except Exception as e:
        print(f"❌ [Django] 发送指令失败: {e}")
        return False


# 权限检查：必须是超级管理员
def is_superuser(user):
    return user.is_superuser


# --- 页面: 后台管理主页 ---
@login_required
@user_passes_test(is_superuser)
def system_manage_page(request):
    """渲染管理主页"""
    backups = SystemBackup.objects.all()[:10]
    rooms = RoomStatus.objects.all().order_by('room_id')
    return render(request, 'system_manage.html', {
        'backups': backups,
        'rooms': rooms
    })


# ==========================================
# [新增] 获取房间状态 JSON 接口 (用于前端轮询)
# ==========================================
@login_required
def get_rooms_json(request):
    """
    返回房间状态的 JSON 数据
    前端 JS 会每隔几秒调用一次此接口来实现无感刷新
    """
    try:
        # 获取所有房间并按 ID 排序
        rooms = RoomStatus.objects.all().order_by('room_id')

        data = []
        for r in rooms:
            # 格式化时间，防止 None 报错
            if r.last_update:
                time_str = r.last_update.strftime("%H:%M:%S")
            else:
                time_str = "--:--:--"

            data.append({
                'room_id': r.room_id,
                'status': r.status,  # 1=入住, 0=空闲
                'guest_name': r.guest_name if r.guest_name else '',  # 处理空值
                'last_update': time_str
            })

        # safe=False 允许返回列表对象
        return JsonResponse(data, safe=False)
    except Exception as e:
        return JsonResponse({'error': str(e)}, status=500)


# --- API: 同步添加用户 ---
@csrf_exempt
@login_required
@user_passes_test(is_superuser)
def sync_user(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            username = data.get('username')
            password = data.get('password')

            if not username or not password:
                return JsonResponse({'status': 'error', 'msg': '参数不完整'})

            # 构造协议包 (Type: CMD_ADD_USER)
            cmd = {
                "type": "CMD_ADD_USER",
                "username": username,
                "password": password
            }

            if send_mqtt_command(cmd):
                # 记录日志
                SystemLog.objects.create(category="security", message=f"后台添加用户: {username}")
                return JsonResponse({'status': 'success', 'msg': f'指令已发送：添加用户 {username}'})
            else:
                return JsonResponse({'status': 'error', 'msg': 'MQTT 连接失败，无法发送指令'})

        except Exception as e:
            return JsonResponse({'status': 'error', 'msg': str(e)})
    return JsonResponse({'status': 'error', 'msg': 'Method Not Allowed'})


# --- API: 同步删除用户 ---
@csrf_exempt
@login_required
@user_passes_test(is_superuser)
def sync_delete_user(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            username = data.get('username')

            if not username:
                return JsonResponse({'status': 'error', 'msg': '用户名不能为空'})

            # 构造协议包 (Type: CMD_DEL_USER)
            cmd = {
                "type": "CMD_DEL_USER",
                "username": username
            }

            if send_mqtt_command(cmd):
                # 记录日志
                SystemLog.objects.create(category="security", message=f"后台删除用户: {username}")
                return JsonResponse({'status': 'success', 'msg': f'指令已发送：删除用户 {username}'})
            else:
                return JsonResponse({'status': 'error', 'msg': 'MQTT 连接失败'})

        except Exception as e:
            return JsonResponse({'status': 'error', 'msg': str(e)})
    return JsonResponse({'status': 'error', 'msg': 'Method Not Allowed'})


# --- API: 备份数据 ---
@login_required
@user_passes_test(is_superuser)
def backup_data(request):
    backup_dir = os.path.join(settings.BASE_DIR, 'backups')
    if not os.path.exists(backup_dir):
        os.makedirs(backup_dir)

    # 生成新文件名
    timestamp = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
    filename = f"hotel_backup_{timestamp}.json"
    filepath = os.path.join(backup_dir, filename)

    try:
        # 1. 执行备份：导出数据到文件
        with open(filepath, 'w', encoding='utf-8') as f:
            call_command('dumpdata', 'smart_hotel', stdout=f)

        size_kb = os.path.getsize(filepath) / 1024

        # 2. 在数据库创建新记录
        new_backup = SystemBackup.objects.create(
            filename=filename,
            file_path=filepath,
            size_kb=round(size_kb, 2)
        )

        # 3. 清理旧备份：查询所有 ID 不等于当前新备份 ID 的记录
        old_backups = SystemBackup.objects.exclude(id=new_backup.id)

        for bk in old_backups:
            # (A) 删除物理文件
            if bk.file_path and os.path.exists(bk.file_path):
                try:
                    os.remove(bk.file_path)
                except Exception as e:
                    print(f"删除旧备份文件失败: {e}")

            # (B) 删除数据库记录
            bk.delete()

        return JsonResponse({'status': 'success', 'msg': f'备份成功！旧版本已清理，当前版本: {filename}'})

    except Exception as e:
        return JsonResponse({'status': 'error', 'msg': f'备份失败: {str(e)}'})

# 恢复数据
@csrf_exempt
@login_required
@user_passes_test(is_superuser)
def restore_data(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            backup_id = data.get('id')

            try:
                backup = SystemBackup.objects.get(id=backup_id)
            except SystemBackup.DoesNotExist:
                return JsonResponse({'status': 'error', 'msg': '备份记录未找到'})

            if not os.path.exists(backup.file_path):
                return JsonResponse({'status': 'error', 'msg': '备份物理文件已丢失'})

            # 1. 恢复 Django 数据库 (此处实际逻辑需根据业务需求完善，通常需要重启服务或重新加载)
            # 这里主要是演示通知上位机

            cmd_payload = {
                "type": "CMD_SYSTEM_RESTORE",  # Qt 识别的指令头
                "action": "execute_local_restore",  # 辅助描述
                "msg": "Server requested local restore"
            }

            # 3. 发送 MQTT 指令
            publish.single(
                topic=MQTT_CMD_TOPIC,
                payload=json.dumps(cmd_payload),
                hostname=MQTT_BROKER,
                auth={'username': "wy", 'password': "wy123"}
            )

            print("📡 [Django] 已发送恢复指令给上位机")
            return JsonResponse({'status': 'success', 'msg': '已发送指令，上位机将执行本地恢复！'})

        except Exception as e:
            return JsonResponse({'status': 'error', 'msg': str(e)})
    return JsonResponse({'status': 'error', 'msg': 'Method Not Allowed'})