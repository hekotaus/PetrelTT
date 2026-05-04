#pragma once
#include "app/tAppConfig.h"
//#include "tReport.h"
class tReportRoot;
struct tPetrelProjectConfig {
    //tProjectSettings Project;
    //tApplicationNameAndVer App;
    const QString PluginDir = "../x64/Debug";
    const QString TestProcDir = "C:/PetrelTT/TestProcedures";
    const QString WorkingDir = "C:/PetrelTT/Work";
    const QString TmpDir = "C:/tmp/PetrelTT";
    const QString ReportDir = "C:/PetrelTT/Reports";
    QString TestProcRevDir = "";
    QString AppName = "Petrel test Tool";
    tVersion AppVer;// = "";
    QString OperatorName = "";
    QStringList OperatorList;
    //QString Operators = "";
    QString DutName = "";
    QString DutRevision = "";
    QString DutPartNumber = "";
    QString DutSerialNumber = "";
    QString TestProcedureVer = "";
    QString TestSpecsVer = "";
    QString TestType = "";
    QString TestDateTime = "";
    QString CurDutDir = "";
    QString CurTestProcDir = "";

    // Reports
    tReportRoot* ReportAutoTest = nullptr;
    tReportRoot* ReportManualTest = nullptr;
    tReportRoot* ReportConfig = nullptr;
    tReportRoot* ReportTestProc = nullptr;
    tReportRoot* ReportReports = nullptr;
    tReportRoot* ReportCurrent = nullptr;

};


