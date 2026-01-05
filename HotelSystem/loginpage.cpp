//登录界面
#include "loginpage.h"
#include "dbmanager.h" // 引入数据库管理
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QEvent>
#include <QStyleOption>

LoginPage::LoginPage(QWidget *parent) : QWidget(parent) {
    this->setFixedSize(800, 480);
    this->setObjectName("loginPage");
    this->setStyleSheet("#loginPage { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #001f3f, stop:1 #003366); }");

    setupUi();
}
void LoginPage::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
void LoginPage::setupUi() {
    // 3. 创建中心磨砂玻璃容器
    QWidget *centerBox = new QWidget(this);
    centerBox->setFixedSize(400, 280); // 稍微加宽，让中文显示不拥挤

    // 容器样式：半透明白色背景 + 柔和白边框
    centerBox->setStyleSheet(
                "QWidget { "
                "   background-color: rgba(255, 255, 255, 0.1); "
                "   border-radius: 20px; "
                "   border: 1px solid rgba(255, 255, 255, 0.2);"
                "}"
                );

    // 4. 标题 (中文)
    QLabel *titleLabel = new QLabel("酒店房卡管理系统", centerBox);
    titleLabel->setAlignment(Qt::AlignCenter);
    // 字体样式：字号加大到 32px，加粗，纯白，使用微米黑字体

    titleLabel->setStyleSheet(
                "QLabel { "
                "   color: #ffffff; "                  // 纯白色
                "   font-family: 'WenQuanYi Micro Hei'; " // 嵌入式系统标准中文字体
                "   font-size: 32px; "                 // 维持 32px 的大标题感
                "   font-weight: bold; "               // 加粗
                "   background: transparent; "         // 背景透明
                "   border: none; "                    // 无边框
                "}"
                );
    // 5. 输入框样式 (高对比度设计)
    QString editStyle =
            "QLineEdit { "
            "   border: 1px solid rgba(255, 255, 255, 0.4); "
            "   border-radius: 8px; "
            "   padding: 5px 10px; "
            "   background: rgba(0, 0, 0, 0.4); " // 背景加深(40%黑)，衬托白字
            "   color: #ffffff; "          // 文字纯白
            "   font-family: 'WenQuanYi Micro Hei'; "
            "   font-size: 18px; "
            "   font-weight: bold; "
            "   selection-background-color: #0074D9;"
            "}"
            // 聚焦时的样式：亮青色边框，背景更深
            "QLineEdit:focus { "
            "   border: 2px solid #00FFFF; "
            "   background: rgba(0, 0, 0, 0.6); "
            "}";

    userEdit = new QLineEdit(centerBox);
    userEdit->setPlaceholderText("请输入管理员账号"); // 中文占位符
    userEdit->setStyleSheet(editStyle);

    passEdit = new QLineEdit(centerBox);
    passEdit->setPlaceholderText("请输入管理员密码"); // 中文占位符
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setStyleSheet(editStyle);

    // 6. 登录按钮 (中文)
    QPushButton *loginBtn = new QPushButton("立即登录", centerBox);
    loginBtn->setFixedHeight(45);
    loginBtn->setStyleSheet(
                "QPushButton { "
                "   background-color: #0074D9; " // 经典的蓝色
                "   color: white; "
                "   border-radius: 22px; "
                "   font-family: 'WenQuanYi Micro Hei'; "
                "   font-size: 20px; "
                "   font-weight: bold; "
                "   border: none;"
                "}"
                "QPushButton:pressed { background-color: #0056a3; margin-top: 2px; }"
                );
    connect(loginBtn, &QPushButton::clicked, this, &LoginPage::onLogin);

    // 布局管理
    QVBoxLayout *boxLayout = new QVBoxLayout(centerBox);
    boxLayout->setContentsMargins(40, 30, 40, 30);
    boxLayout->setSpacing(20);

    boxLayout->addWidget(titleLabel);
    boxLayout->addWidget(userEdit);
    boxLayout->addWidget(passEdit);
    boxLayout->addSpacing(10);
    boxLayout->addWidget(loginBtn);

    // 将中心框放置在主界面
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(centerBox, 0, Qt::AlignCenter);

    // 为底部键盘预留一点视觉空间 (可选)
    mainLayout->addSpacing(50);
}

void LoginPage::onLogin() {
    QString user = userEdit->text().trimmed();
    QString pass = passEdit->text().trimmed();

    // checkAdmin 返回: 0=管理员, 1=普通用户, -1=失败
    int role = DBManager::instance().checkAdmin(user, pass);

    if (role != -1) {
        // 【关键】登录成功，把 role (0或1) 发送给 MainWindow
        emit loginSuccess(role);
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误！");
    }
}
void LoginPage::clearInput() {
    if(userEdit) userEdit->clear();
    if(passEdit) passEdit->clear();
    // 把光标重新置于用户名框，方便下一次直接输入
    if(userEdit) userEdit->setFocus();
}
