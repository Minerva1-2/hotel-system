#ifndef RFIDTHREAD_H
#define RFIDTHREAD_H

#include <QThread>
#include "rc522.h" // 必须包含驱动头文件

class RfidThread : public QThread
{
    Q_OBJECT
public:
    explicit RfidThread(QObject *parent = nullptr);
    ~RfidThread();
    void stop();

protected:
    void run() override;

signals:
    void cardDetected(QString cardId);
    void hardwareStatus(bool isOk);

private:
    bool m_running;
    RC522 *m_driver;
};

#endif // RFIDTHREAD_H
