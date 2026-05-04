#pragma once
#include <qstring.h>

class tTestProcedure;

class tTestDevice { // This is a base class for DUT and DPT classes
protected:
    tTestProcedure* TP = nullptr;
    void SetTestProgress(double fraction);
    bool IsInited = false;

public:
    //tTestDevice();
    tTestDevice(tTestProcedure* tp);
    virtual bool Init();
    virtual void Done();
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

    bool Init() {
        // TODO: add opening port code
        return true;
    }

    bool Init(QString sport) {
        bool res = true;
        return SetPort(sport) && Init();
    }

    void Done() override {
        // TODO: add closing port code
    }

};
