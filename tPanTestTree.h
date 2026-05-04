#pragma once
#include "tPanPetrel.h"
#include <QTreeWidget>

class tPanTestTree : public tPanPetrel {
	int TreeY = 0;
public:
    QTreeWidget TreeView;
    tPanTestTree(QWidget* parent, int id/*, tLogger* log, tPetrelProjectConfig& cfg*/);
	void resizeEvent(QResizeEvent* event);
	void SetHeight(int h);

};
