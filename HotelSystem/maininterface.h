#ifndef MAININTERFACE_H
#define MAININTERFACE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

class MainInterface : public QWidget
{
    Q_OBJECT
public:
    explicit MainInterface(QWidget *parent = nullptr);
    void setRole(int role);

signals:
    void logout(); // 发送注销信号给 MainWindow
    void goRoomStatus(); // 【新增】跳转房态信号
    void goCheckIn();
    void goCheckOut();
    void goSystemSet();
    void goCardManage();
    void goExtendStay();

protected:
    void paintEvent(QPaintEvent *event) override; // 绘制深海蓝背景

private slots:
    void updateTime(); // 更新时间槽函数
    void onRoomStatusClicked();
    void onCheckInClicked();
    void onCheckOutClicked();
    void onSettingsClicked();

private:
    void setupUi();

    QPushButton* createMenuBtn(QString text, QString subText); // 辅助函数：创建按钮
    QPushButton *btnRoom;
    QPushButton *btnIn;
    QPushButton *btnOut;
    QPushButton *btnSet;
    QPushButton* btnSystem;
    QLabel *timeLabel;
    QTimer *timer;
};



#endif // MAININTERFACE_H
