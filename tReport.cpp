#include "tReport.h"
#include "tReportRoot.h"
#include "common/tTickTock.h"

tReportSignaler g_ReportSignaler;

const QColor tReport::orangeRed = QColor(0xffff4500);
const QColor tReport::orange = QColor(0xffffa500);
const QColor tReport::dodgerBlue = QColor(0xff1e90ff);
const tReport::tRepStatus tReport::StatusList[int(tTestStatus::LAST)] = {
    { "", Qt::black },
    { "SKIPPED", Qt::gray },
    { "OK", Qt::darkGreen },
    { "FAILED", Qt::red },
    { "TEST ERROR", orangeRed},
    { "TESTED", orange},
    { "PENDING", dodgerBlue},
    { "PROGRESS...", Qt::blue},
    { "INTERRUPTED", orange},
};

bool tReport::IsCorrectlyFinished() const { // One of the final states
    return (
        (Status == tTestStatus::Failed) ||
        (Status == tTestStatus::Passed) ||
        (Status == tTestStatus::TestError) ||
        (Status == tTestStatus::Skipped)
        );
}

bool tReport::IsFinalStatus() const { // One of the final states
    return (
        (Status == tTestStatus::Failed) ||
        (Status == tTestStatus::Passed) ||
        (Status == tTestStatus::TestError) ||
        (Status == tTestStatus::Skipped) ||
        (Status == tTestStatus::Interrupted)
        );
}

bool tReport::IsNotFinished() const { // Test shceduled but did not finish
    return ((Status == tTestStatus::Pending) || (Status == tTestStatus::Testing));
}

void tReport::LinkSpec(tTestSpec* spec) {
    TestSpec = spec;
//    if (TestSpec != nullptr)
//        Result.SetValueType(TestSpec->GetValueType());
}

void tReport::SetResult(const tTestResult& testRes) {
    if (Status != tTestStatus::Testing) {
        SetStatus(tTestStatus::TestError);
        AddDetails(Name + ": Attempt to set value not at testing time!");
        return;
    }

    if (TestSpec == nullptr) {
        SetStatus(tTestStatus::TestError);
        AddDetails(Name + ": Attempt to set value to a test without TestSpec!");
        return;
    }

    if (TestSpec->GetIsPureGroup()) {
        SetStatus(tTestStatus::TestError);
        AddDetails(Name + ": Attempt to set value to a pure group!");
        return;
    }

    ResultValue = testRes.ToString();
    ResultComment = TestSpec->CheckTypeToString();// testRes.GetResultComment();
}

tReport::~tReport() {
    if (IsRoot) {
        delete ReportTextRtf;
        delete ReportText;
    }
}

tReport::tReport(tReportRoot* root, tReport* parent, int level, QString name) :
    //QObject(parent),
    IsRoot(root == this)
{
    Root = root;
    Parent = parent;
    Name = name;
    Level = level;

    if (IsRoot) {
        ReportText = new QTextDocument(); // Buffer/shadow
        //ReportTextVisible = new QTextDocument();// Root->ReportTextVisible; // Application UI
        ReportTextRtf = new QTextDocument(); //Root->ReportTextRtf; // Print here to output file
        //ReportView = Root->ReportView;
        //Expanded = NewChildExpanded;
        //ShowDetails = NewChildShowDetails;
    } else {
        //ShowDetails = Parent->ShowDetails;
        ReportText = Root->ReportText; // Buffer/shadow
        //ReportTextVisible = Root->ReportTextVisible; // Application UI
        ReportTextRtf = Root->ReportTextRtf; // Print here to output file
        ReportView = Root->ReportView;
        //IsExpanded = Root->NewChildExpanded;
        //IsShowDetails = Root->NewChildShowDetails;

        ViewSettings = Parent->ChildrenViewSettings;
        ChildrenViewSettings = Parent->ChildrenViewSettings;
    }

    Indent = IndentSize * Level;
    if (Indent > MaxIndent) Indent = MaxIndent;

    // Font settings for Test Name and Test Status
    IndentRightPx = 0;
    if (Level == 0)
        IndentLeftPx = 0;
    else
        IndentLeftPx = (Level - 1) * 40;// (int)(IndentSize * StringWidthK + StringWidthB);
}

tReport* tReport::AddReportPending(QString name, bool allowDuplicates) {
    bool re = Root->GetRefreshEnabled();
    Root->SetRefreshEnabled(false); // To avoid double refreshing
    tReport* res = AddReport(name, allowDuplicates);
    Root->SetRefreshEnabled(re);
    res->SetStatus(tTestStatus::Pending);
    res->Details = ""; // 20250828 to avoid adding details to the previous tets
    return res;
}

tReport* tReport::AddReportTesting(QString name, bool allowDuplicates) {
    bool re = Root->GetRefreshEnabled();
    Root->SetRefreshEnabled(false); // To avoid double refreshing
    tReport* res = AddReport(name, allowDuplicates);
    Root->SetRefreshEnabled(re);
    res->SetStatus(tTestStatus::Testing);
    return res;
}

tReport* tReport::AddReportSkipped(QString name, bool allowDuplicates) {
    bool re = Root->GetRefreshEnabled();
    Root->SetRefreshEnabled(false); // To avoid double refreshing
    tReport* res = AddReport(name, allowDuplicates);
    Root->SetRefreshEnabled(re);
    res->SetStatus(tTestStatus::Skipped);
    return res;
}

tReport* tReport::AddReport(QString name, bool allowDuplicates) { // Add child
    bool duplicateFound = false;
    if (!allowDuplicates) {
        // TODO: investigate behaviour of FindItemIdByName.
        // Probably, we need to search not from Root, as during TP loading, the same test name appears many times
        // Also, may be, we don't need to find duplicates every time. Just do it on TP load or verification
        // However, this may cause untracked error during manual tests
#if true
        while (Root->FindItemIdByName(name) != -1) { // Duplicate found
            name += "-";
            duplicateFound = true;
        }
#else
        if (Root.FindItemIdByName(name) != -1) // Duplicate found
            duplicateFound = true;
#endif
    }
    NewItemId = NewItemId + eRep_NUM; // +0= test name, +1 = test Status, +2 = Result, +3=Test Description, +4=Details
    tReport newChild = tReport(Root, this, Level + 1, name);
    newChild.ID = NewItemId;

    if (duplicateFound /*&& (!newChild.GetSpec().IsPureTest())*/) {
        newChild.AddDetails("Test with the same name found!");
        newChild.AddDetails(name);
        newChild.SetStatus(tTestStatus::TestError);
    }
    Children.push_back(newChild);
    Root->Refresh();
    tReport* ptr = &(*Children.rbegin());
    return ptr;
}

void tReport::SetShowDetailsSubtree(bool value) {
    //if (Level == 0) Root->
    ChildrenViewSettings.IsShowDetails = value; // Newly created reports wiil show details
    ViewSettings.IsShowDetails = value;
    for(tReport& child : Children) {
        child.SetShowDetailsSubtree(value); // child.Dispose(); called by the child
    }
    if (Level == 0) Build();
}

void tReport::SetShowDetails(bool value, bool refresh) {
    ViewSettings.IsShowDetails = value;
    if (refresh) Root->Refresh();
}

void tReport::SetShowDescriptionSubtree(bool value) {
    //if (Level == 0) Root->
    ChildrenViewSettings.IsShowDescription = value; // Newly created reports wiil show details
    ViewSettings.IsShowDescription = value;
    for (tReport& child : Children) {
        child.SetShowDescriptionSubtree(value); // child.Dispose(); called by the child
    }
    if (Level == 0) Build();
}

void tReport::SetShowDescription(bool value, bool refresh) {
    ViewSettings.IsShowDescription = value;
    if (refresh) Root->Refresh();
}

void tReport::AddDetails(const QString& details) {
    Details += details + "\n";
    Root->Refresh();
}

void tReport::SetStatus() { // Summarise children's GroupStatus and local TestStatus, call Parent.SetGroupStatus()
    // combine GroupStatus and TestStatus
    // Select the Highest TestStatus

    // 1. No specs => Not a group => Test
    // 2. No specs && noChildren => New created group requires particular Status
    // 3. No specs && hasChildren => Manual test group
    if (TestSpec == nullptr) {
        if (Children.size() == 0)
            Status = TestStatus;
        else
            Status = GroupStatus;
    } else {
        if (TestSpec->GetIsPureGroup()) Status = GroupStatus;
        else if (TestSpec->GetIsPureTest()) Status = TestStatus;
        else Status = (tTestStatus)max((int)GroupStatus, (int)TestStatus);
    }
    // Parent.SetGroupStatus or Root.Refresh
    if (IsRoot) {
        Root->Refresh();
        g_ReportSignaler.SignalStatusUpdate();
    } else {
        //Parent->SetGroupStatus();
        //tick(QString("SetBranchStatus " + Name).toStdString());
        Parent->SetBranchStatus(Status);
        //tock_s();
    }
}

void tReport::SetStatus(tTestStatus newStatus, QString details) { // Change Local TestStatus
    SetStatus(newStatus);
    AddDetails(details);
}

void tReport::SetStatus(tTestStatus newStatus) { // Change Local TestStatus
    bool ChangeAllowed = false;
    if (GetSpec() != nullptr) {
        switch (TestStatus) {
        case tTestStatus::None: // change to any
            ChangeAllowed = !((newStatus == tTestStatus::Passed) || (newStatus == tTestStatus::Failed));
            break;
        case tTestStatus::Pending: // change only to TESTING or SKIPPED or INTERRUPTED or TESTERROR
            ChangeAllowed = (
                (newStatus == tTestStatus::Testing) ||
                (newStatus == tTestStatus::Skipped) ||
                (newStatus == tTestStatus::Interrupted) ||
                (newStatus == tTestStatus::TestError)
                );
            break;
        case tTestStatus::Tested: // Change to PASSED or FAILED
            ChangeAllowed = (
                (newStatus == tTestStatus::Passed) || 
                (newStatus == tTestStatus::Failed) ||
                (newStatus == tTestStatus::TestError)
                );
            break;
            // These are final resuls, which cannot be changed
        case tTestStatus::Skipped: // only change to Test error or Interrupted allowed for EndPoints
        case tTestStatus::Passed:
        case tTestStatus::Failed:
        case tTestStatus::TestError:
            ChangeAllowed = (
                (newStatus == tTestStatus::TestError) || 
                (newStatus == tTestStatus::Interrupted));
            break;
        case tTestStatus::Testing: // change only to TESTED or TEST ERROR or SKIPPED or INTERRUPTED
            ChangeAllowed = (
                (newStatus == tTestStatus::Tested) || 
                (newStatus == tTestStatus::TestError) ||
                (newStatus == tTestStatus::Interrupted) || 
                (newStatus == tTestStatus::Skipped)); // 20191025 added ->Skipped
            break;
        case tTestStatus::Interrupted: ChangeAllowed = false; break;
        }
    } else { // Despite it's test with not specs linked, do some weak checks any way
        switch (TestStatus) {
        case tTestStatus::None:
            ChangeAllowed = true;
            break;
        case tTestStatus::Pending: // change only to TESTING or SKIPPED or INTERRUPTED or TEST ERROR
            ChangeAllowed = (
                (newStatus == tTestStatus::Testing) ||
                (newStatus == tTestStatus::Skipped) ||
                (newStatus == tTestStatus::Interrupted) ||
                (newStatus == tTestStatus::TestError));
            break;
        case tTestStatus::Tested: // Change to PASSED or FAILED
            ChangeAllowed = ((newStatus == tTestStatus::Passed) || (newStatus == tTestStatus::Failed));
            break;
            // These are final resuls, which cannot be changed
        case tTestStatus::Skipped: // no change allowed for EndPoints
        case tTestStatus::Passed:
        case tTestStatus::Failed:
        case tTestStatus::TestError:
            ChangeAllowed = (newStatus == tTestStatus::Interrupted);
            break;
        case tTestStatus::Testing: // change to any
            ChangeAllowed = true;
            break;
        case tTestStatus::Interrupted: ChangeAllowed = false; break;
        }
    }
    if (ChangeAllowed) {
        // Logging here significantly increases Specs Loading time and Log readability!
        //Log.AddTestMessage(
        //    "Test Status changed from " + TestStatusMessages[(int)Status] + 
        //    " to " + TestStatusMessages[(int)newStatus]);
        TestStatus = newStatus;
    } else {
        QString sTmp =
            "Unable to change Test Status from " + GetStatusMessage(TestStatus) +
            " to " + GetStatusMessage(newStatus);
        if (Log) Log->LogErrorMessage(sTmp);
        AddDetails(QString("ERROR: ") + sTmp + "!");

        TestStatus = tTestStatus::TestError;
    }

    if (TestStatus == tTestStatus::Tested) {
        //bool re = Root->GetRefreshEnabled();
        Root->SetRefreshEnabled(false);
    }

    SetStatus(); // Group + Local and Parent
}

void tReport::TestStatusChanged(tTestInfo testInfo) { // Called by TestProcedure via Signal
    if (testInfo.Status == tTestStatus::None) return; // this is for test groups
    if ((testInfo.Status == tTestStatus::Tested) ||
        (testInfo.Status == tTestStatus::Skipped) ||
        (testInfo.Status == tTestStatus::TestError)) {
        bool re = Root->GetRefreshEnabled();
        Root->SetRefreshEnabled(false);
        //DateTime t0 = DateTime.Now;
        AddDetails(testInfo.Details);
        if (testInfo.Result.IsValueSet())
            SetResult(testInfo.Result);
        //DateTime t1 = DateTime.Now;
        //Console.WriteLine("AddDetails(" + testInfo.Details + ") " + (t1 - t0).ToString());
        Root->SetRefreshEnabled(re);
    }

    if (testInfo.Status == tTestStatus::Tested) {
        // value type check is already done in Setvalue()
///        if ((TestSpec != nullptr) && (TestSpec->GetCheckType() == tTestSpec::tCheckType::Internal))
///            SetResultValue(testInfo.Result, testInfo.ResultInternal);
///        else
///            SetResultValue(testInfo.Result);
    }
    SetStatus(testInfo.Status);
}

void tReport::SetGroupStatus() { // Update Group Status from ./* and call ../SetGroupStatus()
    // Calculate new GroupStatus from children
    //TTestStatus newStatus = _Status;
    GroupStatus = tTestStatus::None;
    for(tReport& child : Children) {
        child.SetGroupStatus();
        if (GroupStatus < child.Status)
            GroupStatus = child.Status;
    }
    ///SetStatus();
}

void tReport::SetBranchStatus(tTestStatus childStatus) { // Update Group Status from ./* and call ../SetGroupStatus()
    // Calculate new GroupStatus from children
    //TTestStatus newStatus = _Status;
    //GroupStatus = tTestStatus::None;

    SetGroupStatus();

    if (GroupStatus < childStatus) 
        GroupStatus = childStatus;
        
    //if ((GroupStatus == tTestStatus::Pending) || 
    //    (GroupStatus == tTestStatus::Testing) ||
    //    (GroupStatus == tTestStatus::Tested)
    //    )
    //    GroupStatus = childStatus;
    SetStatus();
}


void tReport::Expand(bool expand) { // Expands this item
    if (Level == 0) expand = true; // Never collapse Root
    ViewSettings.IsExpanded = expand;
    //Root.Refresh();
}
#if 0
void tReport::ToggleExpandAndDetails(int itemId) {
    // Scan all subtree and find item with required Id
    // Root toglles Detailed
    // others toogle expand and details
    if (ID == itemId) {
        bool toggle;
        if (IsRoot) toggle = ViewSettings.IsShowDetails; else toggle = ViewSettings.IsExpanded;
        toggle = !toggle;
        Expand(toggle);
        ViewSettings.IsShowDetails = toggle;
        if (IsRoot) Root->Refresh();
    } else {
        for(tReport& child : Children)
            child.ToggleExpandAndDetails(itemId);
    }
}
#endif
void tReport::ExpandNotPassed() {
//    if (Level == 0) Root->NewChildExpanded = expand;

    if (GetStatus() != tTestStatus::Passed) {
        Expand(true);
        SetShowDetails(true, false);
        for (tReport& child : Children) {
            //child.ExpandSubtree(child.GetStatus() != tTestStatus::Passed);
            child.ExpandNotPassed();
        }
    }
    //if (Level > 0) ExpandNotPassed();
    //else 
    if (Level == 0)  
        Root->Refresh();
}

void tReport::ToggleExpandDetails(bool refresh) {
    ViewSettings.IsShowDetails = !ViewSettings.IsShowDetails;
    if (refresh) Root->Refresh();
}

void tReport::ToggleExpandDescription(bool refresh) {
    ViewSettings.IsShowDescription = !ViewSettings.IsShowDescription;
    if (refresh) Root->Refresh();
}

void tReport::ToggleExpandSubtree() {
    ViewSettings.IsExpanded = !ViewSettings.IsExpanded;
    Root->Refresh();
}

void tReport::ExpandSubtree(bool expand) {
    //if (Level == 0) Root->
    ViewSettings.IsExpanded = expand;
    ChildrenViewSettings.IsExpanded = expand;
    for(tReport& child : Children)
        child.ExpandSubtree(expand);
    if (Level > 0) Expand(expand);
    else Root->Refresh();
}

void tReport::InvalidateLineNumbers() {
    for (int i = 0; i < eRep_NUM; i++)
        LineNumber[i] = -1;
    for(tReport& child : Children)
        child.InvalidateLineNumbers();
}
#if 0
QString tReport::FormatResult() { // TODO: remove. Format defined by tSpec and tResultValue
    QString res = "";
///    if (Result->IsValueSet() && (Result.GetValue() != nullptr)) {
        // Value
///        if (TestSpec->IsHex || TestSpec->IsBin)
///            res += "0x" + Result.GetValue().ToString("X8");
///        else
///            res += Result.GetValue().ToString();
///        if (TestSpec->GetValueType() == tTestResult::tValueType::String)
///            res = "'" + res + "'";
        // Units
///        if (TestSpec->GetUnits() != "") {
///            if (TestSpec->GetValueType() == tTestResult::tValueType::String)
///                res += " ";
///            res += TestSpec->GetUnits();
///        }

        // Range
        if ((TestStatus == tTestStatus::Failed) || (TestStatus == tTestStatus::Passed)) {
///            if (TestSpec.GetCheckType() != tTestSpec::tCheckType::Internal) {
///                if (TestStatus == tTestStatus::Failed) res += " is not ";
///                else if (TestStatus == tTestStatus::Passed) res += " is ";
///            }
///            res = res + TestSpec.CheckTypeToString();
            // Status (needed for supertests only)
            if (TestStatus == tTestStatus::Passed) res += " -- PASSED";
            else if (TestStatus == tTestStatus::Failed) res += " -- FAILED";
        }
        res += "\n";
///    } else {
///        res = "";
///    }
    return res;
}
#endif
void tReport::Build() { // Standard
    if (IsRoot)
        ReportText->clear();
//    TestNameLineNumber = ReportText->lineCount();// toPlainText() //TextBox.Lines.Count();TestNameLineNumber = ReportText->lineCount();
    ///TextBox->GetLineFromCharIndex(TextBox->SelectionStart);
//if (Level > 0) TestNameLineNumber--; //For the root we don't have previous line
    ReportView->setDocument(ReportText);
    qreal fontSize = max(FontSizeTestNameMin, FontSizeBase - Level * FontSizeTestNameDec);

    QTextCursor cursor = QTextCursor(Root->ReportText);
    cursor.movePosition(QTextCursor::MoveOperation::End);
    int posBegin = cursor.position();

    for (int iRepEl = 0; iRepEl < eRep_NUM; iRepEl++) {
        tReportFormat& fmt = Root->ReportFormat[iRepEl];
        ReportView->setFontPointSize(fontSize + fmt.FontPtSizeDiff);
        ReportView->setFontFamily(fmt.FontFamily);
        ReportView->setFontItalic(fmt.IsItalic);
        ReportView->setFontWeight(fmt.FontWeight);
        ReportView->setTextColor(fmt.TextColor);
        Qt::Alignment alignment = fmt.Alignment; // must be set after appending text only

        LineNumber[iRepEl] = ReportText->lineCount();
        if (IsRoot && (iRepEl == 0)) LineNumber[iRepEl]--;
        //qDebug() << "ReportText" << ReportText->toPlainText();
        switch (iRepEl) {
        case eRepName:
            if (ViewSettings.IsShowName) {
                //if (Level > 0) TestNameLineNumber--; //For the root we don't have previous line ???
                ReportView->setFontPointSize(IsRoot ? 20 : fontSize);
                QString strExpand = "";
                if ((Children.size() > 0) && !IsRoot) {
                    if (ViewSettings.IsExpanded) strExpand = " <<"; else strExpand = " >>";
                }
                ReportView->append(Name + strExpand);
                if (IsRoot) 
                    alignment = Qt::AlignHCenter;
                ReportView->setAlignment(alignment);
            }
            break; // Name

        case eRepStatus:
            if (ViewSettings.IsShowStatus) {
                ReportView->setTextColor(StatusList[(int)Status].TestStatusColor);
                bool isShowStatus = !IsRoot || ShowRootStatus;
                if (isShowStatus)
                    ReportView->append(StatusList[(int)Status].TestStatusMessage);
                else
                    ReportView->append("");
                ReportView->setAlignment(alignment);
            }
            break; // Status

        case eRepResult:
            if (ViewSettings.IsShowResultValue) {
                QString sRes = ResultValue;/// Result.ToString();
                ///QString sComment = Result.GetResultComment();
                //QString resString;
                if (!sRes.isEmpty()) {

                    if (!ResultComment.isEmpty()) {
                        //QString commPrefix = "";
                        switch (Status) {
                        case tTestStatus::Tested: break; // Just leave result
                        case tTestStatus::Passed: sRes += " is " + ResultComment;  break;
                        case tTestStatus::Failed: sRes += " is not " + ResultComment;  break;
                        } // switch
                        //sRes += ResultComment;
                    }
                    ReportView->append("Result: " + sRes);
                    ReportView->setAlignment(alignment);
                }
            }
            break; // Result

        case eRepDescription:
            if (ViewSettings.IsShowDescription && !Description.isEmpty()) {
                ReportView->append(Description);
                ReportView->setAlignment(alignment);
            }
            break; // Description

        case eRepDetails:
            if (ViewSettings.IsShowDetails && !Details.isEmpty()) {
                LineNumber[eRepDetails] = ReportText->lineCount();
                // Trick to align details multiple lines
                ReportView->setFontPointSize(1);
                ReportView->append("");
                ReportView->setAlignment(Qt::AlignLeft);

                // Details
                ReportView->setFontPointSize(fontSize + fmt.FontPtSizeDiff);
                ReportView->append(Details);
                ReportView->setAlignment(alignment);
            } else {
                //ReportView->append("");
            }
            break;
        }
        
    } // for iRepEl
    LineNumber[eRepNext] = ReportText->lineCount();
    //NextItemLineNumber = ReportText->lineCount();
///        TextBox.GetLineFromCharIndex(TextBox.SelectionStart);

    int posEnd = cursor.position();
    //qDebug() << "Level =" << Level << "Begin =" << posBegin << "End =" << posEnd;
    cursor.setPosition(posBegin, QTextCursor::MoveAnchor);
    cursor.setPosition(posEnd, QTextCursor::KeepAnchor);
    QTextBlockFormat fmt;
    fmt.setTextIndent(IndentLeftPx);
    cursor.mergeBlockFormat(fmt);

    if (ViewSettings.IsExpanded)
        for(tReport& child : Children) child.Build();
    else
        for(tReport& child : Children) child.InvalidateLineNumbers();
}

void tReport::PrintRtf() {
    if (Level != 0) {
#if 0 // TRANSLATE INTO QT
        // Name
        TextBoxRtf->SelectionIndent = IndentLeftPx;
        TextBoxRtf->SelectionFont = FontTestName;
        TextBoxRtf->SelectionColor = FontColorTestName;
        TextBoxRtf->SelectionAlignment = HorizontalAlignment.Left;
        TextBoxRtf->SelectedText = Name + "\n";
        // Status
        TextBoxRtf->SelectionAlignment = HorizontalAlignment.Right;
        TextBoxRtf->SelectionFont = FontTestStatus;
        TextBoxRtf->SelectionColor = TestStatusColors[(int)Status];
        TextBoxRtf->SelectedText = TestStatusMessages[(int)Status] + "\n\n";
        // Details
        TextBoxRtf->SelectionFont = FontTestDetails;
        TextBoxRtf->SelectionAlignment = HorizontalAlignment.Left;
        TextBoxRtf->SelectionColor = FontColorTestDetails;
        TextBoxRtf->SelectedText = FormatResult() + Details + "\n\n";
#endif
    }
    for(tReport& child : Children) child.PrintRtf();
}

void tReport::Remove() { // Remove all the sub-tree
    for(tReport& child : Children) {
        child.Remove(); // child.Dispose(); called by the child
    }
    Children.clear();
}

int tReport::FindItemIdByName(QString name) const {
    int res = -1;
    for(const tReport& child : Children) {
        if (res < 0)
            if (child.Name == name) res = child.ID;
        // TODO: this finds false duplicates!
        //if (res < 0)
        //    res = child.FindItemIdByName(name);

    }
    // Added just on 11/09/2019
    return res;
}

int tReport::GetElementTypeByLineNumber(int lineNum) {
    for (int i = 0; i < eRepNext; i++) {
        if ((lineNum >= LineNumber[i]) && (lineNum < LineNumber[i + 1]))
            return i;
    }
    return -1;
}

tReport* tReport::FindItemByLineNumber(int lineNum) {
    tReport* res = nullptr;
    //for (int i = 0; i < eRepNext; i++) {
    //    if ((lineNum >= LineNumber[i]) && (lineNum < LineNumber[i + 1]))
    //        res = this;
    //}
    if ((lineNum >= LineNumber[0]) && (lineNum < LineNumber[eRepNext]))
        res = this;

    if (res == nullptr) {
        // search children
        for (tReport& ch : Children) {
            res = ch.FindItemByLineNumber(lineNum);
            if (res != nullptr)
                break;
        }
    };
    return res;
}
#if 1
int tReport::FindItemIdByLineNumber(int line) const {
    // Check if line matches any of this items. If not, call children's methods
    //QString s1 = Name;
    int res = -1;
    for (int i = 0; i < (eRep_NUM - 1); i++) {
        if ((line >= LineNumber[i]) && (line < LineNumber[i + 1]))
            res = ID + i;
    }
    if (res == -1) { // didn't find in current layer
        for (const tReport& child : Children) {
            if (res == -1) res = child.FindItemIdByLineNumber(line);
        }
    }

    //if ((line >= TestNameLineNumber) && (line < TestResLineNumber)) res = ID;
    //else if ((line >= TestResLineNumber) && (line < TestDetailsLineNumber)) res = ID + 1;
    //else if ((line >= TestDetailsLineNumber) && (line < NextItemLineNumber)) res = ID + 2;
    //else {
    //    for(const tReport& child : Children) {
    //        if (res == -1) res = child.FindItemIdByLineNumber(line);
    //    }
    //}
    return res;
}

int tReport::GetLineNumberByItemId(int id) const {
    int res = -1;
//    if (id == ID) res = TestNameLineNumber;
//    else if (id == ID + 1) res = TestResLineNumber;
//    else if (id == ID + 2) res = TestDetailsLineNumber;
//    else {
//        for(const tReport& child : Children) {
//            if (res == -1) res = child.GetLineNumberByItemId(id);
//        }
//    }
    return res;
}

#endif
//796