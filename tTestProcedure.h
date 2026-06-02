#pragma once
#include <qstring.h>
#include <qthread.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include "logger/tLogger.h"

#include "tTestProcMailBox.h"
#include "tPanDevCfg.h"
#include "tTestSpecs.h"
#include "tTestDevice.h"
#include "tTestStatus.h"
#include "tTestInfo.h"
#include "tTestProcInfo.h"
#include "tPetrelProjectConfig.h"
#include "tTestResult.h"
#include "tTestDialog.h"

class tReport;
class tTestSpecs;
class tTestProcInfo;
class tTestDevice;
using tTestDelegate = void (*)(tTestInfo&);

enum class tTestMode {
    None,
    Auto,
    Manual,
    Service
};

class tTestProcedure : public QObject {
    Q_OBJECT

protected:
    const QString TestProcName;
    const QString DutName;
    const QString DptName;
    tLogger& Log;
    tPetrelProjectConfig& Cfg;
    tTestProcMailBox* MailBox = nullptr;
    tPanDevCfg* PanDutCfg = nullptr;
    tPanDevCfg* PanDptCfg = nullptr;

    struct tTestStruct {
        tTestInfo Info;
        tTestDelegate Proc; // Function()
    };

    bool IsValid = true;
    tTestStruct CurrentTest;
    //QString TestGroup = "";
    std::map<QString, tTestStruct> TestDict; // Call Test by Name
    std::map<QString, tTestInfo> AllTestInfo;
    tTestSpecs TestSpecs;
    
    QEventLoop* MessageBoxWaiterLoop = nullptr; // used for waiting and getting info from main
    QTimer MessageBoxWaiterTimer;
    int MessageBoxResult = 0;

    bool IsInexistingTestsWarning = true;
    std::atomic<bool> InterruptFlag = false;
    std::atomic<bool> CancelTestingFlag = false;
    QString TestName = "";

    QString TestAssignmentSourceCode(QString dut, QString group, QString specName); // Just a little help for test procedure programmer :)
    bool ValidateManualTestFuncAssignment(tReport* rep); // X3 how to call it!
    void SetTestStatus(tTestStatus newStatus);
    void SetTestInfo();

public:
    tTestResult GetResult() const { return CurrentTest.Info.Result; }
    tTestProcedure(QString name, QString dutName, QString dptName, tLogger& log, tPetrelProjectConfig& cfg);//, 
    ~tTestProcedure();

    tTestSpecs* GetTestSpecs() { return &TestSpecs; }

    // DPT and DUT
    QString GetDutName() const { return DutName; }
    QString GetDptName() const { return DptName; }
    virtual tPanDevCfg* MakePanDutCfg(QWidget* parent, int id) = 0; // need parent to avoid stupid parentless window blinking
    virtual tPanDevCfg* MakePanDptCfg(QWidget* parent, int id) = 0;
    void DeletePanCfg();
    tPanDevCfg* GetPanDutCfg() { return PanDutCfg; }
    tPanDevCfg* GetPanDptCfg() { return PanDptCfg; }
    virtual tTestDialog* GetManualTestDialog(const QString& groupName) { return TestSpecs.GetManualDialog(groupName);  };
    
    bool InitDptAndDut(tReport* rep);
    void DoneDutAndDpt(tReport* rep);
    // Basic init and done methods, can be replaced in plugin
    virtual bool InitAutoTests(tReport* rep);
    virtual bool InitManualTests(tReport* rep);
    virtual void DoneAutoTests(tReport* rep);
    virtual void DoneManualTests(tReport* rep);
    virtual void AssignTestFunctions() = 0; // to be called after loading specs

    bool GetValid() const { return IsValid; }
    void SetValid(bool st) { IsValid = st; }

    tTestDevice* pDUT = nullptr; // Generic pointer to Device Under Test
    tTestDevice* pDPT = nullptr; // Generic pointer to Device Performing Test
    bool IsInterrupted() const { return InterruptFlag; }

    void ResetCancelTestingFlag(); // This is called before starting test session
    void SetupTest(const QString& testName);
    //void SetupManualTest(const QString& groupName, const QString& testName = "Auto");
    void SetTestInfo(QString name, tTestInfo info);

    tTestInfo& GetCurrentTestInfo() { return CurrentTest.Info; }
    tTestResult GetCurrentTestResult() const { return CurrentTest.Info.Result; }

    void ClearAllTestsInfo();
    tTestResult GetTestResultByName(QString testName);
    tTestStatus GetTestStatusByName(QString testName);
    bool IsTestFunctionAssigned(QString testName);
    void ValidateAutoTestFuncAssignment(tReport* rep);

 public slots:
    void slotRunTest();
    void slotInterruptTest(); // Runner -> Procedure <void>
    //void slotFinishTest(); // Move TP back to main thread
    void slotMessageResult(int); // From UI
signals:
    void sigMessageResult(); // Received from UI and re-emitted to waiter
    void sigSetTestInfo(tTestInfo ti);   // Procedure -> Runner <tTestInfo>
    void sigSetTestProgress(double val); // Procedure -> Runner <double>
    void sigSetTestTimeout(double toSec); // Procedure -> Runner <double>
    void sigAddTestDetails(const QString& details); // Procedure -> Runner <QString>
    void sigStartManualTest(const QString& testName); // Procedure->App
    void sigShowMessage(QMessageBox* msgBox);

    // TEST API
public:
    bool AssignTestFunction(const QString& testName, tTestDelegate testProc);
    tTestDialog* FindManualDialog(QString name); //Scan Specs tree until name found.Return ManualTestDialog
    bool AssignManualDialog(QString name, tTestDialog* pDialog); //Scan Specs tree until name found.Return ManualTestDialog

    // Here all *TP results are used only for easier returning from the test
    void Test_DelayAndSetProgress(int delayMs);
    tTestProcedure* Test_CancelTesting(QString details = ""); // Call this function, if there is no sense to continue testing, e.g. the DUT is burnt during flashing firmware, etc
    tTestProcedure* Test_CancelSubtests(QString details = ""); // Call this function, if there is no sense to perform subtests, e.g. Measurement did not happen, cancel chacking all params
    void Test_SetTimeout(double timeoutSec);
    tTestProcedure* Test_AddDetails(const QString& details);
    void Test_SetProgress(double fraction); // 0.0...1.0
    tTestProcedure* Test_Skip(const QString& details = "");
    tTestProcedure* Test_Interrupt(const QString& details = "");
    tTestProcedure* Test_Error(const QString& details = "");
    void Test_StartManualTest(const QString& testName);
    void Test_ShowMessage(QMessageBox* msgBox);
    bool Test_WaitForMessageBoxResult(int& messageBoxResult, int timeoutSec);

    template <typename T>
    tTestProcedure* Test_SetResult(T resValue) {
        if (CurrentTest.Info.Status == tTestStatus::Testing) {
            CurrentTest.Info.Result.SetValue(T(resValue));
        } else {
            CurrentTest.Info.Status = tTestStatus::TestError;
            Test_AddDetails("Attempt to set value not at testing time!");
        }
        return this;
    }
};
