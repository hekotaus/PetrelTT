#include "tTestSpecs.h"
#include "io/tinyxml2.h"
#include "PetrelTools.h"
#include "tReport.h"
#include "tReportRoot.h"
#include "common/tTickTock.h"

tTestSpecs::tTestSpecs(tLogger& log, tPetrelProjectConfig& cfg) 
    : Log(log), Cfg(cfg) {
}

tTestSpec* tTestSpecs::AddGroup(QString name, tTestSpecs* sourceSpecs, tTestForm* manDialog) {
    if (IsReadonly) return nullptr;
    bool duplicate = false; // Check, if the name already exists
    for(tTestSpec& s : Specs)
        if (!duplicate)
            duplicate = duplicate || s.FindByName(name);

    if (!duplicate) {
        tTestSpec newSpec;
        if (sourceSpecs == nullptr) {// Create "empty" spec
            //TTestSpec newSpec = new TTestSpec(name, "", "", null, "");
            tTestSpec newSpec = tTestSpec(name, "", "boolean", nullptr, "Manually added group", "manual");
        } else { // Copy from existing spec
            tTestSpec* srcSpec = sourceSpecs->GetSpec(name);
            if (srcSpec == nullptr)
                newSpec = tTestSpec(name, "", "boolean", nullptr, "Failed to find spec to copy, added 'empty spec'", "manual");
            else {
                newSpec = tTestSpec(*srcSpec);
                newSpec.Validate(nullptr);
            }
        }
        newSpec.ManualTestDialog = manDialog;
        Specs.push_back(newSpec);
        return &(*Specs.rbegin());
    } else
        return nullptr;
}

tTestSpec* tTestSpecs::AddGroup(QString name, tTestForm* manDialog) {
    if (IsReadonly) return nullptr;
    bool duplicate = false; // Check, if the name already exists
    for(tTestSpec& s : Specs)
        if (!duplicate)
            duplicate = duplicate || s.FindByName(name);

    if (!duplicate) {
        //TTestSpec newSpec = new TTestSpec(name, "", "", null, "");
        tTestSpec newSpec = tTestSpec(name, "", "boolean", nullptr, "Manually added group", "manual");
        newSpec.ManualTestDialog = manDialog;
        Specs.push_back(newSpec);
        return &(*Specs.rbegin());
    } else
        return nullptr;
}

//public 
tTestSpec* tTestSpecs::AddSpec(tTestSpec* spec, tTestForm* manTestDialog) {
    if (IsReadonly) return nullptr;
    if (spec == nullptr) return nullptr;
    tTestSpec child = tTestSpec(*spec);
    child.ManualTestDialog = manTestDialog;
    Specs.push_back(child);
    return &(*Specs.rbegin());
}

//public 
void tTestSpecs::LoadAndValidate() {
    tReport* ReportRoot = (tReport*)Cfg.ReportCurrent;
    Report = ReportRoot->AddReport("Loading Auto Test Specs");
    Valid = true;
    bool res = ReadXml(); // can set Valid
    Valid = Valid && res; // Don't do this inline with ReadXml!
    if (Valid)
        Log.LogSystemMessage("Specs read from file. Validating...");
    else
        Log.LogErrorMessage("Failed to read Specs from file!");
    if (Valid) Validate();
}

//public 
void tTestSpecs::Clear() {
    for(tTestSpec& child : Specs) child.Clear();
    Specs.clear();
}

 //private 
bool tTestSpecs::LoadSpecTree(std::list<tTestSpec>& specs, tinyxml2::XMLElement* elParent, tReport* rep) {
    using namespace tinyxml2;
    bool res = true;
    //Log.LogSystemMessage("Reading Spec");
    //Log.IncIndent();
    // Have empty list of specs
    // For each node:
    //   add node to the list
    //   fill parameters
    //   validate parameters
    //   read subtree
    //   call LoadSpecTree
    XMLElement* el = elParent->FirstChildElement("Test");
    while (el != nullptr) { // real all current level Tests
        // Name -- compulsory, unique, not empty
        // type, range -- compulsory for leaf, optional for branch
        // units, desc -- optional
        QString name  = XmlAttribute(el, "name", ""); // not empty
        QString units = XmlAttribute(el, "units", ""); // may be empty
        QString type  = XmlAttribute(el, "type", ""); // not empty, if no children
        QString range = XmlAttribute(el, "range", ""/*nullptr*/); // not empty, if no children
        QString desc  = XmlAttribute(el, "description", ""); // may be empty
        QString mode  = XmlAttribute(el, "mode", "auto manual"); // may be empty
        QString req   = XmlAttribute(el, "requirement", ""); // may be empty

        Log.LogSystemMessage("Reading Spec " + name);

        QString sTestInfo = "'" + name + "'";
        if (type != "") sTestInfo += " <" + type + ">";
        if (units != "") sTestInfo += " [" + units + "]";
        if (range != nullptr) sTestInfo += " (" + range + ")";// else sTmp += "INTERNAL";

        if (name == "") {
            res = false;
            rep->AddDetails("Test name required!");
        }

        // Check, if the name already exists
        bool duplicate = false;
        if (res) {
            for (tTestSpec& s : Specs) {
                if (!duplicate) duplicate = duplicate || s.FindByName(name);
            }
        }
        if (duplicate) {
            rep->AddDetails("Duplicate names are not allowed!");
            rep->AddDetails(sTestInfo);
        }
        res = res && !duplicate;

        if (res) {
            tTestSpec newSpec = tTestSpec(name, units, type, range, desc, mode, req);
            specs.push_back(newSpec);
            //XmlNodeList childNodes = node.SelectNodes("Test");
            if (nullptr != el->FirstChildElement("Test")) {
                if (!LoadSpecTree(specs.rbegin()->Children, el, rep)) {
                    res = false;
                    rep->AddDetails("Failed to load tree under '" + name + "'");
                    rep->AddDetails(sTestInfo);
                }
            }
        }
        Valid = Valid && res;
        el = el->NextSiblingElement();
    } // foreach
    //Log.DecIndent();
    return res;
}

//public 
bool tTestSpecs::ReadXml() {
    using namespace tinyxml2;
    Directory = Cfg.TestProcRevDir;
    tinyxml2::XMLDocument Xml;
    QString fname = Cfg.TestProcRevDir + "TestSpecs.xml";
    tReport* rep1 = Report->AddReport("Read Auto Test Specs XML");
    Log.LogSystemMessage("Loading Test Specifications...");
    Clear();
    QString errmsg = "";

    // Read head
    if (XML_SUCCESS != Xml.LoadFile(fname.toStdString().c_str())) {
        errmsg = "Failed to open Test Specs file " + fname;
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    XMLElement* root = Xml.RootElement();
    if (root == nullptr) {
        errmsg = "Wrong Test Specs file " + fname;
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    QString rootName = root->Name();
    if (rootName != "Testspecs") {
        errmsg = "Failed to read <Testspecs> element";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    sVersion = XmlAttribute(root, "Version"); // QString("%1").arg(root->Attribute("Version"));
    bool res = true;
    //Version = sVersion.toFloat(&res);
    Version = sVersion.toFloat();
    //if (!res) {
    if (sVersion == "") {
        errmsg = "Failed to read VERSION";
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
    //if (DutRevision == "N/A") {
    //    errmsg = "Failed to read REVISION";
    //    Log.LogErrorMessage(errmsg);
    //    rep1->SetStatus(tTestStatus::Failed, errmsg);
    //    return false;
    //}

    DutPn = XmlAttribute(ch1, "PartNumber", "N/A");
    //if (DutPn == "N/A") {
    //    errmsg = "Failed to read PARTNUMBER";
    //    Log.LogErrorMessage(errmsg);
    //    rep1->SetStatus(tTestStatus::Failed, errmsg);
    //    return false;
    //}

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

    // Read Specs tree
    ch1 = root->FirstChildElement("Specs");
    if (ch1 != nullptr) {
        // Recursive test load
        //XMLElement* ch2 = ch1->FirstChildElement("Test");
        res = res && LoadSpecTree(Specs, ch1, rep1);
        //res = res && XmlTools.GetXmlInnertext(ChildNode, out Description);
    }

    if (!res) {
        errmsg = "Failed to load Test Specs XML!";
        Log.LogErrorMessage(errmsg);
        rep1->SetStatus(tTestStatus::Failed, errmsg);
        return false;
    }

    rep1->SetStatus(res ? tTestStatus::Passed : tTestStatus::Failed);
    return res;
}

//private 
void tTestSpecs::Validate() {
    tReport* rep1 = Report->AddReport("Test Specs Validation");
    Log.LogSystemMessage("Test Specs validation");
    //rep1->AddDetails(QString("Test Specs version: %1").arg(Version, 5, 'f', 3) + IsValid((Version > 0), Valid));
    rep1->AddDetails("Test Specs version: " + sVersion + IsValid((sVersion == Cfg.TestSpecsVer), Valid));
    rep1->AddDetails("DUT type: " + DutName + IsValid((DutName == Cfg.DutName), Valid));
    if (!Cfg.DutRevision.isEmpty())
        rep1->AddDetails("DUT revision: " + DutRevision + IsValid((DutRevision == Cfg.DutRevision), Valid));
    if (!Cfg.DutPartNumber.isEmpty())
        rep1->AddDetails("DUT P/N: " + DutPn + IsValid((DutPn == Cfg.DutPartNumber), Valid));
    rep1->AddDetails(Description);
    Log.LogSystemMessage("Validating Specs...");
    tick("Validating Auto specs");
    tReportRoot* root = Report->GetRoot();
    for(tTestSpec& s : Specs) {
        qDebug() << "Validating spec " << s.GetName();
        Valid = Valid && s.Validate(rep1);
    }
    tock_s();
    Log.LogSystemMessage("Test Specs " + IsValid(Valid, Valid));
    if (!Valid) rep1->AddDetails("Failed to validate Test Specs. See log file for details.");
    rep1->SetStatus(Valid ? tTestStatus::Passed : tTestStatus::Failed);
    root->ExpandNotPassed();
}

 //public 
#if 0
void tTestSpecs::ValidateManual() {
    tReport* ReportRoot = (tReport*)Cfg.ReportCurrent;
    Report = ReportRoot->AddReport("Manual Test Specs validation");
    Valid = true;
    Log.LogSystemMessage("Manual Test Specs validation");
    Log.IncIndent();
    Log.LogSystemMessage("Validating Specs...");
    Log.IncIndent();
    for(tTestSpec& s : Specs) {
        Valid = Valid && s.Validate(Report);
    }
    Log.DecIndent();
    Log.DecIndent();
    Log.LogSystemMessage("Test Specs " + IsValid(Valid, Valid));
    Report->SetStatus(Valid ? tTestStatus::Passed : tTestStatus::Failed);
    if (!Valid) Report->AddDetails("Failed to validate Test Specs. See log file for details.");
}
#endif
 //public
void tTestSpecs::BuildNameList(QStringList& specNames) {
    specNames.clear();
    for(tTestSpec& ch : Specs) {
        ch.BuildNameList(specNames);
    }
}
#if 0
//public 
void tTestSpecs::BuildTestTree(QTreeWidgetItem* treeNode) {
    treeNode->setText(0, "Auto test");
    for (tTestSpec& s : Specs) {
        s.BuildTestTree(treeNode);
    }
    treeNode->setExpanded(true); Warning: The QTreeWidgetItem must be added to the QTreeWidget before calling this function.
}
#endif
#if 1
//public
void tTestSpecs::BuildAutoTestTree(QTreeWidgetItem* treeNode) {
    treeNode->setText(0, "Auto test");
    for (tTestSpec& s : Specs) {
        if (s.GetIsAutoTest()) {
            s.BuildTestTree(treeNode, true);
        }
    }
    //treeNode->setExpanded(true); // Warning: The QTreeWidgetItem must be added to the QTreeWidget before calling this function.
}

//public
void tTestSpecs::BuildManualTestTree(QTreeWidgetItem* treeNode) {
    treeNode->setText(0, "Manual test");
    for (tTestSpec& s : Specs) {
        if (s.GetIsManualTest() && (s.Children.size() != 0))
            s.BuildTestTree(treeNode, false);
    }
    //treeNode->setExpanded(true); Warning: The QTreeWidgetItem must be added to the QTreeWidget before calling this function.
}
#endif
//public 
void tTestSpecs::BuildTestReport(tReport* report, QString name, bool allowDuplicates) { // start building test re[ort from "name"
    //report.SetName(name);
    for(tTestSpec& ch : Specs) {
        ch.BuildTestReport(report, allowDuplicates);
    }
}

//public 
void tTestSpecs::FindManualDialog(QString name, tTestForm* manDialog) {
    QString nameUpper = name.toUpper();
    if (manDialog == nullptr) {
        for(tTestSpec& ch : Specs) {
            if (manDialog == nullptr) {
                if (ch.GetName().toUpper() == nameUpper)
                    manDialog = ch.ManualTestDialog;
                else
                    ch.FindManualDialog(name, manDialog);
            }
        }
    }
}

//public
tTestSpec* tTestSpecs::GetSpec(QString name) { // Returns testSpec with specified name
    tTestSpec* res = nullptr;
    QString nameUpper = name.toUpper();
    //if (name == "this") return this;
    for(tTestSpec& ch : Specs) {
        if (ch.GetName().toUpper() == nameUpper)
            res = &ch;
        else
            if (res == nullptr)
                res = ch.GetSpec(name);
    }
    return res;
}

//public
bool tTestSpecs::CloneGroup(QString sourceName, tTestSpec* destParent) { // clone all the children of the group
    bool res = false;
    tTestSpec* src = GetSpec(sourceName);
    //destParent.ValidateSilent();
    if (src != nullptr) {
        // Clone top element
        //destParent.CloneFrom(src.Name, src.Units, src.sType, src.sRange, src.Desc);

        // Added on 20250903 TODO: need to verify!!!
        tTestSpec* newPar = destParent->AddSpec(src);
        src->CloneTree(newPar);
        newPar->Validate(nullptr);
        res = true;
    }
    return res;
}

