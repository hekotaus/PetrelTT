#include "tPetrelProject.h"
#include "tReportRoot.h"
#include "tTestProcedure.h"
#include <qdir.h>
#include <QObject.h>

tPetrelProject* gProject = nullptr;
bool CB_AssignTestFunction(const QString& testName, tTestDelegate testProc) {
    gProject->TPInfo.AssignTestFunction(testName, testProc);
    return true;
}

bool CB_AssignManualDialog(const QString& groupName, tTestDialog* pDialog) {
    bool res = gProject->TPInfo.TestSpecs.AssignManualDialog(groupName, pDialog);
    return res;
}

tTestDialog* CB_GetManualDialog(const QString& groupName) {
    tTestDialog* res = gProject->TPInfo.TestSpecs.GetManualDialog(groupName);
    return res;
}

bool CB_InitAutoTests() {
    return gProject->TPInfo.InitDptAndDut(gProject->Cfg.ReportAutoTest);
}

bool CB_InitManualTests() {
    return gProject->TPInfo.InitDptAndDut(gProject->Cfg.ReportManualTest);
}

void CB_DoneAutoTests() {
    gProject->TPInfo.DoneDutAndDpt(gProject->Cfg.ReportAutoTest);
}

void CB_DoneManualTests() {
    gProject->TPInfo.DoneDutAndDpt(gProject->Cfg.ReportManualTest);
}

void tPetrelProject::AssignCallbacks() {
    TPInfo.TP->cbAssignTestFunction = CB_AssignTestFunction; // TPInfo.AssignTestFunction;
    TPInfo.TP->cbAssignManualDialog = CB_AssignManualDialog;
    TPInfo.TP->cbGetManualDialog = CB_GetManualDialog;

    TPInfo.TP->cbInitAutoTests = CB_InitAutoTests;
    TPInfo.TP->cbInitManualTests = CB_InitManualTests;
    TPInfo.TP->cbDoneAutoTests = CB_DoneAutoTests;
    TPInfo.TP->cbDoneManualTests = CB_DoneManualTests;

}

tPetrelProject::tPetrelProject(tLogger &log)
    : Log(log) 
    , TPInfo(Log, Cfg)
    , TestRunner(TPInfo, Log)
{
    // Adding general params groups to general supergroup
    ParGrpProject = GeneralParamsSuperGroup.Add("Project");
    InitParGrpProjectConfig();
    gProject = this;
}

void tPetrelProject::InitParGrpProjectConfig() {
    ParGrpProject->AddParam(new tParam_QString("Operator", Cfg.OperatorName));
    ParGrpProject->AddParam(new tParam_QStringList("OperatorList", Cfg.OperatorList));
    ParGrpProject->AddParam(new tParam_QString("DutName", Cfg.DutName));
    ParGrpProject->AddParam(new tParam_QString("TestSpecVer", Cfg.TestSpecsVer));
}
#if 0
void tPetrelProject::FindTestProcedures() { // return list of DUTs TestProcList
    Log.LogSystemMessage("Searching test procedures in " + Cfg.TestProcDir + " ...");
    TestProcList.clear();
    QDir dir(Cfg.TestProcDir);
    TestProcList = dir.entryList(QStringList() << "*.TestProcedure", QDir::Dirs);
    constexpr int nlast = sizeof(".TestProcedure") - 1;
    for (QString& s : TestProcList) {
        s.chop(nlast);
        Log.LogSystemMessage("Found Test Procedure directory: " + s);
    }
}

void tPetrelProject::FindPlugins() {
    Log.LogSystemMessage("Searching plug-ins in " + Cfg.PluginDir + " ...");
    PluginList.clear();
    QDir dir(Cfg.PluginDir);
    auto flist = dir.entryList(QStringList() << "*.dll", QDir::Files);
    for (const auto& fname : flist) {
        QString fullName = Cfg.PluginDir + "/" + fname;
        Log.LogSystemMessage("Found file " + fullName);
        QLibrary lib(fullName);
        lib.load();
        if (!lib.isLoaded()) {
            Log.LogSystemMessage("Failed to load dll");
            continue;
        }

        typedef char* (*tDutNameFunc)();
        tDutNameFunc func = (tDutNameFunc) lib.resolve("GetTestProcName");
        if (func != nullptr) {
            QString dutName = func();
            Log.LogSystemMessage("Found: " + dutName + " in " + fname);
            PluginList[dutName] = fname; // if duplicates, picking the last found file
        }
        lib.unload();
    }
}
#endif
void tPetrelProject::DiscoverTestProcedures() {
#if 0
    // OLD

    FindPlugins();
    FindTestProcedures();
    // Match TestProcs and Plugins
    DutNameList.clear();
    for (QString& sTP : TestProcList) {
        if (PluginList.count(sTP) > 0) {
            DutNameList.append(sTP);
            Log.LogSystemMessage("Added DUT: " + sTP);
        } else {
            Log.LogSystemMessage("Plugin not found for DUT: " + sTP + " TestProcName mismatch");
        }
    }
#else
    // NEW
    QStringList testProcList;
    PluginList.clear(); // <DutName, FileName>
    ///TestProcList.clear();
    // Find TP dirs, containing plugin dlls
    Log.LogSystemMessage("Searching test procedures in " + Cfg.TestProcDir + " ...");
    
    QDir dirPetrel(Cfg.TestProcDir);
    testProcList = dirPetrel.entryList(QStringList() << "*.TestProcedure", QDir::Dirs);
    constexpr int nlast = sizeof(".TestProcedure") - 1;
    for (QString& tpName : testProcList) {
        tpName.chop(nlast);
        Log.LogSystemMessage("Found Test Procedure directory: " + tpName);
        // Check, if same name .dll exists and it has TP functions exported
        
        QString fullDllName = Cfg.PluginDir + "/" + tpName + ".TestProcedure/" + tpName + ".dll";
        Log.LogSystemMessage("Checking file " + fullDllName);
        QLibrary lib(fullDllName);
        lib.load();
        if (!lib.isLoaded()) {
            Log.LogSystemMessage("Failed to load dll");
            continue;
        }

        typedef char* (*tDutNameFunc)();
        tDutNameFunc func = (tDutNameFunc) lib.resolve("GetTestProcName");
        if (func != nullptr) {
            QString dutName = func();
            Log.LogSystemMessage("Found: " + dutName + " in " + fullDllName);
            PluginList[dutName] = fullDllName;
            DutNameList.append(tpName);
        }
        lib.unload();

    }

#endif
}

void tPetrelProject::DiscoverSpecVersions() {
    Log.LogSystemMessage("Searching spec versions in " + Cfg.CurDutDir + " ...");
    SpecVerList.clear();

    QDir dir(Cfg.CurDutDir);
    SpecVerList = dir.entryList(QStringList() << "*.Version", QDir::Dirs);
    int nlast = QString(".Version").length();
    for (QString& s : SpecVerList) {
        s.chop(nlast);
        Log.LogSystemMessage("Found: " + s);
    }
}

bool tPetrelProject::OpenPlugin() {
    if (IsPlugged) ClosePlugin();
    if (0 == PluginList.count(Cfg.DutName)) return IsPlugged;
    QString fname = PluginList.at(Cfg.DutName);
    ///PluginLib.setFileName(Cfg.PluginDir + "/" + fname);
    PluginLib.setFileName(fname);
    PluginLib.load();
    IsPlugged = PluginLib.isLoaded();
    return IsPlugged;
}

void tPetrelProject::CloseTestProcedure() {
    // signals TestRunner to TP
    if (TPInfo.TP != nullptr) {
        // signals TestRunner to TP
        QObject::disconnect(&TestRunner, &tTestRunner::sigRunTest, TPInfo.TP, &tTestProcedure::slotRunTest);
        QObject::disconnect(&TestRunner, &tTestRunner::sigInterruptTest, TPInfo.TP, &tTestProcedure::slotInterruptTest); // Runner -> Procedure <void>
        //QObject::disconnect(&TestRunner, &tTestRunner::sigFinishTest, TP, &tTestProcedure::slotFinishTest);

        // signals TP to TestRunner
        QObject::disconnect(TPInfo.TP, &tTestProcedure::sigSetTestInfo, &TestRunner, &tTestRunner::slotSetTestInfo);   // Procedure -> Runner <tTestInfo>
        QObject::disconnect(TPInfo.TP, &tTestProcedure::sigSetTestProgress, &TestRunner, &tTestRunner::slotSetTestProgress); // Procedure -> Runner <double>
        QObject::disconnect(TPInfo.TP, &tTestProcedure::sigSetTestTimeout, &TestRunner, &tTestRunner::slotSetTestTimeout); // Procedure -> Runner <double>
        QObject::disconnect(TPInfo.TP, &tTestProcedure::sigAddTestDetails, &TestRunner, &tTestRunner::slotAddTestDetails); // Procedure -> Runner <QString>
    }
    TPInfo.Clear();
    ClosePlugin();
}

void tPetrelProject::CreateTestProcedure() {
    if (TPInfo.TP != nullptr)
        CloseTestProcedure();
    
    if (!OpenPlugin()) return;

    typedef tTestProcedure*(*tTpFunc)(tLogger& log, tPetrelProjectConfig& cfg, tTestProcInfo* tpinfo);
    tTpFunc func = (tTpFunc)PluginLib.resolve("CreateTestProcedure");
    if (func == nullptr) {
        Log.LogErrorMessage("Plug-in does not contain TestProcedure");
    } else {
        TPInfo.TP = func(Log, Cfg, &TPInfo);
    }
    if (TPInfo.TP == nullptr) {
        Log.LogErrorMessage("TestProcedure is null");
        return;
    }
    BuildDirNames();
    LoadTestProcedure();
}

void tPetrelProject::ClosePlugin() {
    if (!IsPlugged) return;
    if ((TPInfo.TP != nullptr) && (TestRunner.GetRunningTest())) {
        TestRunner.CancelTests();
    }

    TestRunner.SetTP(nullptr);
    if (TPInfo.TP != nullptr) {
        // Delete PanCfgs
        TPInfo.TP->DeletePanCfg();
        
        // TP's vtable lives in the dll, so it must be destroyed before unload.
        // Otherwise it outlives the dll, and when a receiver it is still connected to
        // is destroyed (e.g. TestRunner on exit), ~QObject calls TP->metaObject() and
        // TP->disconnectNotify() through the unloaded vtable -> access violation.
        delete TPInfo.TP;
        TPInfo.TP = nullptr;
    }
    TestSpecs = nullptr; // Pointed into TP
    PluginLib.unload();
    IsPlugged = false;
}

void tPetrelProject::BuildDirNames() {
    Cfg.CurDutDir = "";
    Cfg.CurTestProcDir = "";
    if ((Cfg.TestProcDir != "") && (Cfg.DutName != "")) {
        Cfg.CurDutDir = Cfg.TestProcDir + "/" + Cfg.DutName + ".TestProcedure/";
        //if (Cfg.TestSpecsVer != "")
        Cfg.TestProcRevDir = Cfg.CurDutDir + Cfg.TestSpecsVer + ".Version/";
    }
    Log.LogSystemMessage("Directories:");
    Log.LogSystemMessage("Test Procedures: " + Cfg.TestProcDir);
    Log.LogSystemMessage("Current DUT Test Procedures: " + Cfg.CurDutDir);
    Log.LogSystemMessage("Current DUT Rev Test Procedure: " + Cfg.CurTestProcDir);
}

bool tPetrelProject::LoadTestProcedure() {
    bool res = false;
    TPInfo.SetValid(false);
    ReportsClear();

    if (!TPInfo.LoadAndValidate((tReport*)Cfg.ReportCurrent)) return false; // Load, and Check if current TestProcedure is good
    Cfg.TestProcedureVer = TPInfo.GetVersion();
    if (TPInfo.GetDeprecated())  return false;
    TestSpecs = TPInfo.GetTestSpecs();
    if (TestSpecs == nullptr) {
        return false;
    }
    if (!TestSpecs->LoadAndValidate()) // Check if current TestProcedure is good
        return false;
    TestSpecs->SetReadonly(); // Write protected

    Cfg.TestProcedureVer = TPInfo.GetVersion();
    Cfg.TestSpecsVer = TestSpecs->sVersion;
    AssignCallbacks();
    TPInfo.TP->AssignTestFunctions();
    TPInfo.ValidateAutoTestFuncAssignment((tReport*)Cfg.ReportCurrent);

    if (TPInfo.GetValid()) {
        TestRunner.SetTP(TPInfo.TP);

        // signals TestRunner to TP
        QObject::connect(&TestRunner, &tTestRunner::sigRunTest, TPInfo.TP, &tTestProcedure::slotRunTest, Qt::QueuedConnection);
        QObject::connect(&TestRunner, &tTestRunner::sigInterruptTest, TPInfo.TP, &tTestProcedure::slotInterruptTest, Qt::DirectConnection); // Runner -> Procedure <void>
        //QObject::connect(&TestRunner, &tTestRunner::sigFinishTest, TP, &tTestProcedure::slotFinishTest, Qt::QueuedConnection);

        // signals TP to TestRunner
        QObject::connect(TPInfo.TP, &tTestProcedure::sigSetTestInfo, &TestRunner, &tTestRunner::slotSetTestInfo, Qt::QueuedConnection);   // Procedure -> Runner <tTestInfo>
        QObject::connect(TPInfo.TP, &tTestProcedure::sigSetTestProgress, &TestRunner, &tTestRunner::slotSetTestProgress, Qt::QueuedConnection); // Procedure -> Runner <double>
        QObject::connect(TPInfo.TP, &tTestProcedure::sigSetTestTimeout, &TestRunner, &tTestRunner::slotSetTestTimeout, Qt::QueuedConnection); // Procedure -> Runner <double>
        QObject::connect(TPInfo.TP, &tTestProcedure::sigAddTestDetails, &TestRunner, &tTestRunner::slotAddTestDetails, Qt::QueuedConnection); // Procedure -> Runner <QString>

        QObject::connect(TPInfo.TP, &tTestProcedure::sigCancelTesting, &TestRunner, &tTestRunner::slotCancelTesting, Qt::QueuedConnection); // Procedure -> Runner <QString>
        QObject::connect(TPInfo.TP, &tTestProcedure::sigCancelSubtests, &TestRunner, &tTestRunner::slotCancelSubtests, Qt::QueuedConnection); // Procedure -> Runner <QString>

        Cfg.ReportAutoTest->SetName("Auto test : " + Cfg.DutName);
        Cfg.ReportManualTest->SetName("Manual test : " + Cfg.DutName);
    }

    PopulateAutoTestTree();
    PopulateManualTestTree();

    res = (TPInfo.TP != 0) && (TPInfo.GetValid());
    return (res);
}

//private 
void tPetrelProject::PopulateAutoTestTree() {
    AutoTestTree.setText(0, "Auto test text");// .Nodes.Add(ReportTestAuto.GetName());
    TPInfo.GetTestSpecs()->BuildAutoTestTree(&AutoTestTree);
}

//private 
void tPetrelProject::PopulateManualTestTree() {
    ManualTestTree.setText(0, "Manual test text");// .Nodes.Add(ReportTestAuto.GetName());
    TPInfo.GetTestSpecs()->BuildManualTestTree(&ManualTestTree);
}

void tPetrelProject::ReportsClear() {
    Cfg.ReportAutoTest->Clear();
    Cfg.ReportManualTest->Clear();
    Cfg.ReportConfig->Clear();
    Cfg.ReportTestProc->Clear();
    Cfg.ReportReports->Clear();
}

void tPetrelProject::ExpandTestTree(QTreeWidgetItem* treeNode, bool expand) {
    treeNode->setExpanded(expand);
    for (int i = 0; i < treeNode->childCount(); i++)
        ExpandTestTree(treeNode->child(i), expand);
}

QTreeWidgetItem* tPetrelProject::SearchNode(const QString& searchText, QTreeWidgetItem* startNode) { // shamefully copypasted from StackOverflow
    QTreeWidgetItem* node = nullptr;
    if (startNode->text(0).toLower() == searchText) {
        node = startNode;
        return node;
    }
    for (int i = 0; i < startNode->childCount(); i++) {
        node = SearchNode(searchText, startNode->child(i));//Recursive Search
        if (node != nullptr) {
            return node;
        }
    }
    return node;
}

void tPetrelProject::ColorizeTestTree(QTreeWidgetItem* rootNode) {
    for (tReport* curTestRep : LinearReports) {
        qDebug() << "Colourising" << curTestRep->GetName();
        QTreeWidgetItem* curNode = SearchNode(curTestRep->GetName().toLower(), rootNode);
        if (curNode != nullptr)
            curNode->setForeground(0, QBrush(curTestRep->GetTestColor()));
    }
}

void tPetrelProject::DecolorizeTestTree(QTreeWidgetItem* curNode) {
    curNode->setForeground(0, QBrush(tReport::GetStatusColor(tTestStatus::None)));
    for (int i = 0; i < curNode->childCount(); i++) {
        QTreeWidgetItem* ch = curNode->child(i);
        DecolorizeTestTree(ch);
    }
}

void tPetrelProject::BuildReportsList(std::list<tReport*>& repList, tReport* report) {
    
    if (report->GetStatus() == tTestStatus::Pending) { // to avoid adding "Init Auto test, etc.)
        ///if (TP->IsTestFunctionAssigned(report->GetName().toUpper())) { // Skip pure groups
            qDebug() << "Add rep to list" << report->GetName();
            repList.push_back(report);
        ///}
    }
    for (tReport& ch : report->Children) {
        BuildReportsList(repList, &ch);
    }
    //qDebug() << "Linear report size is" << repList.size();
}

bool tPetrelProject::StartManualTest(const QString & testName) {
    if (TPInfo.TP == nullptr) return false;
    TestRunner.InterruptFlag = false;
    TestRunner.CancelTestsFlag = false;
    TestRunner.SetCurTestTree(&ManualTestTree);

    ResetDeviceInfo();
    Cfg.ReportCurrent->AutoScroll = true;
    QString manTestInitDetails;

    tTestSpec* spec = TestSpecs->GetSpec(testName);
    if (spec == nullptr) {
        Log.LogErrorMessage("Specs for test " + testName + " not found!");
        return false;
    }
    tReport* manRep = spec->BuildTestReport(Cfg.ReportManualTest, true, false, true);

    ///TP->ResetCancelTestingFlag();
    TPInfo.ClearAllTestsInfo();
    LinearReports.clear();
    
    BuildReportsList(LinearReports, manRep);

    QApplication::processEvents();
    if (TestRunner.InterruptFlag)
        StopTests();
    else
        RunManualTests();
    return true;
}
#include "tSerialNumberDialog.h"
void tPetrelProject::RequestSn(QString sn) { // From TP
    int result = 0;
    if (sn.isEmpty()) {
        //int result = msgBox->exec();
        tSerialNumberDialog snDlg;
        result = snDlg.GetSerialNumber(nullptr, sn);
    }
    SetSerialNumber(sn);
    //emit sigMessageResult(result); // In case setting SN is a test
}


// Test controls
void tPetrelProject::StartAutoTests() {
    if (TPInfo.TP == nullptr) return;
    TestRunner.InterruptFlag = false;
    TestRunner.CancelTestsFlag = false;
    TestRunner.SetCurTestTree(&AutoTestTree);
    ResetDeviceInfo();
    LinearReports.clear();
    Cfg.ReportCurrent->Clear();
    Cfg.ReportCurrent->AutoScroll = true;
    QString autoTestInitDetails;
    DecolorizeTestTree(&AutoTestTree); // Uncolor Auto test tree

    tReport* repInitAuto = Cfg.ReportCurrent->AddReport("Init Auto Tests");
    if (TPInfo.TP->InitAutoTests()) {
        repInitAuto->SetStatus(tTestStatus::Passed, autoTestInitDetails);
        TestSpecs->BuildTestReport(Cfg.ReportAutoTest, "Auto test: " + Cfg.DutName, false, true, false);
        Cfg.ReportCurrent->Refresh();

        ///TP->ResetCancelTestingFlag();
        TPInfo.ClearAllTestsInfo();
        LinearReports.clear();
        BuildReportsList(LinearReports, Cfg.ReportCurrent);

        // Colour test tree
        ColorizeTestTree(&AutoTestTree);

        //Application.DoEvents();
        QApplication::processEvents();
        if (TestRunner.InterruptFlag)
            StopTests();
        else {
            RequestSn("");
            RunAutoTests();
        }
    } else {
        repInitAuto->SetStatus(tTestStatus::TestError, autoTestInitDetails);
        StopTests();
    }
}

void tPetrelProject::RunAutoTests() {
    for (tReport* test : LinearReports) {
        if (TPInfo.IsTestFunctionAssigned(test->GetName().toUpper())) { // Skip pure groups
            ///TP->ResetTest(test->GetName(), "Auto"); // Erase old specs, remove old results
            TestRunner.CurrentTestReport = test; // For Signals processing
            if (!TestRunner.InterruptFlag) {
                qDebug() << "Test runner starts test" << TestRunner.CurrentTestReport->GetName();
                TestRunner.RunTest();
            }
        }
    }
    Cfg.ReportCurrent->Refresh();
}

// So far (22/05/2026), it's the same as RunAutoTests, so may not be needed
void tPetrelProject::RunManualTests() {
    for (tReport* test : LinearReports) {
        if (TPInfo.IsTestFunctionAssigned(test->GetName().toUpper())) { // Skip pure groups
            ///TP->ResetTest(test->GetName(), test->GetName()); // Erase old specs, remove old results
            TestRunner.CurrentTestReport = test; // For Signals processing
            if (!TestRunner.InterruptFlag) {
                TestRunner.RunTest();
            }
        }
    }
    Cfg.ReportCurrent->Refresh();
}

void tPetrelProject::StopTests() {
    TestRunner.CancelTests();
    ///if (TP != nullptr) {
        ///TP->slotInterruptTest();
    ///}
}

bool tPetrelProject::InitManualTest() {
    if (TPInfo.TP == nullptr) return false;
    bool res = TPInfo.TP->InitManualTests();
    tTestStatus status = Cfg.ReportManualTest->GetStatus();
    bool expand = (status != tTestStatus::Passed);
    Cfg.ReportManualTest->Expand(status != tTestStatus::Passed, true);
    return res;
}

void tPetrelProject::DoneManualTest() {
    if (TPInfo.TP == nullptr) return;
    TPInfo.TP->DoneManualTests();
}
void tPetrelProject::DoneAutoTest() {
    if (TPInfo.TP == nullptr) return;
    TPInfo.TP->DoneAutoTests();
}

void tPetrelProject::SetCurrentGroupName(const QString& groupName) {
    CurrentGroupName = groupName;
}

QString tPetrelProject::GetCurrentGroupName() const {
    return CurrentGroupName;
}

void tPetrelProject::SetSerialNumber(QString sn) {
    Cfg.DutSerialNumber = sn;
}
