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
    bool SetPort(QString sport) {
        sPort = sport;
        //QSerialPortInfo spi = QSerialPortInfo(sPort);;
        bool bRes = false;

        Port.setPortName(sport);
        bRes = Port.setBaudRate(QSerialPort::BaudRate::Baud115200);
        if (!bRes) { qDebug() << "Failed to set COM port baud rate"; return false; }

        Port.setDataBits(QSerialPort::Data8);
        if (!bRes) { qDebug() << "Failed to set COM port data bits"; return false; }

        Port.setParity(QSerialPort::NoParity);
        if (!bRes) { qDebug() << "Failed to set COM port parity"; return false; }

        Port.setStopBits(QSerialPort::OneStop);
        if (!bRes) { qDebug() << "Failed to set COM port stop bits"; return false; }

        Port.setReadBufferSize(2048);
        if (!bRes) { qDebug() << "Failed to set COM port read buffer sizew"; return false; }

        Port.setWriteBufferSize(2048);
        if (!bRes) { qDebug() << "Failed to set COM port write buffer sizew"; return false; }

        return true;
    }

public:
    tTestDevice_SingleComPort(tTestProcedure* tp, QString sport)
        : tTestDevice(tp)
        , sPort(sport)
    {
    }

    bool Init(tReport* rep) {
        // TODO: add opening port code
        return true;
    }

    bool Init(tReport* rep, QString sport) {
        bool res = true;
        return SetPort(sport) && Init(rep);
    }

    void Done(tReport* rep) override {
        // TODO: add closing port code
    }

    QString ReadLine() {
        if (!Port.canReadLine()) return "";

        Port.readLineInto(&InBuf, INBUF_LEN);
        QString res = QString(InBuf);
        return res;
    }

    void WriteLine(const QString& line) {
        OutBuf = line.toLatin1();
        Port.write(OutBuf);
    }

    //void SetSerialNumber(QString sn) {
    //    SerialNumber = sn;
    //}

};
