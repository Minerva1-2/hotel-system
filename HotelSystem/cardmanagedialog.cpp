#include "cardmanagedialog.h"
#include "dbmanager.h"
#include "cloudmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QStyle>

CardManageDialog::CardManageDialog(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("智能房卡管理终端");
    this->setFixedSize(450, 340);
    this->setStyleSheet("background-color: #2c3e50; color: white; font-family: 'WenQuanYi Micro Hei';");
    setupUi();
    loadRoomList();
}

CardManageDialog::~CardManageDialog() {
}

// 当主界面读到卡时，会自动调用此函数
void CardManageDialog::handleCardDetected(QString cardId) {
    if (cardId.isEmpty()) return;

    // 1. 填入卡号
    cardIdEdit->setText(cardId);
    statusLabel->setText("✅ RFID感应成功! 卡号: " + cardId);

    // 2. 如果这张卡已经绑定了某个房间，自动选中该房间
    QString boundRoom = DBManager::instance().getRoomByCard(cardId);
    if (!boundRoom.isEmpty()) {
        int idx = roomCombo->findText(boundRoom);
        if (idx >= 0) {
            roomCombo->setCurrentIndex(idx);
            statusLabel->setText(QString("✅ 已识别: 房间 %1 的房卡").arg(boundRoom));
        }
    }
    CloudManager::instance().publishLog("card_manage", "Admin Scanned Card: " + cardId);
}

void CardManageDialog::loadRoomList() {
    roomCombo->clear();
    QList<RoomInfo> rooms = DBManager::instance().getAllRooms();
    if(rooms.isEmpty()) {
        roomCombo->addItem("无房间数据");
        roomCombo->setEnabled(false);
    } else {
        for(const RoomInfo &room : rooms) {
            roomCombo->addItem(room.id);
        }
        roomCombo->setEnabled(true);
    }
}

void CardManageDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 25, 30, 20);

    // 卡号区
    QLabel *lbl1 = new QLabel("当前感应到的卡号 (RFID UID):", this);
    lbl1->setStyleSheet("color: #00d2ff; font-weight: bold; font-size: 14px;");

    cardIdEdit = new QLineEdit(this);
    cardIdEdit->setPlaceholderText("请将房卡放置在感应区...");
    cardIdEdit->setFixedHeight(45);


    cardIdEdit->setReadOnly(true);        // 禁止键盘输入
    cardIdEdit->setFocusPolicy(Qt::NoFocus); // 禁止鼠标点击获取焦点（点击无反应）
    cardIdEdit->setAttribute(Qt::WA_TransparentForMouseEvents);

    cardIdEdit->setStyleSheet(
                "QLineEdit { "
                "   background-color: rgba(0, 210, 255, 0.1); " // 淡淡的青色背景，通透感
                "   border: 1px dashed rgba(0, 210, 255, 0.6); " // 虚线边框，暗示感应区
                "   border-radius: 8px; "
                "   padding-left: 12px; "
                "   color: #00d2ff; "      // 亮青色文字
                "   font-family: 'WenQuanYi Micro Hei'; "
                "   font-size: 18px; "     // 字号加大
                "   font-weight: bold; "
                "}"
                );

    // 房间区
    QLabel *lbl2 = new QLabel("绑定房间号 (选择目标房间):", this);
    lbl2->setStyleSheet("margin-top: 5px; font-size: 14px; color: #ddd;");

    roomCombo = new QComboBox(this);
    roomCombo->setFixedHeight(40);
    roomCombo->setStyleSheet(
                "QComboBox { background: rgba(255, 255, 255, 0.08); border: 1px solid rgba(255, 255, 255, 0.2); border-radius: 8px; padding-left: 10px; color: white; font-size: 16px; }"
                "QComboBox:hover { border: 1px solid #00d2ff; }"
                "QComboBox::drop-down { border: none; width: 30px; }"
                "QComboBox QAbstractItemView { background: #001f3f; color: white; selection-background-color: #00d2ff; selection-color: #001f3f; }"
                );

    // 按钮区
    QHBoxLayout *actionLayout = new QHBoxLayout();
    QString btnStyle = "QPushButton { background-color: rgba(%1, 0.15); border: 1px solid rgb(%1); color: rgb(%1); border-radius: 20px; font-weight: bold; font-size: 14px; padding-left: 5px; } QPushButton:hover { background-color: rgba(%1, 0.3); border: 2px solid rgb(%1); } QPushButton:pressed { background-color: rgb(%1); color: #001f3f; }";

    QPushButton *btnReset = new QPushButton(" 重置", this);
    btnReset->setFixedHeight(40);
    btnReset->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    btnReset->setStyleSheet(btnStyle.arg("0, 210, 255"));
    connect(btnReset, &QPushButton::clicked, this, &CardManageDialog::onResetInput);

    QPushButton *btnWrite = new QPushButton(" 绑定/制卡", this);
    btnWrite->setFixedHeight(40);
    btnWrite->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    btnWrite->setStyleSheet(btnStyle.arg("46, 204, 113"));
    connect(btnWrite, &QPushButton::clicked, this, &CardManageDialog::onWriteCard);

    QPushButton *btnClear = new QPushButton(" 注销", this);
    btnClear->setFixedHeight(40);
    btnClear->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    btnClear->setStyleSheet(btnStyle.arg("231, 76, 60"));
    connect(btnClear, &QPushButton::clicked, this, &CardManageDialog::onClearCard);

    actionLayout->addWidget(btnReset);
    actionLayout->addWidget(btnWrite);
    actionLayout->addWidget(btnClear);

    // 底部状态
    statusLabel = new QLabel("RFID 硬件在线：请直接刷卡...", this);
    statusLabel->setStyleSheet("color: rgba(255,255,255,0.5); font-size: 12px; margin-top: 5px;");

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(statusLabel);
    bottomLayout->addStretch();

    QPushButton *btnExit = new QPushButton(" 关闭", this);
    btnExit->setFixedSize(90, 32);
    btnExit->setStyleSheet("QPushButton { background: transparent; border: 1px solid rgba(255, 255, 255, 0.3); color: rgba(255, 255, 255, 0.5); border-radius: 16px; font-size: 13px; } QPushButton:hover { background: rgba(255, 255, 255, 0.1); color: white; border-color: white; }");
    connect(btnExit, &QPushButton::clicked, this, &CardManageDialog::close);
    bottomLayout->addWidget(btnExit);

    mainLayout->addWidget(lbl1);
    mainLayout->addWidget(cardIdEdit);
    mainLayout->addWidget(lbl2);
    mainLayout->addWidget(roomCombo);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(actionLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(bottomLayout);
}

void CardManageDialog::onResetInput() {
    cardIdEdit->clear();
    statusLabel->setText("等待刷卡...");
}

void CardManageDialog::onWriteCard() {
    QString targetRoom = roomCombo->currentText(); // 目标房间
    QString newCardId = cardIdEdit->text().trimmed(); // 当前要绑定的新卡

    // --- 0. 基础校验 ---
    if(targetRoom.isEmpty() || targetRoom == "无房间数据") {
        QMessageBox::warning(this, "警告", "请选择有效的房间号");
        return;
    }
    if(newCardId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先刷卡");
        return;
    }

    // --- 1. 检查【卡冲突】（这张卡是否属于其他房间？） ---
    QString oldRoomBoundToCard = DBManager::instance().getRoomByCard(newCardId);

    // 如果卡已经被绑定，且绑定的不是当前目标房间
    if (!oldRoomBoundToCard.isEmpty() && oldRoomBoundToCard != targetRoom) {
        QString msg = QString("冲突警告：\n卡号 [%1] 当前已绑定在房间 【%2】。\n\n是否强制解绑旧房间，并转移到新房间？")
                .arg(newCardId).arg(oldRoomBoundToCard);

        if (QMessageBox::question(this, "卡片占用", msg) == QMessageBox::Yes) {
            // 用户同意：先从旧房间解绑
            DBManager::instance().unbindCard(newCardId);
        } else {
            return; // 用户取消，停止操作
        }
    }

    // --- 2. 检查【房冲突】（这个房间是否已经有别的卡了？） ---
    QString oldCardInRoom = DBManager::instance().getCardByRoom(targetRoom);

    // 如果房间里有卡，且这张卡不是我们现在手里这张
    if (!oldCardInRoom.isEmpty() && oldCardInRoom != newCardId) {
        QString msg = QString("覆盖警告：\n房间 【%1】 当前已绑定了另一张卡 [%2]。\n\n是否作废旧卡，绑定当前新卡？")
                .arg(targetRoom).arg(oldCardInRoom);

        if (QMessageBox::question(this, "房间占用", msg) == QMessageBox::Yes) {
            // 用户同意：这里不需要专门解绑旧卡，updateRoomCard 会直接覆盖掉 rfid_id 字段
        } else {
            return; // 用户取消
        }
    }

    // --- 3. 检查【重复操作】 ---
    if (oldRoomBoundToCard == targetRoom) {
        QMessageBox::information(this, "提示", "这张卡已经是该房间的房卡了，无需重复绑定。");
        return;
    }

    // --- 4. 执行最终绑定 ---
    // 此时：卡是自由的（或者用户同意转移），房是自由的（或者用户同意覆盖）
    if(DBManager::instance().updateRoomCard(targetRoom, newCardId)) {
        QMessageBox::information(this, "成功", QString("绑定成功！\n\n房间：%1\n卡号：%2").arg(targetRoom).arg(newCardId));
        statusLabel->setText("✅ 绑定完成: " + targetRoom);

        QString logContent = QString("Action:Bind|Room:%1|Card:%2").arg(targetRoom).arg(newCardId);
        CloudManager::instance().publishLog("card_manage", logContent);
    } else {
        QMessageBox::critical(this, "失败", "数据库写入失败，请重试。");
    }
}

void CardManageDialog::onClearCard() {
    QString card = cardIdEdit->text().trimmed();
    if(card.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先刷卡获取要注销的卡号");
        return;
    }

    // 二次确认
    if(QMessageBox::Yes != QMessageBox::question(this, "确认",
                                                 "确定要注销这张房卡吗？\n注销后该卡将无法开门，对应的房间将变为空闲卡状态。",
                                                 QMessageBox::Yes | QMessageBox::No)) {
        return;
    }

    // 调用数据库执行真正的解绑
    if (DBManager::instance().unbindCard(card)) {
        cardIdEdit->clear();
        roomCombo->setCurrentIndex(-1); // 清空房间选择
        statusLabel->setText("✅ 卡片已注销，绑定解除");
        QMessageBox::information(this, "成功", "房卡已成功注销！");

        QString logContent = QString("Action:Unbind|Card:%1").arg(card);
        CloudManager::instance().publishLog("card_manage", logContent);
    } else {
        // 如果返回 false，可能是卡号本身就没绑定，或者数据库错误
        statusLabel->setText("卡片未绑定任何房间");
        QMessageBox::information(this, "提示", "该卡片当前未绑定任何房间，无需注销。");
    }
}
