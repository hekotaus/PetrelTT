#pragma once

struct tTestInfo {
    tTestStatus Status = tTestStatus::None;
    tTestResult Result;
    //bool ResultInternal = false;
    QString Details;
};

