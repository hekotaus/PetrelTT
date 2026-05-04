#include <qfile.h>
#include <qstring.h>
#include "common/tCrc32.h"
#include "tTestProcInfo.h"
#include "tReportRoot.h"
#include "io/tinyxml2.h"

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
    Valid = false;
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
    return res;
}

bool tTestProcInfo::ReadXml(tReport* rep) {
    using namespace tinyxml2;
    //using xel = tinyxml2::XMLElement;
    tinyxml2::XMLDocument Xml;
    QString fname = Cfg.TestProcRevDir + "TestProcedure.xml";

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
    Valid = true;
    
    //IsValidParam("Test Procedure version", sVersion, "positive number", (sVersion == Cfg.TestSpecsVer), rep1, Log, Valid);
    IsValidParam("Test Procedure version", QString("%1").arg(Version, 6, 'f', 3), "positive number", (Version > 0), rep1, Log, Valid);
    IsValidParam("DUT name", DutName, Cfg.DutName, (DutName == Cfg.DutName), rep1, Log, Valid);
    IsValidParam("DUT revision", DutRevision, Cfg.DutRevision, (DutRevision == Cfg.DutRevision), rep1, Log, Valid);
    IsValidParam("DUT P/N", DutPn, "any" /*Cfg.DutPartNumber*/, true /*(DutPn == Cfg.DutPartNumber)*/, rep1, Log, Valid);
    IsValidParam("Application version", Cfg.AppVer.ToQString(), "at least " + ApplicationMinVersion.ToQString(), !(Cfg.AppVer < ApplicationMinVersion), rep1, Log, Valid);
    //rep1->AddDetails(": " +  + tPetrelTools::IsValid(!(), Valid));

    rep1->AddDetails("\nValidating files...");
    for(tFileInfo& f : Files) {
        IsValidFile(f, rep1, Log, Valid);

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
        Valid = Valid && f.Valid;
    }
    rep1->SetStatus(Valid ? tTestStatus::Passed : tTestStatus::Failed);
    return Valid;
}
