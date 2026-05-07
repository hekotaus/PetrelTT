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
    //ParGrpLayerView = GeneralParamsSuperGroup.Add("Layer view");
    //ParGrpSim = GeneralParamsSuperGroup.Add("Sim");
    //ParGrpCli = GeneralParamsSuperGroup.Add("Cli");

    InitParGrpProjectConfig();
//    InitParGrpTrackView();
//    InitParGrpTrackStat();

}

void tPetrelProject::InitParGrpProjectConfig() {
    ParGrpProject->AddParam(new tParam_QString("Operator", Cfg.OperatorName));
    ParGrpProject->AddParam(new tParam_QStringList("OperatorList", Cfg.OperatorList));
    ParGrpProject->AddParam(new tParam_QString("DutName", Cfg.DutName));
    ParGrpProject->AddParam(new tParam_QString("TestSpecVer", Cfg.TestSpecsVer));
}

void tPetrelProject::FindTestProcedures() { // return list of DUTs TestProcList
    Log.LogSystemMessage("Searching test procedures in " + Cfg.TestProcDir + " ...");
    TestProcList.clear();
    QDir dir(Cfg.TestProcDir);
    TestProcList = dir.entryList(QStringList() << "*.TestProcedure", QDir::Dirs);
    int nlast = QString(".TestProcedure").length();
    for (QString& s : TestProcList) {
        s.chop(nlast);
        Log.LogSystemMessage("Found: " + s);
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
            PluginList[dutName] = fname; // if duplicates, picking the last fond file
        }
        lib.unload();
    }
}

void tPetrelProject::DiscoverTestProcedures() {
    FindPlugins();
    FindTestProcedures();
    // Match TestProcs and Plugins
    DutNameList.clear();
    for (QString& sTP : TestProcList) {
        if (PluginList.count(sTP) > 0) {
            DutNameList.append(sTP);
            Log.LogSystemMessage("Added DUT: " + sTP);
        } else {
            Log.LogSystemMessage("Plugin not found for DUT: " + sTP);
        }
    }
}

void tPetrelProject::DiscoverSpecVersions() {
    //Cfg.CurDutDir = Cfg.TestProcDir + "/" + Cfg.DutName + ".TestProcedure";
    Log.LogSystemMessage("Searching spec versions in " + Cfg.CurDutDir + " ...");
    SpecVerList.clear();

    QDir dir(Cfg.CurDutDir);
    SpecVerList = dir.entryList(QStringList() << "*.Version", QDir::Dirs);
    int nlast = QString(".Version").length();
    for (QString& s : SpecVerList) {
        s.chop(nlast);
        Log.LogSystemMessage("Found: " + s);
    }
    //Cfg.CurTestProcDir =
}

bool tPetrelProject::OpenPlugin() {
    if (IsPlugged) ClosePlugin();
    if (0 == PluginList.count(Cfg.DutName)) return IsPlugged;
    QString fname = PluginList.at(Cfg.DutName);
    PluginLib.setFileName(Cfg.PluginDir + "/" + fname);
    PluginLib.load();
    IsPlugged = PluginLib.isLoaded();
    return IsPlugged;
}

void tPetrelProject::CloseTestProcedure() {
    // signals TestRunner to TP
    QObject::disconnect(&TestRunner, SIGNAL(sigRunTest()), TP, SLOT(slotRunTest()));
    QObject::disconnect(&TestRunner, SIGNAL(sigInterruptTest()), TP, SLOT(slotInterruptTest())); // Runner -> Procedure <void>
    // signals TP to TestRunner
    QObject::disconnect(TP, SIGNAL(sigSetTestInfo(tTestInfo*)), &TestRunner, SLOT(slotSetTestInfo(tTestInfo*)));   // Procedure -> Runner <tTestInfo>
    QObject::disconnect(TP, SIGNAL(sigSetTestProgress(double)), &TestRunner, SLOT(slotSetTestProgress(double))); // Procedure -> Runner <double>
    QObject::disconnect(TP, SIGNAL(sigSetTestTimeout(double)), &TestRunner, SLOT(slotSetTestTimeout(double))); // Procedure -> Runner <double>
    QObject::disconnect(TP, SIGNAL(sigAddTestDetails(const QString&)), &TestRunner, SLOT(slotAddTestDetails(const QString&))); // Procedure -> Runner <QString>

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

#if 0
void* tPetrelProject::GetPluginObject(const QString &objName) {
    typedef char* (*tDutNameFunc)();
    tDutNameFunc func = (tDutNameFunc)PluginLib.resolve(objName);
    if (func != nullptr) {
        QString dutName = func();
    }
}
#endif
void tPetrelProject::ClosePlugin() {
    if (!IsPlugged) return;
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
        Cfg.CurDutDir = Cfg.TestProcDir + "/" + Cfg.DutName + ".testProcedure/";
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
        ///////////////////////////////////
        // Add new Test Procedures here  //
        ///////////////////////////////////
        TP->SetValid(true);
        TP->AssignTestFunctions();
        TP->ValidateAutoTestFuncAssignment((tReport*)Cfg.ReportCurrent);
#if 0
        ManualTestSpecs.ValidateManual(); // Check if current TestProcedure is good
#endif

        ///if (TP->GetValid() && ManualTestSpecs.IsValid()) {
        if (TP->GetValid()) {
            ///TestRunner = new TTestRunner(Log, TestProgress, TestProcedure);
            

            TestRunner.SetTP(TP);

            // signals TestRunner to TP
            QObject::connect(&TestRunner, SIGNAL(sigRunTest()), TP, SLOT(slotRunTest()));
            QObject::connect(&TestRunner, SIGNAL(sigInterruptTest()), TP, SLOT(slotInterruptTest())); // Runner -> Procedure <void>
            QObject::connect(&TestRunner, SIGNAL(sigFinishTest()), TP, SLOT(slotFinishTest()));

            // signals TP to TestRunner
            QObject::connect(TP, SIGNAL(sigSetTestInfo(tTestInfo*)), &TestRunner, SLOT(slotSetTestInfo(tTestInfo*)));   // Procedure -> Runner <tTestInfo>
            QObject::connect(TP, SIGNAL(sigSetTestProgress(double)), &TestRunner, SLOT(slotSetTestProgress(double))); // Procedure -> Runner <double>
            QObject::connect(TP, SIGNAL(sigSetTestTimeout(double)), &TestRunner, SLOT(slotSetTestTimeout(double))); // Procedure -> Runner <double>
            QObject::connect(TP, SIGNAL(sigAddTestDetails(const QString&)), &TestRunner, SLOT(slotAddTestDetails(const QString&))); // Procedure -> Runner <QString>


                ///TestProcedure.SetMainSignalQueue(TestRunner.SignalQueue);
                //SwitchTestTab(TestTabs.SelectedIndex); // All good, switch to the current Tab state
                ///SetState(TAppState.TestProcedureValid);
                ///Cfg.TestProcedureValid = true;
            Cfg.ReportAutoTest->SetName("Auto test : " + Cfg.DutName);
            Cfg.ReportManualTest->SetName("Manual test : " + Cfg.DutName);

                // Scan SN files
#if 0
                TestProcedure.SN = new SerialNumbers(Cfg.SnDir, Cfg.SnDbFileName);
                TestProcedure.SN.SetDeviceType(Cfg.DeviceType, Cfg.DeviceRevision);
                PopulateSnList();
#endif
                ////TestProcedure.TestProcInfo.TestProcDir = Cfg.TestProcDir;
                ////TestProcedure.TestProcInfo.WorkingDir = Cfg.WorkingDir;
                ////TestProcedure.TestProcInfo.TempDir = Cfg.TmpDir;
                // It's a question, if we really need this...
            }
//#endif
    }

    ///ImportConfigDialog();
    ///BuildCaption();

    // Populate Auto tab: Auto TestTree
    PopulateAutoTestTree();
    PopulateManualTestTree();
    //TestSpecs.BuildAutoTestReport(ReportTestAuto);
    return res;
}

//private 
void tPetrelProject::PopulateAutoTestTree() {
    //AutoTestTree.Clear();
    //AutoTestTree.Nodes.Add("Auto test: " + Cfg.DeviceType);
    AutoTestTree.setText(0, "Auto test text");// .Nodes.Add(ReportTestAuto.GetName());
    TP->GetTestSpecs()->BuildAutoTestTree(&AutoTestTree);
}

//private 
void tPetrelProject::PopulateManualTestTree() {
    ManualTestTree.setText(0, "Manual test text");// .Nodes.Add(ReportTestAuto.GetName());
    TP->GetTestSpecs()->BuildManualTestTree(&ManualTestTree);

    //    ManualTestTree.BeginUpdate();
//    ManualTestTree.Nodes.Clear();
//    //ManualTestTree.Nodes.Add("Manual test: " + Cfg.DeviceType);
//    ManualTestTree.Nodes.Add(ReportTestManual.GetName());
//    ManualTestSpecs.BuildManualTree(ManualTestTree.Nodes[0]);
//    ManualTestTree.EndUpdate();
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
    //while (startNode != nullptr) {
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
    //}
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
}

// Test controls
void tPetrelProject::StartAutoTests() {
    if (TP == nullptr) return;
    TestRunner.InterruptFlag = false;
    TestRunner.SetCurTestTree(&AutoTestTree);
    //SetState(TAppState.AutoStart);
    ResetDeviceInfo();
    LinearReports.clear();
    Cfg.ReportCurrent->Clear();
    Cfg.ReportCurrent->AutoScroll = true;
    QString autoTestInitDetails;
    DecolorizeTestTree(&AutoTestTree); // Uncolor Auto test tree

    tReport* repInitAuto = Cfg.ReportCurrent->AddReport("Init Auto Tests");
    if (TP->InitAutoTests(autoTestInitDetails)) {
        repInitAuto->SetStatus(tTestStatus::Passed, autoTestInitDetails);
        TestSpecs->BuildTestReport(Cfg.ReportAutoTest, "Auto test: " + Cfg.DutName, false);
        Cfg.ReportCurrent->Refresh();

        TP->ResetCancelTestingFlag();
        TP->ClearAllTestsInfo();
        //Cfg.SN.GetSn();
        //TestProcedure.SetSerialNumber(Cfg.SN.GetSnInfo());
        //LinearReports.clear();
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
            TP->ResetTest(test->GetName(), "Auto"); // Erase old specs, remove old results
            TestRunner.CurrentTestReport = test; // For Signals processing
            ///TestRunner.CurrentTestReport = ;
            if (!TestRunner.InterruptFlag) {
                TestRunner.RunTest();
            }
        }
    } // while
    
    ///StopTests();
    Cfg.ReportCurrent->Refresh();

}

void tPetrelProject::StopTests() {
    if (TP != nullptr) {
        TP->slotInterruptTest();
    }
}

bool tPetrelProject::InitManualTest() {
    if (TP == nullptr) return false;
    return TP->InitManualTest();
}

//bool tPetrelProject::InitAutoTest() {
//    if (TP == nullptr) return false;
//    return TP->InitAutoTests();
//}

void tPetrelProject::DoneManualTest() {
    if (TP == nullptr) return;
    TP->DoneManualTest();
}
void tPetrelProject::DoneAutoTest() {
    if (TP == nullptr) return;
    TP->DoneAutoTests();
}

