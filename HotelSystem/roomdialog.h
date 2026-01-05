#ifndef ROOMDIALOG_H
#define ROOMDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "dbmanager.h" // 必须引用以识别 RoomInfo 结构体

class RoomDialog : public QDialog
{
    Q_OBJECT
public:
    // 构造函数：接收一个 RoomInfo 结构体来决定显示什么内容
    explicit RoomDialog(RoomInfo info, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override; // 用于支持 QSS 背景

private slots:
    void onConfirm(); // 点击确认/结算按钮的逻辑

private:
    void setupUi();
    RoomInfo m_info;    // 存储当前操作的房间数据
    QLineEdit *nameEdit; // 入住时输入姓名的框
};

#endif // ROOMDIALOG_H
