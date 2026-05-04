#pragma once
#include <QProgressBar>
#include <QTreeWidgetItem>
#include <qthread.h>
//#include "EventQueue.h"
#include "tTestProcedure.h"
#include "tTimeOut.h"

class tProgressBar; // TEMPORARY

class tTestRunner : public QObject { // interacts with TestProcedure
    Q_OBJECT
private:
    tTimeout TestTimeout = tTimeout(0, false);
    //private double TestTimeout;
    double SoftInterruptTimeout;
    //QProgressBar* TestProgress = nullptr;
    tTestProcedure* TP = nullptr;
    tLogger& Log;
    ///bool IsAcceptSignals = false;
    QThread TestThread;
    QTreeWidgetItem* TestTree = nullptr;
    //void ProcessSignals();
    void AbortTest(bool byUser);

public:
    tReport* CurrentTestReport = nullptr;
    bool InterruptFlag = false;
    tTestRunner(tLogger& log);
    void SetCurTestTree(QTreeWidgetItem* testTree);
    //void FlushSignals();
    void RunTest();
    void SetTP(tTestProcedure* tp) { TP = tp; }
public slots:
    // Settings
    void slotSetTestInfo(tTestInfo* ti);   // Procedure -> Runner <tTestInfo>
    void slotSetTestProgress(double val); // Procedure -> Runner <double>
    void slotSetTestTimeout(double toSec); // Procedure -> Runner <double>
    void slotAddTestDetails(const QString& details); // Procedure -> Runner <QString>
 signals:
    void sigRunTest();
    void sigFinishTest(); // Move TP back to main thread
    void sigInterruptTest(); // Runner -> Procedure <void>
    void sigSetProgressBar(int); // -> App->ProgressBar
    //UseSerialNumber, // Procedure -> Runner
    //
    //void signalSetTestParams(); // used to transfer params TestForm->TestRunner->TestProcedure <???>
    // Test commands
};
