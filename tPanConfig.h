#pragma once
#include <qobject.h>
#include "panels/tSidePanel.h"
//#include "tDeviceConfigDialog.h"
//#include <qlabel.h>

class tPanConfig : public tPanPetrel {
   // Q_OBJECT

//    void AddControls(tPanelLayout* layout);
    tPanelLayout* Lo1 = nullptr;
    QWidget* CurDlg = nullptr;
    QString DeviceName = "";
    bool IsDut;
public:
    tPanConfig(QWidget* parent, int id, bool isDut) 
        : tPanPetrel(parent, id, isDut ? "DUT config" : "DPT config")
        , IsDut(isDut) 
    {
        //SetDeviceName("");
        int loIdx = AddLayout(0, 0, tPanelStyle(eBorderStyle::None, true), StdH);
        Lo1 = GetLayoutPtr(1);
    }

    //void SetConfigDialog(tDeviceConfigDialog* dlg) {
    //    RemoveWidget(CurDlg);
    //    if (dlg == nullptr) return;
    //    CurDlg = AddWidget(dlg);
    //    Lo1->AddWidget(dlg, X_1_1, PaddingY, W_1_1, CurDlg->height());
    //}

//public slots:

};
