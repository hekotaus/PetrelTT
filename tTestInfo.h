#pragma once
#include "tTestStatus.h"
#include "tTestResult.h"

struct tTestInfo {
    tTestStatus Status = tTestStatus::None;
    tTestResult Result;
    //bool ResultInternal = false;
    QString Details;
};

