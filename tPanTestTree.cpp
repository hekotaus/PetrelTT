#include "tPanTestTree.h"

tPanTestTree::tPanTestTree(QWidget* parent, int id/*, tLogger* log, tPetrelProjectConfig& cfg*/)
    : tPanPetrel(parent, id, "Test tree") {

    AddWidget(&TreeView);

    int loIdx = AddLayout(0, 0, tPanelStyle(eBorderStyle::None, true), StdH);
    Layouts[0]->SetHeight(0);
    //Layouts[0]->SetPaddings({ 0, 0, 0, 0 });
    auto Lo1 = GetLayoutPtr(1);
    auto pad = Lo1->GetPaddings();
    pad.setBottom(5);
    Lo1->SetPaddings(pad);

    int y;
    y = PaddingY;
    int dy = LabH * 1.5;
    //Lo1->AddWidget(&cbShowDescription, X_1_4, y, W_1_4, CbH);
    //Lo1->AddWidget(&cbShowDetails, X_2_4, y, W_1_4, CbH);
    //Lo1->AddWidget(&btnExpandAll, X_3_4, y, W_1_4, CbH);
    //Lo1->AddWidget(&btnCollapseAll, X_4_4, y, W_1_4, CbH);
    y += dy;
    TreeY = y;
    Lo1->AddWidget(&TreeView, PaddingX, y, 100, 100);
    TreeView.setColumnCount(1);
    TreeView.clear();
    TreeView.setHeaderHidden(true);
    TreeView.setIndentation(10);
    SetLayout(1);

    connect(&TreeView, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
}

tPanTestTree::~tPanTestTree() {
    disconnect(&TreeView, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
}

void tPanTestTree::resizeEvent(QResizeEvent* event) {
    qDebug() << "tPanTestTree::resizeEvent" << event->size().width() << "x" << event->size().height();
    if (GetCurLayout() == 1) {
        auto Lo1 = GetLayoutPtr(1);
        tLayoutElement& loel = Lo1->Elements[&TreeView];
        QSize loSiz = Lo1->GetSize();
        //loel.X;// = PaddingX;
        loel.W = loSiz.width() - 2 * PaddingX;
        //loel.Y = PaddingY;
        //qDebug() << "panRep.siz" << event->size().height();
        SetLayout();
        //tPanPetrel::resizeEvent(event);
    } else {
        //SetLayout();
        //
        //resize(width(), 0);
        //event->accept();
    }
    tPanPetrel::resizeEvent(event);
}

#if 1
void tPanTestTree::SetHeight(int h) { // TODO: move to tPanel
	if (GetCurLayout() == 0) h = 0;//{ 
	//	SetAllLayoutsHeight(0);
	//	SetLayout();
	//	emit sizeUpdated(this);
	//	return; 
	//}
	auto lo = GetCurLayoutPtr();
	QMargins paddings = lo->GetPaddings();
	int loH = h - paddings.top() - paddings.bottom();
	if (loH < TreeY) loH = TreeY;
	SetAllLayoutsHeight(loH);
	lo->Elements[&TreeView].H = loH - TreeY - paddings.bottom();// -2 * PaddingY;// -lo->GetPaddings().top() - lo->GetPaddings().bottom();

	SetLayout();
}
#endif

void tPanTestTree::slotCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
    QString groupName = "";
    if (current == nullptr) {
    } else {
        groupName = current->text(0);
    }
    qDebug() << "Pan: Selected test" << groupName;
    emit sigChangeGroupName(groupName);
}