#include "cloudmanager.h"
#include <QDebug>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>

// 单例模式实现
CloudManager& CloudManager::instance()
{
    static CloudManager instance;
    return instance;
}

CloudManager::CloudManager(QObject *parent) : QObject(parent)
{
    // 实例化 MQTT 客户端
    // 初始使用 LocalHost，实际连接时会被 connectToCloud 的参数覆盖
    m_client = new QMQTT::Client(QHostAddress(MQTT_IP), 1883, this);

    // 生成唯一的 Client ID (格式: Hotel_Device_时间戳)
    // 防止因 ID 重复导致掉线
    QString clientId = "Hotel_System_WeiYu_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    m_client->setClientId(clientId);

    // 连接信号槽，用于处理连接状态变化
    connect(m_client, &QMQTT::Client::connected, this, &CloudManager::onConnected);
    connect(m_client, &QMQTT::Client::disconnected, this, &CloudManager::onDisconnected);

    // 【新增】连接收到消息的信号
    connect(m_client, &QMQTT::Client::received, this, &CloudManager::onReceived);
}

// 连接函数 (参数已更新)
void CloudManager::connectToCloud(const QString &host, int port,
                                  const QString &username, const QString &password,
                                  const QString &topicPrefix,
                                  const QString &cmdTopic)
{
    // 如果已经连接，先断开
    if (m_client->isConnectedToHost()) {
        m_client->disconnectFromHost();
    }

    // 1. 设置连接地址
    m_client->setHost(QHostAddress(host));
    m_client->setPort(port);

    // 2. 设置账号密码
    if (!username.isEmpty()) {
        m_client->setUsername(username.toUtf8());
        m_client->setPassword(password.toUtf8());
    }

    // 3. 保存主题前缀
    m_topicPrefix = topicPrefix.isEmpty() ? "topic/hotel" : topicPrefix;

    // 【新增】保存指令主题
    m_cmdTopic = cmdTopic.isEmpty() ? "hotel/admin/commands" : cmdTopic;

    // 4. 发起连接
    qDebug() << "Connecting to MQTT Broker:" << host << "Port:" << port;
    m_client->connectToHost();
}

// 【新增】通用发布接口
void CloudManager::publish(const QString &topic, const QByteArray &payload)
{
    if (m_client->isConnectedToHost()) {
        QMQTT::Message msg(0, topic, payload);
        m_client->publish(msg);
    }
}

// 上报房间状态 (JSON 格式)
void CloudManager::uploadRoomStatus(const QString &roomId, int status, const QString &guestName)
{
    if (!m_client->isConnectedToHost()) {
        qDebug() << "MQTT Warning: Not connected. Skip uploading room status.";
        return;
    }

    // 1. 构建 JSON 数据包
    QJsonObject json;
    json.insert("device_id", m_client->clientId());
    json.insert("room_id", roomId);
    json.insert("status", status); // 0:空闲, 1:入住, 2:清洁
    json.insert("guest_name", guestName);
    json.insert("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    // 2. 转换为字符串
    QJsonDocument doc(json);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);

    // 3. 发布消息
    QString topic = m_topicPrefix + "/room_status";

    QMQTT::Message msg(0, topic, payload, 1); // QoS 1
    m_client->publish(msg);

    qDebug() << "MQTT Upload Status:" << topic << payload;
}

// 上报 RFID 刷卡数据 (JSON 格式)
void CloudManager::publishRfidData(QString cardId, QString roomNumber)
{
    // 连接检查
    if (!m_client->isConnectedToHost()) {
        qDebug() << "MQTT Error: Cannot send RFID data (Not Connected)";
        return;
    }

    // 1. 构建 JSON 数据
    QJsonObject json;
    json.insert("event", "card_swipe");
    json.insert("card_id", cardId);
    json.insert("room_number", roomNumber);
    json.insert("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    // 2. 转换为二进制负载
    QJsonDocument doc(json);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);

    // 3. 发送 MQTT 消息
    QString topic = m_topicPrefix + "/rfid";

    QMQTT::Message msg(0, topic, payload, 0); // 刷卡频率高，QoS 0 即可
    m_client->publish(msg);

    qDebug() << "MQTT Upload RFID:" << topic << payload;
}

// 实现全操作上云的核心函数
void CloudManager::publishLog(const QString &subTopic, const QString &message)
{
    // 确保 MQTT 已连接
    if (m_client && m_client->isConnectedToHost()) {

        QString topic = m_topicPrefix + "/" + subTopic;

        // 构建消息内容
        QString fullMessage = QString("[%1] %2")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
                .arg(message);

        // 构建消息对象 (QoS 1 确保重要日志送达)
        QMQTT::Message msg(0, topic, fullMessage.toUtf8(), 1);

        m_client->publish(msg);

        // 本地打印调试信息
        qDebug() << "[MQTT Log]" << topic << ":" << fullMessage;
    } else {
        qDebug() << "[MQTT Error] Not connected. Log dropped:" << subTopic << message;
    }
}

void CloudManager::onConnected() {
    qDebug() << "Cloud: MQTT Connected Successfully!";

    // 连接成功后，可以发送一条上线消息
    publishLog("system", "Device Online: " + m_client->clientId());

    // 【新增】订阅后台指令频道
    if (!m_cmdTopic.isEmpty()) {
        m_client->subscribe(m_cmdTopic, 0);
        qDebug() << "📡 Subscribed to Admin Command Topic:" << m_cmdTopic;
    }

    emit cloudStatusChanged(true);
}

void CloudManager::onDisconnected() {
    qDebug() << "Cloud: MQTT Disconnected!";
    emit cloudStatusChanged(false);
}

// 收到消息的处理函数
void CloudManager::onReceived(const QMQTT::Message &message)
{
    QString payload = message.payload();
    QString topic = message.topic();

    qDebug() << "📩 Msg Received [" << topic << "]: " << payload;

    // 1. 解析 JSON
    QJsonDocument doc = QJsonDocument::fromJson(message.payload());
    if (doc.isNull()) return;

    QJsonObject root = doc.object();
    QString type = root["type"].toString();

    // -------------------------------------------------------
    // 处理恢复指令 (方案一)
    // -------------------------------------------------------
    if (type == "CMD_SYSTEM_RESTORE") {
        qDebug() << "📡 [Cloud] 收到远程恢复指令";

        // 执行本地恢复逻辑
        bool success = DBManager::instance().restoreDatabase();

        if (success) {
            // 1. 发送信号通知界面刷新 (UI 线程)
            emit systemRestored();

            // 2. 回复服务器：恢复成功
            publishLog("system", "Remote Restore Executed Successfully");
        } else {
            // 3. 回复服务器：恢复失败
            publishLog("error", "Remote Restore Failed: File Error");
        }
    }

    // 过滤：只处理管理指令频道
    if (topic == m_cmdTopic) {
        QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8());
        if (!doc.isObject()) return;

        QJsonObject obj = doc.object();
        QString type = obj.value("type").toString();

        // 处理添加用户指令
        if (type == "CMD_ADD_USER") {
            QString user = obj.value("username").toString();
            QString pass = obj.value("password").toString();
            qDebug() << "⚡ Command: Add User" << user;
            emit requestAddUser(user, pass);
        }
        else if (type == "CMD_DEL_USER") {
            QString user = obj.value("username").toString();
            qDebug() << "⚡ Command: Delete User" << user;
            emit requestDeleteUser(user);
        }
        // 处理系统恢复指令
        else if (type == "CMD_SYSTEM_RESTORE") {
            qDebug() << "⚡ Command: System Restore";
            emit requestSystemRestore();
        }
    }
}
