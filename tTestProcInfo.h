#pragma once

#include <map>
#include <list>
#include <qstring.h>
#include "PetrelTools.h"
#include "app/tAppConfig.h"
#include "logger/tLogger.h"
#include "tTestProcedure.h"
//#include "tReport.h"
#include "tPetrelProjectConfig.h"

#include "tTestInfo.h"
#include "tTestSpecs.h"

class tReport;
struct tTestStruct;

class tFileInfo {
public:
    QString Name;
    QString FullName;
    QString Target;
    QString Description;
    uint32_t CRC = -1;
    bool Valid = false;
    bool CrcUsed = true;
    tFileInfo(QString path, QString name);
    tFileInfo(QString name);
    //tFileInfo(QString name, QString target, QString description, QString crc);
    tFileInfo(QString name, QString target, QString description, bool crcUsed, uint32_t crc);
};

class tTestProcInfo {
    QString sVersion = "-1";
    bool IsDeprecated = false; // If the hardware is not supported anymore, add attribute Deprecated="yes" to the <TESTPROCEDURE> item
    float Version = -2;
    QString DutName = "";
    QString DutRevision = "";
    QString DutPn = "";
    QString sApplicationMinVersion = "-1";
    tVersion ApplicationMinVersion;
    QString Description = "";
    std::list<tFileInfo> Files;
    std::map<QString, QString> Parameters;
    tLogger& Log;
    tPetrelProjectConfig& Cfg;
    
    //tReportRoot*& ReportRoot;
    //tReport* ReportRoot;
    bool Valid = false;
    bool TestManualEnabled = false;
    QString WorkingDir = ""; // Writable directory. Log files are created in WorkingDir/Log
    QString TempDir = ""; // Writable directory for temporary files
    QString TestProcDir = ""; // Contains test Procedure DUT folders as TestProc/DUTType/DutRev
    QString TestProcRevDir = "";
    bool ReadXml(tReport* rep);
    
public:
    tTestProcedure* TP = nullptr;
    tTestProcInfo(tLogger& log, tPetrelProjectConfig& cfg);
    bool IsTestManualEnabled() const { return TestManualEnabled; }
    QString GetTestManual();
    //bool IsValid() const { return Valid; }
    bool IsValidParam(QString name, QString value, QString expected, bool validityCondition, tReport* rep, tLogger& log, bool& globalResult);
    bool IsValidFile(tFileInfo& f, tReport* rep, tLogger& log, bool& globalResult);
    bool Validate(tReport* rep);
    bool LoadAndValidate(tReport* repRoot);
    std::list<tFileInfo> GetFilesByTarget(QString target);
    tFileInfo* GetFileByTarget(QString target);
    bool GetDeprecated() const { return IsDeprecated; }
    QString GetVersion() const { return sVersion; }
    void Clear();

    // Moved from tTestProcedure
private:
    bool IsValid = true;
    bool IsInexistingTestsWarning = true;
    std::map<QString, tTestStruct> TestDict; // Call Test by Name
    std::map<QString, tTestInfo> AllTestInfo;
public:
    tTestSpecs TestSpecs;
    bool GetValid() const { return IsValid; }
    void SetValid(bool st) { IsValid = st; }
    bool IsTestFunctionAssigned(QString testName);
    QString TestAssignmentSourceCode(QString dut, QString group, QString specName); // Just a little help for test procedure programmer :)
    bool ValidateManualTestFuncAssignment(tReport* rep); // X3 how to call it!
    void ValidateAutoTestFuncAssignment(tReport* rep);
    bool GetTestByName(QString testName, tTestStruct& test);
    tTestSpecs* GetTestSpecs() { return &TestSpecs; }
    tTestDialog* GetManualTestDialog(const QString& groupName) { return TestSpecs.GetManualDialog(groupName); };
    // AllTestInfo
    //void SetTestInfo(QString name, tTestInfo info);
    void ClearAllTestsInfo();

    bool InitDptAndDut(tReport* rep);
    void DoneDutAndDpt(tReport* rep);


    //tTestResult GetTestResultByName(QString testName);
    //tTestStatus GetTestStatusByName(QString testName);

public slots:
    bool AssignTestFunction(const QString& testName, tTestDelegate testProc);
};
