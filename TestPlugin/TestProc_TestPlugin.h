#pragma once
#include <QtWidgets/QMainWindow>
#include "PetrelTT/tTestProcMailBox.h"
#include "PetrelTT/tTestProcedure.h"
#include "PetrelTT/tReport.h"

//#include "../ProdTest-TSLT-Common/tTSLT_ProdTester.h"
#include "PetrelTT/tTestDevice.h"
#include "tTestDialog1.h"

char DutName[]; // "TSLT-SLD_production"

class tTestProc_TestPlugin : public tTestProcedure {
    Q_OBJECT
    tTestDevice Dut = tTestDevice(this);
    tTestDevice Dpt = tTestDevice(this);
    //tTestDialog1 TestDialog1;
    void ConnectControls(bool st);
    
public:
    struct tMailBox_TestPlugin : tTestProcMailBox { // Tranfer data from Manual TestDialog to test via this mailbox 
//        char MotorName = ' '; // Set by dialog
//        bool IsLimitSwitch = false;
//        double SgresAvgPos = -1;
//        double SgresRmsPos = -1;
//        double SgresAvgNeg = -1;
//        double SgresRmsNeg = -1;
        //uint8_t MotorIdx = -1;
        // Add fields here 
        //bool Reset; // Reset
    };

    tMailBox_TestPlugin* MB;
    uint8_t DevId = -1; // set by test

    tTestProc_TestPlugin(tPetrelProjectConfig& cfg);
    ~tTestProc_TestPlugin();

    tPanDevCfg* MakePanDutCfg(QWidget* parent, int id) override;
    tPanDevCfg* MakePanDptCfg(QWidget* parent, int id) override;
    tTestDialog* GetManualDialog(const QString& groupName) override;

    void AssignTestFunctions() override;
    tTestDevice* GetDut() { return &Dut; }
    tTestDevice* GetDpt() { return &Dpt; }
    
//    void DutChangedStatus(uint8_t devId);
//public slots:
//    void slotBtnCheckGap_Home();
//    void slotBtnCheckGap_Limit();
//    void slotBtnSetGap_Home();
//    void slotBtnSetGap_Limit();
//    void slotBtnCheckRange();
//    void slotBtnTestAxis();

//    void slotBtnForward();
//    void slotBtnBackward();
//    void slotBtnBrake();
//    void slotBtnStop();
//signals:
//    void sigChangedStatus(uint8_t devId, uint32_t status, uint32_t error);
//    void sigChangedSwitchState(bool st); // To switch dialog
};

//#define PPT_DLL_API __declspec(dllexport)
#define PPT_DLL_API extern "C" __declspec(dllexport)

//PPT_DLL_API tEspResult GetLastResult();
PPT_DLL_API char* GetTestProcName();
PPT_DLL_API tTestProcedure* GetTestProcedure();
PPT_DLL_API tTestProcedure* CreateTestProcedure(tPetrelProjectConfig& cfg);
