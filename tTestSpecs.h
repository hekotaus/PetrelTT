#pragma once

#include <qstring.h>
#include <list>
//#include "tReport.h"
#include "logger/tLogger.h"
#include "tTestForm.h"
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
    //void ValidateManual();
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
    //tTestSpecs(tLogger& log, tReport* rootReport, tPetrelProjectConfig& cfg);
    tTestSpecs(tLogger& log, tPetrelProjectConfig& cfg);
    tTestSpec* AddGroup(QString name, tTestSpecs* sourceSpecs, tTestForm* manDialog = nullptr);
    tTestSpec* AddGroup(QString name, tTestForm* manDialog = nullptr);
    tTestSpec* AddSpec(tTestSpec* spec, tTestForm* manTestDialog = nullptr);
    void LoadAndValidate();
    void Clear();
    bool LoadSpecTree(list<tTestSpec>& specs, tinyxml2::XMLElement* el, tReport* rep);
    bool ReadXml();
    void BuildNameList(QStringList& specNames);
//    void BuildTestTree(QTreeWidgetItem* treeNode);
    void BuildAutoTestTree(QTreeWidgetItem* treeRoot);
    void BuildManualTestTree(QTreeWidgetItem* treeRoot);
    void BuildTestReport(tReport* report, QString name, bool allowDuplicates); // start building test re[ort from "name"
    void FindManualDialog(QString name, tTestForm* manDialog);
    tTestSpec* GetSpec(QString name);
    bool CloneGroup(QString sourceName, tTestSpec* destParent);
};
