#pragma once
#include <qstring.h>
#include <qthread.h>

#include "logger/tLogger.h"

#include "tTestProcMailBox.h"
#include "tPanDevCfg.h"
#include "tTestSpecs.h"
#include "tTestDevice.h"
#include "tReport.h"
#include "tTestStatus.h"
#include "tTestInfo.h"
#include "tTestProcInfo.h"
#include "tPetrelProjectConfig.h"
#include "tTestResult.h"
#include "tTestDialog.h"

class tReport;
class tTestSpecs;
class tTestProcInfo;
//using tTestDelegate = bool (*)(tTestInfo&);
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
    QString TestGroup = "";
    std::map<QString, tTestStruct> TestDict; // Call Test by Name
    std::map<QString, tTestInfo> AllTestInfo;
    tTestSpecs TestSpecs;
    
    //tTestMode TestMode = tTestMode::None;
    bool IsInexistingTestsWarning = true;
    bool InterruptFlag = false;
    bool CancelTestingFlag = false;
    QString TestName = "";

    bool AssignTestFunction(const QString &testName, tTestDelegate testProc);
    QString TestAssignmentSourceCode(QString dut, QString group, QString specName); // Just a little help for test procedure programmer :)
    QString TestProcedureSourceCode(QString dut, QString group, QString specName); // Just a little help for test procedure programmer :)
    QString TestMailBoxSourceCode(QString dut, QString group, QString specName); // Just a little help for test procedure programmer :)
    bool ValidateManualTestFuncAssignment(tReport* rep); // X3 how to call it!
    void SetTestStatus(tTestStatus newStatus);
    void SetTestInfo();
    ///virtual void BuildCustomSpecTree(QString groupName, QString testName, tTestSpec specUnit);

public:
    //tTestSpec* CurrentSpec = nullptr; // todo: move to protected
    tTestResult GetResult() const { return CurrentTest.Info.Result; }
    tTestProcedure(QString name, QString dutName, QString dptName, tLogger& log, tPetrelProjectConfig& cfg);//, 
    // tTestSpecs manualTestSpecs, tTestSpecs autoTestSpecs, tTestSpecs serviceTestSpecs);
    ~tTestProcedure();// {}

    tTestSpecs* GetTestSpecs() { return &TestSpecs; }

    // DPT and DUT
    QString GetDutName() const { return DutName; }
    QString GetDptName() const { return DptName; }
    virtual tPanDevCfg* MakePanDutCfg(QWidget* parent, int id) = 0; // need parent to avoid stupid parentless window blinking
    virtual tPanDevCfg* MakePanDptCfg(QWidget* parent, int id) = 0;
    void DeletePanCfg();
    tPanDevCfg* GetPanDutCfg() { return PanDutCfg; }
    tPanDevCfg* GetPanDptCfg() { return PanDptCfg; }
    virtual tTestDialog* GetManualTestDialog(const QString& groupName) = 0;
    virtual bool InitAutoTests(QString& details);
    virtual bool InitManualTest() = 0;
    virtual void DoneAutoTests();
    virtual void DoneManualTest() = 0;
    virtual void AssignTestFunctions() = 0; // to be called after loading specs

    //public SerialNumbers SN; // Moved to TP
    bool GetValid() const { return IsValid; }
    void SetValid(bool st) { IsValid = st; }
    //tTestProcInfo TestProcInfo;

    ///delegate TTestInfo TestDelegate();
    tTestDevice* pDUT = nullptr; // Generic pointer to Device Under Test
    tTestDevice* pDPT = nullptr; // Generic pointer to Device Performing Test
    //tTestMode GetTestMode() const { return TestMode; }
    //void SetTestMode(tTestMode mode) { TestMode = mode; }
    bool IsInterrupted() const { return InterruptFlag; }

    void ResetCancelTestingFlag(); // This is called before starting test session
    void ResetTest(QString groupName, QString testName = "Auto");
    void SetupTest(QString testName);
///    tTestForm FindManualDialog(QString name);
    void SetTestInfo(QString name, tTestInfo info);

    tTestInfo& GetCurrentTestInfo() { return CurrentTest.Info; }

    void ClearAllTestsInfo();
    tTestResult GetTestResultByName(QString testName);
    tTestStatus GetTestStatusByName(QString testName);
    bool IsTestFunctionAssigned(QString testName);
    void ValidateAutoTestFuncAssignment(tReport* rep);
    ///void UseSn(QString deviceUid);
    ///void SetMainSignalQueue(tSignalQueue signalQueue);

    //bool ReturnTestError(const QString& details = "Test did has not comnpleted");

 public slots:
    void slotRunTest();
    void slotInterruptTest(); // Runner -> Procedure <void>
    void slotFinishTest(); // Move TP back to main thread
signals:
    void sigSetTestInfo(tTestInfo* ti);   // Procedure -> Runner <tTestInfo>
    void sigSetTestProgress(double val); // Procedure -> Runner <double>
    void sigSetTestTimeout(double toSec); // Procedure -> Runner <double>
    void sigAddTestDetails(const QString& details); // Procedure -> Runner <QString>

    // TEST API
public:
//protected: // Moved from tTestUnit
    
    //static tTestStatus Test_BoolToTestStatus(bool val);
    //void PerformOperation(bool result, const QString& operationName, tTestStatus& status, tTestStatus failedStatus = tTestStatus::TestError);
    //static tTestInfo BuildTestInfo(const tTestResult& res, tTestStatus status, const QString& details, bool internalRes);
    //static tTestInfo BuildTestInfo_Failed(const QString& details = "", bool internalRes = false);
    //static tTestInfo BuildTestInfo_Skipped(const QString& details = "", bool internalRes = false);
    //static tTestInfo BuildUnfinishedTestInfo(const tTestResult& res, const QString& details = "");

    void Test_DelayAndSetProgress(int delayMs);
    void Test_CancelTesting(QString details = ""); // Call this function, if there is no sense to continue testing, e.g.the DUT is burnt during flashing firmware, etc
    void Test_SetTimeout(double timeoutSec);
    void Test_AddDetails(const QString& details);
    void Test_SetProgress(double fraction); // 0.0...1.0
    void Test_Skip(const QString& details = "");
    void Test_Interrupt(const QString& details = "");
    void Test_Error(const QString& details = "");

    template <typename T>
    void Test_SetResult(T resValue) {
        if (CurrentTest.Info.Status == tTestStatus::Testing)
            CurrentTest.Info.Result.SetValue(T(resValue));
        else {
            CurrentTest.Info.Status = tTestStatus::TestError;
            Test_AddDetails("Attempt to set value not at testing time!");
        }
    }

};
