#include "tTestSpec.h"
#include "tReport.h"
#include "common/VedroLibTools.h"
#include "common/tTickTock.h"

//public 
tTestSpec::tTestSpec(QString name, QString units, QString type, QString range, QString desc, QString mode, QString req) {
    Name = name;
    Units = units;
    sType = type.toUpper();
    sRange = range;
    Desc = desc;
    Req = req;
    Mode = mode.toUpper();
}

tTestSpec::~tTestSpec() {

}

//public 
tTestSpec* tTestSpec::AddSpec(QString name, QString units, QString type, QString range, QString desc, QString mode, QString req) {
    tTestSpec child = tTestSpec(name, units, type, range, desc, mode, req);
    Children.push_back(child);
    return &(*Children.rbegin());
}

//public 
tTestSpec* tTestSpec::AddSpec(tTestSpec* spec, tTestForm* manTestDialog) {
    if (spec == nullptr) return nullptr;
    tTestSpec child = tTestSpec(
        spec->Name, spec->Units, spec->sType, spec->sRange, spec->Desc, spec->Mode, spec->Req);
    child.ManualTestDialog = manTestDialog;
    Children.push_back(child);
    return &(*Children.rbegin());
}

//public 
tTestSpec* tTestSpec::AddGroup(QString name, tTestForm* manTestDialog) {
    //TTestSpec child = new TTestSpec(name, "", "", null, "");
    //TTestSpec child = new TTestSpec(name, "", "boolean", null, "", "auto manual"); // this cause forever PENDING for pure groups
    tTestSpec child = tTestSpec(name, "", "", nullptr, "", "auto manual");
    //if (manTestDialog != null)
    child.ManualTestDialog = manTestDialog;
    Children.push_back(child);
    return &(*Children.rbegin());
}

//public 
tTestStatus tTestSpec::TestValue(const tTestResult& testResult) {
    if (testResult.GetValueType() != ValueType) {
        qDebug() << "Wrong value type";
        return tTestStatus::TestError;
    }
    if (!testResult.IsValueSet()) {
        qDebug() << "Value not set";
        return tTestStatus::TestError;
    }

    bool bRes = false; // value check result
    bool cRes = false; // conversion result

    if (Check == tCheckType::Internal) {
        bool value = false;
        cRes = testResult.GetValue(value);
        if (!cRes) return tTestStatus::TestError;
        return value ? tTestStatus::Passed : tTestStatus::Failed;
    }

    switch (ValueType) {
        //case TTestResult.TValueType.None: bRes = false; break; // Should never hit
    case tTestResult::tValueType::None: cRes = true;  bRes = true; break; // Auto test groups have no type

    case tTestResult::tValueType::Boolean: { // Match
        bool value = false;
        if (Check == tCheckType::Exact) {
            cRes = testResult.GetValue(value);
            if (!cRes) break;
            bRes = (value == ExactValueBoolean);
        }
        break;
    }
    case tTestResult::tValueType::Integer: { // Exact or set
        int64_t value = 0;
        cRes = testResult.GetValue(value);
        if (!cRes) break;
        if (Check == tCheckType::Exact)
            bRes = (value == ExactValueInteger);
        else if (Check == tCheckType::Set)
            bRes = (0 != SetInteger.count(value));
        else if (Check == tCheckType::Range)
            bRes = ((value >= MinValueInt) && (value <= MaxValueInt));
        else if (Check == tCheckType::Mask) {
            bool bResPos = ((value & MaskPositive) == MaskPositive);
            bool bResNeg = ((value & ~MaskNegative) == 0x00);
            bRes = bResPos && bResNeg;
        }
    }
    break;

    case tTestResult::tValueType::Float: {
        double value = 0;
        cRes = testResult.GetValue(value);
        if (!cRes) break;
        if (Check == tCheckType::Range)
            bRes = ((value >= MinValueFloat) && (value <= MaxValueFloat));
    }
    break;

    case tTestResult::tValueType::String: {
        QString value = "";
        cRes = testResult.GetValue(value);
        if (!cRes) break;
        if (Check == tCheckType::Exact)
            bRes = (value == ExactValueString);
        else if (Check == tCheckType::Set)
            bRes = (0 != SetString.count(value));
        break;
    }
    } // switch
    if (!cRes) return tTestStatus::TestError;
    return bRes ? tTestStatus::Passed : tTestStatus::Failed;
}

//public
void tTestSpec::Clear() {
    for(tTestSpec& child : Children) child.Clear();
    Children.clear();
}

//public 
bool tTestSpec::ConvertToSuperTest() { // Used to convert pure group to group with buil-in test
    //IsTest = (Units != "") || (sType != "") || (sRange != null);
    sType = "boolean";
    sRange = "true";
    Desc = Desc + "Converted to supertest (non-pure group)";
    return Validate(nullptr);
}

//private 
bool tTestSpec::CheckTypeValid() {
    bool res = true;
    if (0);
    else if (sType == "INTEGER") ValueType = tTestResult::tValueType::Integer;
    else if (sType == "FLOAT") ValueType = tTestResult::tValueType::Float;
    else if (sType == "STRING") ValueType = tTestResult::tValueType::String;
    else if (sType == "BOOLEAN") ValueType = tTestResult::tValueType::Boolean;
    else res = false;
    return res;
}

//private 
bool tTestSpec::CheckModeValid() {
    bool res = true;
    bool a = Mode.contains("AUTO");
    bool m = Mode.contains("MANUAL");
    if (a || m) { // Override values set by parent
        IsAutoTest = !m || a; // "AUTO" or "AUTO MANUAL" or ""
        IsManualTest = !a || m; // "MANUAL" or "AUTO MANUAL" or ""
    }
    return res;
}

//private 
bool tTestSpec::CheckRangeInternal() {
    //if ((sRange.ToUpper() == "") && (Type != TValueType.String))
    if (!sRange.isEmpty()) return false;
    Check = tCheckType::Internal;
    ValueType = tTestResult::tValueType::Boolean;
    //Console.WriteLine("Internal test " + Name); // DEBUG ONLY!!!
    return true;
}

//private 
bool tTestSpec::CheckBinMask(QString s, uint32_t& maskPos, uint32_t& maskNeg) {
    bool res = false;
    maskPos = 0;
    maskNeg = 0;
    if (s == nullptr) return false;
    if (s.size() < 2) return false;
    if (s[0] != 'b') return false;
    res = true;
    for (int i = 1; i < s.size(); i++) {
        char ch = s[i].toLatin1();
        res = res && ((ch == '1') || (ch == '0') || (ch == 'x') || (ch == 'X'));
        if (res) {
            maskPos <<= 1;
            maskNeg <<= 1;
            switch (ch) {
            case '0': maskPos |= 0; maskNeg |= 0; break;
            case '1': maskPos |= 1; maskNeg |= 1; break;
            case 'x': case 'X': maskPos |= 0; maskNeg |= 1; break;
            default: res = false; break;
            }
        }
    }
    return res;
}

//private 
bool tTestSpec::CheckRangeMask() {
    if (sRange == nullptr) return false;
    bool res = false;
    switch (ValueType) {
    case tTestResult::tValueType::Integer:
        res = CheckBinMask(sRange, MaskPositive, MaskNegative);
        IsBin = true;
        //MaskPositive
        //MaskNegative
        break;
    }
    if (res) Check = tCheckType::Mask;
    return res;
}

//private 
bool tTestSpec::CheckRangeExact() {
    if (sRange.isEmpty()) return false;
    bool res = false;
    switch (ValueType) {
    case tTestResult::tValueType::String:
        ExactValueString = sRange;
        res = true;
        break;
    case tTestResult::tValueType::Integer:
        ExactValueInteger = sRange.toLong(&res);
        if (res && sRange.startsWith("0x")) {
            IsHex = true;
            Digits = sRange.size() - 2;
        }
        if (res && sRange.startsWith("b")) { 
            IsBin = true; 
            Digits = sRange.size() - 1; 
        }
        //if (res && sRange.StartsWith("b")) { Binary is always treated as a mask
        //    IsBin = true;
        //    Digits = sRange.Length - 1;
        //}
        break;
    case tTestResult::tValueType::Float:
        ExactValueFloat = sRange.toDouble(&res);
        break;
    case tTestResult::tValueType::Boolean:
        ExactValueBoolean = Str2Bool(sRange, &res);
        break;
    }
    if (res) Check = tCheckType::Exact;
    return res;
}

//private 
bool tTestSpec::CheckRangeSet() {
    if (sRange.isEmpty()) return false;
    bool res = true;
    //double fVal = 0;
    int iVal = 0;
    QStringList args = sRange.split('|');
    if ((args.count() > 1) &&
        ((ValueType == tTestResult::tValueType::Integer) || (ValueType == tTestResult::tValueType::String))) {
        for(QString& s : args) {
            if (ValueType == tTestResult::tValueType::Integer) {
                if (res) iVal = s.toUInt(&res);
                if (res) SetInteger.insert(iVal);
                if (res && s.startsWith("0x")) { IsHex = true; Digits = s.size() - 2; }
                if (res && s.startsWith("b")) { IsBin = true; Digits = s.size() - 1; }
            } else {
                SetString.insert(s);
            }
        } // foreach args
    } // if splits and has proper type
    else
        res = false;
    if (res) Check = tCheckType::Set;
    return res;
}

//private 
bool tTestSpec::CheckRangeRange() {
    if (sRange.isEmpty()) return false;
    bool res = ((ValueType == tTestResult::tValueType::Float) || (ValueType == tTestResult::tValueType::Integer));
    // Find "..." and split
    int p = sRange.indexOf("...");
    res = res && (p > 0) && (sRange.size() > (p + 3));
    if (res) {
        QString sMin = sRange.left(p);
        QString sMax = sRange.right(sRange.size() - (p + 3));
        if (ValueType == tTestResult::tValueType::Integer) {
            if (res) MinValueInt = sMin.toLong(&res);
            if (res && sMin.startsWith("0x")) { IsHex = true; Digits = sMin.size() - 2; }
            if (res && sMin.startsWith("b")) { IsBin = true; Digits = sMin.size() - 1; }
            if (res) MaxValueInt = sMax.toLong(&res);
            if (res && sMax.startsWith("0x")) { IsHex = true; Digits = sMax.size() - 2; }
            if (res && sMax.startsWith("b")) { IsBin = true; Digits = sMax.size() - 1; }
        } else {
            if (res) MinValueFloat = sMin.toDouble(&res);
            if (res) MaxValueFloat = sMax.toDouble(&res);
        }
    }
    if (res) Check = tCheckType::Range;
    return res;
}

//private 
bool tTestSpec::CheckRangeValid() {
    // Order is important
    bool res = 
        (CheckRangeInternal() || 
         CheckRangeSet() ||
         CheckRangeRange() || 
         CheckRangeExact() || 
         CheckRangeMask());
    return res;
}

//private 
bool tTestSpec::GetTypeMatchesRange() const {
    bool res = true;
    switch (ValueType) {
        // Internal check disabled
        //case TTestResult.TValueType.Integer: res = (Check != TCheckType.Internal); break; // No internal for Integer
        //case TTestResult.TValueType.Float:   res = (Check == TCheckType.Range); break; // Only ranges for Float
        //case TTestResult.TValueType.String:  res = ((Check != TCheckType.Range)/* && (Check != TCheckType.Internal)*/); break; // No Ranges or Internal for String
        //case TTestResult.TValueType.Boolean: res = (Check == TCheckType.Exact);  break; // Exact match only

        // Internal checks enabled
    case tTestResult::tValueType::Integer: res = true; break; // All checks for Integer
    case tTestResult::tValueType::Float: res = ((Check == tCheckType::Range) || (Check == tCheckType::Internal)); break; // Only ranges for Float
    case tTestResult::tValueType::String: res = ((Check != tCheckType::Range)); break; // No Ranges or Internal for String
    case tTestResult::tValueType::Boolean: res = ((Check == tCheckType::Exact) || (Check == tCheckType::Internal)); break; // Exact match only, internal automatically means true
    }
    return res;
}

//public 
bool tTestSpec::Validate(tReport* rep) {
    tReport* rep1 = nullptr;
    
    qDebug() << "Validating spec" << GetName();
    //tick(QString("Validating spec" + GetName()).toStdString());

    //tick(QString("Set rep status1 " + Name).toStdString());
    if (rep != nullptr) {
        rep1 = rep->AddReport(Name);
        // The following line is incorrect. As we don't have specs, Testing causes infinite Testing state
        //if (rep1->GetStatus() != tTestStatus::TestError) rep1->SetStatus(tTestStatus::Testing); 
    }
    //tock_s();

    bool Valid = true;
    // Name is already valid

    IsTest = (!Units.isEmpty()) || (!sType.isEmpty()) || (!sRange.isEmpty()); // If any present, it's a test
    IsGroup = Children.size() != 0;
    IsSuperTest = IsTest && IsGroup;
    IsPureTest = IsTest && !IsGroup;
    IsPureGroup = IsGroup && !IsTest;
    Valid = Valid && CheckModeValid();
    QString sTestInfo = "";// "'" + Name;
    if (!sType.isEmpty())  sTestInfo += "<" + sType + "> ";
    if (!Units.isEmpty())  sTestInfo += "[" + Units + "] ";
    if (!sRange.isEmpty()) sTestInfo += "(" + sRange + ")"; else sTestInfo += "(internal)";
    if (!Desc.isEmpty())   sTestInfo += "\r\nDescription:\r\n" + Desc;
    if (rep1 != nullptr) rep1->AddDetails(sTestInfo);

    if (IsTest) {
        tick("Add test details");
        if (!IsUnitsValid()) {
            Valid = false;
            if (rep1 != nullptr) rep1->AddDetails("'Units' field is invalid");
        }
        if (!CheckTypeValid()) {
            Valid = false;
            if (rep1 != nullptr) rep1->AddDetails("'Type' field is invalid");
        }
        if (!CheckRangeValid()) {
            Valid = false;
            if (rep1 != nullptr) rep1->AddDetails("'Range' field is invalid");
        }
        if (!GetTypeMatchesRange()) {
            Valid = false;
            if (rep1 != nullptr) {
                if (sRange.isEmpty())
                    rep1->AddDetails("Type '" + sType + "' does not support INTERNAL range");
                else
                    rep1->AddDetails("Type '" + sType + "' does not support range '" + sRange + "'");
            }
        }
        tock_s();
    }
    if (IsGroup)
        for(tTestSpec& s: Children) {
            bool bTmp = s.Validate(rep1);
            Valid = Valid && bTmp;
        }

    if (rep1 != nullptr) {
        //tick(QString("Set rep status2 " + Name).toStdString());
        if (rep1->GetStatus() != tTestStatus::TestError) {
            qDebug() << "Set " << rep1->GetName() << " status to " << (Valid ? "Passed" : "Failed");
            rep1->SetStatus(Valid ? tTestStatus::Passed : tTestStatus::Failed);
        }
        //tock_s();
    }
    //tock_s();
    return Valid;
}

// public 
bool tTestSpec::FindByName(const QString& name) {
    bool res = (Name.toUpper() == name.toUpper());
    if (!res)
        for(tTestSpec& s : Children)
            if (!res)
                res = res || s.FindByName(name);
    return res;
}

//public 
void tTestSpec::BuildNameList(QStringList& specNames) {
    if (IsTest) specNames.append(Name);
    for (tTestSpec& s : Children) {
        s.BuildNameList(specNames);
    }
}

//public 
void tTestSpec::BuildTestTree(QTreeWidgetItem* treeNode, bool includeTests) {
    QTreeWidgetItem* newNode = new QTreeWidgetItem(treeNode);
    newNode->setText(0, Name);
    //treeNode->addChild(newNode);
    for (tTestSpec& s : Children) {
        if (includeTests || s.IsGroup)
            s.BuildTestTree(newNode, includeTests);
    }
    //treeNode->setExpanded(true); Warning: The QTreeWidgetItem must be added to the QTreeWidget before calling this function.
}

//public 
//void tTestSpec::BuildAutoTree(tTreeNode* treeNode) {
//    tTreeNode* newNode = treeNode->Nodes.Add(Name);
//    for (tTestSpec& s : Children) {
//        s.BuildAutoTree(newNode);
//    }
//    treeNode->Expand();
//}
#if 0
//public 
void tTestSpec::BuildManualTree(tTreeNode* treeNode) {
    //treeNode->NodeFont = new System.Drawing.Font("Lucida", 10);
    tTreeNode* newNode = treeNode->Nodes.Add(Name);

    if ((ManualTestDialog == nullptr) /*&& (AutoTest == null)*/)
        newNode->NodeFont = newNode->FontStyleRegular;
    else
        newNode->NodeFont = newNode->FontStyleHasDialog;
    for (tTestSpec& s : Children) {
        s.BuildManualTree(newNode);
    }
    treeNode->Expand();
}
#endif
//public 
tReport* tTestSpec::BuildTestReport(tReport* report, bool allowDuplicates/*, test type*/) {
    tReport* rep = report->AddReportPending(Name, allowDuplicates);
    if (IsTest) rep->LinkSpec(this);
    for (tTestSpec& s : Children) {
        s.BuildTestReport(rep, allowDuplicates);
    }
    return rep;
}

//public 
tTestSpec* tTestSpec::GetSpec(QString name) { // Returns testSpec with specified name
    tTestSpec* res = nullptr;
    for (tTestSpec& s : Children) {
        if (s.GetName().toUpper() == name.toUpper())
            res = &s;
        else
            if (res == nullptr)
                res = s.GetSpec(name);
    }
    return res;
}

//public
bool tTestSpec::CloneTree(tTestSpec* destParent) { // clone all the children of the group
    bool res = false;
    for (tTestSpec& s : Children) {
        tTestSpec* newCh = destParent->AddSpec(&s);
        s.CloneTree(newCh);
        newCh->Validate(nullptr);
    }
    res = true;
    return res;
}

//public 
void tTestSpec::FindManualDialog(QString name, tTestForm* manDialog) {
    if (manDialog == nullptr) {
        for (tTestSpec& s : Children) {
            if (manDialog == nullptr) {
                if (s.Name.toUpper() == name.toUpper())
                    manDialog = s.ManualTestDialog;
                else
                    s.FindManualDialog(name, manDialog);
            }
        }
    }
}

//public 
QString tTestSpec::CheckTypeToString() {
    QString res = "";
    switch (Check) {
    case tTestSpec::tCheckType::Mask: res += "matching mask\n" + GetSRange(); break;
    case tTestSpec::tCheckType::Exact: res += "equal to " + GetSRange(); break;
    case tTestSpec::tCheckType::Range: res += "within [" + GetSRange() + "]"; break;
    case tTestSpec::tCheckType::Set: res += "in [" + GetSRange() + "]"; break;
    case tTestSpec::tCheckType::Internal: res += " (internal check)"; break;
    }
    return res;
}
