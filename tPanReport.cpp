#include "tPanReport.h"

tPanReport::tPanReport(QWidget* parent, int id, tLogger* log, tPetrelProjectConfig& cfg)
    : tPanPetrel(parent, id, "Report")
    , Cfg(cfg)
    //, ReportView(this) 
{

    Cfg.ReportAutoTest = new tReportRoot(tReportType::AutoTest, &ReportView, log, cfg);
    Cfg.ReportManualTest = new tReportRoot(tReportType::ManualTest, &ReportView, log, cfg);
    Cfg.ReportConfig = new tReportRoot(tReportType::Config, &ReportView, log, cfg);
    Cfg.ReportTestProc = new tReportRoot(tReportType::TestProc, &ReportView, log, cfg);
    Cfg.ReportReports = new tReportRoot(tReportType::Reports, &ReportView, log, cfg);

    AddWidget(&ReportView);
    AddWidget(&cbShowDescription);
    AddWidget(&cbShowDetails);
    AddWidget(&btnExpandAll);
    AddWidget(&btnCollapseAll);
    AddWidget(&btnClear);

    int loIdx = AddLayout(0, 0, tPanelStyle(eBorderStyle::None, true), StdH);
    auto Lo1 = GetLayoutPtr(1);
    auto pad = Lo1->GetPaddings();
    pad.setBottom(5);
    Lo1->SetPaddings(pad);

    int y;
    y = PaddingY;
    int dy = LabH * 1.5;
    Lo1->AddWidget(&cbShowDescription, X_1_4, y, W_1_4, CbH);
    Lo1->AddWidget(&cbShowDetails, X_2_4, y, W_1_4, CbH);
    Lo1->AddWidget(&btnExpandAll, X_3_4, y, W_1_4, CbH);
    Lo1->AddWidget(&btnCollapseAll, X_4_4, y, W_1_4, CbH);
    const int X_5_4 = PaddingX + (W_1_4 + SpacingX) * 4;
    Lo1->AddWidget(&btnClear, X_5_4, y, W_1_4, CbH);
    y += dy;
    RepY = y;
    Lo1->AddWidget(&ReportView, PaddingX, y, 100, 100);

    SetLayout(1);

    connect(&cbShowDescription, &QCheckBox::checkStateChanged, this, &tPanReport::slotToggleDescription);
    connect(&cbShowDetails, &QCheckBox::checkStateChanged, this, &tPanReport::slotToggleDetails);
    connect(&btnExpandAll, &QPushButton::clicked, this, &tPanReport::slotExpandAll);
    connect(&btnCollapseAll, &QPushButton::clicked, this, &tPanReport::slotCollapseAll);
    connect(&btnClear, &QPushButton::clicked, this, &tPanReport::slotClear);
    connect(&ReportView, &tReportTextEdit::sigReportDoubleClicked, this, &tPanReport::slotReportDoubleClicked);
}

tPanReport::~tPanReport() {
    disconnect(&cbShowDescription, &QCheckBox::checkStateChanged, this, &tPanReport::slotToggleDescription);
    disconnect(&cbShowDetails, &QCheckBox::checkStateChanged, this, &tPanReport::slotToggleDetails);
    disconnect(&btnExpandAll, &QPushButton::clicked, this, &tPanReport::slotExpandAll);
    disconnect(&btnCollapseAll, &QPushButton::clicked, this, &tPanReport::slotCollapseAll);
    disconnect(&btnClear, &QPushButton::clicked, this, &tPanReport::slotClear);
    disconnect(&ReportView, &tReportTextEdit::sigReportDoubleClicked, this, &tPanReport::slotReportDoubleClicked);
}

void tPanReport::SetCurrentReport(tReportType typ) {
    switch (typ) {
    case tReportType::AutoTest: Cfg.ReportCurrent = Cfg.ReportAutoTest; break;
    case tReportType::ManualTest: Cfg.ReportCurrent = Cfg.ReportManualTest; break;
    case tReportType::Config:  Cfg.ReportCurrent = Cfg.ReportConfig; break;
    case tReportType::TestProc: Cfg.ReportCurrent = Cfg.ReportTestProc; break;
    case tReportType::Reports:  Cfg.ReportCurrent = Cfg.ReportReports; break; // View reports
    default: Cfg.ReportCurrent = nullptr;
    }
    if (Cfg.ReportCurrent == nullptr) return;

    cbShowDetails.setChecked(Cfg.ReportCurrent->GetShowDetails());
    cbShowDescription.setChecked(Cfg.ReportCurrent->GetShowDescription());

    SetCaption("Report: " + Cfg.ReportCurrent->GetName());
    update();
    qDebug() << "Report caption" << GetCaption();
    ReportView.setDocument(Cfg.ReportCurrent->GetDocument());
    Cfg.ReportCurrent->Refresh();
}

void tPanReport::SetHeight(int h) { // TODO: move to tPanel
    auto lo = GetCurLayoutPtr();
    QMargins paddings = lo->GetPaddings();
    int loH = h - paddings.top() - paddings.bottom();
    SetAllLayoutsHeight(loH);
    lo->Elements[&ReportView].H = loH - RepY - PaddingY;// -lo->GetPaddings().top() - lo->GetPaddings().bottom();


    SetLayout();
}

void tPanReport::resizeEvent(QResizeEvent* event) {
    auto Lo1 = GetLayoutPtr(1);
    tLayoutElement& loel = Lo1->Elements[&ReportView];
    QSize loSiz = Lo1->GetSize();
    //loel.X;// = PaddingX;
    loel.W = loSiz.width() - 2 * PaddingX;

    //loel.Y = PaddingY;
    //qDebug() << "panRep.siz" << event->size().height();
    SetLayout();
    tPanPetrel::resizeEvent(event);
}

void tPanReport::slotToggleDetails(Qt::CheckState state) {
    Cfg.ReportCurrent->SetShowDetailsSubtree(state == Qt::Checked);
}

void tPanReport::slotToggleDescription(Qt::CheckState state) {
    Cfg.ReportCurrent->SetShowDescriptionSubtree(state == Qt::Checked);
}

void tPanReport::slotExpandAll() {
    Cfg.ReportCurrent->ExpandSubtree(true);
}

void tPanReport::slotCollapseAll() {
    Cfg.ReportCurrent->ExpandSubtree(false);
}

void tPanReport::slotClear() {
    Cfg.ReportCurrent->Clear();
}

//checkStateChanged(Qt::CheckState state)

void tPanReport::slotReportDoubleClicked(int lineNum) {
    Cfg.ReportCurrent->ReportDoubleClicked(lineNum); // We don't want tReport to be a QObject, so signal it indirectly
}
