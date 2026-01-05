#include "maininterface.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDateTime>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include <QMessageBox>

MainInterface::MainInterface(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(800, 480);
    this->setObjectName("mainInterface");
    this->setStyleSheet("#mainInterface { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #001f3f, stop:1 #003366); }");

    setupUi();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainInterface::updateTime);
    timer->start(1000);
    updateTime();
}

void MainInterface::paintEvent(QPaintEvent *)
{
    QStyleOption opt; opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MainInterface::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // 【优化点1】大幅减小边距：左右20，上下15 (原40/25)，腾出垂直空间
    mainLayout->setContentsMargins(20, 15, 20, 15);
    mainLayout->setSpacing(10); // 减小垂直元素间距

    // --- 1. 顶部栏 ---
    QHBoxLayout *topLayout = new QHBoxLayout();
    QLabel *logoLabel = new QLabel("酒店管理系统", this);
    logoLabel->setStyleSheet(
                "QLabel { color: white; font-family: 'WenQuanYi Micro Hei'; font-size: 22px; font-weight: bold; border-left: 6px solid #00d2ff; padding-left: 10px; }"
                );
    timeLabel = new QLabel(this);
    timeLabel->setStyleSheet("color: #00d2ff; font-family: 'WenQuanYi Micro Hei'; font-size: 16px; font-weight: bold;");

    topLayout->addWidget(logoLabel);
    topLayout->addStretch();
    topLayout->addWidget(timeLabel);

    // --- 2. 中部功能网格 ---
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(15); // 按钮之间的间距设为15

    QPushButton *btnRoom = createMenuBtn("房态管理", "查看/查询房间状态");
    QPushButton *btnIn = createMenuBtn("入住登记", "办理客人入住手续");
    QPushButton *btnOut = createMenuBtn("退房结算", "办理退房与收费");
    QPushButton *btnExtend = createMenuBtn("续房办理", "延长客人居住天数");
    QPushButton *btnCard = createMenuBtn("房卡管理", "读卡 / 制卡 / 清卡");

    // 系统设置按钮
    btnSystem = createMenuBtn("系统设置", "管理员账号与数据维护");

    connect(btnRoom, &QPushButton::clicked, this, &MainInterface::onRoomStatusClicked);
    connect(btnIn, &QPushButton::clicked, this, &MainInterface::onCheckInClicked);
    connect(btnOut, &QPushButton::clicked, this, &MainInterface::onCheckOutClicked);
    connect(btnSystem, &QPushButton::clicked, this, &MainInterface::onSettingsClicked);
    connect(btnCard, &QPushButton::clicked, this, [=](){ emit goCardManage(); });
    connect(btnExtend, &QPushButton::clicked, this, [=](){ emit goExtendStay(); });

    // Row 0
    gridLayout->addWidget(btnRoom, 0, 0);
    gridLayout->addWidget(btnIn, 0, 1);

    // Row 1
    gridLayout->addWidget(btnOut, 1, 0);
    gridLayout->addWidget(btnExtend, 1, 1); // 新按钮放在这里

    // Row 2
    gridLayout->addWidget(btnCard, 2, 0);
    gridLayout->addWidget(btnSystem, 2, 1); // System 不再跨列，改为放在右下角

    // 设置行比例，让三行平分空间
    gridLayout->setRowStretch(0, 1);
    gridLayout->setRowStretch(1, 1);
    gridLayout->setRowStretch(2, 1);

    // --- 3. 底部栏 ---
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    QLabel *userLabel = new QLabel("当前系统状态: 运行中", this);
    userLabel->setStyleSheet("color: rgba(255,255,255,0.5); font-size: 13px;");

    QPushButton *logoutBtn = new QPushButton("退出登录", this);
    logoutBtn->setFixedSize(100, 35); //稍微改小一点适应高度
    logoutBtn->setCursor(Qt::PointingHandCursor);
    logoutBtn->setStyleSheet(
                "QPushButton { background-color: rgba(231, 76, 60, 0.2); color: #e74c3c; border: 1px solid #e74c3c; border-radius: 17px; font-weight: bold; }"
                "QPushButton:pressed { background-color: #e74c3c; color: white; }"
                );
    connect(logoutBtn, &QPushButton::clicked, this, &MainInterface::logout);

    bottomLayout->addWidget(userLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(logoutBtn);

    // 组装
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(gridLayout, 1); // 1 表示中间部分占据所有剩余空间
    mainLayout->addLayout(bottomLayout);
}

// 辅助函数：创建统一风格的磨砂磁贴按钮
QPushButton* MainInterface::createMenuBtn(QString text, QString subText)
{
    QPushButton *btn = new QPushButton(this);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn->setCursor(Qt::PointingHandCursor);

    QVBoxLayout *layout = new QVBoxLayout(btn);
    layout->setContentsMargins(25, 25, 25, 25);

    QLabel *title = new QLabel(text, btn);
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: white; background: transparent;");

    QLabel *sub = new QLabel(subText, btn);
    sub->setStyleSheet("font-size: 14px; color: rgba(255,255,255,0.6); background: transparent;");

    layout->addWidget(title);
    layout->addSpacing(5);
    layout->addWidget(sub);
    layout->addStretch();

    // 默认样式 (玻璃质感)
    btn->setStyleSheet(
                "QPushButton { "
                "   background-color: rgba(255, 255, 255, 0.08); "
                "   border: 1px solid rgba(255, 255, 255, 0.1); "
                "   border-radius: 15px; "
                "   text-align: left; "
                "}"
                "QPushButton:hover { background-color: rgba(255, 255, 255, 0.15); }"
                "QPushButton:pressed { "
                "   background-color: rgba(0, 210, 255, 0.15); "
                "   border: 1px solid #00d2ff; "
                "}"
                );

    return btn;
}

void MainInterface::updateTime()
{
    QString timeStr = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss dddd");
    timeLabel->setText(timeStr);
}

// 权限控制：根据 UI 风格适配了 SetRole
// maininterface.cpp

void MainInterface::setRole(int role) {
    if (role == 0) {
        // =======================
        //      管理员 (Admin)
        // =======================

        // 1. 解锁按钮
        btnSystem->setEnabled(true);

        // 2. 恢复亮色玻璃质感 (和 createMenuBtn 里的一致)
        btnSystem->setStyleSheet(
                    "QPushButton { "
                    "   background-color: rgba(255, 255, 255, 0.08); "
                    "   border: 1px solid rgba(255, 255, 255, 0.1); "
                    "   border-radius: 15px; "
                    "   text-align: left; "
                    "}"
                    "QPushButton:hover { background-color: rgba(255, 255, 255, 0.15); }"
                    "QPushButton:pressed { background-color: rgba(0, 210, 255, 0.15); border: 1px solid #00d2ff; }"
                    );

        // 3. 恢复文字
        QList<QLabel*> labels = btnSystem->findChildren<QLabel*>();
        if(labels.count() >= 2) {
            labels[0]->setText("系统设置");
            labels[1]->setText("管理员账号管理");
            labels[0]->setStyleSheet("font-size: 26px; font-weight: bold; color: white; background: transparent;");
        }
    } else {
        //    普通用户
        // 1. 【核心】禁用点击
        btnSystem->setEnabled(false);
        btnSystem->setStyleSheet(
                    "QPushButton { "
                    "   background-color: rgba(0, 0, 0, 0.3); " // 很暗的背景
                    "   border: 2px dashed rgba(255, 255, 255, 0.1); " // 虚线边框表示不可用
                    "   border-radius: 15px; "
                    "   text-align: left; "
                    "}"
                    );

        // 3. 修改文字提示，告诉用户为什么不能点
        QList<QLabel*> labels = btnSystem->findChildren<QLabel*>();
        if(labels.count() >= 2) {
            labels[0]->setText("系统设置 (已锁定)");
            labels[1]->setText("权限不足：仅管理员可用");

            // 把大标题改成灰色
            labels[0]->setStyleSheet("font-size: 26px; font-weight: bold; color: #777; background: transparent;");
        }
    }
}
void MainInterface::onRoomStatusClicked() { emit goRoomStatus();}
void MainInterface::onCheckInClicked() { emit goCheckIn(); }
void MainInterface::onCheckOutClicked() { emit goCheckOut(); }
void MainInterface::onSettingsClicked() { emit goSystemSet(); }
