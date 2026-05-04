#pragma once

#include "../PetrelTT/tPanPetrel.h"
#include <QLabel>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include "tReportRoot.h"
#if 0
#include "tReportTextEdit.h"
#else
#include <qtextedit.h>
#include <QMouseEvent>

class tReportTextEdit : public QTextEdit {
public:
    Q_OBJECT
    //tReportTextEdit(QWidget *parent) : QTextEdit(parent) {
    //}
    void mouseDoubleClickEvent(QMouseEvent* e) {
        int lineNum = CurrentBlockNumber();
        qDebug() << "Double click at" << lineNum;
        emit sigReportDoubleClicked(lineNum);
    }

    int CurrentBlockNumber() {
        return textCursor().blockNumber();
    }

signals:
    void sigReportDoubleClicked(int lineNum);
};
#endif


class tPanReport : public tPanPetrel {
    Q_OBJECT
public:
    tPetrelProjectConfig& Cfg;
    QCheckBox cbShowDetails = QCheckBox("Details");
    QCheckBox cbShowDescription = QCheckBox("Description");
    QPushButton btnExpandAll = QPushButton("Expand all");
    QPushButton btnCollapseAll = QPushButton("Collapse all");
    int RepY = 0;
    tReportTextEdit ReportView;
    tPanReport(QWidget* parent, int id, tLogger* log, tPetrelProjectConfig& cfg);
    void SetCurrentReport(tReportType typ);
    void SetHeight(int h); // Refer to tPanTestTree
    void resizeEvent(QResizeEvent* event);
public slots:
    void slotToggleDetails(Qt::CheckState state);
    void slotToggleDescription(Qt::CheckState state);
    void slotExpandAll();
    void slotCollapseAll();
    void slotReportDoubleClicked(int);
    //checkStateChanged(Qt::CheckState state)
};
