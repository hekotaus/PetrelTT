#include "tTestProcedure.h"
#include "common/VedroLibTools.h"
#include<qthread.h>
#include <exception>

tTestProcedure::tTestProcedure(
    QString name, QString dutName, QString dptName,
    tLogger& log, tPetrelProjectConfig& cfg)//,
    : TestProcName(name)
    , DutName(dutName)
    , DptName(dptName)
    , Log(log)
    , Cfg(cfg)
    , TestSpecs(tTestSpecs(Log, Cfg))
{
    //connect(this, &tTestProcedure::sigMessageResult, &MessageBoxWaiterLoop, &QEventLoop::quit);
    //connect(&MessageBoxWaiterTimer, &QTimer::timeout, &MessageBoxWaiterLoop, &QEventLoop::quit);
}

tTestProcedure::~tTestProcedure() { 
    //disconnect(this, &tTestProcedure::sigMessageResult, &MessageBoxWaiterLoop, &QEventLoop::quit);
    //disconnect(&MessageBoxWaiterTimer, &QTimer::timeout, &MessageBoxWaiterLoop, &QEventLoop::quit);
    //delete TestProcInfo; 
}

void tTestProcedure::DeletePanCfg() {
    if (PanDutCfg != nullptr) delete PanDutCfg;
    PanDutCfg = nullptr;

    if (PanDptCfg != nullptr) delete PanDptCfg;
    PanDptCfg = nullptr;
}

tTestProcedure* tTestProcedure::Test_CancelTesting(QString details) {
    Test_AddDetails("Testing cancelled! No further tests will be run.");
    if (!details.isEmpty()) Test_AddDetails(details);
    CancelTestingFlag = true;
    //TODO: implement signaling to TestRunner!
    return this;
}

tTestProcedure* tTestProcedure::Test_CancelSubtests(QString details) {
    Test_AddDetails("Testing cancelled! No further tests will be run.");
    if (!details.isEmpty()) Test_AddDetails(details);
    //CancelTestingFlag = true;
    //TODO: implement signaling to TestRunner!
    return this;
}

void tTestProcedure::ResetCancelTestingFlag() { // This is called before starting test session
    CancelTestingFlag = false;
}

void tTestProcedure::SetTestInfo(QString name, tTestInfo info) {
    AllTestInfo[name] = info;
}

void tTestProcedure::ClearAllTestsInfo() {
    AllTestInfo.clear();
}
 
tTestResult tTestProcedure::GetTestResultByName(QString testName) {
    return AllTestInfo[testName].Result;
}

tTestStatus tTestProcedure::GetTestStatusByName(QString testName) {
    return AllTestInfo[testName].Status;
}

// Assign test function to TestName
bool tTestProcedure::AssignTestFunction(const QString& testName, tTestDelegate testProc) {
    bool res = false;
    QString testNameUpper = testName.toUpper();
    if (0 == TestDict.count(testNameUpper)) {
        tTestStruct test = tTestStruct();
        test.Proc = testProc;
        test.Info.Status = tTestStatus::Pending;

        tTestSpec* curSpec = TestSpecs.GetSpec(testNameUpper);
        if (curSpec != nullptr) {
            if (curSpec->GetIsPureGroup()) { // if pure group and function assigned, make non-pure group by adding a Spec internal
                curSpec->ConvertToSuperTest(); // copy new to cur
            }
            test.Info.Result.SetValueType(curSpec->GetValueType());
        } else { // This means we try to assign Test Function to inexisting Test/Spec
            if (IsInexistingTestsWarning)
                Log.LogErrorMessage("Trying to assign Test Function to inexisting Test/Spec\n'" + testName + "'");
        }
        TestDict[testNameUpper] = test;
        res = true;

    }
    return res;
}

bool tTestProcedure::IsTestFunctionAssigned(QString testName) {
    return (TestDict.count(testName) > 0);
}

QString tTestProcedure::TestAssignmentSourceCode(QString dut, QString group, QString specName) { // Just a little help for test procedure programmer :)
    QString SpecName = Str2CppId(specName); // C++ -comatible name
    QString lines = "AssignTestFunction(\"" + specName + "\", Test_" + SpecName + ");\r\n";
    return lines;
}

void tTestProcedure::ValidateAutoTestFuncAssignment(tReport* rep) {
    bool resAuto = true;
    tReport* rep1 = rep->AddReport("Auto Test assignment verification");
    // Scan AutoSpecs and check, all items.isTest()==true have function assigned
    QStringList SpecNames;

    TestSpecs.BuildNameList(SpecNames);
    for(QString& s : SpecNames) {
        rep1->AddDetails(s);
        if (0 == TestDict.count(s.toUpper())) {
            rep1->AddDetails("ERROR! No executable code assigned!");
            resAuto = false;
        }
    }

    QString sTmp = "";
    for(const QString& s : SpecNames) {
        if (0 == TestDict.count(s.toUpper())) {
            sTmp = sTmp + TestAssignmentSourceCode("DUT", "GROUP", s);
        }
    }
    if (sTmp != "") Log.LogSystemMessage("Missing test assignments:\r\n" + sTmp);

    sTmp = "";
    for(const QString& s : SpecNames) {
        if (0 == TestDict.count(s.toUpper())) {
            sTmp = sTmp + "\n" + s;
        }
    }
    if (sTmp != "") Log.LogSystemMessage("Missing test functions for specs:\r\n" + sTmp);
    if (resAuto) rep1->SetStatus(tTestStatus::Passed); else rep1->SetStatus(tTestStatus::Failed);
    IsValid = IsValid && resAuto;
    rep1->Expand(!IsValid, true);
    rep1->SetShowDetails(!IsValid, true);
    //rep->GetRoot()->Refresh();
}

bool tTestProcedure::ValidateManualTestFuncAssignment(tReport* rep) { // X3 how to call it!
    bool resMan = true;
    tReport* rep1 = rep->AddReport("Manual Test assignment verification");
    // Scan AutoSpecs and check, all items.isTest()==true have function assigned
    std::list<QString> SpecNames;
    for (QString& s : SpecNames) {
        if (0 != TestDict.count(s.toUpper())) {
            rep1->AddDetails(s);
        } else {
            rep1->AddDetails("ERROR! Spec " + s + " does not have executable code!");
            resMan = false;
        }
    }
    // TODO: verify, if TestSpec is existing
    if (resMan) rep1->SetStatus(tTestStatus::Passed); else rep1->SetStatus(tTestStatus::Failed);
    return resMan;
}

///bool tTestProcedure::GetTestResultInternal() const {
///    if (CurrentTest.Info.Status == tTestStatus::Tested)
///        return CurrentTest.Info.Result. ResultInternal;
///    else
///        return false;
///}

// PENDING -+-> TESTING -+-> TESTED
//          +------------+-> TEST ERROR
//          +------------+-> SKIPPED
void tTestProcedure::Test_DelayAndSetProgress(int delayMs) {
    int delMs = 50;
    
    int n = delayMs / delMs;
    if (delayMs < 0) return;
    if (n == 0) {
        QThread::msleep(delayMs);
        return;
    }
    for (int i = 0; i < n; i++) {
        QThread::msleep(delMs);
        Test_SetProgress(double(i) / n);
        if (InterruptFlag) return;
    }
}

bool tTestProcedure::AssignManualDialog(QString name, tTestDialog* pDialog) { //Scan Specs tree until name found.Return ManualTestDialog
    return TestSpecs.AssignManualDialog(name, pDialog);
}

tTestDialog* tTestProcedure::FindManualDialog(QString name) {
    tTestDialog* res = nullptr;
    //TestSpecs.FindManualDialog(name, res);
    return res;
}

void tTestProcedure::Test_SetTimeout(double timeoutSec) {
    emit sigSetTestTimeout(timeoutSec);
}

tTestProcedure* tTestProcedure::Test_AddDetails(const QString& details) {
    emit sigAddTestDetails(details);
    return this;
}

void tTestProcedure::Test_SetProgress(double fraction) {
    emit sigSetTestProgress(fraction);
}

tTestProcedure* tTestProcedure::Test_Skip(const QString& details) {
    CurrentTest.Info.Status = tTestStatus::Skipped; 
    if (details != "")
        Test_AddDetails(details);
    return this;
}

tTestProcedure* tTestProcedure::Test_Interrupt(const QString& details) {
    CurrentTest.Info.Status = tTestStatus::Interrupted;
    if (details != "")
        Test_AddDetails(details);
    return this;
}

tTestProcedure* tTestProcedure::Test_Error(const QString& details) {
    CurrentTest.Info.Status = tTestStatus::TestError;
    if (details != "")
        Test_AddDetails(details);
    return this;
}

void tTestProcedure::Test_StartManualTest(const QString& testName) {
    emit sigStartManualTest(testName);
}

void tTestProcedure::Test_ShowMessage(QMessageBox* msgBox) {
    emit sigShowMessage(msgBox);
}

bool tTestProcedure::Test_WaitForMessageBoxResult(int& messageBoxResult, int timeoutSec) {
    MessageBoxWaiterTimer.setSingleShot(true);

    MessageBoxWaiterTimer.start(timeoutSec * 1000);
    MessageBoxWaiterLoop->exec();

    messageBoxResult = MessageBoxResult;
    return MessageBoxWaiterTimer.isActive();
}

void tTestProcedure::SetTestStatus(tTestStatus newStatus) {
    CurrentTest.Info.Status = newStatus;
    emit sigSetTestInfo(CurrentTest.Info);
}

void tTestProcedure::SetTestInfo() {
    emit sigSetTestInfo(CurrentTest.Info);
}

//public 
//void tTestProcedure::SetupManualTest(const QString& groupName, const QString& testName) {
//    //TestGroup = groupName;
//    TestName = testName;
//}

//public 
void tTestProcedure::SetupTest(const QString& testName) {
    TestName = testName;
    InterruptFlag = false;
}

bool tTestProcedure::InitDptAndDut(tReport* rep) {
    tReport* r1 = rep->AddReportTesting("InitManualTest", true);
    bool res = true;
    if (pDPT != nullptr) {
        pDPT->Init(r1);
        if (!res) {
            pDPT->Done(rep);
            return res;
        }
    }
    if (pDUT != nullptr) {
        res = pDUT->Init(r1);
        if (!res) {
            pDUT->Done(rep);
            if (pDPT != nullptr) pDPT->Done(rep);
        }
    }
    r1->SetStatusPassFail(res);
    r1->Expand(!res, true);
    return res;
}

void tTestProcedure::DoneDutAndDpt(tReport* rep) {
    if (pDPT != nullptr) pDPT->Done(rep);
    if (pDUT != nullptr) pDUT->Done(rep);
}

//public virtual 
bool tTestProcedure::InitAutoTests(tReport * rep) { // Called once before running tests
    return InitDptAndDut(rep);
}

//public virtual 
bool tTestProcedure::InitManualTests(tReport* rep) { // Called once when entering Manual mode
    return InitDptAndDut(rep);
}

//public virtual 
void tTestProcedure::DoneAutoTests(tReport* rep) { // Called once after running tests
    DoneDutAndDpt(rep);
}

//public virtual 
void tTestProcedure::DoneManualTests(tReport* rep) { // Called once when leaving Manual mode
    DoneDutAndDpt(rep);
}

//public 
void tTestProcedure::slotRunTest() {
    if (TestName == "") {
        CurrentTest.Info.Details = "ERROR: Test name is empty!";
        return;
    }

    if (TestDict.count(TestName.toUpper()) == 0) {
        CurrentTest.Info.Details = "No test procedure for '" + TestName;
        //SetTestStatus(TTestStatus.TestError);
        return;
    }
    
    CurrentTest = TestDict[TestName.toUpper()];
    if (CurrentTest.Info.Status != tTestStatus::Pending) return;

    if (!InterruptFlag && !CancelTestingFlag) {
        SetTestStatus(tTestStatus::Testing);
        MessageBoxWaiterLoop = new QEventLoop();
        connect(this, &tTestProcedure::sigMessageResult, MessageBoxWaiterLoop, &QEventLoop::quit);
        connect(&MessageBoxWaiterTimer, &QTimer::timeout, MessageBoxWaiterLoop, &QEventLoop::quit);

        try {
            CurrentTest.Proc(CurrentTest.Info); // Run test // TODO: check cancel flag!
        } catch (std::exception& ex) {
            Test_AddDetails("Exception occured during running test: " + QString(ex.what()));
        } catch (...) {
            Test_AddDetails("Unknown exception occured during running test"); 
        }

        if ((CurrentTest.Info.Status == tTestStatus::Testing) && (CurrentTest.Info.Result.IsValueSet()))
            CurrentTest.Info.Status = tTestStatus::Tested;

        if ((CurrentTest.Info.Status != tTestStatus::Tested) &&  // Proc() MUST set status to Tested or Skipped
            (CurrentTest.Info.Status != tTestStatus::Skipped) &&
            (CurrentTest.Info.Status != tTestStatus::Interrupted))
            CurrentTest.Info.Status = tTestStatus::TestError;
    }

    if (InterruptFlag || CancelTestingFlag) {
        CurrentTest.Info.Status = tTestStatus::Interrupted;
    }
    SetTestInfo(); // emit signal once to avoid unwanted switching. Do we really need it in the very end? Yes. To signal end of work
    TestName = "";
    disconnect(this, &tTestProcedure::sigMessageResult, MessageBoxWaiterLoop, &QEventLoop::quit);
    disconnect(&MessageBoxWaiterTimer, &QTimer::timeout, MessageBoxWaiterLoop, &QEventLoop::quit);
    delete MessageBoxWaiterLoop;
    qDebug() << "tTestProcedure::slotRunTest()::moveToThread(main)";
    moveToThread(QApplication::instance()->thread());
}

//void tTestProcedure::slotFinishTest() { // Move TP back to main thread
//    qDebug() << "tTestProcedure::slotFinishTest()::moveToThread(main)";
//    moveToThread(QApplication::instance()->thread()); // Fix Current thread is not the object's thread. Cannot move to target thread
//}

void tTestProcedure::slotMessageResult(int result) { // From UI
    MessageBoxResult = result;
    emit sigMessageResult();
}

#if 0
        public List<TFileInfo> GetFilesByTarget(string target) {
            return TestProcInfo.GetFilesByTarget(target);
        }

        public TFileInfo GetFileByTarget(string target) {
            return TestProcInfo.GetFileByTarget(target);
        }

        // Get parameter from TestProcedure.XML
        public string GetParameter(string name) {
            if (TestProcInfo.Parameters.ContainsKey(name))
                return TestProcInfo.Parameters[name];
            else
                return "";
        }

        // Save parameter to registry
        public void SaveParameter(string param, string value) {
            AppConfig.SaveTestProcParameter(TestProcInfo.DutType, param, value);
        }

        // Load parameter from registry
        public string LoadParameter(string param) {
            string res = AppConfig.LoadTestProcParameter(TestProcInfo.DutType, param);
            if (res == null) res = "";
            return res;
        }

        public void GenerateCode(string workDir) {
            // Create output directory
            string outputDir = workDir + "SourceCodes\" + TestName;
            Directory.CreateDirectory(outputDir);
            // Generate C(onfig)D(ialog)F(orm) file
            // Generate TestProc_<Device>.cs
            // Generate TestProc_<Device>_Unit files
            // Generate TestForm_<Device>_<Unit> files
        }

        public bool FindFailedTests() {
            bool res = false;
            foreach (var test in AllTestInfo) {
                var st = test.Value.Status;
                res = res ||
                    (st == TTestStatus.Failed) ||
                    (st == TTestStatus.Interrupted) ||
                    (st == TTestStatus.TestError);
            }
            return res;
        }
#if 0
        //RichTextBox SpecsRtf = new RichTextBox(); // Print here to output file
        private void AddSpecText(StringBuilder strTable, TTestSpec spec, int level) {
            /*
            strTable.Append(@"\intbl Spec" + @"\cell Value" + @"\cell Range" + @"\cell Units" + @"\row");
            */
            strTable.Append(@"\intbl " + spec.Name + @"\cell " + spec.Req + @"\cell " + spec.sRange + @"\cell " + spec.GetUnits() + @"\row");
            foreach (var ch in spec.Children) {
                AddSpecText(strTable, ch, level + 1);
            }
        }

        private void AddSpecText(RichTextBox specsRtf, TTestSpec spec, int level) {
            //specsRtf.SelectionIndent = level * 10; // IndentLeftPx;
            String sIndent = "";
            for (int i = 0; i < level; i++) sIndent += "  ";
            specsRtf.SelectionColor = FontColorTest;
            specsRtf.SelectionAlignment = HorizontalAlignment.Left;
            specsRtf.SelectionFont = new Font("Arial", 14);
            specsRtf.SelectedText = "\r\n" + sIndent + "L" + level.ToString() + ": " + spec.GetName();
#if true
            if (spec.Req.Length != 0) {
                if (spec.Req == "") {
                }
                if (spec.GetValueType() != TTestResult.TValueType.None) {
                    specsRtf.SelectionFont = new Font("Arial", 12);
                    specsRtf.SelectedText = spec.Name + " shall be " + spec.Req + "\r\n";
                }
            }
#else
            if (level == 0) {
            } else if (level == 1) { // Top group
                specsRtf.SelectionFont = new Font("Arial", 16);
                specsRtf.SelectedText = "\r\n" + spec.Desc;
                if (spec.Desc.Length == 0) specsRtf.SelectedText = "No requirement";
                //specsRtf.SelectedText = "\r\n\r\n";
            } else if (level > 1) {
                if (spec.GetValueType() != TTestResult.TValueType.None) {
                    if (spec.IsTest()) {
                        if (spec.Desc.Length == 0) specsRtf.SelectedText = "No requirement";
                        // Name
                        if (level == 1) {
                            specsRtf.SelectionFont = new Font("Arial", 14);
                        } else {
                            specsRtf.SelectionFont = new Font("Arial", 12);
                        }
                        //specsRtf.SelectedText = spec.GetName() + "\r\n";
                        specsRtf.SelectedText = spec.Desc + "\r\n";
                        // Status
                        //specsRtf.SelectionAlignment = HorizontalAlignment.Right;
                        string res = "";
                        specsRtf.SelectedText = "The value shall be ";// spec.Desc + "\r\n";

                        //specsRtf.SelectionFont = new Font("Courier", 12);
                        res = res + spec.CheckTypeToString();
                        if (spec.GetUnits() != "")
                            res = res + ", [ " + spec.GetUnits() + " ]";
                        specsRtf.SelectedText = res + "\r\n";
                        //SpecsRtf.SelectionFont = FontTestStatus;
                        //SpecsRtf.SelectionColor = TestStatusColors[(int)Status];
                        //SpecsRtf.SelectedText = TestStatusMessages[(int)Status] + "\r\n\r\n";
                        // Details
                        //SpecsRtf.SelectionFont = FontTestDetails;
                        //specsRtf.SelectionAlignment = HorizontalAlignment.Left;
                        //SpecsRtf.SelectionColor = FontColorTestDetails;
                        //SpecsRtf.SelectedText = FormatResult() + Details + "\r\n\r\n";
                    } else {
                        specsRtf.SelectionFont = new Font("Arial", 14);
                        // Currently ignoring higher level groups 
                    }
                }
                //specsRtf.SelectedText = "\r\n";
            }
#endif
            foreach (var ch in spec.Children) {
                AddSpecText(specsRtf, ch, level + 1);
            }
        }

        public void GenSpecsDoc(RichTextBox specsRtf) { // Generate RTF document from AutoSpecs
            //if (level == 0) {
            //tReportRoot.TextBoxVisible.Rtf = SpecsRtf;
            //SpecsRtf =  ReportTextBox;
            //FontTestName = new Font(FontTypeTestFamily, fontSize, fontStyle, GraphicsUnit.Point);
            //}
            specsRtf.Clear();
            StringBuilder strTable = new StringBuilder();
            strTable.Append(@"{\rtf1 ");

            strTable.Append(@"{\fonttbl{\f0\fnil\fcharset0 Courier;}}");
            //Start the row
            strTable.Append(@"\trowd");

            //First cell with width 1000.
            strTable.Append(@"\cellx3300");

            //Second cell with width 1000.Ending point is 2000, which is 1000+1000.
            strTable.Append(@"\cellx4400");

            //Third cell with width 1000.Endingat3000,which is 2000+1000.
            strTable.Append(@"\cellx5800");
            strTable.Append(@"\cellx6600");

            strTable.Append(@"\intbl Spec" + @"\cell Value" + @"\cell Range" + @"\cell Units" + @"\row");
            //Append the row in StringBuilder
            //strTable.Append(@"\intbl \cell \row"); //create the row

            //Add 3 data Rows.Give proper padding space between data.Notice the gap after cell.
            //strTable.Append(@"\intbl   1" + @"\cell Raj" + @"\cell Bangalore" + @"\cell India" + @"\row");
            //strTable.Append(@"\intbl   2" + @"\cell Peter" + @"\cell Mumbai" + @"\cell India" + @"\row");
            //strTable.Append(@"\intbl   3" + @"\cell Chris" + @"\cell Delhi" + @"\cell India" + @"\row");

            foreach (var spec in AutoTestSpecs.Specs) {
            //AddSpecText(specsRtf, spec, 0);
                AddSpecText(strTable, spec, 0);
            }
            strTable.Append(@"\pard");
            strTable.Append(@"}");
            specsRtf.Rtf = strTable.ToString();
        }
}
#endif
#endif

#if 0 // not needed, set in TestRunner
bool tTestProcedure::ReturnTestResult() {
    CurrentTest.Info.Details = "tested";
    CurrentTest.Info.Status = tTestStatus::Tested;
    ///CurrentTest.Info.ResultInternal = false;
    return true;
}
#endif
#if 0
bool tTestProcedure::ReturnTestError(const QString& details) {
    CurrentTest.Info.Details = details;
    CurrentTest.Info.Status = tTestStatus::TestError;
    ///CurrentTest.Info.ResultInternal = false;
    return false;
}
#endif
#if 0
// Moved from tTestUnit
tTestInfo tTestProcedure::BuildTestInfo(const tTestResult& res, tTestStatus status, const QString& details, bool internalRes) {
    tTestInfo info;
    info.Result = res;
    info.Details = details;
    info.Status = status;
    ///info.ResultInternal = internalRes;
    return info;
}

tTestInfo tTestProcedure::BuildTestInfo_Failed(const QString& details, bool internalRes) {
    tTestInfo info;
    //info.Result = 0;
    info.Details = details;
    info.Status = tTestStatus::Failed;
    ///info.ResultInternal = internalRes;
    return info;
}

tTestInfo tTestProcedure::BuildTestInfo_Skipped(const QString& details, bool internalRes) {
    tTestInfo info;
    //info.Result = 0;
    info.Details = details;
    info.Status = tTestStatus::Skipped;
    ///info.ResultInternal = internalRes;
    return info;
}

tTestInfo tTestProcedure::BuildUnfinishedTestInfo(const tTestResult& res, const QString& details) {
    tTestInfo info;
    info.Result = res;
    info.Details = details;
    info.Status = tTestStatus::Testing;
    ///info.ResultInternal = false;
    return info;
}
#endif
//protected 
//void tTestProcedure::SetTestProgress_f(double percentage) {
//    SetTestProgress((int)percentage*100);
//}

//protected static 
//tTestStatus tTestProcedure::Test_BoolToTestStatus(bool val) {
//    return val ? tTestStatus::Tested : tTestStatus::TestError;
//}

//public 
#if 0
void tTestProcedure::PerformOperation(bool result, const QString& operationName, tTestStatus& status, tTestStatus failedStatus) {
    if (status != tTestStatus::Testing) return;
    if (result) {
        AddTestDetails(operationName + " done.");
        status = tTestStatus::Testing;
    } else {
        AddTestDetails("ERROR: failed to " + operationName + "!");
        status = failedStatus;
    }
}
#endif
void tTestProcedure::slotInterruptTest() { // Runner -> Procedure <void>
    InterruptFlag = true; // soft interrupt
}
//700