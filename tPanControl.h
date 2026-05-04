#pragma once
#include <qobject.h>
#include "tPanPetrel.h"
#include <qcombobox.h>
#include <qprogressbar.h>
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
class tPanControl : public tPanPetrel {
    Q_OBJECT

    int TabH = BoxH;
    int SmlH = TabH - 5;
    int TabY = 0;
    int SmlY = 0;

    void AddControls(tPanelLayout* layout);
    void SetCurTab(QPushButton* curBtn);

public:
    QLabel* labOperatorName = nullptr;
    QComboBox* cbOperatorName = nullptr;
    QLabel* labDutName = nullptr;
    QComboBox* cbDutName = nullptr;
    QLabel* labSpecVersion = nullptr;
    QComboBox* cbSpecVersion = nullptr;

    QProgressBar* progTestProgress;
    QPushButton* btnStart = nullptr;
    QPushButton* btnStop = nullptr;

    QPushButton* btnAutoTest = nullptr;
    QPushButton* btnManualTest = nullptr;
    QPushButton* btnConfig = nullptr;
    QPushButton* btnTestProc = nullptr;
    QPushButton* btnReports = nullptr;

    tPanControl(QWidget* parent, int id);
    void PopulateDutList(const QStringList& duts);
    void PopulateSpecVerList(const QStringList& vers);
    void TrySetDutName(QString& dutName); // Return current DUTName
    void TrySetSpecVer(QString& specVer); // Return current DUTName

    // Fill this panel externally
public slots:
    void slotManualTest();
    void slotAutoTest();
    void slotConfig();
    void slotTestProc();
    void slotReports();

};
