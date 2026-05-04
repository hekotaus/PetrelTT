#pragma once

#include <map>
#include <list>
#include <qstring.h>
#include "PetrelTools.h"
#include "app/tAppConfig.h"
#include "logger/tLogger.h"
//#include "tReport.h"
#include "tPetrelProjectConfig.h"

class tReport;

class tFileInfo {
public:
    QString Name;
    QString FullName;
    QString Target;
    QString Description;
    uint32_t CRC;
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
    tTestProcInfo(tLogger& log, tPetrelProjectConfig& cfg);
    bool IsTestManualEnabled() const { return TestManualEnabled; }
    QString GetTestManual();
    bool IsValid() const { return Valid; }
    bool IsValidParam(QString name, QString value, QString expected, bool validityCondition, tReport* rep, tLogger& log, bool& globalResult);
    bool IsValidFile(tFileInfo& f, tReport* rep, tLogger& log, bool& globalResult);
    bool Validate(tReport* rep);
    bool LoadAndValidate(tReport* repRoot);
    std::list<tFileInfo> GetFilesByTarget(QString target);
    tFileInfo* GetFileByTarget(QString target);
    bool GetDeprecated() const { return IsDeprecated; }
    QString GetVersion() const { return sVersion; }
    void Clear();
};
