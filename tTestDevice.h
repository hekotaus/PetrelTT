#pragma once
#include <qstring.h>
#include "tReport.h"

class tTestProcedure;

class tTestDevice { // This is a base class for DUT and DPT classes
protected:
    tTestProcedure* TP = nullptr;
    //void SetTestProgress(double fraction);
    bool IsInited = false;
    QString SerialNumber = "";
public:
    //tTestDevice();
    tTestDevice(tTestProcedure* tp) : TP(tp) { }
    virtual bool Init(tReport* rep) {
        IsInited = true; // Needed for dummy devices
        return IsInited;
    }

    virtual void Done(tReport* rep) {
        IsInited = false;
    }

    bool GetInited() const { return IsInited; };
};

#include <qserialport.h>
#include <qthread.h>
#include <qdeadlinetimer.h>
#include <memory>
// Blocking line I/O over one COM port.
// QSerialPort must be used, and closed, only in the thread that opened it: close() cancels
// the pending overlapped read with CancelIo(), which only affects I/O issued by the calling
// thread. Closing it elsewhere leaves the read pending, and the driver later completes it
// into the freed port. Init/Done run in the GUI thread, tests in TestThread (which is stopped
// after every test), so the port lives in its own IoThread and every port call runs there.
// All I/O waits with waitFor*(), as there are no port events to handle.
class tTestDevice_SingleComPort : public tTestDevice {
    QString sPort;
    QThread IoThread;
    QObject IoContext; // Lives in IoThread, used to run code there. Destroyed before IoThread.
    std::unique_ptr<QSerialPort> Port; // Created, used and deleted in IoThread only
    static const int TIMEOUT_MS = 500;

    // Runs func in IoThread and waits for it
    template <typename F>
    void RunInIoThread(F func) {
        if (QThread::currentThread() == &IoThread) { func(); return; }
        if (!IoThread.isRunning()) IoThread.start();
        QMetaObject::invokeMethod(&IoContext, func, Qt::BlockingQueuedConnection);
    }

    void ClosePort() {
        if (!IoThread.isRunning()) return; // Port has never been opened
        RunInIoThread([this] { Port.reset(); }); // ~QSerialPort closes it
    }

    // IoThread only
    bool EnsurePort() {
        if (Port && Port->isOpen()) return true;
        Port.reset();
        return OpenPort();
    }

    // IoThread only
    bool OpenPort() {

        sPort = "COM4"; // TODO: debug only!

        if (sPort.isEmpty()) { qDebug() << "COM port is not set"; return false; }
        Port = std::make_unique<QSerialPort>();
        bool bRes = false;

        Port->setPortName(sPort);
        bRes = Port->setBaudRate(QSerialPort::BaudRate::Baud115200);
        if (!bRes) { qDebug() << "Failed to set COM port baud rate"; return false; }

        bRes = Port->setDataBits(QSerialPort::Data8);
        if (!bRes) { qDebug() << "Failed to set COM port data bits"; return false; }

        bRes = Port->setParity(QSerialPort::NoParity);
        if (!bRes) { qDebug() << "Failed to set COM port parity"; return false; }

        bRes = Port->setStopBits(QSerialPort::OneStop);
        if (!bRes) { qDebug() << "Failed to set COM port stop bits"; return false; }

        bRes = Port->setFlowControl(QSerialPort::NoFlowControl);
        if (!bRes) { qDebug() << "Failed to set COM port flow control"; return false; }

        bRes = Port->open(QIODeviceBase::ReadWrite);
        if (!bRes) {
            qDebug() << "Failed to open COM port" << sPort << Port->errorString();
            Port.reset();
            return false;
        }

        return true;
    }

public:
    tTestDevice_SingleComPort(tTestProcedure* tp, QString sport)
        : tTestDevice(tp)
        , sPort(sport)
    {
        IoContext.moveToThread(&IoThread);
    }

    ~tTestDevice_SingleComPort() {
        ClosePort();
        IoThread.quit();
        IoThread.wait();
    }

    void SetPortName(const QString& sport) {
        if (sport == sPort) return;
        ClosePort();
        sPort = sport;
    }

    QString GetPortName() const { return sPort; }

    bool Init(tReport* rep) override {
        bool res = false;
        RunInIoThread([this, &res] { Port.reset(); res = OpenPort(); });
        IsInited = res;
        return IsInited;
    }

    bool Init(tReport* rep, QString sport) {
        SetPortName(sport);
        return tTestDevice_SingleComPort::Init(rep);
    }

    void Done(tReport* rep) override {
        ClosePort();
        IsInited = false;
    }

    // Returns one line without the line end, or "" on timeout
    QString ReadLine(int timeoutMs = TIMEOUT_MS) {
        QString res = "";
        RunInIoThread([this, &res, timeoutMs] {
            if (!EnsurePort()) return;
            QDeadlineTimer deadline(timeoutMs);
            while (!Port->canReadLine()) {
                if (!Port->waitForReadyRead(int(deadline.remainingTime()))) {
                    qDebug() << "COM port read timeout" << sPort;
                    return;
                }
            }
            res = QString::fromLatin1(Port->readLine()).trimmed();
        });
        return res;
    }

    // Appends CR LF if the line has no line end. Returns when the data has been sent.
    bool WriteLine(const QString& line, int timeoutMs = TIMEOUT_MS) {
        QByteArray out = line.toLatin1();
        if (!out.endsWith('\n')) out += "\r\n";
        bool res = false;
        RunInIoThread([this, &res, &out, timeoutMs] {
            if (!EnsurePort()) return;
            if (Port->write(out) != out.size()) {
                qDebug() << "COM port write failed" << sPort << Port->errorString();
                return;
            }
            if (!Port->waitForBytesWritten(timeoutMs)) {
                qDebug() << "COM port write timeout" << sPort << Port->errorString();
                return;
            }
            res = true;
        });
        return res;
    }

    //void SetSerialNumber(QString sn) {
    //    SerialNumber = sn;
    //}

};
