#ifndef RC522_H
#define RC522_H

#include <QObject>
#include <QByteArray>
#include <QMutex>

class RC522 : public QObject
{
    Q_OBJECT
public:
    explicit RC522(QObject *parent = nullptr);
    ~RC522();

    // 初始化：rstPin 填 -1 表示物理接 3.3V
    bool init(int csPin, int sckPin, int mosiPin, int misoPin, int rstPin);
    QString readCardId();

private:
    int m_pinCS, m_pinSCK, m_pinMOSI, m_pinMISO, m_pinRST;
    int m_fdCS, m_fdSCK, m_fdMOSI, m_fdMISO, m_fdRST;
    QMutex m_mutex;

    // SPI 模拟
    uint8_t spiTransferByte(uint8_t data);

    // GPIO 操作
    void gpioExport(int pin);
    void gpioSetDir(int pin, bool output);
    int  gpioOpen(int pin);
    void gpioWriteFast(int fd, int val);
    int  gpioReadFast(int fd);

    // 寄存器操作
    void resetHardware();
    void writeReg(uint8_t reg, uint8_t value);
    uint8_t readReg(uint8_t reg);
    void setBitMask(uint8_t reg, uint8_t mask);
    void antennaOn();
    bool pcdRequest(uint8_t reqMode, uint8_t *tagType);
    bool pcdAnticoll(QByteArray &uid);
};

#endif // RC522_H
