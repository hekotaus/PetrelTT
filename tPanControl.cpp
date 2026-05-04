#include "tPanControl.h"
#include "common/VedroLibTools.h"

tPanControl::tPanControl(QWidget* parent, int id)
    //: tSidePanel(parent, id, "Test control") {
    //: tPanel(parent, id, 200, "Test control") {
    : tPanPetrel(parent, id, "Test control") {
    // Replace Sidepan buttons with own
    labOperatorName = AddWidget(new QLabel("Operator name"));
    cbOperatorName = AddWidget(new QComboBox());
    cbOperatorName->setEditable(true);
    cbOperatorName->setDuplicatesEnabled(false);

    labDutName = AddWidget(new QLabel("Device under test"));
    cbDutName = AddWidget(new QComboBox());
    labSpecVersion = AddWidget(new QLabel("Test specification version"));
    cbSpecVersion = AddWidget(new QComboBox());

    progTestProgress = AddWidget(new QProgressBar());
    progTestProgress->setMinimum(0);
    progTestProgress->setMaximum(100);
    progTestProgress->setValue(0);
    btnStart = AddWidget(new QPushButton("Start"));
    btnStop = AddWidget(new QPushButton("Stop"));

    btnAutoTest = AddWidget(new QPushButton("Auto"));
    btnManualTest = AddWidget(new QPushButton("Manual"));
    btnConfig = AddWidget(new QPushButton("Config"));
    btnTestProc = AddWidget(new QPushButton("Proc"));
    btnReports = AddWidget(new QPushButton("Reports"));

    //setStyleSheet("QPushButton:pressed { background-color: rgb(224, 0, 0); border-style: inset; }");
    //setStyleSheet("QPushButton { background-color: rgb(0, 224, 0); border-style: inset; }");

    btnStart->setAutoFillBackground(true);// pbStart->setCheckable(true);
    // Layouts
    // tPanelLayout* layout = LayoutMid;
    
    int loId = AddLayout(0, 0, tPanelStyle(eBorderStyle::None, true), StdH);
    auto pad = Layouts[loId]->GetPaddings();
    //pad.setBottom(10);
    Layouts[loId]->SetPaddings(pad);
    
    // 
    AddControls(GetLayoutPtr(1));
    //AddControls(LayoutMax);
    //auto ss = pbStart->styleSheet();
    //setStyleSheet("QPushButton { background-color: rgb(0, 224, 0); border-style: inset; }");
    //setStyleSheet("QPushButton:pressed { background-color: rgb(224, 0, 0); border-style: inset; }");
    //setStyleSheet(ss);


    connect(btnAutoTest, SIGNAL(clicked()), this, SLOT(slotAutoTest()));
    connect(btnManualTest, SIGNAL(clicked()), this, SLOT(slotManualTest()));
    connect(btnConfig, SIGNAL(clicked()), this, SLOT(slotConfig()));
    connect(btnTestProc, SIGNAL(clicked()), this, SLOT(slotTestProc()));
    connect(btnReports, SIGNAL(clicked()), this, SLOT(slotReports()));
}

void tPanControl::AddControls(tPanelLayout* layout) {
    int y;
    y = PaddingY;
    int dy = LabH * 1.5;
    layout->AddWidget(labOperatorName, X_1_2, y, W_1_2, LabH);
    layout->AddWidget(cbOperatorName, X_2_2, y, W_1_2, CbH);
    y += dy;
    layout->AddWidget(labDutName, X_1_2, y, W_1_2, LabH);
    layout->AddWidget(cbDutName, X_2_2, y, W_1_2, CbH);
    y += dy;
    layout->AddWidget(labSpecVersion, X_1_2, y, W_1_2, LabH);
    layout->AddWidget(cbSpecVersion, X_2_2, y, W_1_2, CbH);
    y += FullH;
    TabY = y;
    y += 5;
    SmlY = y;
    layout->AddWidget(btnAutoTest,   X_1_5, y, W_1_5, SmlH);
    layout->AddWidget(btnManualTest, X_2_5, y, W_1_5, SmlH);
    layout->AddWidget(btnConfig,     X_3_5, y, W_1_5, SmlH);
    layout->AddWidget(btnTestProc,   X_4_5, y, W_1_5, SmlH);
    layout->AddWidget(btnReports,    X_5_5, y, W_1_5, SmlH);
    //TabY = layout->Elements.at(btnAutoTest).Y;


    y += dy;
    layout->AddWidget(progTestProgress, X_1_1, y, W_1_1, CbH);
    y += dy;
    layout->AddWidget(btnStart, X_1_3, y, W_1_3, CbH);
    layout->AddWidget(btnStop, X_3_3, y, W_1_3, CbH);
    SetLayout(1);
    //SetCurTab(btnConfig);
    
}

void tPanControl::SetCurTab(QPushButton* curBtn) {
    //int smlY = max(btnAutoTest->y(), btnManualTest->y()); // One of them is small
    //int tabY = smlY - 5;
    //btnAutoTest->setFixedHeight(SmlH); //MoveY(btnAutoTest, smlY);
    //btnManualTest->setFixedHeight(SmlH); //MoveY(btnManualTest, smlY);
    //btnConfig->setFixedHeight(SmlH); //MoveY(btnConfig, smlY);
    //btnReports->setFixedHeight(SmlH); //MoveY(btnReports, smlY);
    //curBtn->setFixedHeight(CbH); //MoveY(curBtn, tabY);

    auto lo = GetCurLayoutPtr();
    lo->Elements[btnAutoTest].Y = SmlY; lo->Elements[btnAutoTest].H = SmlH;
    lo->Elements[btnManualTest].Y = SmlY; lo->Elements[btnManualTest].H = SmlH;
    lo->Elements[btnConfig].Y = SmlY; lo->Elements[btnConfig].H = SmlH;
    lo->Elements[btnTestProc].Y = SmlY; lo->Elements[btnTestProc].H = SmlH;
    lo->Elements[btnReports].Y = SmlY; lo->Elements[btnReports].H = SmlH;
    lo->Elements[curBtn].Y = TabY; lo->Elements[curBtn].H = TabH;

    SetLayout();

    qDebug() << "Move all buttons" << "to" << SmlY;
    qDebug() << "Move button" << curBtn->text() << "to" << TabY;
}

void tPanControl::PopulateDutList(const QStringList& duts) {
    cbDutName->clear();
    cbDutName->addItems(duts);
}

void tPanControl::PopulateSpecVerList(const QStringList& vers) {
    cbSpecVersion->clear();
    cbSpecVersion->addItems(vers);
}

void tPanControl::TrySetDutName(QString& dutName) { // Return current DUTName
    int res = cbDutName->findText(dutName);
    if (res != -1) return; // Found
    if (cbDutName->count() == 0) {
        dutName = ""; // Empty list
    } else {
        cbDutName->setCurrentIndex(0);
        dutName = cbDutName->currentText(); // return first item  in the list
    }
}

void tPanControl::TrySetSpecVer(QString& specVer) { // Return current DUTName
    int res = cbSpecVersion->findText(specVer);
    if (res != -1) return; // Found
    //if (cbSpecVersion->count() == 0) {
    //    specVer = ""; // Empty list
    //} else {
        cbSpecVersion->setCurrentIndex(cbSpecVersion->count()-1);
        specVer = cbSpecVersion->currentText(); // return last item  in the list
    //}
}

void tPanControl::slotManualTest() { SetCurTab(btnManualTest); };
void tPanControl::slotAutoTest() { SetCurTab(btnAutoTest); };
void tPanControl::slotConfig() { SetCurTab(btnConfig); };
void tPanControl::slotTestProc() { SetCurTab(btnTestProc); };
void tPanControl::slotReports() { SetCurTab(btnReports); };
