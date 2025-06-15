#ifndef BAOKAOCONTROLLER_H
#define BAOKAOCONTROLLER_H

#include <QObject>
#include <QThread>

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

signals:
    void runFinish(bool success);

    void printLog(QString log);

private:
    BaoKaoThread* m_baoKaoThread = nullptr;
};

#endif // BAOKAOCONTROLLER_H
