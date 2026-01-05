#ifndef CHECKOUTPAGE_H
#define CHECKOUTPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include "cloudmanager.h"

class CheckOutPage : public QWidget {
    Q_OBJECT
public:
    explicit CheckOutPage(QWidget *parent = nullptr);

    // 供外部 RFID 线程调用的核心接口
    void setRfidId(QString id);
    void clearForm();

protected:
    // 必须重写此函数，否则 QWidget 子类无法显示样式表渐变背景
    void paintEvent(QPaintEvent *event) override;

signals:
    void backClicked();

private slots:
    void onConfirmOut(); // 确认结算退房

private:
    void setupUi();

    QLineEdit *rfidDisplay;   // 显示感应到的卡号
    QLineEdit *roomDisplay;   // 自动匹配的房号
    QLineEdit *nameDisplay;   // 住客姓名
    QLineEdit *costDisplay;   // 结算金额

    QLabel *statusLight;      // 右侧状态指示灯
    QLabel *hintLabel;        // 右侧提示文字

    QString m_currentRfid;    // 存储当前读取的卡号
    QString m_targetRoomId;   // 存储关联的房号
};

#endif
