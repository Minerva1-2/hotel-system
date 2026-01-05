#ifndef CLOUDMANAGER_H
#define CLOUDMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include "dbmanager.h"
#include "./qmqtt.h"

class CloudManager : public QObject
{
    Q_OBJECT
public:
    static CloudManager& instance();

    // 初始化连接
    void connectToCloud(const QString &host, int port,
                        const QString &username, const QString &password,
                        const QString &topicPrefix = "hotel/test/user001",
                        const QString &cmdTopic = "hotel/admin/commands");
    // 核心功能：上报房间状态
    // status: 0=空闲, 1=入住, 2=维护
    void uploadRoomStatus(const QString &roomId, int status, const QString &guestName);
    void publishRfidData(QString cardId, QString roomNumber);
    void publishLog(const QString &subTopic, const QString &message);
    void publish(const QString &topic, const QByteArray &payload);

private:
    explicit CloudManager(QObject *parent = nullptr);
    QMQTT::Client *m_client;
    QString m_topicPrefix;
    QString m_cmdTopic;

signals:
    void cloudStatusChanged(bool online);

    // 当收到后台“添加用户”指令时触发，参数为用户名和密码
    void requestAddUser(QString username, QString password);
    void requestDeleteUser(QString username);
    // 当收到后台“恢复系统”指令时触发
    void requestSystemRestore();
    void systemRestored();

private slots:
    void onConnected();
    void onDisconnected();
    void onReceived(const QMQTT::Message &message);
};

#endif // CLOUDMANAGER_H
