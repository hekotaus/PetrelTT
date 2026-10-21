#include <qfile.h>
#include <qstring.h>
#include "common/tCrc32.h"
#include "io/tinyxml2.h"
#include "tTestProcInfo.h"
#include "tReportRoot.h"

tFileInfo::tFileInfo(QString path, QString name) {
    Name = name;
    FullName = path + name;
    Target = "";
    Description = "";
    Valid = true;
    CrcUsed = false;
}

tFileInfo::tFileInfo(QString name) {
    Name = name;
    Target = "";
    Description = "";
    Valid = true;
    CrcUsed = false;
}

tFileInfo::tFileInfo(QString name, QString target, QString description, bool crcUsed, uint32_t crc) {
    Name = name;
    Target = target;
    Description = description;
    CrcUsed = crcUsed;
    CRC = crc;
    Valid = true;
} // TFileInfo constructor

tTestProcInfo::tTestProcInfo(tLogger& log, tPetrelProjectConfig& cfg)
    : Cfg(cfg)
    , Log(log)
    , TestSpecs(Log, Cfg)
    //, ReportRoot(Cfg.ReportTestProc)
{
    //Log = log;
    //ReportRoot = Cfg.ReportTestProc;
}

void tTestProcInfo::Clear() {
    sVersion = "-1";
    IsDeprecated = false; // If the hardware is not supported anymore, add attribute Deprecated="yes" to the <TESTPROCEDURE> item
    Version = -2;
    DutName = "";
    DutRevision = "";
    DutPn = "";
    sApplicationMinVersion = "-1";
    Description = "";
    Files.clear();
    Parameters.clear();
    IsValid = false;
    TestManualEnabled = false;
    WorkingDir = ""; // Writable directory. Log files are created in WorkingDir/Log
    TempDir = ""; // Writable directory for temporary files
    TestProcDir = ""; // Contains test Procedure DUT folders as TestProc/DUTType/DutRev
    TestProcRevDir = "";
}

QString tTestProcInfo::GetTestManual() {
    tFileInfo* fi = GetFileByTarget("test manual");
    if (fi != nullptr)
        return fi->FullName;
    else return "";
}

bool tTestProcInfo::LoadAndValidate(tReport* repRoot) {
    tReport* rep = repRoot->AddReport("Loading Test Procedure");
    bool res = ReadXml(rep);
    if (res)
        res = Validate(rep);
    rep->Expand(!res, true);
    return res;
}

bool tTestProcInfo::ReadXml(tReport* rep) {
    using namespace tinyxml2;
    //using xel = tinyxml2::XMLElement;
    tinyxml2::XMLDocument Xml;
    QString fname = Cfg.TestProcRevDir + "TestProcedure.xml";
    Log.LogSystemMessage("Test procedure " + Cfg.TestProcRevDir);
    Files.clear();
    Parameters.clear();
    tReport* rep1 = rep->AddReport("Read TestProcedure XML");
    Log.LogSystemMessage("Loading Test Procedure...");

    QString errmsg = "";

    if (XML_SUCCESS != Xml.LoadFile(fname.toStdString().c_str())) {
        errmsg = "Failed to open Test Procedure file " + fname;
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    XMLElement* root = Xml.RootElement();
    if (root == nullptr) {
        errmsg = "Wrong Test Procedure file " + fname;
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    string rootName = root->Name();
    if (rootName != "Testprocedure") {
        errmsg = "Failed to read <Testprocedure> element";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    sVersion = XmlAttribute(root, "Version"); // QString("%1").arg(root->Attribute("Version"));
    bool res;
    Version = sVersion.toFloat(&res);
    if (!res) {
    //if (sVersion == "") {
        errmsg = "Failed to read VERSION";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    //QString sDeprecated = "NO";
    IsDeprecated = false;
    QString sDeprecated = XmlAttribute(root, "Deprecated").toUpper();
    IsDeprecated = (sDeprecated == "YES");
    if (IsDeprecated) {
        errmsg = "ERROR: This hardware revision is deprecated!";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    XMLElement* ch1 = root->FirstChildElement("Dut");
    if (ch1 == nullptr) {
        errmsg = "Failed to read <Dut> element";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    DutName = XmlAttribute(ch1, "Name", "N/A");
    if (DutName == "N/A") {
        errmsg = "Failed to read Name";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    DutRevision = XmlAttribute(ch1, "Revision", "N/A");
    if (DutRevision == "N/A") {
        errmsg = "Failed to read REVISION";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    DutPn = XmlAttribute(ch1, "PartNumber", "N/A");
    if (DutPn == "N/A") {
        errmsg = "Failed to read PARTNUMBER";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    ch1 = root->FirstChildElement("Application");
    if (ch1 == nullptr) {
        errmsg = "Failed to read <Application> element";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }
    /// TODO: check version!
    ///if (!XmlTools.GetXmlAttribute(ChildNode, "MinVersion", "2019.0.0", out sApplicationMinVersion) ||
    ///    (!Tools.Str2Float(sApplicationMinVersion, ref ApplicationMinVersion)))
    sApplicationMinVersion = XmlAttribute(ch1, "MinVersion", "-1.-1.-1");
    if (sApplicationMinVersion == "") {
        errmsg = "Failed to read MINVERSION";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    } else {
        ApplicationMinVersion = tVersion(sApplicationMinVersion);
    }

    if (ApplicationMinVersion.ToQString() == "-1.-1.-1") {
        errmsg = "Wrong MINVERSION";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    ch1 = root->FirstChildElement("Description");
    if (ch1 == nullptr) {
        errmsg = "Failed to read <Description> element";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    Description = ch1->GetText();
    if (Description.isEmpty()) {
        errmsg = "Failed to read Description text";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    } else
        rep1->AddDetails(Description);

    // Read parameters
    ch1 = root->FirstChildElement("Parameters");
    if (ch1 != nullptr) {
        XMLElement* ch2 = ch1->FirstChildElement("Parameter");
        //XmlNodeList ParamNode = nTmp.SelectNodes("Parameter");
        while (ch2 != nullptr) {
            //foreach(XmlNode File in ParamNode) {
            QString name = XmlAttribute(ch2, "name", "");// XmlTools.GetXmlAttribute(File, "name", null);
            QString value = XmlAttribute(ch2, "value", "");// XmlTools.GetXmlAttribute(File, "value", "");
            QString desc = XmlAttribute(ch2, "desc", "");
            Log.LogSystemMessage("Parameter '" + name + "' = '" + value + "'");
            if (Parameters.count(name) > 0) {
                errmsg = "Duplicate parameter '" + name + "' (" + desc + ") found!";
                Log.LogErrorMessage(errmsg);
                rep1->SetStatus(tTestStatus::Failed, errmsg);
                return false;
            }
            if (name.isEmpty()) {
                errmsg = "Empty parameter name (" + desc + ") found!";
                Log.LogErrorMessage(errmsg);
                rep1->SetStatus(tTestStatus::Failed, errmsg);
                return false;
            } else {
                Parameters[name] = value;
                errmsg = "Added parameter '" + name + "' = '" + value + "' (" + desc + ")";
                Log.LogSystemMessage(errmsg);
                rep1->AddDetails(errmsg);
            }
            ch2 = ch2->NextSiblingElement("Parameter");
        } // foreach param
    } // PARAMETERS is existing

    // Process files
    ch1 = root->FirstChildElement("Files");
    if (ch1 != nullptr) {
        XMLElement* ch2 = ch1->FirstChildElement("File");
        while (ch2 != nullptr) {
            //foreach(XmlNode File in ParamNode) {
            QString name = XmlAttribute(ch2, "name", "");
            QString target = XmlAttribute(ch2, "target", "N/A").toLower();
            QString desc = XmlAttribute(ch2, "desc", "");
            QString crc = XmlAttribute(ch2, "crc", "");
            
            errmsg = "File '" + name + "' for '" + target + "'";
            if (!desc.isEmpty()) errmsg += " (" + desc + ")";
            if (!crc.isEmpty()) errmsg += " CRC: " + crc;
            //Log.LogSystemMessage(errmsg);
            if (name.isEmpty()) {
                errmsg = "Empty file found: " + errmsg;;
                Log.LogErrorMessage(errmsg);
                rep1->SetStatus(tTestStatus::Failed, errmsg);
                return false;
            } else {
                bool crcUsed = (crc != "");
                uint32_t CRC = 0;
                
                if (crcUsed) {
                    bool validCrc = crc.startsWith("0x");
                    if (validCrc) {
                        crc = crc.remove(0, 2);
                        CRC = crc.toUInt(&validCrc, 16);
                    }
                    if (!validCrc) {
                        errmsg = errmsg + "\n  Invalid CRC field '" + crc + "'";
                        Log.LogErrorMessage(errmsg);
                        rep1->SetStatus(tTestStatus::Failed, errmsg);
                        return false;
                    }
                }

                auto f = tFileInfo(name, target, desc, crcUsed, CRC);
                if (f.Valid) {
                    Files.push_back(f);
                    errmsg = "Added file: " + errmsg;
                } else {
                    errmsg = "Failed to add file: " + errmsg + "(CRC error)";
                    Log.LogErrorMessage(errmsg);
                    rep1->SetStatus(tTestStatus::Failed, errmsg);
                    return false;
                }
                Log.LogSystemMessage(errmsg);
                rep1->AddDetails(errmsg);
            }
            ch2 = ch2->NextSiblingElement("File");
        } // foreach param
    } // PARAMETERS is existing
    rep1->SetStatus(tTestStatus::Passed);
    return true;
}

/// <summary>
/// Search TestProcedureInfo for all files which contain target in the Target attribute
/// </summary>
std::list<tFileInfo> tTestProcInfo::GetFilesByTarget(QString target) {
    target = target.toLower();
    std::list<tFileInfo> res;
    for (auto f : Files)
        if (f.Target.contains(target))
            res.push_back(f);
    return res;
}

/// <summary>
/// Search TestProcedureInfo for file with specified Target attribute
/// </summary>
tFileInfo* tTestProcInfo::GetFileByTarget(QString target) {
    target = target.toLower();
    for (auto& f : Files)
        if (f.Target == target)
            return &f;
    return nullptr;
}

bool tTestProcInfo::IsValidParam(QString name, QString value, QString expected, bool validityCondition, tReport* rep, tLogger &log, bool & globalResult) {
    QString msg;
    msg = name + " = '" + value + "' ";
    if (validityCondition) {
        msg += "is valid";
        Log.LogSystemMessage(msg);
    } else {
        msg += "is invalid (expected " + expected + ")";
        Log.LogErrorMessage(msg);
    }
    rep->AddDetails(msg);
    globalResult = globalResult && validityCondition;
    return validityCondition;
}

bool tTestProcInfo::IsValidFile(tFileInfo& f, tReport* rep, tLogger& log, bool& globalResult) {
    f.FullName = Cfg.TestProcRevDir + f.Name;
    QString msg;
    msg = "File name: '" + f.Name + "' ";
    bool res = QFile::exists(f.FullName);
    if (res) {
        msg += "is existing";
    } else {
        msg += "is NOT existing";
    }

    if (res) {
        res = (!f.Target.isEmpty());
        if (!res) msg += "\n  File is not targeted!";
    }

    if (res) {
        if (f.CrcUsed) {
            uint32_t crc = tCrc32::Crc32(f.FullName);
            res = (crc == f.CRC);
            if (res) {
                msg += "\n  CRC is correct";
            } else {
                msg += QString("\n  CRC mismatch: required:0x%1 actual:0x%2")
                    .arg(f.CRC, 8, 16, '0')
                    .arg(crc, 8, 16, '0');
            }
        } else {
            msg += "\n  CRC is not used";
        }
    }

    if (res)
        Log.LogSystemMessage(msg);
    else
        Log.LogErrorMessage(msg);
    rep->AddDetails(msg);
    globalResult = globalResult && res;
    return res;
}

bool tTestProcInfo::Validate(tReport* rep) {
    // Check TP:
    if (GetDeprecated()) return false;

    tReport* rep1 = rep->AddReport("Test Procedure Validation");
    Log.LogSystemMessage("Test Procedure validation");
    IsValid = true;
    
    //IsValidParam("Test Procedure version", sVersion, "positive number", (sVersion == Cfg.TestSpecsVer), rep1, Log, Valid);
    IsValidParam("Test Procedure version", QString("%1").arg(Version, 6, 'f', 3), "positive number", (Version > 0), rep1, Log, IsValid);
    IsValidParam("DUT name", DutName, Cfg.DutName, (DutName == Cfg.DutName), rep1, Log, IsValid);
    IsValidParam("DUT revision", DutRevision, Cfg.DutRevision, (DutRevision == Cfg.DutRevision), rep1, Log, IsValid);
    IsValidParam("DUT P/N", DutPn, "any" /*Cfg.DutPartNumber*/, true /*(DutPn == Cfg.DutPartNumber)*/, rep1, Log, IsValid);
    IsValidParam("Application version", Cfg.AppVer.ToQString(), "at least " + ApplicationMinVersion.ToQString(), !(Cfg.AppVer < ApplicationMinVersion), rep1, Log, IsValid);
    //rep1->AddDetails(": " +  + tPetrelTools::IsValid(!(), Valid));

    rep1->AddDetails("\nValidating files...");
    for(tFileInfo& f : Files) {
        IsValidFile(f, rep1, Log, IsValid);

        //rep1->AddDetails("File name: " + f.Name +
        //    tPetrelTools::IsValid((QFile::exists(f.FullName)), f.Valid));

        //rep1->AddDetails("File target: " + f.Target +
        //    tPetrelTools::IsValid((f.Target != ""), f.Valid));

        // File CRC. If CRC is not used, calculate and store internally
        //uint32_t crc = tCrc32::Crc32(Cfg.TestProcRevDir + f.Name);
        //if (f.CrcUsed) {
        //    rep1->AddDetails(QString("File CRC: 0x%1").arg(f.CRC, 8, '0') +
        //        tPetrelTools::IsValid((crc == f.CRC), f.Valid));
        //} else { // CRC is not used, calculate and store internally
        //    f.CRC = crc;
        //    rep1->AddDetails(QString("File CRC: 0x%1").arg(f.CRC, 8, '0'));
        //}
        if ((f.Target == "test manual") && f.Valid) TestManualEnabled = true;
        IsValid = IsValid && f.Valid;
    }
    rep1->SetStatus(IsValid ? tTestStatus::Passed : tTestStatus::Failed);
    return IsValid;
}

QString tTestProcInfo::TestAssignmentSourceCode(QString dut, QString group, QString specName) { // Just a little help for test procedure programmer :)
    QString SpecName = Str2CppId(specName); // C++ -comatible name
    QString lines = "AssignTestFunction(\"" + specName + "\", Test_" + SpecName + ");\r\n";
    return lines;
}

bool tTestProcInfo::ValidateManualTestFuncAssignment(tReport* rep) { // X3 how to call it!
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

void tTestProcInfo::ValidateAutoTestFuncAssignment(tReport* rep) {
    bool resAuto = true;
    tReport* rep1 = rep->AddReport("Auto Test assignment verification");
    // Scan AutoSpecs and check, all items.isTest()==true have function assigned
    QStringList SpecNames;

    TestSpecs.BuildNameList(SpecNames);
    for (QString& s : SpecNames) {
        rep1->AddDetails(s);
        if (0 == TestDict.count(s.toUpper())) {
            rep1->AddDetails("ERROR! No executable code assigned!");
            resAuto = false;
        }
    }

    QString sTmp = "";
    for (const QString& s : SpecNames) {
        if (0 == TestDict.count(s.toUpper())) {
            sTmp = sTmp + TestAssignmentSourceCode("DUT", "GROUP", s);
        }
    }
    if (sTmp != "") Log.LogSystemMessage("Missing test assignments:\r\n" + sTmp);

    sTmp = "";
    for (const QString& s : SpecNames) {
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

bool tTestProcInfo::IsTestFunctionAssigned(QString testName) {
    return (TestDict.count(testName) > 0);
}

// Assign test function to TestName
bool tTestProcInfo::AssignTestFunction(const QString& testName, tTestDelegate testProc) {
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

bool tTestProcInfo::GetTestByName(QString testName, tTestStruct& test) {
    testName = testName.toUpper();
    if (TestDict.count(testName) == 0) return false;
    test = TestDict[testName];
    return true;
}

//void tTestProcInfo::SetTestInfo(QString name, tTestInfo info) {
//    AllTestInfo[name] = info;
//}

void tTestProcInfo::ClearAllTestsInfo() {
    AllTestInfo.clear();
}

bool tTestProcInfo::InitDptAndDut(tReport* rep) {
    tTestDevice* pDPT = TP->pDPT;
    tTestDevice* pDUT = TP->pDUT;

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

void tTestProcInfo::DoneDutAndDpt(tReport* rep) {
    tTestDevice* pDPT = TP->pDPT;
    tTestDevice* pDUT = TP->pDUT;

    if (pDPT != nullptr) pDPT->Done(rep);
    if (pDUT != nullptr) pDUT->Done(rep);
}

//tTestResult tTestProcInfo::GetTestResultByName(QString testName) {
//    return AllTestInfo[testName].Result;
//}

//tTestStatus tTestProcInfo::GetTestStatusByName(QString testName) {
//    return AllTestInfo[testName].Status;
//}

//////////////////////////////////////////////////////////////////////////
// THESE METHODS MOVED FROM THE OLD TTESTPROCEDURE.                     //
// AT THE MOMENT OF TRANSFER THEY ARE COMMENTED OUT / NOT TESTED IN C++ //
// SOME OF THEM MAY BE RELATED TO TTESTPROCEDURE CLASS                  //
//////////////////////////////////////////////////////////////////////////
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
    foreach(var test in AllTestInfo) {
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
    foreach(var ch in spec.Children) {
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
    foreach(var ch in spec.Children) {
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

    foreach(var spec in AutoTestSpecs.Specs) {
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

