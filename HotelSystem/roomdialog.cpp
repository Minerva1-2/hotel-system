#include "roomdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDateTime>
#include <QPainter>
#include <QStyleOption>

RoomDialog::RoomDialog(RoomInfo info, QWidget *parent)
    : QDialog(parent), m_info(info)
{
    // 1. 窗口基本设置
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    this->setFixedSize(400, 320);
    this->setObjectName("roomDialog");

    // 2. 样式表：深色科技感
    this->setStyleSheet(
        "#roomDialog { background-color: #02111f; border: 2px solid #00d2ff; border-radius: 15px; }"
        "QLabel { color: white; font-family: 'WenQuanYi Micro Hei'; border: none; background: transparent; }"
    );

    setupUi();
}

void RoomDialog::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void RoomDialog::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    // 标题：显示房号
    QLabel *title = new QLabel(QString("房间 %1 - 操作").arg(m_info.id), this);
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #00d2ff;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // 根据房间当前状态显示不同的 UI
    if (m_info.status == 0) { // 空闲
        QLabel *tip = new QLabel("请输入入住客人姓名：", this);
        tip->setStyleSheet("font-size: 16px;");

        nameEdit = new QLineEdit(this);
        nameEdit->setPlaceholderText("姓名");
        nameEdit->setStyleSheet(
            "QLineEdit { background: rgba(255,255,255,0.1); color: white; padding: 8px; "
            "border: 1px solid #444; border-radius: 5px; font-size: 18px; }"
        );
        layout->addWidget(tip);
        layout->addWidget(nameEdit);
    }
    else if (m_info.status == 1) { // 已入住
        QDateTime time = QDateTime::fromTime_t(m_info.checkInTime);
        QLabel *info = new QLabel(QString(
            "当前客人：%1\n"
            "入住时间：%2\n"
            "房间价格：¥ %3 / 晚").arg(m_info.guest).arg(time.toString("MM-dd hh:mm")).arg(m_info.price), this);
        info->setStyleSheet("font-size: 16px; line-height: 25px;");
        layout->addWidget(info);
    }
    else { // 打扫中
        QLabel *tip = new QLabel("该房间正在打扫中，\n清理完成后请点击确认。", this);
        tip->setStyleSheet("font-size: 18px; color: #f39c12;");
        tip->setAlignment(Qt::AlignCenter);
        layout->addWidget(tip);
    }

    layout->addStretch();

    // 底部按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnCancel = new QPushButton("取消", this);
    btnCancel->setFixedSize(100, 40);
    btnCancel->setStyleSheet("QPushButton { background: #444; color: white; border-radius: 5px; border: none; }");
    connect(btnCancel, &QPushButton::clicked, this, &RoomDialog::reject);

    QPushButton *btnOk = new QPushButton(this);
    btnOk->setFixedSize(140, 40);
    // 根据状态给按钮上色
    if (m_info.status == 0) {
        btnOk->setText("办理入住");
        btnOk->setStyleSheet("QPushButton { background: #2ecc71; color: white; font-weight: bold; border-radius: 5px; }");
    } else if (m_info.status == 1) {
        btnOk->setText("退房结算");
        btnOk->setStyleSheet("QPushButton { background: #e74c3c; color: white; font-weight: bold; border-radius: 5px; }");
    } else {
        btnOk->setText("完成打扫");
        btnOk->setStyleSheet("QPushButton { background: #f39c12; color: white; font-weight: bold; border-radius: 5px; }");
    }
    connect(btnOk, &QPushButton::clicked, this, &RoomDialog::onConfirm);

    btnLayout->addWidget(btnCancel);
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    layout->addLayout(btnLayout);
}

void RoomDialog::onConfirm() {
    if (m_info.status == 0) {
        QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "提示", "请输入姓名");
            return;
        }
        DBManager::instance().checkIn(m_info.id, name);
    }
    else if (m_info.status == 1) {
        int cost = DBManager::instance().checkOut(m_info.id);
        QMessageBox::information(this, "结算", QString("退房成功！\n应收总额：¥ %1").arg(cost));
    }
    else {
        DBManager::instance().finishClean(m_info.id);
    }
    accept(); // 成功后关闭弹窗
}
