#pragma once
//#include <qobject.h>
#include <QTextDocument>
#include <qstring.h>
#include <qcolor.h>
#include <qtextedit.h>

// VedroLib
#include "logger/tLogger.h"

// Local
#include "tReportCommon.h"
#include "tTestResult.h"
#include "tTestProcedure.h"
#include "tPetrelProjectConfig.h"
#include "tTestStatus.h"
#include "tTestInfo.h"
#include "tTestProcInfo.h"

class tTestSpec;
class tReportRoot;

class tReportSignaler : public QObject {
    Q_OBJECT
public:
    void SignalStatusUpdate() { emit sigStatusUpdated(); }
    void ConnectStatusUpdate() {};
signals:
    void sigStatusUpdated();
};
extern tReportSignaler g_ReportSignaler;

struct tReportFormat {
    QString FontFamily = "Lucida Sans Typewriter";
    bool IsItalic = false;
    int FontWeight = QFont::Normal;
    Qt::Alignment Alignment = Qt::AlignLeft;
    QColor TextColor = Qt::black;
    qreal FontPtSizeDiff = 0; // Change to base font size defined by report level
};

class tReport {

public:
    enum eElement {
        eRepName,
        eRepStatus,
        eRepResult,
        eRepResultComments,
        eRepDescription,
        eRepDetails,
        eRep_NUM,
        eRepNext = eRep_NUM, // Used for line numbers
    };

    struct tVewSettings {
        bool IsExpanded = true;
        bool IsShowName = true;
        bool IsShowStatus = true;
        bool IsShowResultValue = true;
        bool IsShowDetails = true;
        bool IsShowDescription = true;
    };

    struct tRepStatus {
        QString TestStatusMessage;
        QColor TestStatusColor;
        tRepStatus(QString message, QColor color) : TestStatusMessage(message), TestStatusColor(color) {};
    };

    static const QColor orangeRed;// = QColor(0xffff4500);
    static const QColor orange;// = QColor(0xffffa500);
    static const QColor dodgerBlue;// = QColor(0xff1e90ff);
protected:

    static const tRepStatus StatusList[int(tTestStatus::LAST)];
    tLogger* Log = nullptr;
    QTextDocument* ReportText = nullptr; // Buffer/shadow
    //QTextDocument* ReportTextVisible = nullptr; // Application UI
    QTextDocument* ReportTextRtf = nullptr; // Print here to output file
    QTextEdit* ReportView = nullptr;

    QString Name = "";
    QString Details = "";
    QString ResultValue = ""; // From (tResultValue::ToStr()
    QString ResultComment = ""; // Includes comments on fitting value to the range
    QString Description = "";

    tTestStatus Status = tTestStatus::None; // Combined Local TestStatus and children's GroupStatus
    tTestStatus GroupStatus = tTestStatus::None; // All children Status
    tTestStatus TestStatus = tTestStatus::None; // Local test Status for Group
    tTestSpec* TestSpec = nullptr;

    tReport* Parent = nullptr;
    tReportRoot* Root;

    const int MaxIndent = 10;
    const int IndentSize = 1;

    tVewSettings ViewSettings;
    tVewSettings ChildrenViewSettings;

    int ID = 0;
    int NewItemId = 0; // Used to create new items
    int Level;
    int Indent = 0;
    int IndentLeftPx = 0;
    int IndentRightPx = 0;
    int ReportNamePx = 0;

    //tPetrelProjectConfig Cfg;

public:
    const bool IsRoot = false;
    std::list<tReport> Children;
    bool ShowRootStatus = false;
    int GetId() const { return ID; }

    int LineNumber[eRep_NUM+1];
    int NextItemLineNumber = 0;

protected:
    // Settings for fonts
    const qreal FontSizeBase = 16;
    const qreal FontSizeTestNameDec = 1;
    const qreal FontSizeTestNameMin = 10;
    
    tReport(tReportRoot* root, tReport* parent, int level, QString name);

    void SetStatus();
    
    void SetBranchStatus(tTestStatus childStatus); // Only from current item to the root
    void Build();

private:
    //void EnableUpdate(bool value) { Root->EnableUpdate(value); } // Not used yet...
    void InvalidateLineNumbers();
    //QString FormatResult();

public:
    ~tReport();

    QString GetName() const { return Name; }
    
    tTestStatus GetStatus() const { return Status; }
    tTestStatus GetStatus() { return Status; }
    
    tTestStatus GetTestStatus() const { return TestStatus; }
    tTestStatus GetTestStatus() { return TestStatus; }
    QColor GetTestColor() const { return StatusList[int(Status)].TestStatusColor; }
    
    ///tTestResult GetResult() const { return Result; }
    //bool GetResultInternal() const { return Result.GetInternalCheckValue(); }

    QString GetDetails() const { return Details; }
    QString GetDetails() { return Details; }

    QString GetDescription() const { return Description; }
    QString GetDescription() { return Description; }

    tReport* GetParent() { return Parent; }
    bool IsCorrectlyFinished() const;
    bool IsFinalStatus() const;
    bool IsNotFinished()    const;
    void LinkSpec(tTestSpec* spec);

    tTestSpec* GetSpec() const { return TestSpec; }
    tTestSpec* GetSpec() { return (TestSpec); }

    // Add report convenience functions
    tReport* AddReportPending(QString name, bool allowDuplicates = false);
    tReport* AddReportTesting(QString name, bool allowDuplicates = false);
    tReport* AddReportSkipped(QString name, bool allowDuplicates = false);
    tReport* AddReport(QString name, bool allowDuplicates = false); // Add child

    void SetGroupStatus();

    //template <class T>
    //void SetResultValue(T value); // for standard checks

    //template <class T>
    //void SetResultValue(T value, bool internalValue); // for internal checks

    void ToggleExpandDetails(bool refresh);
    void ToggleExpandDescription(bool refresh);
    void ToggleExpandSubtree();

    void Expand(bool expand);
    void ExpandSubtree(bool expand);
    void ExpandNotPassed();

    void SetShowDetailsSubtree(bool value);
    void SetShowDetails(bool value, bool refresh);
    void SetShowDescriptionSubtree(bool value);
    void SetShowDescription(bool value, bool refresh);

    bool GetExpanded() const { return ViewSettings.IsExpanded; }
    bool GetShowDetails() const { return ViewSettings.IsShowDetails; }
    bool GetShowDescription() const { return ViewSettings.IsShowDescription; }

    void AddDetails(const QString& details);
    void SetStatus(tTestStatus newStatus, QString details);
    void SetStatus(tTestStatus newStatus);
    void SetResult(const tTestResult& testRes);
    void SetDescription(const QString& description) { Description = description; };

    void TestStatusChanged(tTestInfo testInfo);

    virtual void PrintRtf();
    void Remove();
    tReportRoot* GetRoot() { return Root; }

    int FindItemIdByName(QString name) const;

    tReport* FindItemByLineNumber(int lineNum);
    int GetElementTypeByLineNumber(int lineNum);

    int GetLineNumberByItemId(int id) const;
    int FindItemIdByLineNumber(int lineNum) const;

    static QString GetStatusMessage(tTestStatus status) { return StatusList[int(status)].TestStatusMessage; }
    static QColor GetStatusColor(tTestStatus status) { return StatusList[int(status)].TestStatusColor; }
};
