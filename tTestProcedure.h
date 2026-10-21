#pragma once
#include <qstring.h>
#include <qthread.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include "tTestProcMailBox.h"
#include "tPanDevCfg.h"
#include "tTestDevice.h"
#include "tTestStatus.h"
#include "tTestInfo.h"
#include "tPetrelProjectConfig.h"
#include "tTestResult.h"
#include "tTestDialog.h"

class tTestDevice;

enum class tTestMode {
    None,
    Auto,
    Manual,
    Service
};

using tTestDelegate = void (*)(tTestInfo&);

struct tTestStruct {
    tTestInfo Info;
    tTestDelegate Proc; // Function()
};

class tTestProcedure : public QObject {
    Q_OBJECT

protected:
    const QString TestProcName;
    const QString DutName;
    const QString DptName;
    tPetrelProjectConfig& Cfg;
    tTestProcMailBox* MailBox = nullptr;
    tPanDevCfg* PanDutCfg = nullptr;
    tPanDevCfg* PanDptCfg = nullptr;
    tTestStruct CurrentTest;
    QEventLoop* MessageBoxWaiterLoop = nullptr; // used for waiting and getting info from main
    QTimer MessageBoxWaiterTimer;
    int MessageBoxResult = 0;

    std::atomic<bool> InterruptFlag = false;
    ///std::atomic<bool> CancelTestingFlag = false;
    //QString TestName = "";

    void SetTestStatus(tTestStatus newStatus);
    void SetTestInfo();
    

public:
    void DeletePanCfg();
    tTestResult GetResult() const { return CurrentTest.Info.Result; }
    tTestProcedure(QString name, QString dutName, QString dptName, tPetrelProjectConfig& cfg);//, 
    ~tTestProcedure();

    // DPT and DUT
    QString GetDutName() const { return DutName; }
    QString GetDptName() const { return DptName; }
    virtual tPanDevCfg* MakePanDutCfg(QWidget* parent, int id) = 0; // need parent to avoid stupid parentless window blinking
    virtual tPanDevCfg* MakePanDptCfg(QWidget* parent, int id) = 0;
    
    tPanDevCfg* GetPanDutCfg() { return PanDutCfg; }
    tPanDevCfg* GetPanDptCfg() { return PanDptCfg; }
    
//    bool InitDptAndDut(tReport* rep);
//    void DoneDutAndDpt(tReport* rep);
    // Basic init and done methods, can be replaced in plugin
    virtual bool InitAutoTests() { if (cbInitAutoTests != nullptr) return true; return cbInitAutoTests; };
    virtual bool InitManualTests() { if (cbInitManualTests != nullptr) return true; return cbInitManualTests; };
    virtual void DoneAutoTests() { if (cbDoneAutoTests != nullptr) cbDoneAutoTests; };
    virtual void DoneManualTests() { if (cbDoneManualTests != nullptr) cbDoneManualTests; };

    virtual void AssignTestFunctions() = 0; // to be called after loading specs

    tTestDevice* pDUT = nullptr; // Generic pointer to Device Under Test
    tTestDevice* pDPT = nullptr; // Generic pointer to Device Performing Test
    bool IsInterrupted() const { return InterruptFlag; }
    void InterruptReset() { InterruptFlag = false; }

    tTestInfo& GetCurrentTestInfo() { return CurrentTest.Info; }
    tTestResult GetCurrentTestResult() const { return CurrentTest.Info.Result; }

 public slots:
    void slotRunTest(tTestStruct currentTest);
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
    void sigRequestSn(const QString&); // Procedure->App
    void sigCancelTesting(); // Procedure -> Runner
    void sigCancelSubtests(); // Procedure -> Runner

    // TEST API

public:
    bool (*cbAssignTestFunction)(const QString& testName, tTestDelegate testProc) = nullptr;
    bool (*cbAssignManualDialog)(const QString& groupName, tTestDialog* pDialog) = nullptr;
    tTestDialog* (*cbGetManualDialog)(const QString& groupName) = nullptr;
    bool (*cbInitAutoTests)() = nullptr;
    bool (*cbInitManualTests)() = nullptr;
    void (*cbDoneAutoTests)() = nullptr;
    void (*cbDoneManualTests)() = nullptr;

    //void CB_AssignTestFunction(const QString& testName, tTestDelegate testProc);
    //void Test_AssignTestFunction(const QString& testName, tTestDelegate testProc);

    //tTestDialog* FindManualDialog(QString name); //Scan Specs tree until name found.Return ManualTestDialog
    bool AssignTestFunction(const QString& testName, tTestDelegate testProc);
    bool AssignManualDialog(const QString groupName, tTestDialog* pDialog);
    virtual tTestDialog* GetManualDialog(const QString& groupName);

    // Here all *TP results are used only for easier returning from the test
    void Test_DelayAndSetProgress(int delayMs);
    tTestProcedure* Test_CancelTesting(QString details = ""); // Call this function, if there is no sense to continue testing, e.g. the DUT is burnt during flashing firmware, etc.
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
    //bool Test_SetSerialNumber(); // 
    void Test_SetSerialNumber(const QString&); // Set SN. If empty, make Shell request SN from user
    bool Test_WaitForSetSerialNumber(int& messageBoxResult, int timeoutSec);

    template <typename T>
    tTestProcedure* Test_SetResult(T resValue) {
        if (CurrentTest.Info.Status == tTestStatus::Testing) {
            if (!CurrentTest.Info.Result.SetValue(T(resValue))) {
                Test_AddDetails(CurrentTest.Info.Result.GetResultComment());

            }
        } else {
            CurrentTest.Info.Status = tTestStatus::TestError;
            Test_AddDetails("Attempt to set value not at testing time!");
        }
        return this;
    }
};
