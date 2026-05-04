#pragma once
#include <qobject.h>
#include "panels/tSidePanel.h"
#include <qlabel.h>

//#include <qlineedit.h>
//class tPushButton : public QPushButton {
    //void SetStyle() {
    //    setStyleSheet("QLineEdit { background-color: yellow }");
    //}
//public:
//    explicit tPushButton(QWidget* parent = nullptr) : QPushButton(parent) {};
//    explicit tPushButton(const QString& text, QWidget* parent = nullptr) : QPushButton(text, parent) {};
//    tPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr) : QPushButton(icon, text, parent) {};
//};

//class tPanControl : public tSidePanel {
class tPanDebug : public tSidePanel {
    Q_OBJECT
    
    void AddControls(tPanelLayout* layout) {
        int y = PaddingY;
        layout->AddWidget(&btnReloadTP, X_1_2, y, W_1_2, CbH);

    };

public:
    QPushButton btnReloadTP {"Reload TP"};
    tPanDebug(QWidget* parent, int id) : tSidePanel(parent, id, "Debug") {
        
        AddWidget(&btnReloadTP);
        AddControls(LayoutMid);
        AddControls(LayoutMax);

    };

//public slots:

};
