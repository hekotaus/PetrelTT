#include "tTestRunner.h"
#include "tPetrelProject.h"

tTestRunner::tTestRunner(tLogger& log)
    : Log(log) {
    //TestProgress->setValue(0);
    //emit sigSetProgressBar(0); // not conneced yet, makes no sense

    //connect(this, &tTestRunner::sigMessageResult, &WorkerWaiterLoop, &QEventLoop::quit);
    connect(&WorkerWaiterTimer, &QTimer::timeout, &WorkerWaiterLoop, &QEventLoop::quit);
    connect(this, &tTestRunner::sigTestFinished, &WorkerWaiterLoop, &QEventLoop::quit);
}

tTestRunner::~tTestRunner() {
    disconnect(&WorkerWaiterTimer, &QTimer::timeout, &WorkerWaiterLoop, &QEventLoop::quit);
    disconnect(this, &tTestRunner::sigTestFinished, &WorkerWaiterLoop, &QEventLoop::quit);
}

//private 
void tTestRunner::AbortTest() {
    // hard interrupt. May cause problems, as it does not release any resourses
    TestThread.terminate();
    if (TestThread.wait(QDeadlineTimer(TimeoutHardInterruptSec))) {
        CurrentTestReport->SetStatus(tTestStatus::TestError, "Hard interrupted");
    } else {
        CurrentTestReport->SetStatus(tTestStatus::TestError, "ERROR: Failed to hard interrupt!");
        Log.LogErrorMessage("Failed to abort timed out test thread");
    }
}

void tTestRunner::CancelTests() {
    InterruptFlag = true;
    CancelTestsFlag = true;
    emit sigInterruptTest(); // To TP
    CurrentTestReport->AddDetails("User interrupt");
}

//public 
void tTestRunner::RunTest() {
    IsRunningTest = true;
    InterruptFlag = false;
    QString testName = CurrentTestReport->GetName(); // Group name for Manual or test name for Auto
    emit sigSetProgressBar(0);
    ///TestTimeout.Restart(10.0);
    qDebug() << "CurrentTestReport status" << CurrentTestReport->GetStatusMessage();
    //TestProcedure.ResetTest(testName, "Auto"); // Erase old specs, remove old results
    TP->SetupTest(testName); // This gets a new specs structure
    LastTestInfo.Status = tTestStatus::Pending;
    LastTestInfo.Details = "";
    LastTestInfo.Result.Clear();
    // New test cycle
    // 1. Run test thread
    TP->moveToThread(&TestThread);
    qDebug() << "Test Runner is starting the test thread...";
    TestThread.start(); // Need to do it every time, as we can interrupt the test with exit()/terminate()
    qDebug() << "Test Runner is signaling run test...";
    emit sigRunTest();
    qDebug() << "Test Runner is entering waiting loop...";
    // Wait untils test TESTED or SKIPPED or TEST ERROR or timeout
    
    // 2. Wait in the loop
    // Connect Stop button activated sigInterruptTest
    connect(this, &tTestRunner::sigInterruptTest, &WorkerWaiterLoop, &QEventLoop::quit);
    WorkerWaiterTimer.setSingleShot(true);
    WorkerWaiterTimer.start(TimeoutNormalSec * 1000);
    WorkerWaiterLoop.exec(); // Interrupts by timer or test status
    // Disconnect Stop button activated sigInterruptTest
    disconnect(this, &tTestRunner::sigInterruptTest, &WorkerWaiterLoop, &QEventLoop::quit);
    qDebug() << "exit test waiting loop 1";

    bool timeout = !WorkerWaiterTimer.isActive();
    WorkerWaiterTimer.stop();
    qDebug() << "timeout 1 = " << timeout;

    // 3. If timeout occured, issue Soft interrupt
    if (timeout || InterruptFlag) { // TODO: add || IsInterrupt (set by StopButton)
        CurrentTestReport->AddDetails("Test timed out. Soft interrupt");
        emit sigInterruptTest();
        WorkerWaiterTimer.start(TimeoutSoftInterruptSec * 1000);
        // 4. Run 2nd loop with Soft exit waiting
        WorkerWaiterLoop.exec(); // Interrupts by timer or test status
        qDebug() << "exit test waiting loop 2";
        timeout = !WorkerWaiterTimer.isActive();
        qDebug() << "timeout 2 = " << timeout;
    }

    // 5. If Not finished HardInterrupt else Finish normally
    if (timeout) { // Everything is bad. Terminate thread!
        CurrentTestReport->AddDetails("Soft interrupt failed. Hard interrupt");
        AbortTest(); // Hard interrupt
        LastTestInfo.Status = CurrentTestReport->GetStatus();
        // Move to thread ???
        // ATM, this is very bad. Must be avoided!
        qDebug() << "tTestRunner::RunTest()::TP->moveToThread(main)";
        //emit sigFinishTest();
        //TP->moveToThread(QApplication::instance()->thread());
    } else { // Just softly wait end of thread
        TestThread.exit();
        TestThread.wait(TimeoutWaitSec * 1000);
    }

    emit sigSetProgressBar(0);
    LastTestInfo.Status = CurrentTestReport->GetStatus();
    if (LastTestInfo.Status == tTestStatus::Testing) { // Check value range
        CurrentTestReport->SetStatus(tTestStatus::TestError, "Test not finished (result has not been set)");
    } else if (LastTestInfo.Status == tTestStatus::Tested) { // Check value range
        tTestSpec* spec = TP->GetTestSpecs()->GetSpec(testName);
        if ((spec != nullptr) && (spec->GetIsTest())) {
            LastTestInfo.Status = spec->TestValue(LastTestInfo.Result);
        }
    }

    if (CurrentTestReport->GetStatus() != LastTestInfo.Status)
        CurrentTestReport->SetStatus(LastTestInfo.Status);
    IsRunningTest = false;
    emit sigTestFinished(LastTestInfo.Status); // for App
}

//public 
void tTestRunner::SetCurTestTree(QTreeWidgetItem* testTree) {
    TestTree = testTree;
}

void tTestRunner::slotSetTestInfo(tTestInfo ti) {   // Procedure -> Runner <tTestInfo>
    Log.LogSystemMessage("SetTestStatus");
    LastTestInfo = ti;
    CurrentTestReport->TestStatusChanged(ti);

    if (!tReport::IsNotFinished(ti.Status)) {
        emit sigTestFinished(ti.Status);
    }

    // Trying:
// if changing status to Testing, highlight it in the Test Tree
// if changing from Testing un-highlight
// OR change tree font color to TestReport font color
    if (TestTree != nullptr) {
        // Do for the current test and all its parents
        tReport* curTestReport = CurrentTestReport;
        do {
            QString curName = curTestReport->GetName();
            
            // TestTree is set from SetCurTestTree on the GUI thread before RunTest starts => safe
            QTreeWidgetItem* curNode = tPetrelProject::SearchNode(curName.toLower(), TestTree);
            if (curNode != nullptr) {
                //curNode->setFont() .ForeColor = tReport::TestStatusColors[(int)curTest.GetStatus()];
                QColor color = curTestReport->GetTestColor();
                curNode->setForeground(0, QBrush(color));
                curTestReport = curTestReport->GetParent();
            } else {
                curTestReport = nullptr; // Node not found, exit
            }
        } while (curTestReport != nullptr);
    }
}

void tTestRunner::slotSetTestProgress(double val) { // Procedure -> Runner <double>
    //Log.LogSystemMessage("SetTestProgress");
    int value = std::min(100.0, std::max(0.0, 100.0*val));
    emit sigSetProgressBar(value);
}

void tTestRunner::slotSetTestTimeout(double toSec) { // Procedure -> Runner <double>
    Log.LogSystemMessage("SetTestTimeout");
    //TestTimeout.Restart(toSec);
    WorkerWaiterTimer.start(toSec * 1000);
}

void tTestRunner::slotAddTestDetails(const QString& details) { // Procedure -> Runner <QString>
    Log.LogSystemMessage("SetTestDetails");
    CurrentTestReport->AddDetails(details);
}
