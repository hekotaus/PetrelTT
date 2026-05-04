#include "tReportRoot.h"

tReportRoot::tReportRoot(tReportType reportType, QTextEdit* reportView, tLogger* log, tPetrelProjectConfig& cfg)
    : Cfg(cfg), RootType(reportType), 
    tReport(this, nullptr, 0, "Test report") { // call to create root report item
    //ReportType = reportType;
    ReportView = reportView;
    ReportView->setDocument(ReportText);
    Summary.resize(int(TestSummary::LAST));
    //ReportView = reportView;
    switch (RootType) {
    case tReportType::AutoTest:
        Name = "Auto Test";
        //NewChildExpanded = false;
        //NewChildShowDetails = false;
        // DEBUG ONLY!!! 
        //if (false) {
        //    NewChildExpanded = true;
        //    NewChildShowDetails = true;
        //}
        AutoScroll = false;
        break;
    case tReportType::ManualTest:
        Name = "Manual Test";
        //NewChildExpanded = true;
        //NewChildShowDetails = true;
        AutoScroll = true;
        break;
    case tReportType::Config:
        Name = "Test Config";
        //NewChildExpanded = true;
        //NewChildShowDetails = true;
        AutoScroll = true;
        break;
    case tReportType::TestProc:
        Name = "Test Procedure";
        ChildrenViewSettings.IsExpanded = false;
        ChildrenViewSettings.IsShowDetails = false;
        AutoScroll = true;
        break;
    case tReportType::Reports:
        Name = "Test Reports";
        //NewChildExpanded = true;
        //NewChildShowDetails = true;
        AutoScroll = true;
        break;
    }

    {
        ReportFormat[eRepStatus].FontFamily = "Courier";
        ReportFormat[eRepResult].FontFamily = "Courier";
        ReportFormat[eRepResultComments].FontFamily = "Courier";
        ReportFormat[eRepDetails].FontFamily = "Courier";

        ReportFormat[eRepDescription].IsItalic = true;

        ReportFormat[eRepName].FontWeight = QFont::Bold;
        ReportFormat[eRepStatus].FontWeight = QFont::Bold;
        ReportFormat[eRepResult].FontWeight = QFont::Bold;

        ReportFormat[eRepStatus].Alignment = Qt::AlignRight;
        ReportFormat[eRepResult].Alignment = Qt::AlignRight;

        ReportFormat[eRepResult].FontPtSizeDiff = -1;
        ReportFormat[eRepResultComments].FontPtSizeDiff = -3;
        ReportFormat[eRepDetails].FontPtSizeDiff = -2;
        ReportFormat[eRepDescription].FontPtSizeDiff = -2;
    }

    //TextBoxVisible = textBox;
    ///TextBox = new RichTextBox(); // This is an invisible buffer
    ///ReportText.setVisible(false);
    ReportText->setTextWidth(ReportView->width());
    //ReportText.setTextHeight(ReportView->height());
    Log = log;
    //Cfg = cfg;
    TestDateTime = CurrentDateTime();
    ViewSettings.IsExpanded = true; // Root always expanded
    Root = this;
    Parent = nullptr;
    //Summary = new string[(int)TestSummary::LAST];
    Cfg.TestDateTime = TestDateTime;
    RtfFileName = Cfg.ReportDir +
        CurrentDateTime(true, true) + "--" +
        Cfg.DutName + "-rev." + Cfg.DutRevision + "--" +
        "PN-" + Cfg.DutPartNumber.replace('/', '-') + "--" +
        "SN-" + Cfg.DutSerialNumber.replace('/', '-') + "--" +
        Cfg.TestType + ".rtf";
    Clear();
}

void tReportRoot::SetVisible(bool visible) {
    Visible = visible;
    Refresh();
}

void tReportRoot::SetName(QString name) {
    if (name != "") Name = name;
}

void tReportRoot::Clear() {
    for(tReport& child : Children) {
        child.Remove();
    }
    Children.clear();
    SetStatus(tTestStatus::None, "");
}

void tReportRoot::SaveFirstVisibleItem() {
    StoredFirstVisibleItem = GetFirstVisibleItem();
}

void tReportRoot::LoadFirstVisibleItem() {
    SetFirstVisibleItem(StoredFirstVisibleItem);
    StoredFirstVisibleItem = 0;
}

void tReportRoot::Refresh() {
    if (!RefreshEnabled) return;
    if (!Visible) return;
    FirstVisibleItem = GetFirstVisibleItem();
    ReportView->setDocument(ReportText);
    ReportText->clear();
    if (ViewSettings.IsShowDetails) MakeSummary();
    Build();

    if (AutoScroll) {
#if true
        FirstVisibleItem = -1;
        ScrollToEnd();
#else
        // TODO: Scroll to last active item (TESTING)
        FirstVisibleItem = FindLastActiveItem();
        SetFirstVisibleItem();
        TextBoxVisible.SelectionStart = TextBox.SelectionStart;
        TextBoxVisible.SelectionLength = TextBox.SelectionLength;
        TextBoxVisible.ScrollToCaret();
#endif
    } else {
///        SetFirstVisibleItem();
///        TextBoxVisible.SelectionStart = TextBox.SelectionStart;
///        TextBoxVisible.SelectionLength = TextBox.SelectionLength;
///        TextBoxVisible.ScrollToCaret();
    }
///    TextBoxVisible.ClearUndo();
///    TextBox.ClearUndo();
    //Application.DoEvents();
}

void tReportRoot::PrintRtfPage1() {
#if 0 // RTF
    // Name
    TextBoxRtf.SelectionIndent = IndentLeftPx;
    TextBoxRtf.SelectionFont = FontTestName;
    TextBoxRtf.SelectionColor = FontColorTestName;
    TextBoxRtf.SelectionAlignment = HorizontalAlignment.Center;
    TextBoxRtf.SelectedText = "\r\n" + Name + "\r\n\r\n";
    // Status
    TextBoxRtf.SelectionAlignment = HorizontalAlignment.Center;
    TextBoxRtf.SelectionFont = FontTestStatus;
    TextBoxRtf.SelectionColor = TestStatusColors[(int)Status];
    TextBoxRtf.SelectedText = TestStatusMessages[(int)Status] + "\r\n\r\n";
    // Details
    TextBoxRtf.SelectionFont = FontTestDetails;
    TextBoxRtf.SelectionAlignment = HorizontalAlignment.Left;
    TextBoxRtf.SelectionColor = FontColorTestDetails;
    TextBoxRtf.SelectedText = Details + "\r\n\r\n";
    foreach(TReport child in Children) {
        // Name
        TextBoxRtf.SelectionIndent = IndentLeftPx;
        TextBoxRtf.SelectionFont = FontTestName;
        TextBoxRtf.SelectionColor = FontColorTestName;
        TextBoxRtf.SelectionAlignment = HorizontalAlignment.Left;
        TextBoxRtf.SelectedText = "\r\n" + child.GetName() + "\r\n";
        // Status
        TextBoxRtf.SelectionAlignment = HorizontalAlignment.Right;
        TextBoxRtf.SelectionFont = FontTestStatus;
        TextBoxRtf.SelectionColor = TestStatusColors[(int)child.GetStatus()];
        TextBoxRtf.SelectedText = TestStatusMessages[(int)child.GetStatus()] + "\r\n\r\n";
    }
    TextBoxRtf.SelectedText = "\f"; // Page break
#endif
}

void tReportRoot::PrintRtf() {
    //TextBox.Text = "";//TODO rtf?
///    TextBoxRtf.Text = "";
///    MakeSummary();
///    PrintRtfPage1();
///    base.PrintRtf();
///    TextBoxRtf.ClearUndo();
}

QString tReportRoot::BuildSummaryString(QString parameter, QString value) {
    return "    " + parameter + ": " + value + "\n";
}

void tReportRoot::MakeSummary() {
    Summary[(int)TestSummary::TestDate]         = BuildSummaryString("           Test date", TestDateTime);
    Summary[(int)TestSummary::Operator]         = BuildSummaryString("            Operator", Cfg.OperatorName);
    Summary[(int)TestSummary::TestSoftwareVer]  = BuildSummaryString("            Software", Cfg.AppName + " ver." + Cfg.AppVer.ToQString());
    Summary[(int)TestSummary::DeviceType]       = BuildSummaryString("         Device Type", Cfg.DutName);
    Summary[(int)TestSummary::DeviceRev]        = BuildSummaryString("     Device Revision", Cfg.DutRevision);
    Summary[(int)TestSummary::DevicePN]         = BuildSummaryString("          Device P/N", Cfg.DutPartNumber);
    Summary[(int)TestSummary::DeviceSN]         = BuildSummaryString("          Device S/N", Cfg.DutSerialNumber);
    Summary[(int)TestSummary::TestProcedureVer] = BuildSummaryString(" Test Procedure ver.", Cfg.TestProcedureVer);
    Summary[(int)TestSummary::TestSpecsVer]     = BuildSummaryString("     Test Specs ver.", Cfg.TestSpecsVer);
    Summary[(int)TestSummary::TestType]         = BuildSummaryString("           Test type", Cfg.TestType);
    Summary[(int)TestSummary::TestStatus]       = BuildSummaryString("         Test Status", GetStatusMessage(Status));
    //Summary[(int)TestSummary::TestStatistics]   = BuildSummaryString("Test Statistics", BuildTestStatistics());
    //Summary[(int)TestSummary::Comments]         = BuildSummaryString("Comments", "");
    switch (RootType) {
    case tReportType::AutoTest:
        Details =
            Summary[(int)TestSummary::TestDate] +
            Summary[(int)TestSummary::Operator] +
            Summary[(int)TestSummary::TestSoftwareVer] +
            Summary[(int)TestSummary::DeviceType] +
            Summary[(int)TestSummary::DeviceRev] +
            Summary[(int)TestSummary::DevicePN] +
            Summary[(int)TestSummary::DeviceSN] +
            Summary[(int)TestSummary::TestProcedureVer] +
            Summary[(int)TestSummary::TestSpecsVer] +
            Summary[(int)TestSummary::TestType];// +
        //Summary[(int)TestSummary::TestStatus];
        break;
    case tReportType::ManualTest:
        Details =
            Summary[(int)TestSummary::TestDate] +
            Summary[(int)TestSummary::Operator] +
            Summary[(int)TestSummary::TestSoftwareVer] +
            Summary[(int)TestSummary::DeviceType] +
            Summary[(int)TestSummary::DeviceRev] +
            Summary[(int)TestSummary::DevicePN] +
            Summary[(int)TestSummary::DeviceSN] +
            Summary[(int)TestSummary::TestProcedureVer] +
            Summary[(int)TestSummary::TestSpecsVer] +
            Summary[(int)TestSummary::TestType];// +
        //Summary[(int)TestSummary::TestStatus];
        break;
    case tReportType::Config:
        Details = "" +
        Summary[(int)TestSummary::TestDate] +
        //Summary[(int)TestSummary::Operator] +
        //Summary[(int)TestSummary::TestSoftwareVer] +
        //Summary[(int)TestSummary::DeviceType] +
        //Summary[(int)TestSummary::DeviceRev] +
        //Summary[(int)TestSummary::DevicePN] +
        //Summary[(int)TestSummary::DeviceSN] +
        //Summary[(int)TestSummary::TestProcedureVer] +
        //Summary[(int)TestSummary::TestSpecsVer] +
        //Summary[(int)TestSummary::TestType] +
        Summary[(int)TestSummary::TestStatus];
        break;
    case tReportType::TestProc:
        Details =
            //Summary[(int)TestSummary::TestDate] +
            Summary[(int)TestSummary::Operator] +
            Summary[(int)TestSummary::TestSoftwareVer] +
            Summary[(int)TestSummary::DeviceType] +
            Summary[(int)TestSummary::DeviceRev] +
            Summary[(int)TestSummary::DevicePN] +
            //Summary[(int)TestSummary::DeviceSN] +
            Summary[(int)TestSummary::TestProcedureVer] +
            Summary[(int)TestSummary::TestSpecsVer]
            //Summary[(int)TestSummary::TestType] +
            //Summary[(int)TestSummary::TestStatus]
            ;
        break;
    case tReportType::Reports:
        Details =
            Summary[(int)TestSummary::TestDate] +
            Summary[(int)TestSummary::Operator] +
            Summary[(int)TestSummary::TestSoftwareVer] +
            Summary[(int)TestSummary::DeviceType] +
            Summary[(int)TestSummary::DeviceRev] +
            Summary[(int)TestSummary::DevicePN] +
            Summary[(int)TestSummary::DeviceSN] +
            Summary[(int)TestSummary::TestProcedureVer] +
            Summary[(int)TestSummary::TestSpecsVer] +
            Summary[(int)TestSummary::TestType] +
            Summary[(int)TestSummary::TestStatus];
        break;

    }
    //Details = string.Join("\r\n", Summary);
}

void tReportRoot::SetFirstVisibleItem(int firstVisibleItem) {
    FirstVisibleItem = firstVisibleItem;
}

int tReportRoot::GetFirstVisibleItem() const {
    int firstLine = GetFirstVisibleLineNumber();
    return FindItemIdByLineNumber(firstLine);
}

void tReportRoot::SetFirstVisibleItem() {   // Correct item number according to visibility
    // TODO: if (!Parent.Expanded) SetFirstVisibleItem(Parent.TestNameItem) // Never hit null, as Root is always expanded
    if (((FirstVisibleItem % 3) == 1) && !ViewSettings.IsExpanded) FirstVisibleItem -= 1;
    if (((FirstVisibleItem % 3) == 2) && !ViewSettings.IsShowDetails) FirstVisibleItem -= 2;
    // Set it as first visible
    int line = GetLineNumberByItemId(FirstVisibleItem);
    if (line < 0) line = 0; // TODO: would be nice instead, if we tried to get Parent's line number until reach visible line or root
///    TextBox.SelectionStart = TextBox.GetFirstCharIndexFromLine(line);
///    TextBox.SelectionLength = TextBox.Lines[line].Length;
///    TextBox.ScrollToCaret();
}

int tReportRoot::GetFirstVisibleLineNumber() const {
    return GetLineNumberByPoint(QPoint(5, 5));
}

int tReportRoot::GetLineNumberByPoint(QPoint point) const {
///    int ChInd = TextBoxVisible.GetCharIndexFromPosition(point);
///    return TextBoxVisible.GetLineFromCharIndex(ChInd);
    return 0;
}

void tReportRoot::ScrollToTop() {
///    TextBoxVisible.SelectionStart = 0;
///    TextBoxVisible.ScrollToCaret();
}

void tReportRoot::ScrollToEnd() {
///    SendMessage(TextBoxVisible.Handle, WM_VSCROLL, (System.IntPtr)SB_PAGEBOTTOM, System.IntPtr.Zero);
}

bool tReportRoot::SaveToRtfFile(QString& fname) {
    return true;
#if 0
    TextBoxRtf = new RichTextBox();
    TextBoxRtf.Visible = false;
    TextBoxRtf.Width = TextBox.Width;
    TextBoxRtf.Height = TextBox.Height;
    PrintRtf();
    bool res = true;
    if (fname == "") {
        RtfFileName = Cfg.ReportDir +
            Tools.NowToStr(true, true) + "--" +
            Cfg.DeviceType + "-rev." + Cfg.DeviceRevision + "--" +
            "PN-" + Cfg.DevicePartNumber.Replace('/', '-') + "--" +
            "SN-" + Cfg.DeviceSerialNumber.Replace('/', '-') + "--" +
            Cfg.TestType + "-" +
            TestStatusMessages[(int)Status] +
            ".rtf";
        fname = RtfFileName;
    }

    try {
        TextBoxRtf.SaveFile(fname);
    } catch (IOException) {
        res = false;
    }
    return res;
#endif
}

void tReportRoot::ReportDoubleClicked(int lineNum) {
    qDebug() << "tReport::DoubleClicked()";
    tReport* rep = FindItemByLineNumber(lineNum);
    
    if (rep != nullptr) {
        qDebug() << "Clicked item:" << rep->GetName();
        // Get clicked element type
        int elType = rep->GetElementTypeByLineNumber(lineNum);
        switch (elType) {
        case eRepName:
            if (rep->Children.size() > 0) {
                rep->ToggleExpandSubtree();
            } else {
            }
            rep->SetShowDetails(rep->GetExpanded(), false);
            rep->SetShowDescription(rep->GetExpanded(), false);
            Root->Refresh();
            break;
        case eRepStatus: rep->ToggleExpandDetails(true); break;
        case eRepResult: rep->ToggleExpandDetails(true); break;
        case eRepDescription: rep->ToggleExpandDescription(true); break;
        case eRepDetails: rep->ToggleExpandDetails(true); break;
        }
    }

}