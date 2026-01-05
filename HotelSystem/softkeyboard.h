#ifndef SOFTKEYBOARD_H
#define SOFTKEYBOARD_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QList>

class SoftKeyboard : public QDialog {
    Q_OBJECT
public:
    explicit SoftKeyboard(QWidget *parent = nullptr);
    static SoftKeyboard* instance();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void onButtonClicked();
private:
    static SoftKeyboard* m_instance;
    void initUI();
    void updateKeyLabels();
    bool isCapital = false;
    bool isNumber = false;
    QList<QPushButton*> m_letterButtons;
    QPushButton *btnShift;
    QPushButton *btnMode;
};
#endif // SOFTKEYBOARD_H
