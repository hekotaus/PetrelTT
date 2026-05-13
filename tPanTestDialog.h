#pragma once
#include <qobject.h>
#include "tPanPetrel.h"
#include <qlabel.h>
#include "tTestDialog.h"

class tPanTestDialog : public tPanPetrel {
    int TabH = BoxH;
    int SmlH = TabH - 5;
    int TabY = 0;
    int SmlY = 0;

public:
    //QLabel labTestGroupName;
    tTestDialog* pTestDialog = nullptr;

    tPanTestDialog(QWidget* parent, int id)
        : tPanPetrel(parent, id, "Test dialog") {
        //tPanPetrel::
        //    AddWidget(&labTestGroupName);
        int loIdx = AddLayout(0, 0, tPanelStyle(eBorderStyle::None, true), StdH);
        //int y;
        //y = PaddingY;
        auto Lo1 = GetLayoutPtr(1);

        auto pad = Lo1->GetPaddings();
        pad.setBottom(5);
        Lo1->SetPaddings(pad);

        //int dy = LabH * 1.5;
        //labTestGroupName.setText("Manual test");
        //Lo1->AddWidget(&labTestGroupName, X_1_4, y, W_1_4, CbH);
        //SetLayout(1);
    }
    ~tPanTestDialog() {
        RemoveWidget(pTestDialog);
        pTestDialog = nullptr;
    }

#if 1
    void resizeEvent(QResizeEvent* event) {
        auto Lo1 = GetLayoutPtr(1);
        tLayoutElement& loel = Lo1->Elements[pTestDialog];
        QSize loSiz = Lo1->GetSize();
        //loel.X;// = PaddingX;
        loel.W = loSiz.width() - 2 * PaddingX;
        //loel.Y = PaddingY;
        //qDebug() << "panRep.siz" << event->size().height();
        SetLayout();
        tPanPetrel::resizeEvent(event);
    }
#endif

    void Show(bool st) {
        if (st) SetLayout(1); else SetLayout(0);
    }

    void SetTestDialog(tTestDialog* testDialog) {
        auto Lo1 = GetLayoutPtr(1);
        RemoveWidget(pTestDialog);
        pTestDialog = testDialog;
        if (pTestDialog != nullptr) {
            AddWidget(pTestDialog);
            Lo1->AddWidget(pTestDialog, 0, 0, 100, 100);
        }
        SetLayout();
        resize(width(), height());
    }

    //template <typename T>
    //void AddWidget(T widget, int x, int y, int w, int h) {
    //    auto Lo1 = GetLayoutPtr(1);
    //    Lo1->AddWidget(widget, x, y, w, h); 
    //}

    //void SetCaption(const QString& caption) {
    //    tPanPetrel::SetCaption("Manual test dialog " + caption);
    //}
//public slots:

};
