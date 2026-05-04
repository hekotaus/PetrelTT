#include "tTestRunner.h"
#include "tPetrelProject.h"

tTestRunner::tTestRunner(tLogger& log)
    : Log(log) {
    //TestProgress->setValue(0);
    emit sigSetProgressBar(0);
}

//private 
void tTestRunner::AbortTest(bool byUser) {
    // hard interrupt. May cause problems, as it does not release any resourses
    TestThread.terminate();
    if (TestThread.wait(QDeadlineTimer(500))) {
        if (byUser)
            CurrentTestReport->SetStatus(tTestStatus::Interrupted, "Hard interrupt by user");
        else
            CurrentTestReport->SetStatus(tTestStatus::TestError, "Hard interrupt auto");
    } else {
        CurrentTestReport->SetStatus(tTestStatus::TestError, "ERROR: Failed to hard interrupt!");
        Log.LogErrorMessage("Failed to abort timed out test thread");
    }
}

//public 
void tTestRunner::RunTest() {
    //InterruptFlag = false;
    QString testName = CurrentTestReport->GetName(); // Group name for Manual or test name for Auto
    //TReport rep = spec.BuildManualTestReport(Report);
    ///IsAcceptSignals = true;
    emit sigSetProgressBar(0);
    TestTimeout.Restart(10.0);

    //TestProcedure.ResetTest(testName, "Auto"); // Erase old specs, remove old results
    TP->SetupTest(testName); // This gets a new specs structure
    //TP->CurrentSpec = CurrentTestReport->GetSpec();
//    TP->SetCurrentTest();
    //tTestInfo info;
    //info.Status = CurrentTestReport->GetStatus();
    //info.Result = CurrentTestReport->GetResult();
    //info.Details = CurrentTestReport->GetDetails();
    //TP->SetCurrentTestInfo(info);

    TP->moveToThread(&TestThread);
    TestThread.start();
    emit sigRunTest();

    // Wait untils test TESTED or SKIPPED or TEST ERROR or timeout
    do {
        QApplication::processEvents();
        QThread::msleep(100);
    } while (!InterruptFlag && !TestTimeout.IsExpired() && CurrentTestReport->IsNotFinished());
    QApplication::processEvents();

    // When reached this point, test changed it's state or interrupt happened
    if (TestTimeout.IsExpired()) {
        CurrentTestReport->AddDetails("Test timeout");
        AbortTest(false); // Hard interrupt
    } else if (InterruptFlag) {
        CurrentTestReport->AddDetails("Interrupted by user");
        CurrentTestReport->AddDetails("Soft interrupt");
        SoftInterruptTimeout = 3.0;
        bool isSoftInterruptTimeout = false;
        emit sigInterruptTest();
        tTimeout softInterruptTimeout = tTimeout(SoftInterruptTimeout, true);
        while ((!softInterruptTimeout.IsExpired()) && CurrentTestReport->IsNotFinished()) {
            QApplication::processEvents();
            QThread::msleep(100);
        }
        QApplication::processEvents();
        if ((isSoftInterruptTimeout) && CurrentTestReport->IsNotFinished())
            AbortTest(true);
        if (CurrentTestReport->IsNotFinished())
            CurrentTestReport->SetStatus(tTestStatus::Interrupted, "Interrupted by User");
    } else { // Normal finish, just wait for thread peacefully
        TestThread.exit();
        TestThread.wait();
    }
    emit sigFinishTest();
    ///TP->moveToThread(QApplication::instance()->thread()); // TODO: Current thread is not the object's thread. Cannot move to target thread
    //TestProgress->setEnabled(false);
    //Application.DoEvents();
    emit sigSetProgressBar(0);

    tTestInfo& info = TP->GetCurrentTestInfo();
    ///info.Status = CurrentTestReport->GetStatus();
    ///info.Result = CurrentTestReport->GetResult();
    ///info.ResultInternal = CurrentTestReport->GetResultInternal();
    ///info.Details = CurrentTestReport->GetDetails();
    
    if (info.Status == tTestStatus::Testing) { // Check value range
        info.Details += "Test not finished (result has not been set)";
        //if (info.Result.IsValueSet())
        //    info.Status = tTestStatus::Tested;
        info.Status = tTestStatus::TestError; // Result is not set
    }

    if (info.Status == tTestStatus::Tested) { // Check value range
        //tTestSpec* spec = CurrentTestReport->GetSpec();
        tTestSpec* spec = TP->GetTestSpecs()->GetSpec(testName);
        if ((spec != nullptr) && (spec->GetIsTest())) {
            info.Status = spec->TestValue(info.Result);
        }
    }
    //TP->SetTestInfo(CurrentTestReport->GetName(), info); // TODO: needed???
    if (CurrentTestReport->GetStatus() != info.Status)
        CurrentTestReport->SetStatus(info.Status);
}

//public 
void tTestRunner::SetCurTestTree(QTreeWidgetItem* testTree) {
    TestTree = testTree;
}

void tTestRunner::slotSetTestInfo(tTestInfo* ti) {   // Procedure -> Runner <tTestInfo>
    Log.LogSystemMessage("SetTestStatus");
    CurrentTestReport->TestStatusChanged(*ti);



    // Trying:
// if changing status to Testing, highlight it in the Test Tree
// if changing from Testing un-highlight
// OR change tree font color to TestReport font color
    if (TestTree != nullptr) {
        // Do for the current test and all its parents
        tReport* curTestReport = CurrentTestReport;
        do {
            QString curName = curTestReport->GetName();
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
    TestTimeout.Restart(toSec);
}

void tTestRunner::slotAddTestDetails(const QString& details) { // Procedure -> Runner <QString>
    Log.LogSystemMessage("SetTestDetails");
    CurrentTestReport->AddDetails(details);
}

//void tTestRunner::slotSetTestParams() { // used to transfer params TestForm->TestRunner->TestProcedure <???>
//}

