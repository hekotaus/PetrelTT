#include "TestProc_TestPlugin.h"
//#include "../ProdTest-TSLT-Common/tMessageBox.h"
//#include "../TSLTConcept/StepperBoard/Src/StepperSettingsI.h"

tTestProc_TestPlugin* TestProc = nullptr;
#define TP TestProc
tTestDevice* Dut = nullptr;
tTestDevice* Dpt = nullptr; // No DPT here
QMessageBox* pTestMessageBox;
//tMessageBox_MotorSwitchState* pMsgBox_SwState;

// Check switch tests
#if 0
void Test_CheckSwGap(tTestInfo& ti, bool isLimitSwitch = false) {
    // Debug only!
#if 0
    int i = 0;
    do {
        QThread::msleep(1000);
        qDebug() << "testing delay" << i;
        if (TP->IsInterrupted()) {
            qDebug() << "TP:Interrupted";
            return;
        }
    } while (i++ < 30);

    return;
#endif

    // Normal
    qDebug() << "tTestProc_Stp::slotCheckGap()" << TP->MB->MotorName;
    TP->MB->IsLimitSwitch = isLimitSwitch;
    TP->Test_SetTimeout(120.0);
    TP->DevId = tTestDevice_ESP_Stepper::DevName2DevId(TP->MB->MotorName);
    // start test
    bool res = false;
    if (isLimitSwitch)
        res = Dut->RunCheckLimitSwitchAdj(TP->DevId);
    else
        res = Dut->RunCheckHomeSwitchAdj(TP->DevId);
    if (!res) {
        TP->Test_Error("Failed to start embedded test");
        return;
    }
    qDebug() << "AfterRun:PropStp.mCmdStpService" << "=" << Dut->PropStp.mCmdStpService.get();
    // wait for end of test
    do {
        QThread::msleep(1000);
        qDebug() << "Wait:PropStp.mCmdStpService" << "=" << Dut->PropStp.mCmdStpService.get();
        if (TP->IsInterrupted()) break;
    } while (!Dut->CheckServiceCommandFinished());

    TP->Test_AddDetails(Dut->GetTestMessage());
    if (tReport::IsHaveResult(Dut->GetServiceCommandStatus())) {
        TP->Test_SetResult(Dut->GetServiceCommandResult());
        TP->Test_AddDetails("Gap tested");
    } else {
        //TP->Test_AddDetails("Service command not finished");
        TP->Test_Error("Embedded test stopped with Test status " + tReport::GetStatusMessage(Dut->GetServiceCommandStatus()));
    }
}
#if 0
void Test_CheckHomeSwGapX(tTestInfo& ti) {
    TP->MB->MotorName = 'X';
    Test_CheckSwGap(ti);
}

void Test_CheckHomeSwGapY(tTestInfo& ti) {
    TP->MB->MotorName = 'Y';
    Test_CheckSwGap(ti);
}

void Test_CheckHomeSwGapZ(tTestInfo& ti) {
    TP->MB->MotorName = 'Z';
    Test_CheckSwGap(ti);
}

void Test_CheckHomeSwGapC(tTestInfo& ti) {
    TP->MB->MotorName = 'C';
    Test_CheckSwGap(ti);
}

void Test_CheckLimitSwGapC(tTestInfo& ti) {
    Test_CheckSwGap(ti, true);
}

void Test_CheckHomeSwGapM(tTestInfo& ti) {
    TP->MB->MotorName = 'M';
    Test_CheckSwGap(ti);
}
#endif

#include <qmessagebox.h>
// Set switch gap tests

// TODO: move to TP
// Assign signal automatically
// Implement TP:: exec
//QMessageBox TestMessageBox = QMessageBox(QMessageBox::Icon::Information, "title", "text",
//    QMessageBox::Ok, nullptr,
//    Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);


void Test_SetSwGap(tTestInfo& ti, char motorName = ' ', bool isLimitSwitch = false) {
#if 0
    // Debug only
    int i = 0;
    do {
        QThread::msleep(1000);
        qDebug() << "testing delay" << i;
        if (TP->IsInterrupted()) {
            qDebug() << "TP:Interrupted";
            //return;
        }
    } while (i++ < 30);

    return nullptr;
#endif


    if (motorName == ' ') motorName = TP->MB->MotorName;
    TP->MB->IsLimitSwitch = isLimitSwitch;

    TP->Test_SetTimeout(60);
    //uint8_t motIdx = tTestDevice_ESP_Stepper::MotorName2MotId(TP->MB->MotorName);
    TP->DevId = tTestDevice_ESP_Stepper::DevName2DevId(TP->MB->MotorName);
    qDebug() << "Set switch gap";
    QMessageBox& TestMessageBox = *pTestMessageBox;
    //tMessageBox_MotorSwitchState MsgBox_SwState;
    //TestMessageBox.setWindowTitle("Set switch gap");
    TestMessageBox.setText("Back the set screw off ");
    TP->Test_ShowMessage(&TestMessageBox);
    int msgBoxResult = 0;
    // TODO: check result
    TP->Test_WaitForMessageBoxResult(msgBoxResult, 30);
    // Not working
    //if (msgBoxResult == QMessageBox::Cancel) {
    //    return TP->Test_Interrupt();
    //}

    qDebug() << "tTestProc_Stp::slotCheckGap()" << TP->MB->MotorName;
    TP->Test_SetTimeout(20.0);

    // start test
    bool res =
        isLimitSwitch ? Dut->RunSetLimitSwitchAdj(TP->DevId)
        : Dut->RunSetHomeSwitchAdj(TP->DevId);
    if (!res) {
        TP->Test_Error("Failed to start embedded test");
        return;
    }
    qDebug() << "AfterRun:PropStp.mCmdStpService" << "=" << Dut->PropStp.mCmdStpService.get();
    // wait for end of test
    do {
        QThread::msleep(1000);
        qDebug() << "Wait:PropStp.mCmdStpService" << "=" << Dut->PropStp.mCmdStpService.get();
        if (TP->IsInterrupted()) return;
    } while (!Dut->CheckServiceCommandFinished());
    
    auto status = Dut->GetServiceCommandStatus();
    qDebug() << "Service Command finished with status" << tReport::GetStatusMessage(status);
    
    if (!tReport::IsHaveResult(status)) {
        TP->Test_Error("Embedded test stopped with Test status " 
            + tReport::GetStatusMessage(Dut->GetServiceCommandStatus()));
        return;
    }

    double switchGap = Dut->GetServiceCommandResult();
    if (switchGap == -1.0) {
        TP->Test_CancelSubtests(); // Makes no sense to continue
        TP->Test_Error("The screw has been not retracted");
        return;
    }

    TP->Test_SetTimeout(60.0);
    ///TestMessageBox.setText("Turn the set screw until the switch is pressed");
    ///TP->Test_ShowMessage(&TestMessageBox);
    //MsgBox.SetMotorName();
    msgBoxResult = 0;
    TP->Test_ShowMessage(pMsgBox_SwState);
    TP->Test_WaitForMessageBoxResult(msgBoxResult, 30);
    
    // Not working when dialog closed automatically
    res = isLimitSwitch ? Dut->GetLimitSwitchState(TP->DevId) : Dut->GetHomeSwitchState(TP->DevId);
    if (!res) {
        TP->Test_Interrupt("Switch was not pushed");
        return;
    }

    TP->Test_SetResult(Dut->GetServiceCommandResult());
    TP->Test_AddDetails("Gap set");
    TP->Test_AddDetails(Dut->GetTestMessage());
}

void Test_SetHomeSwGapX(tTestInfo& ti) { Test_SetSwGap(ti, 'X'); }
void Test_SetHomeSwGapY(tTestInfo& ti) { Test_SetSwGap(ti, 'Y'); }
void Test_SetHomeSwGapZ(tTestInfo& ti) { Test_SetSwGap(ti, 'X'); }
void Test_SetHomeSwGapC(tTestInfo& ti) { Test_SetSwGap(ti, 'C'); }
void Test_SetHomeSwGapM(tTestInfo& ti) { Test_SetSwGap(ti, 'M'); }
void Test_SetLimitSwGapC(tTestInfo& ti) { Test_SetSwGap(ti, 'C', true); }

// Check range
void Test_CheckRange(tTestInfo& ti, char motorName = ' ') {
    qDebug() << "Test_CheckRange";
    if (motorName == ' ') motorName = TP->MB->MotorName;
    TP->DevId = tTestDevice_ESP_Stepper::DevName2DevId(TP->MB->MotorName);
    TP->Test_SetTimeout(40.0);

    // start test
    qDebug() << "Test_CheckRange.SetCheckRange";
    bool res = Dut->RunCheckRange(TP->DevId);
    if (!res) {
        TP->Test_Error("Failed to start embedded test");
        return;
    }
    qDebug() << "AfterRun:PropStp.mCmdStpService" << "=" << Dut->PropStp.mCmdStpService.get();
    // wait for end of test
    do {
        QThread::msleep(1000);
        qDebug() << "Wait:PropStp.mCmdStpService" << "=" << Dut->PropStp.mCmdStpService.get();
        if (TP->IsInterrupted()) return;
    } while (!Dut->CheckServiceCommandFinished());

    auto status = Dut->GetServiceCommandStatus();
    qDebug() << "Service Command finished with status" << tReport::GetStatusMessage(status);

    if (!tReport::IsHaveResult(status)) {
        TP->Test_AddDetails(Dut->GetTestMessage());
        TP->Test_Error("Embedded test stopped with Test status "
            + tReport::GetStatusMessage(Dut->GetServiceCommandStatus()));
        return;
    }
    TP->Test_SetResult(Dut->GetServiceCommandResult());
    TP->Test_AddDetails(Dut->GetTestMessage());
}

void Test_CheckRangeX(tTestInfo& ti) { Test_CheckRange(ti, 'X'); }
void Test_CheckRangeY(tTestInfo& ti) { Test_CheckRange(ti, 'Y'); }
void Test_CheckRangeZ(tTestInfo& ti) { Test_CheckRange(ti, 'Z'); }
void Test_CheckRangeC(tTestInfo& ti) { Test_CheckRange(ti, 'C'); }
void Test_CheckRangeM(tTestInfo& ti) { Test_CheckRange(ti, 'M'); }


// Axis movement
void Test_TestAxisMovement(tTestInfo& ti, char motorName = ' ') {
    qDebug() << "Test_AxisMovement";
    if (motorName == ' ') motorName = TP->MB->MotorName;
    TP->DevId = tTestDevice_ESP_Stepper::DevName2DevId(TP->MB->MotorName);
    TP->Test_SetTimeout(60.0);

    // start test
    qDebug() << "Test_AxisMovement.SetCheckRange";
    bool res = Dut->RunAxisTest(TP->DevId);
    if (!res) {
        TP->Test_Error("Failed to start embedded test");
        return;
    }
    qDebug() << "AfterRun:PropStp.mCmdStpService" << "=" << Dut->PropStp.mCmdStpService.get();
    // wait for end of test
    do {
        QThread::msleep(1000);
        qDebug() << "Wait:PropStp.mCmdStpService" << "=" << Dut->PropStp.mCmdStpService.get();
        if (TP->IsInterrupted()) return;
    } while (!Dut->CheckServiceCommandFinished());

    auto status = Dut->GetServiceCommandStatus();
    qDebug() << "Service Command finished with status" << tReport::GetStatusMessage(status);

    if (!tReport::IsHaveResult(status)) {
        TP->Test_Error("Embedded test stopped with Test status "
            + tReport::GetStatusMessage(Dut->GetServiceCommandStatus()));
        return;
    }
    TP->Test_SetResult(tReport::IsHaveResult(status));

    tSgTestStats stats;
    tSgTestStats statsPos;
    tSgTestStats statsNeg;
    Dut->GetSgTestStats(stats, statsPos, statsNeg);
    qDebug() << "Max deviation" << stats.MaxDeviationAbs;

    // Get SGRESult array
    std::array<uint16_t, 1500> sgRes;
    Dut->GetSgTestResult(sgRes);
    int i = 0;
    uint16_t* pPos = &sgRes[0];
    uint16_t* pNeg = &sgRes[750];
    while ((i < 1500) && !((*pPos == 0) && (*pNeg == 0))) {
        qDebug() << i++ << "," << *pPos++ << "," << *pNeg++;
    }
    // Calc avg, etc
    TP->Test_AddDetails(Dut->GetTestMessage());
}

void Test_TestAxisMovementX(tTestInfo& ti) { Test_TestAxisMovement(ti, 'X'); }
void Test_TestAxisMovementY(tTestInfo& ti) { Test_TestAxisMovement(ti, 'Y'); }
void Test_TestAxisMovementZ(tTestInfo& ti) { Test_TestAxisMovement(ti, 'Z'); }
void Test_TestAxisMovementC(tTestInfo& ti) { Test_TestAxisMovement(ti, 'C'); }
void Test_TestAxisMovementM(tTestInfo& ti) { Test_TestAxisMovement(ti, 'M'); }

void Test_AxisSgresAvgPos(tTestInfo& ti) { TP->Test_SetResult(TP->MB->SgresAvgPos); }
void Test_AxisSgresRmsPos(tTestInfo& ti) { TP->Test_SetResult(TP->MB->SgresRmsPos); }
void Test_AxisSgresAvgNeg(tTestInfo& ti) { TP->Test_SetResult(TP->MB->SgresAvgNeg); }
void Test_AxisSgresRmsNeg(tTestInfo& ti) { TP->Test_SetResult(TP->MB->SgresRmsNeg); }
#endif
char TestProcName[] = "TestPlugin";

PPT_DLL_API char* GetTestProcName() { 
    return TestProcName;
}

PPT_DLL_API tTestProcedure* GetTestProcedure() {
    return TestProc;
}

PPT_DLL_API tTestProcedure* CreateTestProcedure(tLogger& log, tPetrelProjectConfig& cfg) {
    if (TestProc != nullptr) delete TestProc;
    TestProc = nullptr;
    TestProc = new tTestProc_TestPlugin(log, cfg);
    Dut = TestProc->GetDut();
    Dpt = TestProc->GetDpt();
    return TestProc;
}
#if 0
typedef LxPropModel::LxProperty<uint32_t> PropUInt32;
void cbMotorXStatus(const char* ctx, PropUInt32* prop, int index) {
    TP->DutChangedStatus(1);
}
void cbMotorYStatus(const char* ctx, PropUInt32* prop, int index) {
    TP->DutChangedStatus(2);
}
void cbMotorZStatus(const char* ctx, PropUInt32* prop, int index) {
    TP->DutChangedStatus(3);
}
void cbMotorCStatus(const char* ctx, PropUInt32* prop, int index) {
    TP->DutChangedStatus(4);
}
void cbMotorMStatus(const char* ctx, PropUInt32* prop, int index) {
    TP->DutChangedStatus(5);
}
#endif

tTestProc_TestPlugin::tTestProc_TestPlugin(tLogger& log, tPetrelProjectConfig& cfg)
    : tTestProcedure(TestProcName, "SLD board", "TSLT tester", log, cfg)
{
    MB = new tMailBox_TestPlugin();
    MailBox = MB;
    pDUT = &Dut; // Generic pointer
    pDPT = &Dpt;
    //tTestDelegate f = Test_Par1;

    // Create message boxes here, while working in the UI thread
    pTestMessageBox = new QMessageBox(QMessageBox::Icon::Information, "title", "text",
        QMessageBox::Ok, nullptr, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    //pMsgBox_SwState = new tMessageBox_MotorSwitchState();

    ConnectControls(true);

//    Dut.PropStp.mMonMotorXStatus.subscribeNotify(cbMotorXStatus, "ObjData");
//    Dut.PropStp.mMonMotorYStatus.subscribeNotify(cbMotorYStatus, "ObjData");
//    Dut.PropStp.mMonMotorZStatus.subscribeNotify(cbMotorZStatus, "ObjData");
//    Dut.PropStp.mMonMotorCStatus.subscribeNotify(cbMotorCStatus, "ObjData");
//    Dut.PropStp.mMonMotorMStatus.subscribeNotify(cbMotorMStatus, "ObjData");

//    Dut.PropStp.mMonMotorXError.subscribeNotify(cbMotorXStatus, "ObjData");
//    Dut.PropStp.mMonMotorYError.subscribeNotify(cbMotorYStatus, "ObjData");
//    Dut.PropStp.mMonMotorZError.subscribeNotify(cbMotorZStatus, "ObjData");
//    Dut.PropStp.mMonMotorCError.subscribeNotify(cbMotorCStatus, "ObjData");
//    Dut.PropStp.mMonMotorMError.subscribeNotify(cbMotorMStatus, "ObjData");
}

tTestProc_TestPlugin::~tTestProc_TestPlugin() {
    ConnectControls(false);
    pDUT = nullptr;
    pDPT = nullptr;
    if (PanDutCfg != nullptr) delete PanDutCfg;
    if (PanDptCfg != nullptr) delete PanDptCfg;
    //delete TestDialog1;
    delete MB;
    MailBox = nullptr;
    MB = nullptr;
    delete pTestMessageBox;
    //delete pMsgBox_SwState;
}

//void tTestProc_Stp::DutChangedStatus(uint8_t devId) {
//    if (devId > 5) return;
//    uint32_t status = Dut.GetStatusWord(devId);
//    emit sigChangedStatus(devId, Dut.GetStatusWord(devId), Dut.GetErrorWord(devId));
//
//    uint32_t msk = MB->IsLimitSwitch ? nsMotor::mskStatLimitSwitch : nsMotor::mskStatHomeSwitch;
//    if ((devId == DevId) && (status & msk))
//        emit sigChangedSwitchState(true);
//}

void tTestProc_TestPlugin::AssignTestFunctions() {
//    AssignTestFunction("Check Home switch gap X", Test_CheckHomeSwGapX);
//    AssignTestFunction("Check Home switch gap Y", Test_CheckHomeSwGapY);
//    AssignTestFunction("Check Home switch gap Z", Test_CheckHomeSwGapZ);
//    AssignTestFunction("Check Home switch gap C", Test_CheckHomeSwGapC);
//    AssignTestFunction("Check Home switch gap M", Test_CheckHomeSwGapM);
//    AssignTestFunction("Check Limit switch gap C", Test_CheckLimitSwGapC);

    // Manual test dialogues
//    AssignManualDialog("Motor X", &TestDialogMotor);
//    AssignManualDialog("Motor Y", &TestDialogMotor);
//    AssignManualDialog("Motor Z", &TestDialogMotor);
//    AssignManualDialog("Motor C", &TestDialogMotor);
//    AssignManualDialog("Motor M", &TestDialogMotor);
}

tPanDevCfg* tTestProc_TestPlugin::MakePanDutCfg(QWidget *parent, int id) {
    if (PanDutCfg != nullptr) delete PanDutCfg;
    PanDutCfg = new tPanDevCfg(parent, id, "DUT config: " + DutName);
    return PanDutCfg;
}

tPanDevCfg* tTestProc_TestPlugin::MakePanDptCfg(QWidget* parent, int id) {
    if (PanDptCfg != nullptr) delete PanDptCfg;
    PanDptCfg = new tPanDevCfg(parent, id, "DPT config: " + DptName);
    return PanDptCfg;
}

tTestDialog* tTestProc_TestPlugin::GetManualTestDialog(const QString& groupName) {
    tTestDialog* manDialog = tTestProcedure::GetManualTestDialog(groupName);
    if (manDialog == nullptr) {
        qDebug() << "Test dialog for" << groupName << "is not assigned";
    }
//    else if (groupName == "Motor X") { TestDialogMotor.SetName('X'); } 
//    else if (groupName == "Motor Y") { TestDialogMotor.SetName('Y'); } 
//    else if (groupName == "Motor Z") { TestDialogMotor.SetName('Z'); }
//    else if (groupName == "Motor C") { TestDialogMotor.SetName('C'); }
//    else if (groupName == "Motor M") { TestDialogMotor.SetName('M'); }
    return manDialog;
}

void tTestProc_TestPlugin::ConnectControls(bool st) {
    if (st) {
//        connect(&TestDialogMotor.btnCheckGap_Home,  &QPushButton::clicked, this, &tTestProc_Stp::slotBtnCheckGap_Home);
    } else {
//        disconnect(&TestDialogMotor.btnCheckGap_Home, &QPushButton::clicked, this, &tTestProc_Stp::slotBtnCheckGap_Home);
    }
}

//void tTestProc_TestPlugin::slotBtnCheckGap_Home() {
//    //MB->MotorName = TestForm_Motor.GetName();
//    MB->MotorName = TestDialogMotor.GetName();
//    Test_StartManualTest(QString("Check Home switch gap %1").arg(MB->MotorName));
//    //Test_StartManualTest("Home switch gap X");
//}

//void tTestProc_TestPlugin::slotBtnCheckGap_Limit() {
//    //MB->MotorName = TestForm_Motor.GetName();
//    MB->MotorName = TestDialogMotor.GetName();
//    Test_StartManualTest(QString("Check Limit switch gap %1").arg(MB->MotorName));
//    //Test_StartManualTest("Home switch gap X");
//}

