from django.db import models


class RoomStatus(models.Model):
    """存储上位机上报的房间状态"""
    room_id = models.CharField(max_length=20, verbose_name="房间号", unique=True)
    status = models.IntegerField(verbose_name="状态", default=0)  # 0:空闲, 1:入住, 2:清洁
    guest_name = models.CharField(max_length=50, verbose_name="住客姓名", blank=True)
    last_update = models.DateTimeField(auto_now=True, verbose_name="最后更新时间")
    device_id = models.CharField(max_length=100, verbose_name="设备ID", blank=True)

    class Meta:
        verbose_name = "房间实时状态"
        verbose_name_plural = verbose_name

    def __str__(self):
        return f"{self.room_id} - {'入住' if self.status == 1 else '空闲'}"


class RfidLog(models.Model):
    """存储刷卡记录"""
    card_id = models.CharField(max_length=50, verbose_name="卡号")
    room_number = models.CharField(max_length=20, verbose_name="房间号")
    timestamp = models.DateTimeField(auto_now_add=True, verbose_name="刷卡时间")
    raw_event = models.CharField(max_length=50, default="card_swipe")

    class Meta:
        ordering = ['-timestamp']
        verbose_name = "刷卡日志"
        verbose_name_plural = verbose_name


class SystemLog(models.Model):
    """存储上位机上报的通用日志"""
    category = models.CharField(max_length=50, verbose_name="分类")  # business, error, system
    message = models.TextField(verbose_name="日志内容")
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ['-created_at']


class SystemBackup(models.Model):
    """系统备份记录"""
    filename = models.CharField(max_length=100, verbose_name="备份文件名")
    file_path = models.CharField(max_length=255, verbose_name="文件路径")
    created_at = models.DateTimeField(auto_now_add=True, verbose_name="备份时间")
    size_kb = models.FloatField(verbose_name="大小(KB)", default=0)

    class Meta:
        ordering = ['-created_at']