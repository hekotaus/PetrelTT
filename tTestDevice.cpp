#include "tTestDevice.h"
#include "tTestProcedure.h"

void tTestDevice::SetTestProgress(double fraction) {
    if (TP != nullptr) {
        TP->Test_SetProgress(fraction);
    }
}

tTestDevice::tTestDevice(tTestProcedure* tp) {
    TP = tp;
}

bool tTestDevice::Init() {
    //Inited = true;
    return IsInited;
}

void tTestDevice::Done() {
    IsInited = false;
}

bool tTestDevice::GetInited() const {
    return IsInited;
}