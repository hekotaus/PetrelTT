#pragma once

#include <qstring.h>
#include <list>
#include "logger/tLogger.h"
#include "tTestSpec.h"
#include "tPetrelProjectConfig.h"
#include "io/tinyxml2.h"

class tReport;

class tTestSpecs {
private:
    tLogger& Log;
    tPetrelProjectConfig& Cfg;
    tReport* Report = nullptr;
    bool Valid = false;
    bool IsReadonly = false;
    QString Directory;
    void Validate();
    static QString IsValid(bool condition, bool& res) {
        res = res && condition;
        if (condition) return " -- Valid"; else return " -- Invalid";
    }

public:
    QString sVersion = "";
    float Version = 0.0f;
    QString DutName = "";
    QString DutRevision = "";
    QString DutPn = "";
    QString Description = "";
    std::list<tTestSpec> Specs;
    bool IsValid() { return Valid; }
    void SetReadonly() { IsReadonly = true; }
    tTestSpecs(tLogger& log, tPetrelProjectConfig& cfg);
    tTestSpec* AddGroup(QString name, tTestSpecs* sourceSpecs);
    tTestSpec* AddGroup(QString name);
    tTestSpec* AddSpec(tTestSpec* spec);
    void LoadAndValidate();
    void Clear();
    bool LoadSpecTree(std::list<tTestSpec>& specs, tinyxml2::XMLElement* el, tReport* rep);
    bool ReadXml();
    void BuildNameList(QStringList& specNames);
    void BuildAutoTestTree(QTreeWidgetItem* treeRoot); // Build test tree for the panel
    void BuildManualTestTree(QTreeWidgetItem* treeRoot); // Build test tree for the panel
    void BuildTestReport(tReport* report, QString name, bool allowDuplicates, bool includeAuto, bool includeManual); // start building test re[ort from "name"
    tTestSpec* GetSpec(QString name);
};
