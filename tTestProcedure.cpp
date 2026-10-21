#include "tTestProcedure.h"
#include "common/VedroLibTools.h"
#include<qthread.h>
#include <exception>

tTestProcedure::tTestProcedure(
    QString name, QString dutName, QString dptName,
    tPetrelProjectConfig& cfg)
    : TestProcName(name)
    //, DutName(dutName)
    , DptName(dptName)
    , Cfg(cfg)
{
}

tTestProcedure::~tTestProcedure() {
    DeletePanCfg();
}

void tTestProcedure::DeletePanCfg() {
    if (PanDutCfg != nullptr) delete PanDutCfg;
    PanDutCfg = nullptr;

    if (PanDptCfg != nullptr) delete PanDptCfg;
    PanDptCfg = nullptr;
}

tTestProcedure* tTestProcedure::Test_CancelTesting(QString details) {
    Test_AddDetails("Testing cancelled! No further tests will be run.");
    if (!details.isEmpty()) Test_AddDetails(details);
    ///CancelTestingFlag = true;
    //TODO: implement signaling to TestRunner!
    emit sigCancelTesting();
    return this;
}

tTestProcedure* tTestProcedure::Test_CancelSubtests(QString details) {
    Test_AddDetails("Testing cancelled! No further tests will be run.");
    if (!details.isEmpty()) Test_AddDetails(details);
    //CancelTestingFlag = true;
    //TODO: implement signaling to TestRunner!
    emit sigCancelSubtests();
    return this;
}

///void tTestProcedure::ResetCancelTestingFlag() { // This is called before starting test session
///    CancelTestingFlag = false;
///}

///bool tTestProcedure::GetTestResultInternal() const {
///    if (CurrentTest.Info.Status == tTestStatus::Tested)
///        return CurrentTest.Info.Result. ResultInternal;
///    else
///        return false;
///}

// PENDING -+-> TESTING -+-> TESTED
//          +------------+-> TEST ERROR
//          +------------+-> SKIPPED
void tTestProcedure::Test_DelayAndSetProgress(int delayMs) {
    int delMs = 50;
    
    int n = delayMs / delMs;
    if (delayMs < 0) return;
    if (n == 0) {
        QThread::msleep(delayMs);
        return;
    }
    for (int i = 0; i < n; i++) {
        QThread::msleep(delMs);
        Test_SetProgress(double(i) / n);
        if (InterruptFlag) return;
    }
}

bool tTestProcedure::AssignTestFunction(const QString& testName, tTestDelegate testProc) {
    if (cbAssignTestFunction != nullptr)
        return cbAssignTestFunction(testName, testProc);
    else return false;
}

bool tTestProcedure::AssignManualDialog(QString groupName, tTestDialog* pDialog) { //Scan Specs tree until name found.Return ManualTestDialog
    if (cbAssignManualDialog != nullptr)
        return cbAssignManualDialog(groupName, pDialog);
    else return false;
}

tTestDialog* tTestProcedure::GetManualDialog(const QString& groupName) {
    if (cbGetManualDialog != nullptr)
        return cbGetManualDialog(groupName);
    else return nullptr;
}

void tTestProcedure::Test_SetTimeout(double timeoutSec) {
    emit sigSetTestTimeout(timeoutSec);
}

tTestProcedure* tTestProcedure::Test_AddDetails(const QString& details) {
    emit sigAddTestDetails(details);
    return this;
}

void tTestProcedure::Test_SetProgress(double fraction) {
    emit sigSetTestProgress(fraction);
}

tTestProcedure* tTestProcedure::Test_Skip(const QString& details) {
    CurrentTest.Info.Status = tTestStatus::Skipped; 
    if (details != "")
        Test_AddDetails(details);
    return this;
}

tTestProcedure* tTestProcedure::Test_Interrupt(const QString& details) {
    CurrentTest.Info.Status = tTestStatus::Interrupted;
    if (details != "")
        Test_AddDetails(details);
    return this;
}

tTestProcedure* tTestProcedure::Test_Error(const QString& details) {
    CurrentTest.Info.Status = tTestStatus::TestError;
    if (details != "")
        Test_AddDetails(details);
    return this;
}

void tTestProcedure::Test_StartManualTest(const QString& testName) {
    emit sigStartManualTest(testName);
}

void tTestProcedure::Test_ShowMessage(QMessageBox* msgBox) {
    emit sigShowMessage(msgBox);
}

bool tTestProcedure::Test_WaitForMessageBoxResult(int& messageBoxResult, int timeoutSec) {
    MessageBoxWaiterTimer.setSingleShot(true);

    MessageBoxWaiterTimer.start(timeoutSec * 1000);
    MessageBoxWaiterLoop->exec();

    messageBoxResult = MessageBoxResult;
    return MessageBoxWaiterTimer.isActive();
}

void tTestProcedure::Test_SetSerialNumber(const QString& sn) { // Set SN. If empty, make Shell request SN from user
    emit sigRequestSn(sn);
}

bool tTestProcedure::Test_WaitForSetSerialNumber(int& messageBoxResult, int timeoutSec) {
    MessageBoxWaiterTimer.setSingleShot(true);

    MessageBoxWaiterTimer.start(timeoutSec * 1000);
    MessageBoxWaiterLoop->exec();

    messageBoxResult = MessageBoxResult;
    return MessageBoxWaiterTimer.isActive();
}

void tTestProcedure::SetTestStatus(tTestStatus newStatus) {
    CurrentTest.Info.Status = newStatus;
    emit sigSetTestInfo(CurrentTest.Info);
}

void tTestProcedure::SetTestInfo() {
    emit sigSetTestInfo(CurrentTest.Info);
}

//public 
//void tTestProcedure::SetupManualTest(const QString& groupName, const QString& testName) {
//    //TestGroup = groupName;
//    TestName = testName;
//}

//public 
//void tTestProcedure::_SetupTest(const QString& testName) {
    //TestName = testName;
//    InterruptFlag = false;
//}

//public 
void tTestProcedure::slotRunTest(tTestStruct currentTest) {
#if 0
    if (TestName == "") {
        CurrentTest.Info.Details = "ERROR: Test name is empty!";
        return;
    }

    if (!TestProcInfo.GetTestByName(TestName, CurrentTest)) {
        CurrentTest.Info.Details = "No test procedure for '" + TestName;
        //SetTestStatus(TTestStatus.TestError);
        return;
    }
#endif
    CurrentTest = currentTest;

    ///CurrentTest = TestProcInfo->TestDict[TestName.toUpper()];
    if (CurrentTest.Info.Status != tTestStatus::Pending) return;

    ///if (!InterruptFlag && !CancelTestingFlag) {
    if (!InterruptFlag) {
        SetTestStatus(tTestStatus::Testing);
        MessageBoxWaiterLoop = new QEventLoop();
        connect(this, &tTestProcedure::sigMessageResult, MessageBoxWaiterLoop, &QEventLoop::quit);
        connect(&MessageBoxWaiterTimer, &QTimer::timeout, MessageBoxWaiterLoop, &QEventLoop::quit);

        try {
            CurrentTest.Proc(CurrentTest.Info); // Run test // TODO: check cancel flag!
        } catch (std::exception& ex) {
            Test_AddDetails("Exception occured during running test: " + QString(ex.what()));
        } catch (...) {
            Test_AddDetails("Unknown exception occured during running test"); 
        }

        if ((CurrentTest.Info.Status == tTestStatus::Testing) && (CurrentTest.Info.Result.IsValueSet()))
            CurrentTest.Info.Status = tTestStatus::Tested;

        if ((CurrentTest.Info.Status != tTestStatus::Tested) &&  // Proc() MUST set status to Tested or Skipped
            (CurrentTest.Info.Status != tTestStatus::Skipped) &&
            (CurrentTest.Info.Status != tTestStatus::Interrupted))
            CurrentTest.Info.Status = tTestStatus::TestError;
    }

    ///if (InterruptFlag || CancelTestingFlag) {
    if (InterruptFlag) {
        CurrentTest.Info.Status = tTestStatus::Interrupted;
    }
    SetTestInfo(); // emit signal once to avoid unwanted switching. Do we really need it in the very end? Yes. To signal end of work
//    TestName = "";
    disconnect(this, &tTestProcedure::sigMessageResult, MessageBoxWaiterLoop, &QEventLoop::quit);
    disconnect(&MessageBoxWaiterTimer, &QTimer::timeout, MessageBoxWaiterLoop, &QEventLoop::quit);
    delete MessageBoxWaiterLoop;
    qDebug() << "tTestProcedure::slotRunTest()::moveToThread(main)";
    moveToThread(QApplication::instance()->thread());
}

//void tTestProcedure::slotFinishTest() { // Move TP back to main thread
//    qDebug() << "tTestProcedure::slotFinishTest()::moveToThread(main)";
//    moveToThread(QApplication::instance()->thread()); // Fix Current thread is not the object's thread. Cannot move to target thread
//}

void tTestProcedure::slotMessageResult(int result) { // From UI
    MessageBoxResult = result;
    emit sigMessageResult();
}

void tTestProcedure::slotInterruptTest() { // Runner -> Procedure <void>
    InterruptFlag = true; // soft interrupt
}
//700