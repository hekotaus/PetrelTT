#pragma once

#include <qdatetime.h>

class tTimeout {
    QDateTime Start;
    double TimeoutSec = 0;
    bool Running = false;
    bool Expired = false;
public:
    tTimeout() {
        Set(0);
        Restart();
    }
    tTimeout(double timeoutSec, bool running) {
        Set(timeoutSec);
        if (TimeoutSec <= 0) running = false;
        if (running) Restart();
    }
    void Set(double timeoutSec) {
        TimeoutSec = timeoutSec;
        Expired = false;
    }
    void Restart(double timeoutSec) {
        Set(timeoutSec);
        Restart();
    }
    void Restart() {
        Start = QDateTime::currentDateTime();
        Running = (TimeoutSec > 0);
    }
    void Stop() {
        Running = false;
    }
    bool IsExpired() {
        if (Running) {
            int dt_ms = (QDateTime::currentDateTime() - Start).count();
            Expired = (dt_ms * 0.001 > TimeoutSec);
        }
        return Expired;
    }
    bool IsRunning() {
        return Running;
    }
    double PercentagePassed() {
        if (TimeoutSec < 0.001) return 0;
        int dt_ms = (QDateTime::currentDateTime() - Start).count();
        return 100.0 * dt_ms * 0.001 / TimeoutSec;
    }

};
