#ifndef SYSTEMSETPAGE_H
#define SYSTEMSETPAGE_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QPaintEvent>

class SystemSetPage : public QWidget
{
    Q_OBJECT
public:
    explicit SystemSetPage(QWidget *parent = nullptr);

// 【修改点 1】将刷新和清理函数定义为 public slots
// 这样做的好处是既可以像普通函数一样调用，也可以被信号(Signal)连接
public slots:
    void clearInput();       // 清空输入框
    void refreshUserList();  // 【核心】刷新用户下拉列表数据

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void backClicked();

private slots:
    void onBack();

    void onChangePassword();
    void onAddUser();
    void onDeleteUser();

    void onBackupData();
    void onRestoreData();
    void onClearData();

private:
    void setupUi();

    // === 界面控件指针 ===
    QLineEdit *newPassEdit;
    QLineEdit *confirmPassEdit;

    QLineEdit *addUserEdit;
    QLineEdit *addPassEdit;

    QComboBox *roleCombo;
    QComboBox *delUserCombo; // 这个控件需要在 refreshUserList 中被更新
};

#endif // SYSTEMSETPAGE_H
