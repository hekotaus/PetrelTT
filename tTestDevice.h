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
class tTestDevice_SingleComPort : public tTestDevice {
    QString sPort;
    QSerialPort Port;
    static const size_t INBUF_LEN = 0x100;
    static const size_t OUTBUF_LEN = 0x100;
    QByteArray InBuf = QByteArray(INBUF_LEN, 0);
    QByteArray OutBuf = QByteArray(OUTBUF_LEN, 0);
protected:
    bool OpenPort(QString sport) {
        sPort = sport;
        //QSerialPortInfo spi = QSerialPortInfo(sPort);;
        bool bRes = false;

        Port.setPortName(sport);
        bRes = Port.setBaudRate(QSerialPort::BaudRate::Baud115200);
        if (!bRes) { qDebug() << "Failed to set COM port baud rate"; return false; }

        bRes = Port.setDataBits(QSerialPort::Data8);
        if (!bRes) { qDebug() << "Failed to set COM port data bits"; return false; }

        bRes = Port.setParity(QSerialPort::NoParity);
        if (!bRes) { qDebug() << "Failed to set COM port parity"; return false; }

        bRes = Port.setStopBits(QSerialPort::OneStop);
        if (!bRes) { qDebug() << "Failed to set COM port stop bits"; return false; }

        bRes = Port.setFlowControl(QSerialPort::NoFlowControl);
        if (!bRes) { qDebug() << "Failed to set COM port flow control"; return false; }


        //Port.setReadBufferSize(2048);
        //if (!bRes) { qDebug() << "Failed to set COM port read buffer size"; return false; }

        //Port.setWriteBufferSize(2048);
        //if (!bRes) { qDebug() << "Failed to set COM port write buffer size"; return false; }

        bRes = Port.open(QIODeviceBase::ReadWrite);
        if (!bRes) {
            qDebug() << "Failed to open COM port";
            qDebug() << Port.errorString();
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

    bool Init(tReport* rep) {
        return Init(rep, "COM4");
    }

    bool Init(tReport* rep, QString sport) {
        bool res = OpenPort(sport);
        return res;
    }

    void Done(tReport* rep) override {
        // TODO: add closing port code
        Port.close();
    }

    QString ReadLine() {
        //if (!Port.canReadLine())
        //    return "Can't read line";

        //Port.readLineInto(&InBuf, INBUF_LEN);
        //int64_t len = Port.read(InBuf.data(), INBUF_LEN);
        QByteArray inbuf = Port.readAll();
        int64_t len = inbuf.size();
        qDebug() << "Respnse len =" << len;
        QString res = QString(InBuf);
        return res;
    }

    void WriteLine(QString line) {
        //if (!line.endsWith("\n")) 
            line = line + "\r\n";
        OutBuf = line.toLatin1();
        int64_t len = Port.write(OutBuf);
        qDebug() << "Sent len =" << len;
    }

    //void SetSerialNumber(QString sn) {
    //    SerialNumber = sn;
    //}

};
