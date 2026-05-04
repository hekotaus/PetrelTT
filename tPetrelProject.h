#pragma once
#include "param/tParamGroup.h"
#include "param/tParam.h"
#include "logger/tLogger.h"
#include "tTestProcedure.h"
#include "tTestProcInfo.h"
#include "tTestSpecs.h"
#include "tTestRunner.h"
#include "tPetrelProjectConfig.h"

#include <map>
#include <qlibrary.h>

struct tProjectState {
    int tmp;
};

class tPetrelProject {
    tLogger& Log;
    bool Changed = false;
    bool IsPlugged = false; // If the dll is connected
    QLibrary PluginLib;
    //tTestSpecs ManualTestSpecs;
    //tTestSpecs AutoTestSpecs = tTestSpecs(Log, (tReport*)(Cfg.ReportTestProc), Cfg);
    //tTestSpecs AutoTestSpecs = tTestSpecs(Log, Cfg);
    //tTestSpecs ServiceTestSpecs;
    QTreeWidgetItem AutoTestTree;
    QTreeWidgetItem ManualTestTree;

    
    std::list<tReport*> LinearReports; // Used for linearisation of the test tree

    //QTreeWidgetItem* ManualTestTree = nullptr;

    tTestSpecs* TestSpecs = nullptr;
    void InitParGrpProjectConfig();
    void FindPlugins();// return PluginList map<DUTname, filename>
    void FindTestProcedures(); // return list of DUTs TestProcList
    void PopulateAutoTestTree();
    void PopulateManualTestTree();
    bool OpenPlugin();
    void ClosePlugin();
    bool LoadTestProcedure();
    void ResetDeviceInfo() { }
    void ColorizeTestTree(QTreeWidgetItem* rootNode);
    void DecolorizeTestTree(QTreeWidgetItem* curNode);
    void BuildReportsList(std::list<tReport*>& repList, tReport* report);

public:
    std::map<QString, QString> PluginList; // <DUTname, filename>
    QStringList TestProcList;
    QStringList DutNameList;
    QStringList SpecVerList;

    tParamGroup* ParGrpProject;

    //tAmlParamSuperGroup ProcessParamsSuperGroup;
    tParamSuperGroup GeneralParamsSuperGroup;
    //tAmlParamSuperGroup UniversalParamsSuperGroup; // Include both Process and General

    tPetrelProjectConfig Cfg;
    tProjectState State;
    tTestProcedure* TP = nullptr;
    tTestProcInfo TPInfo;
    tTestRunner TestRunner;

    tPetrelProject(tLogger& log);
    
    void DiscoverTestProcedures();
    void DiscoverSpecVersions();
    void BuildDirNames();

    void CreateTestProcedure();
    void CloseTestProcedure();
    
    void ReportsClear();
    QTreeWidgetItem* GetAutoTestTree() { return &AutoTestTree; }
    QTreeWidgetItem* GetManualTestTree() { return &ManualTestTree; }
    void ExpandTestTree(QTreeWidgetItem* treeNode, bool expand);
    static QTreeWidgetItem* SearchNode(const QString& SearchText, QTreeWidgetItem* startNode);
    void ColorizeAutoTestTree() { ColorizeTestTree(&AutoTestTree); }

    //void StartTest(const QString& testName);
    void StartAutoTests();
    void StartManualTest() {};
    void RunAutoTests();
    void StopTests();
    bool InitManualTest();
    //bool InitAutoTest();
    void DoneManualTest();
    void DoneAutoTest();

    template <class T>
    T CallPluginFunction(T typ, const QString& funcName) {
        T res = T();
        typedef T (*tDutNameFunc)();
        tDutNameFunc func = (tDutNameFunc)PluginLib.resolve(funcName.toStdString().c_str());
        if (func != nullptr) {
            res = func();
        }
        return res;
    }

    //template <class T>
    //T* GetPluginObjectPtr(const QString& objName) {
    //    T* res = (T*)PluginLib.resolve(objName);
    //    return res;
    //}
};
