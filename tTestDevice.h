#pragma once
#include <qstring.h>
#include "tReport.h"

class tTestProcedure;

class tTestDevice { // This is a base class for DUT and DPT classes
protected:
    tTestProcedure* TP = nullptr;
    void SetTestProgress(double fraction);
    bool IsInited = false;

public:
    //tTestDevice();
    tTestDevice(tTestProcedure* tp);
    virtual bool Init(tReport* rep);
    virtual void Done(tReport* rep);
    bool GetInited() const;
};

class tTestDevice_SingleComPort : public tTestDevice {
    QString sPort;

protected:
    bool SetPort(QString sport) {
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

};
