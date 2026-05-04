#pragma once
#include "tReport.h"
#include "tReportCommon.h"

class tReportRoot : public tReport { // Only one, never deleted.
private:
///private static extern int SendMessage(System.IntPtr hWnd, int wMsg, System.IntPtr wParam, System.IntPtr lParam);
    ///const int WM_VSCROLL = 277;
    ///const int SB_PAGEBOTTOM = 7;
    tPetrelProjectConfig& Cfg;

    int FirstVisibleItem = -1;
    QString RtfFileName = "";
    QString TestDateTime;
    int StoredFirstVisibleItem = 0;
    bool RefreshEnabled = true; // Disable to reduce Report lodaing time
    QStringList Summary;

    void PrintRtf();
    void MakeSummary();
    QString BuildSummaryString(QString parameter, QString value);
    void SetFirstVisibleItem(int firstVisibleItem);
    int GetFirstVisibleItem() const;
    void SetFirstVisibleItem();
    int GetFirstVisibleLineNumber() const;

protected:

public:
    //QTextDocument* ReportText = new QTextDocument(nullptr); // Buffer/shadow
    //QTextDocument* ReportTextVisible = new QTextDocument(nullptr); // Application UI
    //QTextDocument* ReportTextRtf = new QTextDocument(nullptr); // Print here to output file
    //QTextEdit* ReportView = nullptr;

    const tReportType RootType = tReportType::None;
    bool AutoScroll = true;
    bool Visible = true;
    //bool NewChildShowDetails = true;
    //bool NewChildExpanded = true;

    ///void SetRefreshEnabled(bool value) { RefreshEnabled = value; }
    void SetRefreshEnabled(bool value) { RefreshEnabled = true; }
    bool GetRefreshEnabled() const { return RefreshEnabled; }

    enum class TestSummary {
        DeviceType,
        DeviceRev,
        DevicePN,
        DeviceSN,
        TestDate,
        TestType,
        TestSoftwareVer,
        TestProcedureVer,
        TestSpecsVer,
        TestStatus,
        //TestStatistics,
        Operator,
        //Comments,
        LAST
    };

    tReportFormat ReportFormat[eRep_NUM];

    tReportRoot(tReportType reportType, QTextEdit* reportView, tLogger* log, tPetrelProjectConfig& cfg); // call to create root report item
    void SetVisible(bool visible);
    void SetName(QString name);
    void Clear();
    void SaveFirstVisibleItem();
    void LoadFirstVisibleItem();
    void Refresh();
    void PrintRtfPage1();
    int GetLineNumberByPoint(QPoint point) const;
    void ScrollToTop();
    void ScrollToEnd();
    bool SaveToRtfFile(QString& fname);
    //QTextDocument* GetDocument() { return ReportTextVisible; }
    QTextDocument* GetDocument() { return ReportText; }
    //void SetDocument(QTextDocument* doc) { ReportTextVisible}
    void ReportDoubleClicked(int);
};
