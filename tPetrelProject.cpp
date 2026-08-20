#include "tPetrelProject.h"
#include "tReportRoot.h"
#include <qdir.h>
#include <QObject.h>

tPetrelProject::tPetrelProject(tLogger &log)
    : Log(log) 
    , TPInfo(Log, Cfg)
    , TestRunner(Log)
{
    // Adding general params groups to general supergroup
    ParGrpProject = GeneralParamsSuperGroup.Add("Project");
    InitParGrpProjectConfig();
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
    if (TP != nullptr) {
        // signals TestRunner to TP
        QObject::disconnect(&TestRunner, &tTestRunner::sigRunTest, TP, &tTestProcedure::slotRunTest);
        QObject::disconnect(&TestRunner, &tTestRunner::sigInterruptTest, TP, &tTestProcedure::slotInterruptTest); // Runner -> Procedure <void>
        //QObject::disconnect(&TestRunner, &tTestRunner::sigFinishTest, TP, &tTestProcedure::slotFinishTest);

        // signals TP to TestRunner
        QObject::disconnect(TP, &tTestProcedure::sigSetTestInfo, &TestRunner, &tTestRunner::slotSetTestInfo);   // Procedure -> Runner <tTestInfo>
        QObject::disconnect(TP, &tTestProcedure::sigSetTestProgress, &TestRunner, &tTestRunner::slotSetTestProgress); // Procedure -> Runner <double>
        QObject::disconnect(TP, &tTestProcedure::sigSetTestTimeout, &TestRunner, &tTestRunner::slotSetTestTimeout); // Procedure -> Runner <double>
        QObject::disconnect(TP, &tTestProcedure::sigAddTestDetails, &TestRunner, &tTestRunner::slotAddTestDetails); // Procedure -> Runner <QString>

    }
    TPInfo.Clear();
    ClosePlugin();
}

void tPetrelProject::CreateTestProcedure() {
    if (TP != nullptr)
        CloseTestProcedure();
    
    if (!OpenPlugin()) return;

    typedef tTestProcedure*(*tTpFunc)(tLogger& log, tPetrelProjectConfig& cfg);
    tTpFunc func = (tTpFunc)PluginLib.resolve("CreateTestProcedure");
    if (func == nullptr) {
        Log.LogErrorMessage("Plug-in does not contain TestProcedure");
    } else {
        TP = func(Log, Cfg);
    }
    if (TP == nullptr) {
        Log.LogErrorMessage("TestProcedure is null");
        return;
    }
    BuildDirNames();
    LoadTestProcedure();
}

void tPetrelProject::ClosePlugin() {
    if (!IsPlugged) return;
    if ((TP != nullptr) && (TestRunner.GetRunningTest())) {
        TestRunner.CancelTests();
    }

    TestRunner.SetTP(nullptr);
    // Delete PanCfgs
    TP->DeletePanCfg();
    TP = nullptr;
    PluginLib.unload();
    IsPlugged = false;
}

void tPetrelProject::BuildDirNames() {
    Cfg.CurDutDir = "";
    Cfg.CurTestProcDir = "";
    if ((Cfg.TestProcDir != "") && (Cfg.DutName != "")) {
        Cfg.CurDutDir = Cfg.TestProcDir + "/" + Cfg.DutName + ".TestProcedure/";
        if (Cfg.TestSpecsVer != "")
            Cfg.TestProcRevDir = Cfg.CurDutDir + Cfg.TestSpecsVer + ".Version/";
    }
    Log.LogSystemMessage("Directories:");
    Log.LogSystemMessage("Test Procedures: " + Cfg.TestProcDir);
    Log.LogSystemMessage("Current DUT Test Procedures: " + Cfg.CurDutDir);
    Log.LogSystemMessage("Current DUT Rev Test Procedure: " + Cfg.CurTestProcDir);
}

bool tPetrelProject::LoadTestProcedure() {
    bool res = false;
    TP->SetValid(false);
    ReportsClear();
    
    if (!TPInfo.LoadAndValidate((tReport*)Cfg.ReportCurrent)) return false; // Load, and Check if current TestProcedure is good
    Cfg.TestProcedureVer = TPInfo.GetVersion();
    if (TPInfo.GetDeprecated())  return false;
    TestSpecs = TP->GetTestSpecs();
    if (TestSpecs == nullptr) {
        return false;
    }
    TestSpecs->LoadAndValidate(); // Check if current TestProcedure is good
    TestSpecs->SetReadonly(); // Write protected

    if (TPInfo.IsValid() && TestSpecs->IsValid()) {
        Cfg.TestProcedureVer = TPInfo.GetVersion();
        Cfg.TestSpecsVer = TestSpecs->sVersion;
        TP->SetValid(true);
        TP->AssignTestFunctions();
        TP->ValidateAutoTestFuncAssignment((tReport*)Cfg.ReportCurrent);

        if (TP->GetValid()) {
            TestRunner.SetTP(TP);

            // signals TestRunner to TP
            QObject::connect(&TestRunner, &tTestRunner::sigRunTest, TP, &tTestProcedure::slotRunTest, Qt::QueuedConnection);
            QObject::connect(&TestRunner, &tTestRunner::sigInterruptTest, TP, &tTestProcedure::slotInterruptTest, Qt::DirectConnection); // Runner -> Procedure <void>
            //QObject::connect(&TestRunner, &tTestRunner::sigFinishTest, TP, &tTestProcedure::slotFinishTest, Qt::QueuedConnection);

            // signals TP to TestRunner
            QObject::connect(TP, &tTestProcedure::sigSetTestInfo, &TestRunner, &tTestRunner::slotSetTestInfo, Qt::QueuedConnection);   // Procedure -> Runner <tTestInfo>
            QObject::connect(TP, &tTestProcedure::sigSetTestProgress, &TestRunner, &tTestRunner::slotSetTestProgress, Qt::QueuedConnection); // Procedure -> Runner <double>
            QObject::connect(TP, &tTestProcedure::sigSetTestTimeout, &TestRunner, &tTestRunner::slotSetTestTimeout, Qt::QueuedConnection); // Procedure -> Runner <double>
            QObject::connect(TP, &tTestProcedure::sigAddTestDetails, &TestRunner, &tTestRunner::slotAddTestDetails, Qt::QueuedConnection); // Procedure -> Runner <QString>

            Cfg.ReportAutoTest->SetName("Auto test : " + Cfg.DutName);
            Cfg.ReportManualTest->SetName("Manual test : " + Cfg.DutName);
        }
    }

    PopulateAutoTestTree();
    PopulateManualTestTree();

    res = (TP != 0) && (TP->GetValid());
    return (res);
}

//private 
void tPetrelProject::PopulateAutoTestTree() {
    AutoTestTree.setText(0, "Auto test text");// .Nodes.Add(ReportTestAuto.GetName());
    TP->GetTestSpecs()->BuildAutoTestTree(&AutoTestTree);
}

//private 
void tPetrelProject::PopulateManualTestTree() {
    ManualTestTree.setText(0, "Manual test text");// .Nodes.Add(ReportTestAuto.GetName());
    TP->GetTestSpecs()->BuildManualTestTree(&ManualTestTree);
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
    if (TP == nullptr) return false;
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

    TP->ResetCancelTestingFlag();
    TP->ClearAllTestsInfo();
    LinearReports.clear();
    
    BuildReportsList(LinearReports, manRep);

    QApplication::processEvents();
    if (TestRunner.InterruptFlag)
        StopTests();
    else
        RunManualTests();
    return true;
}

// Test controls
void tPetrelProject::StartAutoTests() {
    if (TP == nullptr) return;
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
    if (TP->InitAutoTests(Cfg.ReportAutoTest)) {
        repInitAuto->SetStatus(tTestStatus::Passed, autoTestInitDetails);
        TestSpecs->BuildTestReport(Cfg.ReportAutoTest, "Auto test: " + Cfg.DutName, false, true, false);
        Cfg.ReportCurrent->Refresh();

        TP->ResetCancelTestingFlag();
        TP->ClearAllTestsInfo();
        LinearReports.clear();
        BuildReportsList(LinearReports, Cfg.ReportCurrent);

        // Colour test tree
        ColorizeTestTree(&AutoTestTree);

        //Application.DoEvents();
        QApplication::processEvents();
        if (TestRunner.InterruptFlag)
            StopTests();
        else
            RunAutoTests();
    } else {
        repInitAuto->SetStatus(tTestStatus::TestError, autoTestInitDetails);
        StopTests();
    }
}

void tPetrelProject::RunAutoTests() {
    for (tReport* test : LinearReports) {
        if (TP->IsTestFunctionAssigned(test->GetName().toUpper())) { // Skip pure groups
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
        if (TP->IsTestFunctionAssigned(test->GetName().toUpper())) { // Skip pure groups
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
    if (TP != nullptr) {
        TP->slotInterruptTest();
    }
}

bool tPetrelProject::InitManualTest() {
    if (TP == nullptr) return false;
    bool res = TP->InitManualTests(Cfg.ReportManualTest);
    tTestStatus status = Cfg.ReportManualTest->GetStatus();
    bool expand = (status != tTestStatus::Passed);
    Cfg.ReportManualTest->Expand(status != tTestStatus::Passed, true);
    return res;
}

void tPetrelProject::DoneManualTest() {
    if (TP == nullptr) return;
    TP->DoneManualTests(Cfg.ReportManualTest);
}
void tPetrelProject::DoneAutoTest() {
    if (TP == nullptr) return;
    TP->DoneAutoTests(Cfg.ReportAutoTest);
}


void tPetrelProject::SetCurrentGroupName(const QString& groupName) {
    CurrentGroupName = groupName;
}

QString tPetrelProject::GetCurrentGroupName() const {
    return CurrentGroupName;
}
