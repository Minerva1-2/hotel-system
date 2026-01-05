#include "mainwindow.h"
#include "softkeyboard.h"
#include "dbmanager.h"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 设置字体
    QFont font("WenQuanYi Zen Hei", 12);
    a.setFont(font);

    // 2. 初始化数据库
    if (!DBManager::instance().openDb()) {
        return -1; // DB打开失败直接退出
    }

    // 3. 安装半透明软键盘
    SoftKeyboard::instance();
    a.installEventFilter(SoftKeyboard::instance());
    // 4. 显示主窗口
    MainWindow w;
    w.showFullScreen(); // 开发板上全屏

    return a.exec();
}
