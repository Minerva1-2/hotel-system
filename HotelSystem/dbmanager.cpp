#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QVariantMap>
#include <QCoreApplication>
#include <QFile>
#include <QSqlDatabase>
#include <QDate>

DBManager::DBManager() {}

DBManager::~DBManager() {
    if (m_db.isOpen()) m_db.close();
}

DBManager& DBManager::instance() {
    static DBManager instance;
    return instance;
}

bool DBManager::openDb() {
    if(QSqlDatabase::contains("qt_sql_default_connection")) {
        m_db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setDatabaseName(QCoreApplication::applicationDirPath() + "/hotel.db");
    }
    if (!m_db.open()) return false;
    initTables();
    return true;
}

void DBManager::initTables() {
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS admin (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT, role INTEGER)");

    query.exec("SELECT count(*) FROM admin");
    if (query.next() && query.value(0).toInt() == 0) {
        query.exec("INSERT INTO admin (username, password, role) VALUES ('admin', '123', 0)");
    }

    query.exec("CREATE TABLE IF NOT EXISTS rooms (room_id TEXT PRIMARY KEY, type TEXT, price INTEGER, status INTEGER, guest TEXT, id_card TEXT, checkin_time INTEGER, checkout_time INTEGER, rfid_id TEXT)");

    query.exec("SELECT count(*) FROM rooms");
    if (query.next() && query.value(0).toInt() == 0) {
        m_db.transaction();
        for (int floor = 1; floor <= 6; ++floor) {
            QString typeName;
            int price = 0;
            switch (floor) {
            case 1: typeName = "标准单人间"; price = 168; break;
            case 2: typeName = "标准双人间"; price = 268; break;
            case 3: typeName = "豪华大床房"; price = 368; break;
            case 4: typeName = "商务双人间"; price = 468; break;
            case 5: typeName = "行政套房"; price = 688; break;
            case 6: typeName = "总统套房"; price = 1888; break;
            }
            int roomsPerFloor = (floor == 6) ? 4 : 8;
            for (int i = 1; i <= roomsPerFloor; ++i) {
                QString roomId = QString::number(floor * 100 + i);
                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO rooms VALUES (?, ?, ?, 0, '', '', 0, 0, '')");
                insertQuery.addBindValue(roomId);
                insertQuery.addBindValue(typeName);
                insertQuery.addBindValue(price);
                insertQuery.exec();
            }
        }
        m_db.commit();
    }
}

int DBManager::checkAdmin(QString user, QString pass) {
    QSqlQuery query;
    query.prepare("SELECT password, role FROM admin WHERE username = :user");
    query.bindValue(":user", user);
    if (query.exec() && query.next()) {
        if (query.value(0).toString() == pass) return query.value(1).toInt();
    }
    return -1;
}

QString DBManager::getRoomByCard(const QString &cardId) {
    QSqlQuery query;
    query.prepare("SELECT room_id FROM rooms WHERE rfid_id = :cardId");
    query.bindValue(":cardId", cardId);
    if (query.exec() && query.next()) return query.value(0).toString();
    return QString();
}

QList<RoomInfo> DBManager::getAllRooms() {
    QList<RoomInfo> rooms;
    QSqlQuery query("SELECT room_id, type, price, status, guest, checkin_time, rfid_id, id_card FROM rooms ORDER BY room_id ASC");
    while (query.next()) {
        RoomInfo info;
        info.id = query.value(0).toString();
        info.type = query.value(1).toString();
        info.price = query.value(2).toInt();
        info.status = query.value(3).toInt();
        info.guest = query.value(4).toString();
        info.checkInTime = query.value(5).toLongLong();
        info.rfidId = query.value(6).toString();
        info.idCard = query.value(7).toString();
        rooms.append(info);
    }
    return rooms;
}

// 【新增功能】模糊搜索在住客户
QList<RoomInfo> DBManager::searchGuests(QString keyword) {
    QList<RoomInfo> results;
    QSqlQuery query;

    // 只查询已入住(status=1)的房间
    QString sql = "SELECT room_id, type, status, guest, id_card, checkin_time, rfid_id "
                  "FROM rooms WHERE status = 1 ";

    if (!keyword.isEmpty()) {
        sql += "AND (room_id LIKE :k OR guest LIKE :k OR id_card LIKE :k OR rfid_id LIKE :k)";
    }

    sql += " ORDER BY room_id ASC";

    query.prepare(sql);
    if (!keyword.isEmpty()) {
        query.bindValue(":k", "%" + keyword + "%");
    }

    if (query.exec()) {
        while (query.next()) {
            RoomInfo info;
            info.id = query.value(0).toString();
            info.type = query.value(1).toString();
            info.status = query.value(2).toInt();
            info.guest = query.value(3).toString();
            info.idCard = query.value(4).toString();
            info.checkInTime = query.value(5).toLongLong();
            info.rfidId = query.value(6).toString();
            results.append(info);
        }
    } else {
        qDebug() << "Search failed:" << query.lastError().text();
    }
    return results;
}

bool DBManager::checkIn(QString roomId, QString guestName, QString idCard, QString rfidId) {
    QSqlQuery query;
    query.prepare("UPDATE rooms SET status=1, guest=:name, id_card=:id_card, checkin_time=:in, rfid_id=:rfid WHERE room_id=:id");
    query.bindValue(":name", guestName);
    query.bindValue(":id_card", idCard);
    query.bindValue(":in", (qint64)QDateTime::currentDateTime().toTime_t());
    query.bindValue(":rfid", rfidId);
    query.bindValue(":id", roomId);
    return query.exec();
}

int DBManager::checkOut(QString roomId) {
    QSqlQuery query;
    query.prepare("SELECT price, checkin_time FROM rooms WHERE room_id=:id");
    query.bindValue(":id", roomId);
    if (query.exec() && query.next()) {
        int price = query.value(0).toInt();
        qint64 inTime = query.value(1).toLongLong();
        qint64 outTime = (qint64)QDateTime::currentDateTime().toTime_t();
        int days = ((outTime - inTime) / 86400) + 1;
        if(days < 1) days = 1;

        QSqlQuery update;
        update.prepare("UPDATE rooms SET status=2, guest='', id_card='', rfid_id='', checkin_time=0 WHERE room_id=:id");
        update.bindValue(":id", roomId);
        if (update.exec()) return price * days;
    }
    return 0;
}

QVariantMap DBManager::getIncompleteInfoByRfid(QString rfidId) {
    QVariantMap result;
    // 初始化 found 为 false，防止查询失败时 UI 逻辑出错
    result["found"] = false;

    QSqlQuery query;
    // 确保查询取出需要的四个字段：房间号、客人名、单价、入住时间
    query.prepare("SELECT room_id, guest, price, checkin_time FROM rooms WHERE rfid_id = :rfid AND status = 1");
    query.bindValue(":rfid", rfidId);

    if (query.exec() && query.next()) {
        result["found"] = true;
        result["room_id"] = query.value(0).toString();
        result["guest_name"] = query.value(1).toString();

        // --- 核心修复：计算金额 ---
        int price = query.value(2).toInt();
        qint64 checkInTime = query.value(3).toLongLong();

        // 获取当前时间戳 (秒)
        // 注意：使用 toTime_t() 保持与 checkIn/checkOut 函数中存储格式的一致性
        qint64 currentTime = (qint64)QDateTime::currentDateTime().toTime_t();

        // 计算入住时长 (秒)
        qint64 duration = currentTime - checkInTime;
        if (duration < 0) duration = 0; // 防止系统时间错误导致负数

        // 计算天数：
        // 逻辑：(时长 / 一天的秒数) + 1
        // 例子：入住 100秒 -> 0 + 1 = 算 1 天
        // 例子：入住 25小时(90000秒) -> 1 + 1 = 算 2 天
        int days = (duration / 86400) + 1;

        // 确保至少收一天房费
        if (days < 1) days = 1;

        // 计算总价
        int totalCost = days * price;

        result["cost"] = totalCost;

        // 打印调试信息方便排查
        qDebug() << "[ReadCard] Room:" << result["room_id"].toString()
                 << " Duration(s):" << duration
                 << " Days:" << days
                 << " Price:" << price
                 << " Total:" << totalCost;
    }
    // else { 没找到记录 result["found"] 已经是 false 了 }

    return result;
}
bool DBManager::finishClean(QString roomId) {
    QSqlQuery query;
    query.prepare("UPDATE rooms SET status=0 WHERE room_id=:id");
    query.bindValue(":id", roomId);
    return query.exec();
}

bool DBManager::updatePassword(QString newPass) {
    QSqlQuery query;
    query.prepare("UPDATE admin SET password = :newPass WHERE username = 'admin'");
    query.bindValue(":newPass", newPass);
    return query.exec();
}

bool DBManager::addUser(QString user, QString pass, int role) {
    QSqlQuery query;
    query.prepare("INSERT INTO admin (username, password, role) VALUES (:u, :p, :r)");
    query.bindValue(":u", user);
    query.bindValue(":p", pass);
    query.bindValue(":r", role);
    return query.exec();
}

bool DBManager::addUser(QString user, QString pass) {
    return addUser(user, pass, 1);
}

bool DBManager::deleteUser(QString user) {
    QSqlQuery query;
    query.prepare("DELETE FROM admin WHERE username = :name");
    query.bindValue(":name", user);
    return query.exec();
}

QList<QString> DBManager::getAllUsers() {
    QList<QString> list;
    QSqlQuery query("SELECT username FROM admin");
    while(query.next()) list.append(query.value(0).toString());
    return list;
}

void DBManager::clearBusinessData() {
    QSqlQuery query;
    query.exec("UPDATE rooms SET status=0, guest='', id_card='', rfid_id='', checkin_time=0, checkout_time=0");
}

QPair<QDate, double> DBManager::getStayInfo(const QString &roomId) {
    QSqlQuery query;
    query.prepare("SELECT checkout_time, price FROM rooms WHERE room_id = :rid");
    query.bindValue(":rid", roomId);
    if (query.exec() && query.next()) {
        return qMakePair(QDateTime::fromTime_t(query.value(0).toLongLong()).date(), query.value(1).toDouble());
    }
    return qMakePair(QDate::currentDate(), 0.0);
}

bool DBManager::extendStay(QString roomNo, int extraDays) {
    // 简化版，实际请保持您原有的逻辑
    QSqlQuery query;
    query.prepare("UPDATE rooms SET checkout_time = checkout_time + :secs WHERE room_id = :rid");
    query.bindValue(":secs", extraDays * 86400);
    query.bindValue(":rid", roomNo);
    return query.exec();
}

bool DBManager::updateRoomCard(QString roomNo, QString cardId) {
    QSqlQuery query;
    query.prepare("UPDATE rooms SET rfid_id = :card WHERE room_id = :room");
    query.bindValue(":card", cardId);
    query.bindValue(":room", roomNo);
    return query.exec();
}

bool DBManager::unbindCard(QString cardId) {
    QSqlQuery query;
    query.prepare("UPDATE rooms SET rfid_id = '' WHERE rfid_id = :card");
    query.bindValue(":card", cardId);
    return query.exec();
}

QString DBManager::getCardByRoom(const QString &roomId) {
    QSqlQuery query;
    query.prepare("SELECT rfid_id FROM rooms WHERE room_id = :rid");
    query.bindValue(":rid", roomId);
    if (query.exec() && query.next()) return query.value(0).toString();
    return QString();
}
bool DBManager::backupDatabase(const QString &targetPath)
{
    // 1. 获取当前正在使用的数据库连接
    QSqlDatabase db = QSqlDatabase::database();

    // 2. 获取当前数据库文件的物理路径
    QString currentDbPath = db.databaseName();

    if (currentDbPath.isEmpty()) {
        qDebug() << "备份失败：无法获取当前数据库路径";
        return false;
    }

    // 3. 如果目标备份文件已存在，先删除它（否则 copy 会失败）
    if (QFile::exists(targetPath)) {
        QFile::remove(targetPath);
    }

    // 4. 执行文件复制
    // 注意：SQLite数据库在运行时复制通常是安全的，但最好没有大量写入操作时进行
    if (QFile::copy(currentDbPath, targetPath)) {
        qDebug() << "数据库已成功备份到：" << targetPath;
        return true;
    } else {
        qDebug() << "备份失败：无法复制文件从" << currentDbPath << "到" << targetPath;
        return false;
    }
}
bool DBManager::restoreDatabase()
{
    // 获取应用程序运行路径 (避免相对路径带来的找不到文件问题)
    QString dbName = QCoreApplication::applicationDirPath() + "/hotel.db";
    QString backupName = QCoreApplication::applicationDirPath() + "/hotel_backup.db";

    // 1. 检查备份文件是否存在
    if (!QFile::exists(backupName)) {
        qDebug() << "Error: Local backup file not found at" << backupName;
        return false;
    }

    // 2. 【关键】关闭当前的数据库连接
    // 既然 m_database 报错，我们直接通过 Qt 的静态方法获取默认连接
    {
        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            db.close();
        }
    }

    // 【重要】必须移除连接，彻底释放 SQLite 文件的文件锁
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    // 3. 删除旧的数据库文件
    if (QFile::exists(dbName)) {
        if (!QFile::remove(dbName)) {
            qDebug() << "Error: Could not delete current database. Check file permissions.";
            // 尝试重新打开以恢复服务
            openDb();
            return false;
        }
    }

    // 4. 复制备份文件
    if (!QFile::copy(backupName, dbName)) {
        qDebug() << "Error: Could not copy backup file.";
        openDb(); // 尝试恢复服务
        return false;
    }

    // 5. 重新初始化数据库连接
    // 直接调用你原本就有的 openDb() 函数，它里面应该包含了 addDatabase 和 open 的逻辑
    if (!openDb()) {
        qDebug() << "Error: Could not reopen database after restore.";
        return false;
    }

    qDebug() << "Success: Database restored from local backup.";
    return true;
}
