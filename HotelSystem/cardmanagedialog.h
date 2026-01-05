#ifndef CARDMANAGEDIALOG_H
#define CARDMANAGEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
// 注意：不要在这里包含 rfidthread.h，避免重复引用

class CardManageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CardManageDialog(QWidget *parent = nullptr);
    ~CardManageDialog();

public slots:
    // 【关键】公开槽函数，用于接收主界面的 RFID 信号
    void handleCardDetected(QString cardId);

private slots:
    void onResetInput(); // 重置输入框
    void onWriteCard();  // 绑定/制卡
    void onClearCard();  // 注销

private:
    void setupUi();
    void loadRoomList();

    QLineEdit *cardIdEdit;
    QComboBox *roomCombo;
    QLabel *statusLabel;
};

#endif // CARDMANAGEDIALOG_H
