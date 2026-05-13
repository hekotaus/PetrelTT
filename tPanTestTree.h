#pragma once
#include "tPanPetrel.h"
#include <QTreeWidget>

class tPanTestTree : public tPanPetrel {
    Q_OBJECT
	int TreeY = 0;
public:
    QTreeWidget TreeView;
    tPanTestTree(QWidget* parent, int id/*, tLogger* log, tPetrelProjectConfig& cfg*/);
    ~tPanTestTree();
	void resizeEvent(QResizeEvent* event);
	void SetHeight(int h);
public slots:
    void slotCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
signals:
    void sigChangeGroupName(const QString& testName);
};
