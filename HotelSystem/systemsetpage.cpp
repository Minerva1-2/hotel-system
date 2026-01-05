#include "systemsetpage.h"
#include "dbmanager.h"
#include "cloudmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>
#include <QStyleOption>
#include <QPainter>
#include <QComboBox>
#include <QFrame>
#include <QPushButton>
#include <QDebug>
#include <QApplication>

// 构造函数
SystemSetPage::SystemSetPage(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(800, 480);
    this->setObjectName("systemPage");
    // 深蓝色科技感背景
    this->setStyleSheet("#systemPage { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #001f3f, stop:1 #003366); }");

    // 1. 初始化指针为空，防止野指针
    newPassEdit = nullptr;
    confirmPassEdit = nullptr;
    addUserEdit = nullptr;
    addPassEdit = nullptr;
    roleCombo = nullptr;
    delUserCombo = nullptr;

    // 2. 构建界面
    setupUi();
}

// 绘制事件（支持样式表背景）
void SystemSetPage::paintEvent(QPaintEvent *) {
    QStyleOption opt; opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

// =========================================================
// 公有槽函数 (Public Slots) - 供 MainWindow 或 本地调用
// =========================================================

// 清空输入框并刷新数据
void SystemSetPage::clearInput() {
    if(newPassEdit) newPassEdit->clear();
    if(confirmPassEdit) confirmPassEdit->clear();
    if(addUserEdit) addUserEdit->clear();
    if(addPassEdit) addPassEdit->clear();

    // 每次进入页面或清空时，强制刷新一次用户列表
    refreshUserList();
}

// 【核心逻辑】刷新用户下拉列表
void SystemSetPage::refreshUserList() {
    // 安全检查：如果界面还没初始化好，直接返回
    if (!delUserCombo) return;

    delUserCombo->clear();

    // 1. 从数据库获取最新用户列表
    QList<QString> users = DBManager::instance().getAllUsers();

    // 2. 移除 admin (超级管理员不可被操作)
    users.removeAll("admin");

    // 3. 填充下拉框
    if (users.isEmpty()) {
        delUserCombo->addItem("无其他用户");
        delUserCombo->setEnabled(false);
    } else {
        delUserCombo->addItems(users);
        delUserCombo->setEnabled(true);
    }

    qDebug() << "用户列表已刷新，当前用户数:" << users.size();
}

// =========================================================
// 界面初始化
// =========================================================
void SystemSetPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 15, 40, 15);
    mainLayout->setSpacing(10);

    // --- 标题 ---
    QLabel *title = new QLabel("系统设置与数据安全", this);
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #00d2ff; font-family: 'WenQuanYi Micro Hei';");
    title->setAlignment(Qt::AlignCenter);
    title->setFixedHeight(40);
    mainLayout->addWidget(title);

    // --- 内容区 ---
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(25);

    // === 样式表定义 ===
    QString cardStyle = "#adminCard, #userCard { background: rgba(255, 255, 255, 0.05); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 12px; }";
    QString editStyle = "QLineEdit { background: rgba(0, 0, 0, 0.25); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 6px; padding-left: 10px; color: white; font-size: 14px; font-family: 'WenQuanYi Micro Hei'; } QLineEdit:focus { border: 1px solid #00d2ff; background: rgba(0, 0, 0, 0.4); }";
    QString comboStyle = "QComboBox { background: rgba(0, 0, 0, 0.25); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 6px; padding-left: 10px; color: white; font-size: 14px; } QComboBox::drop-down { border: none; } QComboBox QAbstractItemView { background: #001f3f; color: white; selection-background-color: #00d2ff; }";

    // 按钮样式
    QString btnBlueStyle = "QPushButton { background: rgba(0, 210, 255, 0.15); color: #00d2ff; border: 1px solid #00d2ff; border-radius: 6px; font-weight: bold; font-size: 15px; } QPushButton:pressed { background: #00d2ff; color: #001f3f; padding-top: 2px; }";
    QString btnGreenStyle = "QPushButton { background: rgba(46, 204, 113, 0.15); color: #2ecc71; border: 1px solid #2ecc71; border-radius: 6px; font-weight: bold; font-size: 14px; } QPushButton:pressed { background: #2ecc71; color: white; padding-top: 2px; }";
    QString btnRedStyle = "QPushButton { background: rgba(231, 76, 60, 0.15); color: #e74c3c; border: 1px solid #e74c3c; border-radius: 6px; font-weight: bold; font-size: 14px; } QPushButton:pressed { background: #e74c3c; color: white; padding-top: 2px; }";

    // ====== 左侧模块：管理员密码修改 ======
    QWidget *adminCard = new QWidget(this);
    adminCard->setObjectName("adminCard");
    adminCard->setStyleSheet(cardStyle);

    QVBoxLayout *cardLayout = new QVBoxLayout(adminCard);
    cardLayout->setContentsMargins(20, 25, 20, 25);
    cardLayout->setSpacing(20);

    QLabel *adminTitle = new QLabel("[ 管理员密码修改 ]", this);
    adminTitle->setAlignment(Qt::AlignCenter);
    adminTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #fff;");

    newPassEdit = new QLineEdit(this);
    newPassEdit->setFixedHeight(45);
    newPassEdit->setStyleSheet(editStyle);
    newPassEdit->setEchoMode(QLineEdit::Password);
    newPassEdit->setPlaceholderText("输入新密码");

    confirmPassEdit = new QLineEdit(this);
    confirmPassEdit->setFixedHeight(45);
    confirmPassEdit->setStyleSheet(editStyle);
    confirmPassEdit->setEchoMode(QLineEdit::Password);
    confirmPassEdit->setPlaceholderText("确认新密码");

    QPushButton *btnSave = new QPushButton("确认修改", this);
    btnSave->setFixedHeight(45);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet(btnBlueStyle);
    connect(btnSave, &QPushButton::clicked, this, &SystemSetPage::onChangePassword);

    cardLayout->addWidget(adminTitle);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(newPassEdit);
    cardLayout->addWidget(confirmPassEdit);
    cardLayout->addStretch();
    cardLayout->addWidget(btnSave);

    contentLayout->addWidget(adminCard, 40);

    // ====== 右侧模块列 ======
    QVBoxLayout *rightCol = new QVBoxLayout();
    rightCol->setSpacing(15);

    // --- 右上：员工账号管理 ---
    QWidget *userCard = new QWidget(this);
    userCard->setObjectName("userCard");
    userCard->setStyleSheet(cardStyle);

    QVBoxLayout *userCardLayout = new QVBoxLayout(userCard);
    userCardLayout->setContentsMargins(15, 15, 15, 15);
    userCardLayout->setSpacing(10);

    QLabel *userTitle = new QLabel("[ 员工账号管理 ]", this);
    userTitle->setAlignment(Qt::AlignCenter);
    userTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #fff; margin-bottom: 5px;");
    userCardLayout->addWidget(userTitle);

    QGridLayout *userGrid = new QGridLayout();
    userGrid->setVerticalSpacing(15);
    userGrid->setHorizontalSpacing(10);

    // 添加用户的输入框
    addUserEdit = new QLineEdit(this);
    addUserEdit->setStyleSheet(editStyle);
    addUserEdit->setPlaceholderText("新用户名");
    addUserEdit->setFixedHeight(40);

    addPassEdit = new QLineEdit(this);
    addPassEdit->setStyleSheet(editStyle);
    addPassEdit->setPlaceholderText("密码");
    addPassEdit->setFixedHeight(40);

    roleCombo = new QComboBox(this);
    // 角色对应：1=普通员工/前台, 0=管理员/经理
    roleCombo->addItem("前台", 1);
    roleCombo->addItem("经理", 0);
    roleCombo->setFixedHeight(40);
    roleCombo->setStyleSheet(comboStyle);

    QPushButton *btnAdd = new QPushButton("添加", this);
    btnAdd->setStyleSheet(btnGreenStyle);
    btnAdd->setFixedSize(70, 40);
    connect(btnAdd, &QPushButton::clicked, this, &SystemSetPage::onAddUser);

    userGrid->addWidget(addUserEdit, 0, 0);
    userGrid->addWidget(addPassEdit, 0, 1);
    userGrid->addWidget(roleCombo, 0, 2);
    userGrid->addWidget(btnAdd, 0, 3);

    // 分割线
    QFrame *line = new QFrame(this); line->setFrameShape(QFrame::HLine); line->setStyleSheet("color: rgba(255,255,255,0.1);");
    userGrid->addWidget(line, 1, 0, 1, 4);

    // 删除用户的下拉框
    delUserCombo = new QComboBox(this);
    delUserCombo->setStyleSheet(comboStyle);
    delUserCombo->setFixedHeight(40);

    QPushButton *btnDel = new QPushButton("删除", this);
    btnDel->setStyleSheet(btnRedStyle);
    btnDel->setFixedSize(70, 40);
    connect(btnDel, &QPushButton::clicked, this, &SystemSetPage::onDeleteUser);

    userGrid->addWidget(delUserCombo, 2, 0, 1, 3);
    userGrid->addWidget(btnDel, 2, 3);
    userGrid->setColumnStretch(0, 1); userGrid->setColumnStretch(1, 1);

    userCardLayout->addLayout(userGrid);
    rightCol->addWidget(userCard);

    // --- 右下：系统运维 (备份/还原/清空) ---
    QGroupBox *sysGroup = new QGroupBox(this);
    sysGroup->setStyleSheet("QGroupBox { background: rgba(255, 255, 255, 0.02); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 12px; }");
    sysGroup->setFixedHeight(130);

    QVBoxLayout *sysLayout = new QVBoxLayout(sysGroup);
    sysLayout->setContentsMargins(15, 15, 15, 10);
    sysLayout->setSpacing(5);

    QHBoxLayout *backupLayout = new QHBoxLayout();
    backupLayout->setSpacing(10);

    QPushButton *btnBackup = new QPushButton("备份数据", this);
    btnBackup->setFixedHeight(35);
    btnBackup->setCursor(Qt::PointingHandCursor);
    btnBackup->setStyleSheet("QPushButton { background: rgba(52, 152, 219, 0.2); border: 1px solid #3498db; color: #3498db; border-radius: 6px; font-weight: bold; } QPushButton:pressed { background: #3498db; color: white; }");
    connect(btnBackup, &QPushButton::clicked, this, &SystemSetPage::onBackupData);

    QPushButton *btnRestore = new QPushButton("还原数据", this);
    btnRestore->setFixedHeight(35);
    btnRestore->setCursor(Qt::PointingHandCursor);
    btnRestore->setStyleSheet("QPushButton { background: rgba(230, 126, 34, 0.2); border: 1px solid #e67e22; color: #e67e22; border-radius: 6px; font-weight: bold; } QPushButton:pressed { background: #e67e22; color: white; }");
    connect(btnRestore, &QPushButton::clicked, this, &SystemSetPage::onRestoreData);

    backupLayout->addWidget(btnBackup);
    backupLayout->addWidget(btnRestore);

    QPushButton *btnClear = new QPushButton("清空入住记录 (不备份)", this);
    btnClear->setFixedHeight(20);
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet("QPushButton { background: transparent; color: #777; border: none; font-size: 12px; text-decoration: underline; } QPushButton:hover { color: #aaa; }");
    connect(btnClear, &QPushButton::clicked, this, &SystemSetPage::onClearData);

    sysLayout->addLayout(backupLayout);
    sysLayout->addWidget(btnClear);

    rightCol->addWidget(sysGroup);

    contentLayout->addLayout(rightCol, 60);
    mainLayout->addLayout(contentLayout);

    // --- 底部返回按钮 ---
    QPushButton *btnBack = new QPushButton(" 返回主菜单", this);
    btnBack->setFixedSize(140, 45);
    btnBack->setCursor(Qt::PointingHandCursor);
    btnBack->setStyleSheet("QPushButton { background-color: rgba(255, 255, 255, 0.1); color: #ffffff; border: 1px solid rgba(255, 255, 255, 0.3); border-radius: 22px; font-size: 15px; } QPushButton:hover { background-color: rgba(255, 255, 255, 0.2); } QPushButton:pressed { background-color: rgba(255, 255, 255, 0.05); }");
    connect(btnBack, &QPushButton::clicked, this, &SystemSetPage::onBack);
    mainLayout->addWidget(btnBack, 0, Qt::AlignRight | Qt::AlignBottom);

    // 初始化时刷新一次列表
    refreshUserList();
}

// =========================================================
// 槽函数实现
// =========================================================

// 本地点击添加用户
void SystemSetPage::onAddUser() {
    QString u = addUserEdit->text().trimmed();
    QString p = addPassEdit->text().trimmed();
    int role = roleCombo->currentData().toInt();

    if(u.isEmpty() || p.isEmpty()) {
        QMessageBox::warning(this, "提示", "内容不能为空");
        return;
    }

    // 1. 写入数据库
    if(DBManager::instance().addUser(u, p, role)) {
        QMessageBox::information(this, "成功", "用户添加成功");

        // 2. 上报日志
        QString logContent = QString("Action:AddUser|User:%1|Role:%2").arg(u).arg(role);
        CloudManager::instance().publishLog("security", logContent);

        // 3. 清空输入并刷新界面
        addUserEdit->clear();
        addPassEdit->clear();
        refreshUserList(); // 【重要】手动刷新列表
    } else {
        QMessageBox::warning(this, "失败", "用户名已存在或数据库错误");
    }
}

// 本地点击删除用户
void SystemSetPage::onDeleteUser() {
    if (!delUserCombo || !delUserCombo->isEnabled()) return;

    QString user = delUserCombo->currentText();
    if (user.isEmpty() || user == "无其他用户") {
        QMessageBox::warning(this, "提示", "请选择有效的用户！");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  "确定要永久删除用户 [" + user + "] 吗？",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (DBManager::instance().deleteUser(user)) {
            QMessageBox::information(this, "成功", "用户已删除。");
            CloudManager::instance().publishLog("security", "Deleted User: " + user);

            refreshUserList(); // 【重要】刷新列表
        } else {
            QMessageBox::critical(this, "失败", "删除失败，数据库错误。");
        }
    }
}

// 修改管理员密码
void SystemSetPage::onChangePassword() {
    QString n = newPassEdit->text();
    QString c = confirmPassEdit->text();

    if(n.isEmpty()) { QMessageBox::warning(this, "提示", "密码不能为空"); return; }
    if(n != c) { QMessageBox::warning(this, "错误", "两次密码不一致"); return; }

    if(DBManager::instance().updatePassword(n)) {
        QMessageBox::information(this, "成功", "管理员密码修改成功！");
        newPassEdit->clear();
        confirmPassEdit->clear();
        CloudManager::instance().publishLog("security", "Admin Password Changed");
    } else {
        QMessageBox::warning(this, "失败", "修改失败，请检查数据库。");
    }
}

// 备份数据
void SystemSetPage::onBackupData() {
    if(DBManager::instance().backupDatabase("hotel_backup.db")) {
        QMessageBox::information(this, "备份成功", "数据库已备份。\n文件: hotel_backup.db");
        CloudManager::instance().publishLog("system", "Manual Backup Created");
    } else {
        QMessageBox::critical(this, "备份失败", "无法写入备份文件。");
    }
}

// 还原数据
void SystemSetPage::onRestoreData() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "还原确认",
                                  "确定要加载备份吗？\n当前数据将被覆盖！",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if(DBManager::instance().restoreDatabase()) {
            QMessageBox::information(this, "还原成功", "系统数据已恢复。");
            refreshUserList(); // 还原后立即刷新
            CloudManager::instance().publishLog("system", "Manual Restore Performed");
        } else {
            QMessageBox::warning(this, "还原失败", "未找到 hotel_backup.db");
        }
    }
}

// 清空数据
void SystemSetPage::onClearData() {
    if (QMessageBox::Yes == QMessageBox::question(this, "清空记录", "确定清空所有业务数据吗？", QMessageBox::Yes | QMessageBox::No)) {
        DBManager::instance().clearBusinessData();
        QMessageBox::information(this, "成功", "记录已清空");
        CloudManager::instance().publishLog("system", "Business Data Cleared");
    }
}

// 返回主菜单
void SystemSetPage::onBack() {
    emit backClicked();
}
