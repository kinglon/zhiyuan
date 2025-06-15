#ifndef BAOKAOCONTROLLER_H
#define BAOKAOCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QTimer>

class BaoKaoThread : public QThread
{
    Q_OBJECT
public:
    explicit BaoKaoThread(QObject *parent = nullptr);

private:
    virtual void run() override;

    void runInternal();

    void getScorePosRange(int& min, int& max);

signals:
    void printLog(QString log);

    void timerPrintLog(bool start, QString log);

public:
    bool m_success = false;
};

class BaoKaoController : public QObject
{
    Q_OBJECT
public:
    explicit BaoKaoController(QObject *parent = nullptr);

    void run();

private:
    void onThreadFinish();

    void onTimerPrintLog(bool start, QString log);

signals:
    void runFinish(bool success);

    void printLog(QString log);

private:
    BaoKaoThread* m_baoKaoThread = nullptr;

    QTimer* m_logTimer = nullptr;
};

#endif // BAOKAOCONTROLLER_H
