#include "mainwindow.h"
#include "loginpage.h"
#include "maininterface.h"
#include "roomstatuspage.h"
#include "checkinpage.h"
#include "checkoutpage.h"
#include "rfidthread.h"
#include "systemsetpage.h"
#include "cardmanagedialog.h"
#include "dbmanager.h"
#include "cloudmanager.h"
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDebug>
#include <QCalendarWidget>
#include <QTimer>
#include <functional>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setFixedSize(800, 480);
    this->setWindowFlags(Qt::FramelessWindowHint);
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // 1. 实例化页面
    loginPage = new LoginPage(this);
    mainInterface = new MainInterface(this);
    roomPage = new RoomStatusPage(this);
    checkInPage = new CheckInPage(this);
    checkOutPage = new CheckOutPage(this);
    systemPage = new SystemSetPage(this);

    stack->addWidget(loginPage);
    stack->addWidget(mainInterface);
    stack->addWidget(roomPage);
    stack->addWidget(checkInPage);
    stack->addWidget(checkOutPage);
    stack->addWidget(systemPage);

    // --- 信号连接 (导航逻辑) ---
    // (导航逻辑保持不变)

    // 登录 -> 主页
    connect(loginPage, &LoginPage::loginSuccess, this, [=](int role){
        mainInterface->setRole(role);
        stack->setCurrentIndex(1);
        loginPage->clearInput();
    });

    // 主页 -> 注销
    connect(mainInterface, &MainInterface::logout, this, [=](){
        stack->setCurrentIndex(0);
        loginPage->clearInput();
    });

    // 主页 -> 房态
    connect(mainInterface, &MainInterface::goRoomStatus, this, [=](){
        roomPage->refreshRooms();
        stack->setCurrentIndex(2);
    });
    connect(roomPage, &RoomStatusPage::backClicked, this, [=](){
        stack->setCurrentIndex(1);
    });

    // 主页 -> 入住
    connect(mainInterface, &MainInterface::goCheckIn, this, [=](){
        checkInPage->refreshRoomList();
        stack->setCurrentIndex(3);
    });
    connect(checkInPage, &CheckInPage::backClicked, this, [=](){
        stack->setCurrentIndex(1);
    });

    // 主页 -> 退房
    connect(mainInterface, &MainInterface::goCheckOut, this, [=](){
        if(checkOutPage) checkOutPage->clearForm();
        stack->setCurrentIndex(4);
    });
    connect(checkOutPage, &CheckOutPage::backClicked, this, [=](){
        stack->setCurrentIndex(1);
    });

    // 主页 -> 设置
    connect(mainInterface, &MainInterface::goSystemSet, this, [=](){
        systemPage->clearInput();
        stack->setCurrentIndex(5);
    });
    connect(systemPage, &SystemSetPage::backClicked, this, [=](){
        stack->setCurrentIndex(1);
    });

    // ============================================================
    // 【核心修改】房卡管理模块 - 借用 RFID 信号
    // ============================================================
    connect(mainInterface, &MainInterface::goCardManage, this, [=](){
        CardManageDialog dlg(this);
        QMetaObject::Connection conn = connect(rfidThread, &RfidThread::cardDetected,
                                               &dlg, &CardManageDialog::handleCardDetected);
        dlg.exec();
        disconnect(conn);
    });

    // ============================================================
    // 续房业务逻辑 (完整版)
    // ============================================================
    connect(mainInterface, &MainInterface::goExtendStay, this, [this](){
        // 1. 创建对话框并设置样式
        QDialog dlg(this);
        dlg.setWindowTitle("办理续房业务");
        dlg.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        dlg.setFixedSize(480, 360);

        QString dialogStyle = "QDialog { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0b1019, stop:1 #001f3f); border: 2px solid #00d2ff; border-radius: 15px; }";
        QString labelStyle = "QLabel { color: rgba(255,255,255,0.8); font-family: 'WenQuanYi Micro Hei'; font-size: 16px; } QLabel#Title { color: #00d2ff; font-size: 20px; font-weight: bold; padding-bottom: 10px; border-bottom: 1px solid rgba(0,210,255,0.3); }";
        QString inputStyle =
                "QComboBox { background-color: rgba(255, 255, 255, 0.08); border: 1px solid rgba(255, 255, 255, 0.2); border-radius: 8px; padding: 5px 15px; color: white; font-size: 16px; font-family: 'WenQuanYi Micro Hei'; }"
                "QComboBox:hover { border: 1px solid #00d2ff; background-color: rgba(0, 210, 255, 0.1); }"
                "QComboBox::drop-down { border: none; width: 30px; }"
                "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #00d2ff; margin-right: 10px; }"
                "QPushButton#DateBtn { background-color: rgba(0, 0, 0, 0.2); border: 1px solid rgba(0, 210, 255, 0.5); border-radius: 8px; padding: 5px 15px; color: #00d2ff; font-size: 18px; font-weight: bold; font-family: 'WenQuanYi Micro Hei'; text-align: center; }"
                "QPushButton#DateBtn:hover { background-color: rgba(0, 210, 255, 0.15); border: 1px solid #00d2ff; }";
        QString calendarStyle = "QCalendarWidget QWidget { alternate-background-color: #0b1019; background-color: #0b1019; } QCalendarWidget QToolButton { color: white; icon-size: 20px; font-weight: bold; } QCalendarWidget QMenu { background-color: #001f3f; color: white; } QCalendarWidget QSpinBox { color: white; background: #001f3f; selection-background-color: #00d2ff; } QCalendarWidget QAbstractItemView:enabled { color: white; background-color: #0b1019; selection-background-color: #00d2ff; selection-color: #000000; } QCalendarWidget QAbstractItemView:disabled { color: #555; }";

        dlg.setStyleSheet(dialogStyle + labelStyle + inputStyle + calendarStyle);

        // 2. 布局初始化
        QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);
        mainLayout->setContentsMargins(30, 25, 30, 25);
        mainLayout->setSpacing(15);

        QLabel *title = new QLabel("续房办理 / Extend Stay", &dlg);
        title->setObjectName("Title");
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);

        QGridLayout *formGrid = new QGridLayout();
        formGrid->setVerticalSpacing(25);
        formGrid->setHorizontalSpacing(15);

        // 3. 表单控件
        QLabel *l1 = new QLabel("选择房间:", &dlg); l1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QComboBox *comboRooms = new QComboBox(&dlg);
        comboRooms->setFixedHeight(45);
        comboRooms->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // 加载在住房间
        QList<RoomInfo> rooms = DBManager::instance().getAllRooms();
        for(const RoomInfo &room : rooms) {
            if(room.status == 1) comboRooms->addItem(room.id);
        }
        if(comboRooms->count() == 0) {
            comboRooms->addItem("无在住房间");
            comboRooms->setEnabled(false);
        }

        QLabel *l2 = new QLabel("当前退房日:", &dlg); l2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QLabel *lblOldDate = new QLabel("-", &dlg);
        lblOldDate->setStyleSheet("color: #ff9f43; font-weight: bold; font-size: 18px;");
        lblOldDate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        QLabel *l3 = new QLabel("续住至:", &dlg); l3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QPushButton *btnDate = new QPushButton(QDate::currentDate().toString("yyyy-MM-dd"), &dlg);
        btnDate->setObjectName("DateBtn");
        btnDate->setFixedHeight(45);
        btnDate->setCursor(Qt::PointingHandCursor);
        btnDate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        formGrid->addWidget(l1, 0, 0); formGrid->addWidget(comboRooms, 0, 1);
        formGrid->addWidget(l2, 1, 0); formGrid->addWidget(lblOldDate, 1, 1);
        formGrid->addWidget(l3, 2, 0); formGrid->addWidget(btnDate, 2, 1);
        formGrid->setColumnStretch(0, 1); formGrid->setColumnStretch(1, 3);
        mainLayout->addLayout(formGrid);

        // 分割线
        QFrame *line = new QFrame(&dlg);
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet("background-color: rgba(255,255,255,0.1); max-height: 1px;");
        mainLayout->addSpacing(10);
        mainLayout->addWidget(line);
        mainLayout->addStretch();

        // 按钮组
        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(20);
        QPushButton *btnCancel = new QPushButton("取 消", &dlg);
        QPushButton *btnOk = new QPushButton("确认续房", &dlg);
        btnCancel->setFixedSize(120, 45);
        btnOk->setFixedSize(160, 45);
        btnCancel->setStyleSheet("QPushButton { background: transparent; border: 1px solid rgba(255,255,255,0.3); color: #aaa; border-radius: 22px; } QPushButton:pressed { background: rgba(255,255,255,0.1); color: white; }");
        btnOk->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #00d2ff, stop:1 #3a7bd5); border: none; color: white; border-radius: 22px; font-weight: bold; font-size: 16px; } QPushButton:pressed { padding-top: 2px; background: #3a7bd5; }");
        btnLayout->addStretch();
        btnLayout->addWidget(btnCancel);
        btnLayout->addWidget(btnOk);
        btnLayout->addStretch();
        mainLayout->addLayout(btnLayout);

        // 4. 逻辑：更新日期显示
        std::function<void()> updateFunc = [=]() {
            QString rid = comboRooms->currentText();
            if(rid.isEmpty() || rid == "无在住房间") return;

            QPair<QDate, double> info = DBManager::instance().getStayInfo(rid);
            QDate oldDate = info.first;
            lblOldDate->setText(oldDate.toString("yyyy-MM-dd"));

            QDate selectedDate = QDate::fromString(btnDate->text(), "yyyy-MM-dd");
            if (selectedDate <= oldDate) {
                btnDate->setText(oldDate.addDays(1).toString("yyyy-MM-dd"));
            }
        };

        // 5. 逻辑：弹出日历
        connect(btnDate, &QPushButton::clicked, [=, &dlg]() {
            QDialog calDlg(&dlg);
            calDlg.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
            calDlg.setAttribute(Qt::WA_TranslucentBackground);
            calDlg.setFixedSize(350, 260);

            QVBoxLayout *vl = new QVBoxLayout(&calDlg);
            vl->setContentsMargins(0,0,0,0);
            QFrame *frame = new QFrame;
            frame->setStyleSheet("QFrame { background-color: #0b1019; border: 1px solid #00d2ff; border-radius: 8px; }");
            QVBoxLayout *fl = new QVBoxLayout(frame);
            fl->setContentsMargins(5,5,5,5);

            QCalendarWidget *calendar = new QCalendarWidget();
            calendar->setStyleSheet(calendarStyle);
            calendar->setGridVisible(false);
            calendar->setNavigationBarVisible(true);

            QString rid = comboRooms->currentText();
            QPair<QDate, double> info = DBManager::instance().getStayInfo(rid);
            calendar->setMinimumDate(info.first.addDays(1));
            calendar->setSelectedDate(QDate::fromString(btnDate->text(), "yyyy-MM-dd"));

            fl->addWidget(calendar);
            vl->addWidget(frame);

            connect(calendar, &QCalendarWidget::clicked, [&](const QDate &date){
                btnDate->setText(date.toString("yyyy-MM-dd"));
                calDlg.accept();
                updateFunc();
            });

            // 智能计算位置 (保持不变)
            QRect screenRect(0, 0, 800, 480);
            QPoint btnGlobalPos = btnDate->mapToGlobal(QPoint(0, 0));
            int calW = calDlg.width();
            int calH = calDlg.height();
            int btnH = btnDate->height();
            int btnW = btnDate->width();
            int targetY = btnGlobalPos.y() + btnH + 2;
            if (targetY + calH > screenRect.height()) targetY = btnGlobalPos.y() - calH - 2;
            if (targetY < 0) targetY = 0;
            int targetX = btnGlobalPos.x() - (calW - btnW) / 2;
            if (targetX < 0) targetX = 0;
            if (targetX + calW > screenRect.width()) targetX = screenRect.width() - calW;
            calDlg.move(targetX, targetY);
            calDlg.exec();
        });

        connect(comboRooms, &QComboBox::currentTextChanged, updateFunc);
        connect(btnOk, &QPushButton::clicked, &dlg, &QDialog::accept);
        connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);

        updateFunc();

        // 6. 确认办理
        if(dlg.exec() == QDialog::Accepted) {
            if (comboRooms->currentText() == "无在住房间") return;

            QString rid = comboRooms->currentText();
            QDate oldDate = QDate::fromString(lblOldDate->text(), "yyyy-MM-dd");
            QDate newDate = QDate::fromString(btnDate->text(), "yyyy-MM-dd");
            int extraDays = oldDate.daysTo(newDate);

            if(DBManager::instance().extendStay(rid, extraDays)) {
                QMessageBox::information(this, "办理成功", QString("房间 %1 续房成功！\n延期至：%2").arg(rid).arg(newDate.toString("yyyy-MM-dd")));
                QString logContent = QString("Action:ExtendStay|Room:%1|ExtraDays:%2|NewDate:%3")
                        .arg(rid)
                        .arg(extraDays)
                        .arg(newDate.toString("yyyy-MM-dd"));
                CloudManager::instance().publishLog("business", logContent);
                if(this->roomPage) this->roomPage->refreshRooms();
            } else {
                QMessageBox::critical(this, "办理失败", "数据库操作异常，请检查日志。");
                CloudManager::instance().publishLog("error", "Extend Stay Failed for Room: " + rid);
            }
        }
    });

    // 连接 CloudManager 的后台控制指令
    // 1. 处理 [远程添加用户]
    connect(&CloudManager::instance(), &CloudManager::requestAddUser,
            this, [this](QString username, QString password){
        bool success = DBManager::instance().addUser(username, password);
        if (success) {
            qDebug() << "✅ [远程指令] 用户添加成功:" << username;
            CloudManager::instance().publishLog("system", "User added via remote: " + username);

            if (systemPage) {
                systemPage->refreshUserList();
            }
        } else {
            qDebug() << "❌ [远程指令] 用户添加失败:" << username;
        }
    });
    connect(&CloudManager::instance(), &CloudManager::requestDeleteUser,
            this, [this](QString username){

        // 保护机制：禁止远程删除 admin
        if(username == "admin") {
            CloudManager::instance().publishLog("warning", "Remote delete blocked: Cannot delete admin.");
            return;
        }

        // 调用 DBManager 删除
        bool success = DBManager::instance().deleteUser(username);

        if (success) {
            qDebug() << "✅ [远程指令] 用户已删除:" << username;
            CloudManager::instance().publishLog("security", "User deleted via remote: " + username);

            // 刷新系统设置页面的列表 (如果页面已初始化)
            if (systemPage) {
                systemPage->refreshUserList();
            }
        } else {
            qDebug() << "❌ [远程指令] 删除失败 (用户可能不存在):" << username;
            CloudManager::instance().publishLog("error", "Remote delete failed: " + username);
        }
    });
    // 2. 处理 [远程系统恢复]
    connect(&CloudManager::instance(), &CloudManager::systemRestored,
            this, &MainWindow::onSystemRestored);
    // 初始化 RFID 线程
    rfidThread = new RfidThread(this);
    connect(rfidThread, &RfidThread::cardDetected, this, [=](QString id){
        int currentIndex = stack->currentIndex();
        qDebug() << "RFID Scanned:" << id << " Page:" << currentIndex;

        // 路由逻辑：根据当前页面填入不同表格
        if (currentIndex == 3) {
            checkInPage->setRfidId(id);
        } else if (currentIndex == 4) {
            checkOutPage->setRfidId(id);
        }
    });

    rfidThread->start();

    // ============================================================
    // 【最后】连接云端 (必须放在 connect 信号连接之后)
    // ============================================================
    // 请确保这些宏定义在 .pro 文件中已正确设置
    // 这里的参数列表已经更新，以支持默认的 cmdTopic
    CloudManager::instance().connectToCloud(MQTT_IP,
                                            MQTT_PORT,
                                            MQTT_USERNAME,
                                            MQTT_PASSWORD,
                                            MQTT_TOPIC);
}

MainWindow::~MainWindow() {
    if(rfidThread) {
        rfidThread->stop();
        rfidThread->wait(1000);
    }
}

void MainWindow::showMainInterface() {
    stack->setCurrentIndex(1);
}


void MainWindow::closeEvent(QCloseEvent *event) {
    event->accept();
}
void MainWindow::onSystemRestored()
{
    qDebug() << "接收到恢复信号，正在刷新界面...";

    if (roomPage) {
        roomPage->refreshRooms();
    }

    // 2. 刷新入住办理页 (假设指针叫 checkInPage)
    if (checkInPage) {
        checkInPage->refreshRoomList(); // 重新加载房间列表下拉框
    }
}
