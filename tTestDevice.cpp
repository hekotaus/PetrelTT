#include "tTestDevice.h"
#include "tTestProcedure.h"
#include "tReport.h"

void tTestDevice::SetTestProgress(double fraction) {
    if (TP != nullptr) {
        TP->Test_SetProgress(fraction);
    }
}

tTestDevice::tTestDevice(tTestProcedure* tp) {
    TP = tp;
}

bool tTestDevice::Init(tReport* rep) {
    IsInited = true; // Needed for dummy devices
    return IsInited;
}

void tTestDevice::Done(tReport* rep) {
    IsInited = false;
}

bool tTestDevice::GetInited() const {
    return IsInited;
}