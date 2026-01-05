import json
import time
import paho.mqtt.client as mqtt


# 模拟发送一条入住数据
def publish_test_data():
    client = mqtt.Client()
    client.connect("192.168.137.1", 1883, 60)

    data = {
        "room_id": "888",
        "customer_name": "测试用户",
        "card_id": "TEST_CARD_001",
        "action": "CHECK_IN"
    }

    payload = json.dumps(data)
    client.publish("topic/test", payload)
    print(f"已发送测试数据: {payload}")
    client.disconnect()


if __name__ == "__main__":
    publish_test_data()