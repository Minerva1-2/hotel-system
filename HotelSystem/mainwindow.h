#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTime>
#include <QCloseEvent>
#include "loginpage.h"
#include "maininterface.h"
#include "roomstatuspage.h"
#include "checkinpage.h"
#include "checkoutpage.h"
#include "cloudmanager.h"
#include "rfidthread.h"
#include "systemsetpage.h"
#include "dbmanager.h"
#include "cardmanagedialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 重写关闭事件，用于软件退出时备份
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showMainInterface();
    void onSystemRestored();

private:
    QStackedWidget           *stack;
    LoginPage                *loginPage;
    RoomStatusPage           *roomPage;
    CheckInPage              *checkInPage;
    CheckOutPage             *checkOutPage;
    MainInterface            *mainInterface;
    RfidThread               *rfidThread;
    SystemSetPage            *systemPage;
};

#endif // MAINWINDOW_H
