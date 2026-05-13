#include "PetrelTT.h"
#include "param/tParam.h"
#include "common/tTickTock.h"
#include "widgets/tParamWidget_QComboBox.h"
#include <algorithm>

using namespace std;

struct tProjectSignaler : QObject {
public slots:
    
};

extern tProjectSignaler g_ProjectSignaler;

tPetrelTT::tPetrelTT(QWidget *parent)
    : QMainWindow(parent) {
    //ui.setupUi(this);
    Logger.SetLogToQdebug(true);
    Logger.SetDefaultSource("Tester");
    Logger.SetShowSource(false);
    AppConfig.FixHighScale();
    AppConfig.RestoreMainWindow(this);
    AppConfig.SetMainWindowMinSize(QSize(1600, 600));
    AppConfig.SetVersion("PetrelTT", 2026, 1, 0);
    Project.Cfg.AppName = AppConfig.GetName();
    Project.Cfg.AppVer = AppConfig.GetVersion();
    AppConfig.SetWindowTitle();
    AppConfig.SetScheme(0);

    Project.ParGrpProject->LoadFromConfig();
    //Project.GeneralParamsSuperGroup.LoadFromConfig();
    PanDebug = AddSidePanel(new tPanDebug(this, ePanDebugId));

    PanLog = AddSidePanel(new tPanLog(this, ePanLogId));
    connect(&Logger, SIGNAL(sigLogString(QString)), PanLog, SLOT(slotAddLog(QString)));
    
    PanTestDialog = AddSidePanel(new tPanTestDialog(this, ePanTestDialogId));

    PanReport = AddSidePanel(new tPanReport(this, ePanReportId, &Logger, Project.Cfg));
    connect(&g_ReportSignaler, SIGNAL(sigStatusUpdated()), this, SLOT(slotColorizeTree()));

    Logger.LogSystemMessage(CurrentDateTime());
    Logger.LogSystemMessage("Init Test shell...");

    PanTestTree = AddSidePanel(new tPanTestTree(this, ePanTestTreeId/*, &Logger, Project.Cfg*/));

    const int dockWidth = 400 + 20 + 5;// Constant::SidePanelWidth + tDock::ScrollBarVWidth + tDock::SpacingH;
    DockLeft = new tDock(this, eDockOrientation::Vertical, dockWidth, "Left");
    DockRight = new tDock(this, eDockOrientation::Vertical, dockWidth, "Right");
    DockCenter = new tDock(this, eDockOrientation::Vertical, -1, "Center");

    DockLeft->SetMargins(QMargins(10, 10, 10, 10));
    DockCenter->SetMargins(QMargins(10, 10, 10, 10));
    DockRight->SetMargins(QMargins(10, 10, 10, 10));

    DockLeft->SetDockScrollSide(eDockScrollSide::None);
    DockCenter->SetDockScrollSide(eDockScrollSide::None);
    DockRight->SetDockScrollSide(eDockScrollSide::None);
    
    PanControl = AddSidePanel(new tPanControl(this, ePanControlId)); //new tPanControl(DockTopLeft, ePanControlId);
    connect(PanControl->btnAutoTest, SIGNAL(clicked()), this, SLOT(slotSetModeAutoTest()));
    connect(PanControl->btnManualTest, SIGNAL(clicked()), this, SLOT(slotSetModeManualTest()));
    connect(PanControl->btnConfig, SIGNAL(clicked()), this, SLOT(slotSetModeConfig()));
    connect(PanControl->btnTestProc, SIGNAL(clicked()), this, SLOT(slotSetModeTestProc()));
    connect(PanControl->btnReports, SIGNAL(clicked()), this, SLOT(slotSetModeReports()));
    
    connect(PanControl->cbDutName, SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotPopulateTestSpecVersions()));
    connect(PanControl->cbSpecVersion, SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotSelectSpec()));
    connect(PanControl->btnStart, SIGNAL(clicked()), this, SLOT(slotStartTest()));
    connect(PanControl->btnStop, SIGNAL(clicked()), this, SLOT(slotStopTest()));
    connect(&Project.TestRunner, SIGNAL(sigSetProgressBar(int)), PanControl->progTestProgress, SLOT(setValue(int)));

    connect(&PanDebug->btnReloadTP, SIGNAL(clicked()), this, SLOT(slotSelectSpec()));

    connect(PanTestTree, SIGNAL(sigChangeGroupName(const QString&)), this, SLOT(slotTestGroupChanged(const QString&)));

    auto& cfg = Project.Cfg;
    auto pParGrpProj = Project.ParGrpProject;
    pParGrpProj->GetParamByStorage(&cfg.OperatorName)->AssignWidget(new tParamWidget_QComboBox_QString(PanControl->cbOperatorName));
    
    // Do not assign the same widget to another param, as they conflict
    //pParGrpProj->GetParamByStorage(&cfg.OperatorList)->AssignWidget(new tParamWidget_QComboBox_QStringList(PanControl->cbOperatorName));
    DockLeft->Add(PanControl); // to be topleft
    DockLeft->Add(PanTestTree);
    DockLeft->slotScrollV(0);

    DockRight->Add(PanDebug);    
    DockRight->Add(PanLog);

    DockCenter->Add(PanTestDialog);
    DockCenter->Add(PanReport);
    Logger.LogSystemMessage("Test shell initialised");

    PanControl->slotTestProc(); //  switch panel to config mode
    SetState(St::TestProc);
    LoadProjectConfig();
    LoadPanelConfig();
    //testReport();
    PopulateTestProcedures();
    //SetState(St::Config);
}

void tPetrelTT::testReport() {
    auto root = Project.Cfg.ReportCurrent->AddReport("root");
    root->SetShowDetails(true, true);
    root->SetShowDetailsSubtree(true);

    auto r1 = root->AddReport("Test1");
    r1->AddDetails("details and fabric");
    r1->SetDescription("Description test1");
    r1->SetStatus(tTestStatus::Testing);
    tTestResult res1;
    res1.SetValueType(tTestResult::tValueType::Float);
    res1.SetValue(2.4);
    //res1.SetResultComment("Fit something");
    r1->SetResult(res1);
    //r1->SetStatus(tTestStatus::Testing);

    auto r11 = r1->AddReport("Test11");
    r11->AddDetails("details and fabric11");
    r11->SetStatus(tTestStatus::Failed);

    auto r12 = r1->AddReport("Test12");
    r12->AddDetails("details and fabric112");
    r12->SetStatus(tTestStatus::Passed);

    //r1->SetStatus(tTestStatus::Tested);

    //root->SetStatus(tTestStatus::Tested);
    //root->SetGroupStatus();

#if 0
    auto r2 = root->AddReportPending("Test2");
    r2->AddDetails("pending details2");

    auto r21 = r2->AddReportSkipped("Test3");
    r21->AddDetails("skipped details3");
    r21->SetStatus(tTestStatus::Skipped);

    auto r4 = root->AddReportTesting("Test4");
    r4->AddDetails("testing details4");
#endif
    //tReport* rep = ReportCurrentRoot->AddReport("Test");
    //PanReport->ReportView.setCurrentCharFormat();
    Project.Cfg.ReportCurrent->ExpandSubtree(true);
}

tPetrelTT::~tPetrelTT() {
    PanTestDialog->SetTestDialog(nullptr); // Unuse it before destructing
    Project.CloseTestProcedure();
}


void tPetrelTT::SetState(St st) { 
    auto oldState = State;
    State = st; 

    switch (State) {
    case St::Init: break;
    case St::Config: PanReport->SetCurrentReport(tReportType::Config); break;
    case St::ManualStopped: 
        PanReport->SetCurrentReport(tReportType::ManualTest); 
        if (oldState != St::ManualRunning) {
            PanTestTree->TreeView.takeTopLevelItem(0);
            PanTestTree->TreeView.insertTopLevelItem(0, Project.GetManualTestTree());
            Project.GetManualTestTree()->setExpanded(true);
            Project.InitManualTest();
        }
        break;
    case St::ManualRunning: break;
    case St::AutoStopped: 
        PanReport->SetCurrentReport(tReportType::AutoTest); 
        if (oldState != St::AutoRunning)
            PanTestTree->TreeView.takeTopLevelItem(0);
            PanTestTree->TreeView.insertTopLevelItem(0, Project.GetAutoTestTree());
            Project.ExpandTestTree(Project.GetAutoTestTree(), true); // TODO: expand all auto tree
        break;
    case St::AutoRunning: break;
    case St::TestProc: PanReport->SetCurrentReport(tReportType::TestProc); break;
    case St::Reports: PanReport->SetCurrentReport(tReportType::Reports); break;
    }
    bool isInit = (State == St::Init);
    bool isTestStopped = ((State == St::ManualStopped) || (State == St::AutoStopped));
    bool isTestRunning = ((State == St::ManualRunning) || (State == St::AutoRunning));
    bool isConfig = (State == St::Config);
    bool isTestProc = (State == St::TestProc);
    bool isManual = ((State == St::ManualRunning) || (State == St::ManualStopped));

    PanControl->btnAutoTest->setEnabled(!isInit);
    PanControl->btnManualTest->setEnabled(!isInit);
    PanControl->btnConfig->setEnabled(!isInit);
    PanControl->btnReports->setEnabled(!isInit);
    PanControl->btnTestProc->setEnabled(!isInit);

    PanControl->btnStart->setEnabled(isTestStopped);
    PanControl->btnStop->setEnabled(isTestRunning);
    PanControl->cbOperatorName->setEnabled(!isTestRunning);
    PanControl->cbDutName->setEnabled(isTestProc || isInit);
    PanControl->cbSpecVersion->setEnabled(isTestProc || isInit);
    
    if (Project.TP != nullptr) {
        tPanPetrel* pan = Project.TP->GetPanDutCfg();
        if (pan != nullptr) pan->SetLayout(isConfig ? 1 : 0);

        pan = Project.TP->GetPanDptCfg();
        if (pan != nullptr) pan->SetLayout(isConfig ? 1 : 0);
    }

    if (isTestStopped || isTestRunning) {
        PanTestTree->SetLayout(1);
    } else {
        PanTestTree->SetLayout(0);
        ArrangeDocks();
    }

    if ((oldState == St::ManualRunning) && (State == St::ManualStopped)) { // Stopping manual test
        Project.InitManualTest();
    }

    if ((oldState == St::ManualStopped) && (State != St::ManualRunning)) { // Switch from manual test tab
        Project.DoneManualTest();
    }

    PanTestDialog->Show(isManual);
    //PanControl->progTestProgress->setEnabled(isConfig);
}

void tPetrelTT::LoadPanelConfig() {
    //Log->AddNormalMessage("Loading parameters...");
    QSettings Settings;
    Settings.beginGroup("SidePanels");
    for (tPanel* pan : SidePanels) {
        qDebug() << pan->GetCaption();
        QStringList panCfg = Settings.value(pan->GetCaption(), ",1").toString().split(',');
        int id = pan->GetId();
        //if (id > eLeftPanelStart && id < eLeftPanelEnd) DockLeft->Add(pan);
        //else if (id > eRightPanelStart && id < eRightPanelEnd) DockRight->Add(pan);
        pan->SetLayout(panCfg[1].toInt());
    }
    Settings.endGroup();
}

void tPetrelTT::LoadProjectConfig() {
    //Project.ProcessParamsSuperGroup.LoadFromConfig();
    Project.GeneralParamsSuperGroup.LoadFromConfig();
    auto par = Project.ParGrpProject->GetParamByStorage(&Project.Cfg.OperatorName);
    QString opName = par->GetCurValueAsQString();

    for (const QString& s : Project.Cfg.OperatorList) {
        PanControl->cbOperatorName->addItem(s);
    }
    par->TrySetCurValueFromQString(opName, tParamSrc::Config, false);
    //par->UpdateWidget(true);// > AssignWidget(new tParamWidget_QComboBox_QString(PanControl->cbOperatorName));
}

void tPetrelTT::SaveConfig() {
    ///    ParamApp->SaveToConfig(); // Paths
    ///    Project.ParamAppVersion->SaveToConfig(); // Write only
    auto cb = PanControl->cbOperatorName;
    auto& ol = Project.Cfg.OperatorList;
    ol.clear();
    //Project.Cfg.OperatorList.append(PanControl->cbOperatorName->currentText());
    int n = cb->count();
    QString s;
    for (int i = 0; i < n; i++) {
        s = cb->itemText(i);
        if (!s.isEmpty())
            ol.append(s);
    }
    s = cb->currentText();
    if (!s.isEmpty() && !ol.contains(s))
        ol.append(s);

    //Project.Cfg.OperatorList.removeDuplicates();
    Project.GeneralParamsSuperGroup.SaveToConfig();
    QSettings Settings;
    // Side panels
    Settings.beginGroup("SidePanels");
    for (tPanel* pan : SidePanels) {
        // Panel Caption, Layout, Dock, 
        auto par = pan->parentWidget();
        QString panCfg = QString("%1,%2").arg(par == DockLeft ? "LeftDock" : "RightDock").arg(pan->GetCurLayout());
        Settings.setValue(pan->GetCaption(), panCfg);
    }
    Settings.endGroup();
};

void tPetrelTT::closeEvent(QCloseEvent* event) {
    SaveConfig();
}

void tPetrelTT::ArrangeDocks() {
    //DockLeft->setAutoFillBackground(true);
    //DockCenter->setAutoFillBackground(true);
    //DockRight->setAutoFillBackground(true);

    DockLeft->slotScrollV(0); // Why the hell is it needed?
    DockCenter->slotScrollV(0);
    DockRight->slotScrollV(0);

    if (DockLeft == nullptr) return;
    if (DockRight == nullptr) return;
    if (DockCenter == nullptr) return;

    int borderX = 2;
    int borderY = 2;
    int w = width() - 1 - borderX - borderX;
    int h = height() - 1 - borderY;
    int dockY = DockLeft->GetMarginTop();
    int dockH = h - dockY - DockLeft->GetMarginBottom();
    int marginX = DockLeft->GetMarginRight() / 2;

    DockLeft->SetMaxDockHeight(dockH);
    DockCenter->SetMaxDockHeight(dockH);
    DockRight->SetMaxDockHeight(dockH);

    int dlW = DockLeft->width();
    int dlX = DockLeft->GetMarginLeft();
    DockLeft->move(dlX, dockY);
    PanTestTree->SetHeight(dockH - PanTestTree->pos().y());// -DockLeft->GetMarginBottom());// -PanReport->pos().y());

    int drW = DockRight->width();
    int drX = w - DockRight->width() - DockRight->GetMarginRight();
    DockRight->move(drX, dockY);
    PanLog->SetHeight(dockH - PanLog->pos().y());

    int dcX = dlX + DockLeft->width() + marginX;
    int dcW = drX - dcX - marginX;
    DockCenter->move(dcX, dockY);
    DockCenter->resize(dcW, dockH); // Only W used
    PanReport->SetHeight(dockH - PanReport->pos().y());
}

void tPetrelTT::resizeEvent(QResizeEvent* event) {
    ArrangeDocks();
    event->accept();
}

void tPetrelTT::LoadTestProcedure() {
    // Connect to dll
    SetState(St::Init);
    //Project.CloseTestProcedure();
    Project.CreateTestProcedure();
    if (Project.TP == nullptr) return;

    PanDutConfig = Project.TP->MakePanDutCfg(DockLeft, ePanDutCfgId);
    DockLeft->Add(PanDutConfig);
    PanDutConfig->SetLayout(0);

    PanDptConfig = Project.TP->MakePanDptCfg(DockLeft, ePanDptCfgId);
    DockLeft->Add(PanDptConfig);
    PanDptConfig->SetLayout(0);

    if (Project.TP->GetValid()) SetState(St::TestProc);
}

void tPetrelTT::CloseTestProcedure() {
    // Connect to dll
    SetState(St::Init);
    Project.CloseTestProcedure();
    //Project.CreateTestProcedure();
    if (Project.TP == nullptr) return;
    DockLeft->Remove(PanDutConfig);
    DockLeft->Remove(PanDptConfig);
}

void tPetrelTT::slotSelectSpec() {
    if (IsLoadTp)
        LoadTestProcedure();
}

void tPetrelTT::PopulateTestProcedures() {
    Logger.LogSystemMessage("Looking for test procedures...");
    Project.BuildDirNames();
    Project.DiscoverTestProcedures(); // Call from better spot
    PanControl->PopulateDutList(Project.DutNameList);
    PanControl->TrySetDutName(Project.Cfg.DutName);
    
    //PopulateTestSpecVersions()
    Project.DiscoverSpecVersions();
    IsLoadTp = false;
    PanControl->PopulateSpecVerList(Project.SpecVerList); // This causes slotSelectSpec->LoadTestProcedure();
    qDebug() << "TrySetSpecVer" << Project.Cfg.TestSpecsVer;
    PanControl->TrySetSpecVer(Project.Cfg.TestSpecsVer);
    IsLoadTp = true;
    qDebug() << "Cfg.TestSpecsVer" << Project.Cfg.TestSpecsVer;
    LoadTestProcedure();
    qDebug() << "Cfg.TestSpecsVer" << Project.Cfg.TestSpecsVer;
}


void tPetrelTT::slotPopulateTestSpecVersions() {
    //PanControl->TrySetDutName(Project.Cfg.DutName);
    Project.Cfg.DutName = PanControl->cbDutName->currentText();
    Project.BuildDirNames();
    Project.DiscoverSpecVersions();
    IsLoadTp = false;
    PanControl->PopulateSpecVerList(Project.SpecVerList); // This causes slotSelectSpec->LoadTestProcedure();
    qDebug() << "TrySetSpecVer" << Project.Cfg.TestSpecsVer;
    PanControl->TrySetSpecVer(Project.Cfg.TestSpecsVer);
    IsLoadTp = true;
    qDebug() << "Cfg.TestSpecsVer" << Project.Cfg.TestSpecsVer;
    //Project.CloseTestProcedure();
    //LoadTestProcedure();
    qDebug() << "Cfg.TestSpecsVer" << Project.Cfg.TestSpecsVer;
    Project.Cfg.ReportCurrent->Refresh();
    //SetState(tPetrelTT::St::TestProc);
}

void tPetrelTT::slotTestGroupChanged(const QString& groupName) {

    switch (State) {
    case St::ManualStopped:
        if (Project.TP != nullptr) {
            // Hide previous and Show new
            tTestDialog* pDialog = Project.TP->GetManualTestDialog(groupName);
            PanTestDialog->SetTestDialog(pDialog);
            PanTestDialog->SetCaption("Test " + groupName);
        }
        break;
        
    }
    qDebug() << "Petrel: Selected test" << groupName;


}