#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class LoginPage : public QWidget
{
    Q_OBJECT
public:
    explicit LoginPage(QWidget *parent = nullptr);
    void clearInput();

protected:
    // 【修改点 1】声明绘制事件，用于支持样式表背景
    void paintEvent(QPaintEvent *event) override;

signals:
    void loginSuccess(int role);

private slots:
    void onLogin();

private:
    void setupUi();
    QLineEdit *userEdit;
    QLineEdit *passEdit;
};

#endif // LOGINPAGE_H
