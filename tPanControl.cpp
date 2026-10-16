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

    connect(btnAutoTest, &QPushButton::clicked, this, &tPanControl::slotAutoTest);
    connect(btnManualTest, &QPushButton::clicked, this, &tPanControl::slotManualTest);
    connect(btnConfig, &QPushButton::clicked, this, &tPanControl::slotConfig);
    connect(btnTestProc, &QPushButton::clicked, this, &tPanControl::slotTestProc);
    connect(btnReports, &QPushButton::clicked, this, &tPanControl::slotReports);

    connect(cbDutName, &QComboBox::currentTextChanged, this, &tPanControl::slotDutChanged);
    connect(cbSpecVersion, &QComboBox::currentTextChanged, this, &tPanControl::slotSpecVerChanged);

}

tPanControl::~tPanControl() {
    disconnect(btnAutoTest, &QPushButton::clicked, this, &tPanControl::slotAutoTest);
    disconnect(btnManualTest, &QPushButton::clicked, this, &tPanControl::slotManualTest);
    disconnect(btnConfig, &QPushButton::clicked, this, &tPanControl::slotConfig);
    disconnect(btnTestProc, &QPushButton::clicked, this, &tPanControl::slotTestProc);
    disconnect(btnReports, &QPushButton::clicked, this, &tPanControl::slotReports);

    disconnect(cbDutName, &QComboBox::currentTextChanged, this, &tPanControl::slotDutChanged);
    disconnect(cbSpecVersion, &QComboBox::currentTextChanged, this, &tPanControl::slotSpecVerChanged);

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
    AllowUpdate = false;
    cbDutName->clear();
    cbDutName->addItems(duts);
    AllowUpdate = true;
}

void tPanControl::PopulateSpecVerList(const QStringList& vers) {
    AllowUpdate = false;
    cbSpecVersion->clear();
    cbSpecVersion->addItem("");
    cbSpecVersion->addItems(vers);
    AllowUpdate = true;
}

void tPanControl::TrySetDutName(QString& dutName, bool allowUpdate) { // Return current DUTName
    AllowUpdate = allowUpdate;
    int idx = cbDutName->findText(dutName);
    if (idx != -1) {
        cbDutName->setCurrentIndex(idx);
        AllowUpdate = true;
        return; // Found
    }
    if (cbDutName->count() == 0) {
        dutName = ""; // Empty list
    } else {
        cbDutName->setCurrentIndex(0);
        dutName = cbDutName->currentText(); // return first item  in the list
    }
    AllowUpdate = true;
}

QString tPanControl::TrySetSpecVer(const QString& specVer) {
    qDebug() << "TrySetSpecVer oldSpecVer=" << specVer;
    int idx = cbSpecVersion->findText(specVer);
    if (idx != -1) {
        cbSpecVersion->setCurrentIndex(idx);
        return specVer; // Found
    }
    if (cbSpecVersion->count() == 0) {
        qDebug() << "Empty versions list!";
        return ""; // Empty list
    } else {
        //cbSpecVersion->setCurrentIndex(cbSpecVersion->count()-1);
        cbSpecVersion->setCurrentIndex(0);
        return cbSpecVersion->currentText(); // return last item  in the list
    }
}

QString tPanControl::GetSpecsVer() const {
    return cbSpecVersion->currentText();
}

void tPanControl::slotManualTest() { SetCurTab(btnManualTest); };
void tPanControl::slotAutoTest() { SetCurTab(btnAutoTest); };
void tPanControl::slotConfig() { SetCurTab(btnConfig); };
void tPanControl::slotTestProc() { SetCurTab(btnTestProc); };
void tPanControl::slotReports() { SetCurTab(btnReports); };


void tPanControl::slotDutChanged() {
    // emit signal only when selected by user
    if (AllowUpdate)
        emit sigDutChanged();
}

void tPanControl::slotSpecVerChanged() {
    // emit signal only when selected by user
    if (AllowUpdate)
        emit sigSpecVerChanged();
}

