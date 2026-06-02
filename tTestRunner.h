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
    //tTimeout TestTimeout = tTimeout(0, false); // TODO: remove
    QTimer WorkerWaiterTimer;
    QEventLoop WorkerWaiterLoop;
    
    tTestInfo LastTestInfo; // updated in the slot

    //private double TestTimeout;
    int TimeoutNormalSec = 10; // How many seconds wait for standard test
    int TimeoutSoftInterruptSec = 5; // How much to wait for graceful thread shutdown by Interrupt test flag
    int TimeoutHardInterruptSec = 1; // How much to wait for hard thread shutdown by terminate()
    int TimeoutWaitSec = 1;
    //QProgressBar* TestProgress = nullptr;
    tTestProcedure* TP = nullptr;
    tLogger& Log;
    ///bool IsAcceptSignals = false;
    QThread TestThread;
    QTreeWidgetItem* TestTree = nullptr;
    bool IsRunningTest = false;
    //void ProcessSignals();
    void AbortTest();

public:
    tReport* CurrentTestReport = nullptr;
    bool InterruptFlag = false;
    bool CancelTestsFlag = false;
    tTestRunner(tLogger& log);
    ~tTestRunner();
    void SetCurTestTree(QTreeWidgetItem* testTree);
    //void FlushSignals();
    void RunTest();
    void SetTP(tTestProcedure* tp) { TP = tp; }
    bool GetRunningTest() const { return IsRunningTest; }
    void CancelTests();
public slots:
    // Settings
    void slotSetTestInfo(tTestInfo ti);   // Procedure -> Runner <tTestInfo>
    void slotSetTestProgress(double val); // Procedure -> Runner <double>
    void slotSetTestTimeout(double toSec); // Procedure -> Runner <double>
    void slotAddTestDetails(const QString& details); // Procedure -> Runner <QString>
 signals:
    void sigRunTest();
    void sigFinishTest(); // Runner -> Procedure Move TP back to main thread
    void sigInterruptTest(); // Runner -> Procedure <void>

    void sigSetProgressBar(int); // Runner->App->ProgressBar
    void sigTestFinished(tTestStatus status); // Test finished. Can be emitted a few times. Runner->App Runner->Runner waiting loop
};
