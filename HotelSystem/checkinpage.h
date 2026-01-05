#ifndef CHECKINPAGE_H
#define CHECKINPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include "cloudmanager.h"

class CheckInPage : public QWidget {
    Q_OBJECT
public:
    explicit CheckInPage(QWidget *parent = nullptr);
    void refreshRoomList();
    void setRfidId(QString id); // 供外部 RFID 线程调用的接口

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void backClicked();

private slots:
    void onConfirm();

private:
    void setupUi();
    void clearForm();

    QComboBox *roomCombo;
    QLineEdit *nameEdit;
    QLineEdit *idEdit;
    QLineEdit *rfidEdit;       // 只读卡号行
    QLabel *statusLight;       // 呼吸灯指示器
    QLabel *rfidHintLabel;     // 状态提示文本
    QString m_currentRfid;
};

#endif
