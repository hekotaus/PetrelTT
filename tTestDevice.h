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
// QSerialPort only queues data in write() and sends it from its own thread's event loop,
// and it must not be used from any other thread. Init/Done run in the GUI thread while
// tests run in TestThread, so the port is (re)opened in whichever thread uses it,
// and all I/O waits with waitFor*() instead of relying on an event loop.
class tTestDevice_SingleComPort : public tTestDevice {
    QString sPort;
    std::unique_ptr<QSerialPort> Port;
    static const int TIMEOUT_MS = 500;

    void ClosePort() {
        if (!Port) return;
        QSerialPort* port = Port.release();
        QThread* owner = port->thread();
        if ((owner == QThread::currentThread()) || !owner->isRunning()) {
            delete port; // closes it
        } else {
            // Must be closed in its own thread; wait, so the port is free to be reopened here
            QMetaObject::invokeMethod(port, [port] { delete port; }, Qt::BlockingQueuedConnection);
        }
    }

    bool EnsurePort() {
        if (Port && (Port->thread() == QThread::currentThread()) && Port->isOpen()) return true;
        ClosePort();
        return OpenPort();
    }

protected:
    bool OpenPort() {
        if (sPort.isEmpty()) { qDebug() << "COM port is not set"; return false; }
        Port = std::make_unique<QSerialPort>(); // Owned by the current thread
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
    }

    ~tTestDevice_SingleComPort() {
        ClosePort();
    }

    void SetPortName(const QString& sport) {
        if (sport == sPort) return;
        ClosePort();
        sPort = sport;
    }

    QString GetPortName() const { return sPort; }

    bool Init(tReport* rep) override {
        ClosePort();
        IsInited = OpenPort();
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
        if (!EnsurePort()) return "";
        QDeadlineTimer deadline(timeoutMs);
        while (!Port->canReadLine()) {
            if (!Port->waitForReadyRead(int(deadline.remainingTime()))) {
                qDebug() << "COM port read timeout" << sPort;
                return "";
            }
        }
        return QString::fromLatin1(Port->readLine()).trimmed();
    }

    // Appends CR LF if the line has no line end. Returns when the data has been sent.
    bool WriteLine(const QString& line, int timeoutMs = TIMEOUT_MS) {
        if (!EnsurePort()) return false;
        QByteArray out = line.toLatin1();
        if (!out.endsWith('\n')) out += "\r\n";
        if (Port->write(out) != out.size()) {
            qDebug() << "COM port write failed" << sPort << Port->errorString();
            return false;
        }
        if (!Port->waitForBytesWritten(timeoutMs)) {
            qDebug() << "COM port write timeout" << sPort << Port->errorString();
            return false;
        }
        return true;
    }

    //void SetSerialNumber(QString sn) {
    //    SerialNumber = sn;
    //}

};
