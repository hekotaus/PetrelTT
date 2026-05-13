#pragma once

#include <QtWidgets/QMainWindow>
//#include "ui_PetrelTT.h"

// VedroLib
#include "app/tAppConfig.h"
#include "panels/tDock.h"
#include "panels/tSidePanel.h"
#include "logger/tLogger.h"

// Panels
#include "tPanControl.h"
#include "tPanConfig.h"
#include "tPanTestTree.h"
#include "tPanTestDialog.h"
#include "tPanReport.h"
#include "tPanDebug.h"
#include "tPanLog.h"
#include "tPetrelProject.h"
#include "tTestDialog.h"

//#include "tDeviceConfigDialog.h"

enum ePanelsId {
    // Side panels
    //eLeftPanelStart = 0,
    ePanControlId,
    ePanDptCfgId,
    ePanDutCfgId,
    ePanTestTreeId,
    //eLeftPanelEnd,

    // CENTRE
    ePanTestDialogId,
    ePanReportId,

    // RIGHT
    //eRightPanelStart,
    ePanDebugId,
    ePanLogId,
    //eRightPanelEnd,

    // Hidden
    // Graph views
    //ePanTrackViewId,
    //eGraphViewEnd,

    // Auto
    ePanAutoId = 100
};

class tPetrelTT : public QMainWindow {
    Q_OBJECT

    enum class St {
        Init,
        Config,
        ManualStopped,
        ManualRunning,
        AutoStopped,
        AutoRunning,
        TestProc,
        Reports,
    };

    //tDock* DockTopLeft = nullptr;
    tDock* DockLeft = nullptr;
    tDock* DockRight = nullptr;
    //tDock* DockTop = nullptr;
    //tDock* DockBottom = nullptr;
    tDock* DockCenter = nullptr;
    //tDock* DockMain = nullptr;

    tPanControl* PanControl = nullptr;
    tPanTestDialog* PanTestDialog = nullptr;
    tPanTestTree* PanTestTree = nullptr;
    tPanDevCfg* PanDutConfig = nullptr;
    tPanDevCfg* PanDptConfig = nullptr;
    //tPanConfig TODO: check if used ever

    tPanLog* PanLog = nullptr;
    //tPanTestTree* PanTestTree = nullptr;
    tPanReport* PanReport = nullptr;
    tPanDebug* PanDebug = nullptr;

    tLogger Logger = { true };

    tReportRoot* ReportAutoTestRoot = nullptr;
    tReportRoot* ReportManualTestRoot = nullptr;
    tReportRoot* ReportConfigRoot = nullptr;
    tReportRoot* ReportTestProcRoot = nullptr;
    tReportRoot* ReportReportsRoot = nullptr;
    tReportRoot* ReportCurrentRoot = nullptr;

    tPetrelProject Project = tPetrelProject(Logger);
    St State;

    bool IsLoadTp = false; // True, if should react to change of combo box immediately

    //std::list<tSidePanel*> SidePanels; // Collection of side panels
    std::list<tPanel*> SidePanels; // Collection of side panels
    template <typename T>
    T* AddSidePanel(T* panel) { SidePanels.push_back(panel); return panel; }
    tAppConfig AppConfig = tAppConfig("PetrelTT", "Ellex", "ellex.com");
    void resizeEvent(QResizeEvent* event);
    void closeEvent(QCloseEvent* event);

    void SetScheme(int i);
    void LoadPanelConfig();
    void LoadProjectConfig();
    void SaveConfig();

    void SetState(St st);
    void PopulateTestProcedures();
    
    void LoadTestProcedure();
    void CloseTestProcedure();

public:
    tPetrelTT(QWidget *parent = nullptr);
    ~tPetrelTT();

    
private:
    void testReport();
    //Ui::PetrelTTClass ui;

    //tDeviceConfigDialog* DutCfgDialog = nullptr;
    //tDeviceConfigDialog* DptCfgDialog = nullptr;
public slots:
    void ArrangeDocks();

    void slotSetModeAutoTest() { SetState(St::AutoStopped); }
    void slotSetModeManualTest() { SetState(St::ManualStopped); }
    void slotSetModeConfig() { SetState(St::Config); }
    void slotSetModeTestProc() { SetState(St::TestProc); }
    void slotSetModeReports() { SetState(St::Reports); }
    void slotStartTest() {
        if (State == St::AutoStopped) {
            SetState(St::AutoRunning);
            Project.StartAutoTests();
            SetState(St::AutoStopped);
        }
    }
    void slotStopTest() { 
        Project.StopTests(); 
    }

    void slotSelectSpec();

    void slotColorizeTree() { 
        Project.ColorizeAutoTestTree(); 
    }
    void slotPopulateTestSpecVersions();
    void slotTestGroupChanged(const QString& groupName);
};

