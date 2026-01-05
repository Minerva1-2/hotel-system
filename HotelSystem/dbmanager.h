#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QList>
#include <QVariantMap>
#include <QFile>
#include <QDate>
#include <QCoreApplication>

struct RoomInfo {
    QString id;
    QString type;
    int price;
    int status;
    QString guest;
    qint64 checkInTime;
    QString rfidId;
    QString idCard;
};

class DBManager
{
public:
    static DBManager& instance();
    bool openDb();
    void initTables();
    void clearBusinessData();
    int checkAdmin(QString user, QString pass);
    QList<RoomInfo> getAllRooms();
    QString getRoomByCard(const QString &cardId);

    bool checkIn(QString roomId, QString guestName, QString idCard, QString rfidId);
    QVariantMap getIncompleteInfoByRfid(QString rfidId);
    int checkOut(QString roomId);
    bool finishClean(QString roomId);
    bool updatePassword(QString newPass);

    bool restoreDatabase();
    bool backupDatabase(const QString &targetPath);

    QList<RoomInfo> searchGuests(QString keyword);

    bool addUser(QString user, QString pass, int role);
    bool addUser(QString user, QString pass);
    bool deleteUser(QString user);

    QPair<QDate, double> getStayInfo(const QString &roomId);
    bool extendStay(QString roomNo, int extraDays);
    bool updateRoomCard(QString roomNo, QString cardId);
    bool unbindCard(QString cardId);
    QString getCardByRoom(const QString &roomId);
    QList<QString> getAllUsers();

private:
    DBManager();
    ~DBManager();
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;
    QMap<QString, double> m_roomPriceCache;
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
