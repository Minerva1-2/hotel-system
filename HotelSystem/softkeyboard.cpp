#include "softkeyboard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QKeyEvent>
#include <QDebug>

// 单例模式初始化
SoftKeyboard* SoftKeyboard::m_instance = nullptr;

// 字符表定义
const QStringList CHARS_LOWER = {"q","w","e","r","t","y","u","i","o","p","a","s","d","f","g","h","j","k","l","z","x","c","v","b","n","m"};
const QStringList CHARS_UPPER = {"Q","W","E","R","T","Y","U","I","O","P","A","S","D","F","G","H","J","K","L","Z","X","C","V","B","N","M"};
const QStringList CHARS_SYMBOL = {"1","2","3","4","5","6","7","8","9","0","-","/",":",";","(",")","$","&&","@","\"",".",",","?","!","'","_"};

SoftKeyboard* SoftKeyboard::instance() {
    if (!m_instance) m_instance = new SoftKeyboard();
    return m_instance;
}

SoftKeyboard::SoftKeyboard(QWidget *parent) : QDialog(parent) {
    // 无边框窗口，始终在最顶层
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    this->setFixedSize(800, 260); // 高度 260

    // 【修改点 1】键盘整体背景
    // 使用极深的蓝色背景，顶部加一条 2px 的亮青色边框，营造科技感
    this->setStyleSheet(
                "SoftKeyboard { "
                "   background-color: #02111f; " // 极深蓝(接近黑)
                "   border-top: 2px solid #00d2ff; " // 顶部青色高光线
                "}"
                );

    initUI();
}

void SoftKeyboard::initUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(5, 10, 5, 5);

    // 【修改点 2】通用按键样式 (普通字母/数字)
    // 采用半透明玻璃质感，按下时变亮青色
    QString btnStyle =
            "QPushButton { "
            "   background-color: rgba(255, 255, 255, 0.1); " // 平时：10%透明度的白
            "   border: 1px solid rgba(255, 255, 255, 0.15); "
            "   border-radius: 5px; "
            "   font-family: 'WenQuanYi Micro Hei'; "
            "   font-size: 20px; "
            "   color: #ffffff; " // 白色文字
            "}"
            "QPushButton:pressed { "
            "   background-color: #00d2ff; " // 按下：亮青色
            "   color: #000000; "            // 按下：黑色文字(对比度最高)
            "   border: 1px solid #00d2ff; "
            "}";

    // 【修改点 3】功能键样式 (Shift, Del, 123)
    // 稍微深一点的背景，区分于字母键
    QString funcBtnStyle =
            "QPushButton { "
            "   background-color: rgba(255, 255, 255, 0.05); " // 更淡一点
            "   border: 1px solid rgba(255, 255, 255, 0.1); "
            "   border-radius: 5px; "
            "   font-family: 'WenQuanYi Micro Hei'; "
            "   font-size: 18px; "
            "   font-weight: bold; "
            "   color: #cccccc; " // 灰白色文字
            "}"
            "QPushButton:pressed { background-color: #cccccc; color: #000; }";

    // --- 第一行 (q-p) ---
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(5);
    for(int i=0; i<10; i++) {
        QPushButton *btn = new QPushButton();
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setStyleSheet(btnStyle);
        connect(btn, &QPushButton::clicked, this, &SoftKeyboard::onButtonClicked);
        m_letterButtons.append(btn);
        row1->addWidget(btn);
    }
    mainLayout->addLayout(row1);

    // --- 第二行 (a-l) ---
    QHBoxLayout *row2 = new QHBoxLayout();
    row2->setSpacing(5);
    row2->setContentsMargins(20, 0, 20, 0); // 缩进实现交错布局
    for(int i=0; i<9; i++) {
        QPushButton *btn = new QPushButton();
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setStyleSheet(btnStyle);
        connect(btn, &QPushButton::clicked, this, &SoftKeyboard::onButtonClicked);
        m_letterButtons.append(btn);
        row2->addWidget(btn);
    }
    mainLayout->addLayout(row2);

    // --- 第三行 (Shift, z-m, Del) ---
    QHBoxLayout *row3 = new QHBoxLayout();
    row3->setSpacing(5);

    btnShift = new QPushButton("Shift");
    btnShift->setFixedSize(90, 50);
    btnShift->setStyleSheet(funcBtnStyle);
    connect(btnShift, &QPushButton::clicked, this, &SoftKeyboard::onButtonClicked);
    row3->addWidget(btnShift);

    for(int i=0; i<7; i++) {
        QPushButton *btn = new QPushButton();
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setStyleSheet(btnStyle);
        connect(btn, &QPushButton::clicked, this, &SoftKeyboard::onButtonClicked);
        m_letterButtons.append(btn);
        row3->addWidget(btn);
    }

    QPushButton *btnDel = new QPushButton("Del");
    btnDel->setFixedSize(90, 50);
    btnDel->setStyleSheet(funcBtnStyle);
    connect(btnDel, &QPushButton::clicked, this, &SoftKeyboard::onButtonClicked);
    row3->addWidget(btnDel);
    mainLayout->addLayout(row3);

    // --- 第四行 (123, Space, Done) ---
    QHBoxLayout *row4 = new QHBoxLayout();
    row4->setSpacing(5);

    btnMode = new QPushButton("123");
    btnMode->setFixedSize(100, 50);
    btnMode->setStyleSheet(funcBtnStyle);
    connect(btnMode, &QPushButton::clicked, this, &SoftKeyboard::onButtonClicked);
    row4->addWidget(btnMode);

    QPushButton *btnSpace = new QPushButton("Space");
    btnSpace->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    btnSpace->setFixedHeight(50);
    btnSpace->setStyleSheet(btnStyle);
    connect(btnSpace, &QPushButton::clicked, this, &SoftKeyboard::onButtonClicked);
    row4->addWidget(btnSpace);

    // 【修改点 4】Done (确认) 按钮 - 使用系统主题蓝
    QPushButton *btnEnter = new QPushButton("完成");
    btnEnter->setFixedSize(120, 50);
    btnEnter->setStyleSheet(
                "QPushButton { "
                "   background-color: #0074D9; " // 宝蓝色
                "   color: white; "
                "   font-weight: bold; "
                "   border-radius: 5px; "
                "   font-family: 'WenQuanYi Micro Hei'; "
                "   font-size: 20px; "
                "}"
                "QPushButton:pressed { background-color: #0056a3; }"
                );
    connect(btnEnter, &QPushButton::clicked, this, &SoftKeyboard::onButtonClicked);
    row4->addWidget(btnEnter);

    mainLayout->addLayout(row4);
    updateKeyLabels();
}

void SoftKeyboard::updateKeyLabels() {
    for(int i=0; i<m_letterButtons.size(); i++) {
        if (i >= CHARS_LOWER.size()) break;
        if (isNumber) {
            if (i < CHARS_SYMBOL.size()) m_letterButtons[i]->setText(CHARS_SYMBOL[i]);
            else m_letterButtons[i]->setText("");
        } else {
            m_letterButtons[i]->setText(isCapital ? CHARS_UPPER[i] : CHARS_LOWER[i]);
        }
    }

    // 切换 Shift 键的视觉状态 (亮着表示大写开启)
    if (isNumber) {
        btnMode->setText("ABC");
        btnShift->setEnabled(false);
        btnShift->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0.05); color: #555; border:none; border-radius: 5px;}");
    } else {
        btnMode->setText("123");
        btnShift->setEnabled(true);
        if(isCapital) {
            // Shift 按下/锁定状态：亮青色
            btnShift->setStyleSheet("QPushButton { background-color: #00d2ff; color: #000; border-radius: 5px; font-weight: bold;}");
        } else {
            // Shift 普通状态
            btnShift->setStyleSheet("QPushButton { background-color: rgba(255,255,255,0.05); color: #ccc; border: 1px solid rgba(255,255,255,0.1); border-radius: 5px; font-weight: bold;}");
        }
    }
}

void SoftKeyboard::onButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString text = btn->text();
    QObject *focusObj = QApplication::focusWidget();

    if (text == "&&") text = "&";

    if (btn == btnShift) { isCapital = !isCapital; updateKeyLabels(); return; }
    if (btn == btnMode) { isNumber = !isNumber; updateKeyLabels(); return; }

    if (text == "完成") {
        this->hide();
        // 尝试发送回车键，触发登录
        if (focusObj) {
            QKeyEvent ev(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            QCoreApplication::sendEvent(focusObj, &ev);
        }
        return;
    }

    if (!focusObj) return;

    int key = 0;
    if (text == "Del") key = Qt::Key_Backspace;
    else if (text == "Space") { key = Qt::Key_Space; text = " "; }
    else if (!text.isEmpty()) key = text.at(0).unicode();

    QKeyEvent press(QEvent::KeyPress, key, Qt::NoModifier, (text == "Del") ? QString() : text);
    QCoreApplication::sendEvent(focusObj, &press);
}

// 事件过滤器：点击输入框时弹出
bool SoftKeyboard::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QLineEdit *edit = qobject_cast<QLineEdit*>(watched);
        if (edit) {
            // 固定在底部 (屏幕高度 480 - 键盘高度 260 = 220)
            this->move(0, 220);

            isNumber = false;
            updateKeyLabels();
            this->show();
            this->raise(); // 确保键盘浮在最上面
        }
    }
    return QObject::eventFilter(watched, event);
}
