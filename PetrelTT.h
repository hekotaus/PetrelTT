#pragma once

#include <QtWidgets/QMainWindow>

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

enum ePanelsId {
    // Side panels
    ePanControlId,
    ePanDptCfgId,
    ePanDutCfgId,
    ePanTestTreeId,

    // CENTRE
    ePanTestDialogId,
    ePanReportId,

    // RIGHT
    ePanDebugId,
    ePanLogId,
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
    const int SidePanelWidth = 400;
    tDock* DockLeft = nullptr;
    tDock* DockRight = nullptr;
    tDock* DockCenter = nullptr;

    tPanControl* PanControl = nullptr;
    tPanTestDialog* PanTestDialog = nullptr;
    tPanTestTree* PanTestTree = nullptr;
    tPanDevCfg* PanDutConfig = nullptr;
    tPanDevCfg* PanDptConfig = nullptr;

    tPanLog* PanLog = nullptr;
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
    St State = St::Init;

    bool IsLoadTp = false; // True, if should react to change of combo box immediately

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
public slots:
    void ArrangeDocks();

    void slotSetModeAutoTest() { SetState(St::AutoStopped); }
    void slotSetModeManualTest() { SetState(St::ManualStopped); }
    void slotSetModeConfig() { SetState(St::Config); }
    void slotSetModeTestProc() { SetState(St::TestProc); }
    void slotSetModeReports() { SetState(St::Reports); }
    void slotStartTest(); // Start button pressed
    void slotStopTest() { // Stop button pressed
        Project.StopTests(); 
    }

    void slotSelectSpec();

    void slotColorizeTree() {
        if (Project.Cfg.ReportCurrent == Project.Cfg.ReportAutoTest)
            Project.ColorizeAutoTestTree(); 
    }
    void slotPopulateTestSpecVersions();
    void slotTestGroupChanged(const QString& groupName);

    void slotStartManualTest(const QString&); // From TP
    void slotTestFinished(tTestStatus status); // From test runner. Informs on test status changes
    void slotShowMessage(QMessageBox*); // From TP
signals:
    void sigMessageResult(int);
};
