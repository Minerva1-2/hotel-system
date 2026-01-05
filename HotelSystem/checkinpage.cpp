#include "checkinpage.h"
#include "dbmanager.h"
#include "cloudmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

CheckInPage::CheckInPage(QWidget *parent) : QWidget(parent) {
    this->setFixedSize(800, 480);
    this->setObjectName("checkInPage");
    // 深蓝色渐变背景
    this->setStyleSheet("#checkInPage { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #001f3f, stop:1 #003366); }");

    setupUi();
}

void CheckInPage::paintEvent(QPaintEvent *) {
    QStyleOption opt; opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CheckInPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 25, 40, 25);
    mainLayout->setSpacing(0);

    // 1. 顶部标题
    QLabel *titleLabel = new QLabel("新客登记与房卡绑定", this);
    titleLabel->setStyleSheet("color: #00d2ff; font-size: 28px; font-weight: bold; font-family: 'WenQuanYi Micro Hei';");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    mainLayout->addSpacing(30);

    // 2. 中央玻璃质感卡片
    QWidget *centerCard = new QWidget(this);
    centerCard->setObjectName("centerCard");
    centerCard->setStyleSheet(
                "#centerCard { "
                "   background: rgba(255, 255, 255, 0.08); "
                "   border: 1px solid rgba(255, 255, 255, 0.15); "
                "   border-radius: 20px; "
                "}"
                );

    QHBoxLayout *cardLayout = new QHBoxLayout(centerCard);
    cardLayout->setContentsMargins(40, 30, 40, 30);
    cardLayout->setSpacing(50);

    // --- 左侧：对齐表单 ---
    QWidget *leftSection = new QWidget(this);
    QFormLayout *formLayout = new QFormLayout(leftSection);
    formLayout->setVerticalSpacing(18);
    formLayout->setLabelAlignment(Qt::AlignRight);

    QString lblStyle = "color: rgba(255, 255, 255, 0.9); font-size: 17px; font-family: 'WenQuanYi Micro Hei';";
    QString editStyle =
            "QLineEdit, QComboBox { "
            "   background: rgba(0, 0, 0, 0.4); "
            "   border: 1px solid rgba(0, 210, 255, 0.3); "
            "   border-radius: 8px; "
            "   padding: 8px 12px; "
            "   color: #fff; "
            "   font-size: 16px; "
            "} "
            "QLineEdit:focus { border: 1px solid #00d2ff; background: rgba(0, 0, 0, 0.6); }";

    nameEdit = new QLineEdit(this); nameEdit->setStyleSheet(editStyle);
    idEdit = new QLineEdit(this); idEdit->setStyleSheet(editStyle);
    roomCombo = new QComboBox(this); roomCombo->setStyleSheet(editStyle);

    // --- 卡号行配置 ---
    rfidEdit = new QLineEdit(this);
    rfidEdit->setReadOnly(true);
    rfidEdit->setFocusPolicy(Qt::NoFocus);
    rfidEdit->setAttribute(Qt::WA_TransparentForMouseEvents); // 鼠标穿透

    rfidEdit->setPlaceholderText("等待感应...");
    rfidEdit->setStyleSheet(
                "background: rgba(0, 210, 255, 0.1); "
                "border: 1px dashed rgba(0, 210, 255, 0.5); "
                "color: #00d2ff; border-radius: 8px; padding: 8px 12px; font-size: 16px; font-weight: bold;"
                );

    // 统一添加行
    QLabel *nL = new QLabel("住客姓名:", this); nL->setStyleSheet(lblStyle);
    QLabel *iL = new QLabel("证件号码:", this); iL->setStyleSheet(lblStyle);
    QLabel *rL = new QLabel("分配房号:", this); rL->setStyleSheet(lblStyle);
    QLabel *fL = new QLabel("感应卡号:", this); fL->setStyleSheet("color: #00d2ff; font-size: 17px; font-weight: bold;");

    formLayout->addRow(nL, nameEdit);
    formLayout->addRow(iL, idEdit);
    formLayout->addRow(rL, roomCombo);
    formLayout->addRow(fL, rfidEdit);

    // --- 右侧：状态指示区 ---
    QVBoxLayout *visualLayout = new QVBoxLayout();
    visualLayout->setAlignment(Qt::AlignCenter);

    statusLight = new QLabel(this);
    statusLight->setFixedSize(100, 100);
    // 初始红色光晕
    statusLight->setStyleSheet(
                "background: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(231, 76, 60, 0.7), stop:1 rgba(231, 76, 60, 0)); "
                "border: 2px solid #e74c3c; border-radius: 50px;"
                );

    rfidHintLabel = new QLabel("请靠近感应区", this);
    rfidHintLabel->setStyleSheet("color: #e74c3c; font-size: 16px; font-weight: bold;");

    visualLayout->addWidget(statusLight, 0, Qt::AlignCenter);
    visualLayout->addSpacing(15);
    visualLayout->addWidget(rfidHintLabel, 0, Qt::AlignCenter);

    cardLayout->addWidget(leftSection, 3);
    cardLayout->addLayout(visualLayout, 2);
    mainLayout->addWidget(centerCard);

    // --- 3. 底部操作区 ---
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 30, 0, 0);
    bottomLayout->setSpacing(25);

    // 1. 返回按钮
    QPushButton *btnBack = new QPushButton(" ← 返回主页", this);
    btnBack->setFixedSize(140, 50);
    btnBack->setStyleSheet(
                "QPushButton { "
                "   background-color: rgba(255, 255, 255, 0.05); "
                "   color: rgba(255, 255, 255, 0.7); "
                "   border: 1px solid rgba(255, 255, 255, 0.2); "
                "   border-radius: 25px; "
                "   font-size: 17px; "
                "   font-family: 'WenQuanYi Micro Hei'; "
                "} "
                "QPushButton:pressed { "
                "   background-color: rgba(255, 255, 255, 0.1); "
                "   border: 1px solid rgba(255, 255, 255, 0.4); "
                "}"
                );
    connect(btnBack, &QPushButton::clicked, this, &CheckInPage::backClicked);

    // 2. 确认按钮
    QPushButton *btnOk = new QPushButton("立即办理入住 ", this);
    btnOk->setFixedSize(200, 50);
    btnOk->setStyleSheet(
                "QPushButton { "
                "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #00d2ff, stop:1 #3a7bd5); "
                "   color: white; "
                "   border-radius: 25px; "
                "   font-size: 18px; "
                "   font-weight: bold; "
                "   font-family: 'WenQuanYi Micro Hei'; "
                "   border: 1px solid rgba(255, 255, 255, 0.3); "
                "} "
                "QPushButton:pressed { "
                "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #3a7bd5, stop:1 #00d2ff); "
                "   padding-top: 2px; padding-left: 2px; "
                "}"
                );
    connect(btnOk, &QPushButton::clicked, this, &CheckInPage::onConfirm);

    bottomLayout->addWidget(btnBack);
    bottomLayout->addStretch();
    bottomLayout->addWidget(btnOk);

    mainLayout->addLayout(bottomLayout);
}

// 【核心修改 2】 当检测到 RFID 时，立即发送 MQTT 消息
void CheckInPage::setRfidId(QString id) {
    if (id.isEmpty()) return;
    m_currentRfid = id;

    // 1. 界面更新
    rfidEdit->setText(id);
    statusLight->setStyleSheet(
                "background: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(46, 204, 113, 0.7), stop:1 rgba(46, 204, 113, 0)); "
                "border: 2px solid #2ecc71; border-radius: 50px;"
                );
    rfidHintLabel->setText("房卡就绪");
    rfidHintLabel->setStyleSheet("color: #2ecc71; font-size: 16px; font-weight: bold;");

    // 2. 【新增功能】发送数据到 MQTTX
    QString currentRoom = roomCombo->currentText(); // 获取当前选中的房间（如果有）

    // 调用 CloudManager 发送 (注意：需要在 CloudManager 中实现此函数)
    // 格式：publishRfidInfo(卡号, 房间号)
    CloudManager::instance().publishRfidData(id, currentRoom);
    CloudManager::instance().publishLog("card_swipe", "Card Scanned: " + id);
    qDebug() << "RFID Scanned & Sent to MQTT:" << id;
}

void CheckInPage::clearForm() {
    nameEdit->clear();
    idEdit->clear();
    rfidEdit->clear();

    m_currentRfid = "";

    statusLight->setStyleSheet(
                "background: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(231, 76, 60, 0.7), stop:1 rgba(231, 76, 60, 0)); "
                "border: 2px solid #e74c3c; border-radius: 50px;"
                );
    rfidHintLabel->setText("等待感应...");
    rfidHintLabel->setStyleSheet("color: #e74c3c; font-size: 16px; font-weight: bold;");
}

void CheckInPage::refreshRoomList() {
    roomCombo->clear();
    clearForm();
    roomCombo->addItem("--- 请选择房间 ---", "");

    QList<RoomInfo> rooms = DBManager::instance().getAllRooms();
    for (const RoomInfo &r : rooms) {
        if (r.status == 0) {
            roomCombo->addItem(r.id + " (" + r.type + ")", r.id);
        }
    }
    roomCombo->setCurrentIndex(0);
}

void CheckInPage::onConfirm() {
    QString roomId = roomCombo->currentData().toString();
    QString name = nameEdit->text().trimmed();
    QString idNum = idEdit->text().trimmed();
    QString rfid = rfidEdit->text();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "信息缺失", "请输入住客姓名！");
        nameEdit->setFocus();
        return;
    }
    if (idNum.isEmpty()) {
        QMessageBox::warning(this, "信息缺失", "根据规定，必须录入身份证号码！");
        idEdit->setFocus();
        return;
    }
    if (idNum.length() < 5) {
        QMessageBox::warning(this, "格式错误", "请输入有效的证件号码！");
        idEdit->setFocus();
        return;
    }

    if (roomId.isEmpty()) {
        QMessageBox::warning(this, "信息缺失", "请选择一个房间！");
        return;
    }
    if (rfid.isEmpty()) {
        QMessageBox::warning(this, "操作缺失", "请将房卡放置在感应区进行绑定！");
        return;
    }

    // 数据库操作
    if (DBManager::instance().checkIn(roomId, name, idNum, rfid)) {

        QMessageBox::information(this, "登记成功",
                                 QString("已成功办理入住！\n\n房间：%1\n住客：%2\n房卡已激活").arg(roomId).arg(name));

        // 1. 上报房间状态变更 (原有逻辑 - 更新仪表盘状态)
        CloudManager::instance().uploadRoomStatus(roomId, 1, name);

        // 【新增 2. 上报详细操作日志】 (用于后台记录流水)
        // 组装日志格式: Action:CheckIn|Room:101|Name:张三|ID:123456...
        QString logContent = QString("Action:CheckIn|Room:%1|Name:%2|ID:%3|RFID:%4")
                .arg(roomId).arg(name).arg(idNum).arg(rfid);

        // 发送到 topic/hotel/checkin
        CloudManager::instance().publishLog("checkin", logContent);

        refreshRoomList();
    } else {
        QMessageBox::critical(this, "系统错误", "数据库写入失败，请检查系统日志。");
    }
}
