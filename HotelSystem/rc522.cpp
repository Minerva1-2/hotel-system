#include "rc522.h"
#include <QDebug>
#include <QThread>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

// 寄存器定义
#define CommandReg    0x01
#define FIFODataReg   0x09
#define FIFOLevelReg  0x0A
#define BitFramingReg 0x0D
#define ModeReg       0x11
#define TxControlReg  0x14
#define TxASKReg      0x15
#define TModeReg      0x2A
#define TPrescalerReg 0x2B
#define TReloadRegH   0x2C
#define TReloadRegL   0x2D
#define PCD_IDLE       0x00
#define PCD_TRANSCEIVE 0x0C
#define PCD_RESETPHASE 0x0F
#define PICC_REQIDL    0x26
#define PICC_ANTICOLL  0x93

RC522::RC522(QObject *parent) : QObject(parent) {
    m_fdCS = m_fdSCK = m_fdMOSI = m_fdMISO = m_fdRST = -1;
}

RC522::~RC522() {
    if (m_fdCS >= 0) close(m_fdCS);
}

bool RC522::init(int csPin, int sckPin, int mosiPin, int misoPin, int rstPin) {
    m_pinCS = csPin; m_pinSCK = sckPin; m_pinMOSI = mosiPin;
    m_pinMISO = misoPin; m_pinRST = rstPin;

    // 1. 导出引脚
    gpioExport(m_pinCS);   gpioSetDir(m_pinCS, true);
    gpioExport(m_pinSCK);  gpioSetDir(m_pinSCK, true);
    gpioExport(m_pinMOSI); gpioSetDir(m_pinMOSI, true);
    gpioExport(m_pinMISO); gpioSetDir(m_pinMISO, false); // 输入

    if (m_pinRST >= 0) {
        gpioExport(m_pinRST); gpioSetDir(m_pinRST, true);
        m_fdRST = gpioOpen(m_pinRST);
    } else {
        m_fdRST = -1;
    }

    m_fdCS   = gpioOpen(m_pinCS);
    m_fdSCK  = gpioOpen(m_pinSCK);
    m_fdMOSI = gpioOpen(m_pinMOSI);
    m_fdMISO = gpioOpen(m_pinMISO);

    if (m_fdCS < 0 || m_fdSCK < 0 || m_fdMOSI < 0 || m_fdMISO < 0) {
        qDebug() << "[RC522] GPIO Error. Run unlock_leds.sh first!";
        return false;
    }

    gpioWriteFast(m_fdCS, 1);
    gpioWriteFast(m_fdSCK, 0);
    resetHardware();

    // 初始化序列
    writeReg(CommandReg, PCD_RESETPHASE);
    QThread::msleep(10);
    writeReg(TModeReg, 0x8D);
    writeReg(TPrescalerReg, 0x3E);
    writeReg(TReloadRegL, 30);
    writeReg(TReloadRegH, 0);
    writeReg(TxASKReg, 0x40);
    writeReg(ModeReg, 0x3D);
    antennaOn();

    return true;
}

// === 核心降速区 ===
uint8_t RC522::spiTransferByte(uint8_t data) {
    uint8_t rx = 0;
    for (int i = 0; i < 8; i++) {
        gpioWriteFast(m_fdMOSI, (data & 0x80) ? 1 : 0);
        data <<= 1;
        gpioWriteFast(m_fdSCK, 1);
        usleep(200); // 【重要】LED负载大，必须给足时间让电平稳定
        rx <<= 1;
        if (gpioReadFast(m_fdMISO)) rx |= 1;
        gpioWriteFast(m_fdSCK, 0);
        usleep(200);
    }
    return rx;
}

void RC522::writeReg(uint8_t reg, uint8_t value) {
    uint8_t addr = (reg << 1) & 0x7E;
    gpioWriteFast(m_fdCS, 0);
    spiTransferByte(addr);
    spiTransferByte(value);
    gpioWriteFast(m_fdCS, 1);
}

uint8_t RC522::readReg(uint8_t reg) {
    uint8_t addr = ((reg << 1) & 0x7E) | 0x80;
    gpioWriteFast(m_fdCS, 0);
    spiTransferByte(addr);
    uint8_t val = spiTransferByte(0x00);
    gpioWriteFast(m_fdCS, 1);
    return val;
}

void RC522::resetHardware() {
    if (m_fdRST < 0) { QThread::msleep(50); return; }
    gpioWriteFast(m_fdRST, 1); QThread::msleep(10);
    gpioWriteFast(m_fdRST, 0); QThread::msleep(10);
    gpioWriteFast(m_fdRST, 1); QThread::msleep(50);
}

void RC522::antennaOn() {
    uint8_t i = readReg(TxControlReg);
    if (!(i & 0x03)) writeReg(TxControlReg, readReg(TxControlReg) | 0x03);
}

bool RC522::pcdRequest(uint8_t reqMode, uint8_t *tagType) {
    writeReg(BitFramingReg, 0x07);
    tagType[0] = reqMode;
    writeReg(CommandReg, PCD_TRANSCEIVE);
    writeReg(FIFOLevelReg, 0x80);
    writeReg(FIFODataReg, reqMode);
    writeReg(CommandReg, PCD_TRANSCEIVE);
    writeReg(BitFramingReg, readReg(BitFramingReg) | 0x80);
    QThread::msleep(10);
    if ((readReg(FIFOLevelReg)) > 0) return true;
    return false;
}

bool RC522::pcdAnticoll(QByteArray &uid) {
    writeReg(BitFramingReg, 0x00);
    uint8_t cmd[2] = {PICC_ANTICOLL, 0x20};
    writeReg(CommandReg, PCD_TRANSCEIVE);
    writeReg(FIFOLevelReg, 0x80);
    writeReg(FIFODataReg, cmd[0]);
    writeReg(FIFODataReg, cmd[1]);
    writeReg(CommandReg, PCD_TRANSCEIVE);
    writeReg(BitFramingReg, readReg(BitFramingReg) | 0x80);
    QThread::msleep(10);
    if (readReg(FIFOLevelReg) >= 5) {
        for(int i=0; i<5; i++) uid.append((char)readReg(FIFODataReg));
        return true;
    }
    return false;
}

QString RC522::readCardId() {
    QMutexLocker locker(&m_mutex);
    uint8_t tagType[2];
    if (!pcdRequest(PICC_REQIDL, tagType)) return "";
    QByteArray uid;
    if (pcdAnticoll(uid)) return QString(uid.toHex());
    return "";
}

// Sysfs GPIO
void RC522::gpioExport(int pin) {
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd >= 0) { char buf[16]; int len = sprintf(buf, "%d", pin); write(fd, buf, len); close(fd); }
}
void RC522::gpioSetDir(int pin, bool output) {
    char path[64]; sprintf(path, "/sys/class/gpio/gpio%d/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) { write(fd, output?"out":"in", output?3:2); close(fd); }
}
int RC522::gpioOpen(int pin) {
    char path[64]; sprintf(path, "/sys/class/gpio/gpio%d/value", pin);
    return open(path, O_RDWR);
}
void RC522::gpioWriteFast(int fd, int val) {
    if (fd >= 0) write(fd, val?"1":"0", 1);
}
int RC522::gpioReadFast(int fd) {
    if (fd < 0) return 0;
    char buf[2]={0}; lseek(fd, 0, SEEK_SET); read(fd, buf, 1);
    return (buf[0]=='1')?1:0;
}
