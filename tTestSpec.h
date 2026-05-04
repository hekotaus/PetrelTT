#pragma once

#include <qstring.h>
#include <set>
#include <list>
#include "tTestForm.h"
#include "tTestResult.h"
#include "tTestStatus.h"
//#include "tReport.h"
#include "param/tParamValue.h"
#include <QStandardItem>
#include <QTreeWidgetItem>

class tReport;

class tTestSpec {
public:
    enum class tCheckType {
        None,
        Exact,    // Exact match. Must not be used for Float
        Mask,     // Binary mask. Can be used with Binary Integer only
        Set,      // Set of exact values. Must not be used for Float
        Range,    // Range min...max
        Internal  // Test procedure internal check. Result passed/failed
    };

protected:
    // Test Specs
    QString Name = "";
    QString Desc = ""; // Informal description
    QString Req = ""; // Formal requirement
    QString Units = "";
    tTestResult::tValueType ValueType = tTestResult::tValueType::None;

    QString sType = "";
    tCheckType Check = tCheckType::None;

    bool IsTest = false;
    bool IsGroup = false;
    bool IsSuperTest = false;
    bool IsPureTest = false;
    bool IsPureGroup = false;
    bool IsManualTest = false;
    bool IsAutoTest = false;
    
    bool disposed = false;
public:

    QString sRange = "";
    QString Mode = "";

    // Exact values
    bool ExactValueBoolean = false;
    double ExactValueFloat = 0;
    int64_t ExactValueInteger = 0;
    QString ExactValueString;
    
    // Sets
    std::set<int64_t> SetInteger;
    std::set<QString> SetString;

    // ranges
    double MaxValueFloat = 0;
    double MinValueFloat = 0;
    int MaxValueInt = 0;
    int MinValueInt = 0;
    // Mask
    uint32_t MaskPositive = 0;
    uint32_t MaskNegative = 0;

    bool IsHex = false;
    bool IsBin = false;
    int Digits = 0;

    bool GetIsTest() const { return IsTest; }
    bool GetIsGroup() const { return IsGroup; }
    bool GetIsSuperTest() const { return IsSuperTest; }
    bool GetIsPureTest() const { return IsPureTest; }
    bool GetIsPureGroup() const { return IsPureGroup; }
    bool IsUnitsValid() const { return true; }
    bool GetTypeMatchesRange() const;
    bool GetIsAutoTest() const { return IsAutoTest; }
    bool GetIsManualTest() const { return IsManualTest; }

    bool CheckTypeValid();
    bool CheckModeValid();
    bool CheckRangeInternal();
    bool CheckBinMask(QString s, uint32_t& maskPos, uint32_t& maskNeg);
    bool CheckRangeMask();
    bool CheckRangeExact();
    bool CheckRangeSet();
    bool CheckRangeRange();
    bool CheckRangeValid();

    // Tree management
    //static private TTestSpecs Root;
    //public bool HaveChildren = false;
    //private TTestSpec Parent;
    std::list<tTestSpec> Children;
    tTestForm* ManualTestDialog = nullptr;

public:
    explicit tTestSpec() {}; // empty
    tTestSpec(const tTestSpec& srcSpec) :
        Name(srcSpec.Name),
        Units(srcSpec.Units),
        sType(srcSpec.sType),
        sRange(srcSpec.sRange),
        Desc(srcSpec.Desc),
        Mode(srcSpec.Mode) {
    }; // empty
    tTestSpec(QString name, QString units, QString type, QString range, QString desc, QString mode, QString req = "");
    ~tTestSpec();

    tTestResult::tValueType GetValueType() { return ValueType; }
    QString GetName() const { return Name; }
    QString GetUnits() const { return Units; }
    QString GetSRange() const { return sRange; }
    QString GetMode() const { return Mode; }
    tCheckType GetCheckType() const { return Check; }

    tTestSpec* AddSpec(QString name, QString units, QString type, QString range, QString desc, QString mode, QString req = "");
    tTestSpec* AddSpec(tTestSpec* spec, tTestForm* manTestDialog = nullptr);
    tTestSpec* AddGroup(QString name, tTestForm* manTestDialog = nullptr);
    tTestStatus TestValue(const tTestResult& testResult);
    void Clear();
    bool ConvertToSuperTest(); // Used to convert pure group to group with buil-in test
    bool Validate(tReport* rep);
    bool FindByName(const QString& name);
    void BuildNameList(QStringList& specNames);
    void BuildTestTree(QTreeWidgetItem* treeNode, bool includeTests); // test == non-groups
    //void BuildAutoTree(tTreeNode* treeNode);
    //void BuildManualTree(tTreeNode* treeNode);
    tReport* BuildTestReport(tReport* report, bool allowDuplicates/*, test type*/);
    tTestSpec* GetSpec(QString name); // Returns testSpec with specified name
    bool CloneTree(tTestSpec* destParent); // clone all the children of the group
    void FindManualDialog(QString name, tTestForm* manDialog);
    QString CheckTypeToString();
};
