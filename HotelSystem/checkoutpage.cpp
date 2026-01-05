#include "checkoutpage.h"
#include "dbmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <QDebug>

CheckOutPage::CheckOutPage(QWidget *parent) : QWidget(parent) {
    this->setFixedSize(800, 480);
    this->setObjectName("checkOutPage");
    // 背景保持深色系，底部带一点暖色光晕
    this->setStyleSheet("#checkOutPage { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #001f3f, stop:1 #2d1a05); }");

    setupUi();
}

void CheckOutPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CheckOutPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 25, 40, 25);
    mainLayout->setSpacing(0);

    // 1. 顶部标题
    QLabel *titleLabel = new QLabel("退房结算办理系统", this);
    titleLabel->setStyleSheet("color: #ff9f43; font-size: 28px; font-weight: bold; font-family: 'WenQuanYi Micro Hei';");
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

    // --- 左侧：信息展示区 ---
    QWidget *leftSection = new QWidget(this);
    QFormLayout *formLayout = new QFormLayout(leftSection);
    formLayout->setVerticalSpacing(18);
    formLayout->setLabelAlignment(Qt::AlignRight);

    QString lblStyle = "color: rgba(255, 255, 255, 0.9); font-size: 17px; font-family: 'WenQuanYi Micro Hei';";

    // 【核心修改】移植入住页面的虚线科技风格，但改为橙色调
    QString readStyle =
            "QLineEdit { "
            "   background: rgba(255, 159, 67, 0.1); "        // 微透明橙色背景
            "   border: 1px dashed rgba(255, 159, 67, 0.6); " // 虚线边框
            "   border-radius: 8px; "
            "   padding: 8px 12px; "
            "   color: #ff9f43; "                             // 橙色高亮文字
            "   font-size: 16px; "
            "   font-weight: bold; "
            "   font-family: 'WenQuanYi Micro Hei'; "
            "}";

    rfidDisplay = new QLineEdit(this); rfidDisplay->setStyleSheet(readStyle);
    roomDisplay = new QLineEdit(this); roomDisplay->setStyleSheet(readStyle);
    nameDisplay = new QLineEdit(this); nameDisplay->setStyleSheet(readStyle);

    // 金额框：在通用样式基础上，文字改为红色以示强调
    costDisplay = new QLineEdit(this);
    costDisplay->setStyleSheet(readStyle + "QLineEdit { color: #ff6b6b; border-color: #ff6b6b; }");

    // 设置为只读并穿透鼠标（防软键盘）
    QLineEdit* edits[] = {rfidDisplay, roomDisplay, nameDisplay, costDisplay};
    for(QLineEdit* e : edits) {
        e->setReadOnly(true);
        e->setFocusPolicy(Qt::NoFocus);
        e->setAttribute(Qt::WA_TransparentForMouseEvents);
        e->setPlaceholderText("等待读取...");
    }

    formLayout->addRow(new QLabel("感应卡号:", this), rfidDisplay);
    formLayout->addRow(new QLabel("对应房间:", this), roomDisplay);
    formLayout->addRow(new QLabel("住客姓名:", this), nameDisplay);
    formLayout->addRow(new QLabel("应结金额:", this), costDisplay);

    // 统一设置 Label 样式
    for(int i = 0; i < formLayout->rowCount(); ++i) {
        QWidget *label = formLayout->itemAt(i, QFormLayout::LabelRole)->widget();
        if(label) label->setStyleSheet(lblStyle);
    }

    // --- 右侧：状态指示区 ---
    QVBoxLayout *visualLayout = new QVBoxLayout();
    visualLayout->setAlignment(Qt::AlignCenter);

    statusLight = new QLabel(this);
    statusLight->setFixedSize(100, 100);
    // 初始橙色光晕（等待状态）
    statusLight->setStyleSheet(
                "background: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(255, 159, 67, 0.7), stop:1 rgba(255, 159, 67, 0)); "
                "border: 2px solid #ff9f43; border-radius: 50px;"
                );

    hintLabel = new QLabel("等待识别房卡...", this);
    hintLabel->setStyleSheet("color: #ff9f43; font-size: 16px; font-weight: bold;");

    visualLayout->addWidget(statusLight, 0, Qt::AlignCenter);
    visualLayout->addSpacing(15);
    visualLayout->addWidget(hintLabel, 0, Qt::AlignCenter);

    cardLayout->addWidget(leftSection, 3);
    cardLayout->addLayout(visualLayout, 2);
    mainLayout->addWidget(centerCard);

    // 3. 底部按钮区
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 25, 0, 0);

    QPushButton *btnBack = new QPushButton(" ← 返回主页", this);
    btnBack->setFixedSize(140, 50);
    btnBack->setStyleSheet(
                "QPushButton { background: rgba(255,255,255,0.05); color: rgba(255,255,255,0.7); border: 1px solid rgba(255,255,255,0.2); border-radius: 25px; font-size: 16px; }"
                "QPushButton:pressed { background: rgba(255,255,255,0.1); }"
                );
    connect(btnBack, &QPushButton::clicked, this, &CheckOutPage::backClicked);

    QPushButton *btnOk = new QPushButton("确认结算退房 ", this);
    btnOk->setFixedSize(200, 50);
    btnOk->setStyleSheet(
                "QPushButton { "
                "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ff9f43, stop:1 #ff6b6b); "
                "   color: white; border-radius: 25px; font-size: 18px; font-weight: bold; border: 1px solid rgba(255,255,255,0.3);"
                "}"
                "QPushButton:pressed { background: #ff6b6b; padding-top: 2px; }"
                );
    connect(btnOk, &QPushButton::clicked, this, &CheckOutPage::onConfirmOut);

    bottomLayout->addWidget(btnBack);
    bottomLayout->addStretch();
    bottomLayout->addWidget(btnOk);
    mainLayout->addLayout(bottomLayout);
}

void CheckOutPage::setRfidId(QString id) {
    if (id.isEmpty()) return;
    m_currentRfid = id;
    rfidDisplay->setText(id); // 填入卡号

    // 数据库查询逻辑
    QVariantMap info = DBManager::instance().getIncompleteInfoByRfid(id);

    if (info["found"].toBool()) {
        m_targetRoomId = info["room_id"].toString();

        // 自动填入信息
        roomDisplay->setText(m_targetRoomId);
        nameDisplay->setText(info["guest_name"].toString());
        costDisplay->setText(QString("￥ %1").arg(info["cost"].toInt()));

        // 指示灯变绿
        statusLight->setStyleSheet(
                    "background: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(46, 204, 113, 0.7), stop:1 rgba(46, 204, 113, 0)); "
                    "border: 2px solid #2ecc71; border-radius: 50px;"
                    );
        hintLabel->setText("检索成功");
        hintLabel->setStyleSheet("color: #2ecc71; font-size: 16px; font-weight: bold;");
    } else {
        QMessageBox::warning(this, "检索失败", "该房卡当前未绑定任何在住房间！");
        clearForm();
        // 虽然清空了，但保留刚才刷的卡号显示，方便用户查看是否刷错了卡
        rfidDisplay->setText(id);
    }
}

void CheckOutPage::onConfirmOut() {
    // 1. 基本校验
    if (m_currentRfid.isEmpty() || m_targetRoomId.isEmpty()) {
        QMessageBox::warning(this, "操作受阻", "请先刷卡检索入住信息！");
        return;
    }

    // 2. 临时保存房间号
    // 用局部变量保存 roomId,learForm() 会马上清空 m_targetRoomId
    QString roomIdToClean = m_targetRoomId;

    // 3. 执行退房 (此时数据库状态变为 2:打扫中)
    if (DBManager::instance().checkOut(roomIdToClean)) {

        QMessageBox::information(this, "结算完成",
                                 QString("房间 %1 退房成功。\n房间已进入维护状态。").arg(roomIdToClean));
        CloudManager::instance().uploadRoomStatus(roomIdToClean, 2, "维护中(Cleaning)...");

        // 4. 启动 10秒 (10000毫秒) 延时任务
        QTimer::singleShot(10000, [=](){
            // 10秒后执行
            if (DBManager::instance().finishClean(roomIdToClean)) {
                qDebug() << ">>> celan：Room" << roomIdToClean << "has become empty status!";
                CloudManager::instance().uploadRoomStatus(roomIdToClean, 0, "");
            }
        });

        // 5. 清空界面，准备接待下一位
        clearForm();
    }
}

void CheckOutPage::clearForm() {
    rfidDisplay->clear();
    roomDisplay->clear();
    nameDisplay->clear();
    costDisplay->clear();
    m_currentRfid = "";
    m_targetRoomId = "";

    // 恢复橙色待机状态
    statusLight->setStyleSheet(
                "background: qradialgradient(cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(255, 159, 67, 0.7), stop:1 rgba(255, 159, 67, 0)); "
                "border: 2px solid #ff9f43; border-radius: 50px;"
                );
    hintLabel->setText("等待识别房卡...");
    hintLabel->setStyleSheet("color: #ff9f43; font-size: 16px; font-weight: bold;");
}
