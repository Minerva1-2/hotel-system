#include "rfidthread.h"
#include <QDebug>

// GPIO 定义 (LED 复用引脚)
#define PIN_SDA_CS   71
#define PIN_SCK      72
#define PIN_MOSI     81
#define PIN_MISO     141
#define PIN_RST      -1   // 物理接 3.3V

RfidThread::RfidThread(QObject *parent) : QThread(parent), m_running(true) {
    m_driver = new RC522();
}

RfidThread::~RfidThread() {
    delete m_driver;
}

void RfidThread::stop() {
    m_running = false;
    wait(1000);
}

void RfidThread::run() {
    qDebug() << "=== RFID Continuous Read Mode Started ===";

    // 初始化硬件
    if (!m_driver->init(PIN_SDA_CS, PIN_SCK, PIN_MOSI, PIN_MISO, PIN_RST)) {
        qDebug() << "RFID Error: Init failed. Run ./unlock_leds.sh";
    }

    QString lastCard = "";

    while (m_running) {
        // 1. 尝试读取卡号
        QString currentCard = m_driver->readCardId();

        // 2. 判断是否读到了有效卡号
        if (!currentCard.isEmpty() && currentCard.length() > 6) {

            // 只有当卡号与上一次不同时，才触发信号
            if (currentCard != lastCard) {
                qDebug() << "New Card Detected:" << currentCard;
                emit cardDetected(currentCard);

                // 记录当前卡号，防止一直按着不放时重复触发
                lastCard = currentCard;

                // 【读卡成功冷却】读到后休息 1 秒，避免连击
                QThread::sleep(1);
            }
        }
        else {
            // 【核心修复点】
            // 如果没读到卡（currentCard 为空），说明卡片已经拿开了
            // 此时必须清空 lastCard，这样下次再放回同一张卡时才能再次识别
            lastCard = "";
        }

        // 【循环间歇】
        // 必须保留这个延时，否则会占用 CPU 导致触摸屏卡死
        // 建议 200ms - 300ms，既保证反应速度，又不卡界面
        QThread::msleep(200);
    }
}
