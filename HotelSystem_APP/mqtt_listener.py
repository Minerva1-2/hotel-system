import os
import json
import django
import paho.mqtt.client as mqtt

# 1. 初始化 Django 环境
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'HotelSystem.settings')  # 确保项目名正确
django.setup()

from smart_hotel.models import RoomStatus, RfidLog, SystemLog

# === 配置 (需与 Qt .pro 文件中的 DEFINES 保持一致) ===
BROKER_IP = "192.168.137.1"
BROKER_PORT = 1883
USERNAME = "wy"
PASSWORD = "wy123"

# Qt 上报的主题前缀 (对应 Qt 中的 MQTT_TOPIC)
TOPIC_ROOT = "topic/hotel"
TOPIC_SUB = "#"  # 订阅所有子主题


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"✅ [Python] 已连接 MQTT: {BROKER_IP}")
        client.subscribe(TOPIC_SUB)
        print(f"📡 [Python] 正在监听: {TOPIC_SUB}")
    else:
        print(f"❌ [Python] 连接失败, 错误码: {rc}")


def on_message(client, userdata, msg):
    try:
        topic = msg.topic
        payload_str = msg.payload.decode('utf-8')
        # print(f"📩 [{topic}]: {payload_str}") # 调试用，生产环境可注释

        # -------------------------------------------------------
        # 场景 A: 房间状态上报 (CloudManager::uploadRoomStatus)
        # Topic: topic/hotel/room_status
        # -------------------------------------------------------
        if topic.endswith("/room_status"):
            data = json.loads(payload_str)
            room_id = data.get('room_id')
            if room_id:
                # 更新或创建记录
                RoomStatus.objects.update_or_create(
                    room_id=room_id,
                    defaults={
                        'status': data.get('status', 0),
                        'guest_name': data.get('guest_name', ''),
                        'device_id': data.get('device_id', '')
                    }
                )
                print(f"💾 [Django] 房间 {room_id} 状态已更新")

        # -------------------------------------------------------
        # 场景 B: RFID 刷卡上报 (CloudManager::publishRfidData)
        # Topic: topic/hotel/rfid
        # -------------------------------------------------------
        elif topic.endswith("/rfid"):
            data = json.loads(payload_str)
            RfidLog.objects.create(
                card_id=data.get('card_id', 'unknown'),
                room_number=data.get('room_number', 'unknown'),
                raw_event=data.get('event', 'card_swipe')
            )
            print(f"💾 [Django] 记录刷卡: {data.get('card_id')}")

        # -------------------------------------------------------
        # 场景 C: 通用日志 (CloudManager::publishLog)
        # Topic: topic/hotel/business, topic/hotel/error 等
        # -------------------------------------------------------
        else:
            # 提取子主题作为分类 (例如 business)
            category = topic.split('/')[-1]
            if category not in ['room_status', 'rfid']:  # 避免重复
                SystemLog.objects.create(
                    category=category,
                    message=payload_str
                )
                # print(f"💾 [Django] 日志已归档: {category}")

    except json.JSONDecodeError:
        pass  # 忽略非JSON日志
    except Exception as e:
        print(f"❌ [Error] 处理消息异常: {e}")


if __name__ == "__main__":
    client = mqtt.Client()
    if USERNAME and PASSWORD:
        client.username_pw_set(USERNAME, PASSWORD)

    client.on_connect = on_connect
    client.on_message = on_message

    print(f"🚀 [System] 启动 MQTT 监听服务...")
    try:
        client.connect(BROKER_IP, BROKER_PORT, 60)
        client.loop_forever()
    except Exception as e:
        print(f"❌ 无法连接: {e}")