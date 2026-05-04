#pragma once
#include <qobject.h>
//#include "panels/tSidePanel.h"
#include "tPanPetrel.h"
#include <qlabel.h>
#include <qplaintextedit.h>
#include <qscrollbar.h>

//class tPanLog : public tSidePanel {
class tPanLog : public tPanPetrel {
    Q_OBJECT
    
    int LogY = 0;
    QPlainTextEdit LogView;
    void AddControls(tPanelLayout* layout);

public:
    tPanLog(QWidget* parent, int id) : tPanPetrel(parent, id, "Log") {
        AddWidget(&LogView);
        LogView.setReadOnly(true);
        LogView.setLineWrapMode(QPlainTextEdit::NoWrap);
        
        int loIdx = AddLayout(0, 0, tPanelStyle(eBorderStyle::None, true), StdH);
        Layouts[0]->SetHeight(0);
        auto Lo1 = GetLayoutPtr(1);
        //Lo1->AddWidget(Browser, X_1_1, y, W_1_1, 200);

        int y = PaddingY;
        int dy = LabH * 1.5;
        //Lo1->AddWidget(&cbShowDescription, X_1_4, y, W_1_4, CbH);
        //Lo1->AddWidget(&cbShowDetails, X_2_4, y, W_1_4, CbH);
        //Lo1->AddWidget(&btnExpandAll, X_3_4, y, W_1_4, CbH);
        //Lo1->AddWidget(&btnCollapseAll, X_4_4, y, W_1_4, CbH);
        y += dy;
        LogY = y;
        Lo1->AddWidget(&LogView, PaddingX, y, 100, 100);
        //LogView.setColumnCount(1);
        LogView.clear();
        //LogView.setHeaderHidden(true);
        //LogView.setIndentation(10);
        SetLayout(1);

    };

    void resizeEvent(QResizeEvent* event) {
        if (GetCurLayout() == 1) {
            auto Lo1 = GetLayoutPtr(1);
            tLayoutElement& loel = Lo1->Elements[&LogView];
            QSize loSiz = Lo1->GetSize();
            //loel.X;// = PaddingX;
            loel.W = loSiz.width() - 2 * PaddingX;
            //loel.Y = PaddingY;
            //qDebug() << "panRep.siz" << event->size().height();
            SetLayout();
            tPanPetrel::resizeEvent(event);
        } else {
            //SetLayout();
            resize(width(), 0);
            event->accept();
        }

    }

    void SetHeight(int h) { // TODO: move to tPanel
        auto lo = GetCurLayoutPtr();
        QMargins paddings = lo->GetPaddings();
        int loH = h - paddings.top() - paddings.bottom();
        SetAllLayoutsHeight(loH);
        lo->Elements[&LogView].H = loH - LogY - PaddingY;// -lo->GetPaddings().top() - lo->GetPaddings().bottom();
        SetLayout();
    }

public slots:
    void slotAddLog(QString s) { 
        LogView.setPlainText(LogView.toPlainText() + s);
        //Browser->ensureCursorVisible();
        auto vb = LogView.verticalScrollBar();
        vb->setValue(vb->maximum());
    }

};
