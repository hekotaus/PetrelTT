#pragma once

#include <qwidget.h>
#include <qlabel.h>

#include "tTestProcMailBox.h"

class tTestForm : public QWidget {
protected:
    //QMainWindow* MainWindow = nullptr;
    QWidget* pMainWindow = nullptr;
    QString GroupName;
    QLabel GroupLabel;
    tTestProcMailBox* pMailBox = nullptr;
    bool IsShowGroupName = true;

public:
    tTestForm(QString groupName, tTestProcMailBox* mb) {
        //InitializeComponent();
        GroupName = groupName;
        GroupLabel.setParent(this);
        GroupLabel.setText(GroupName);
        GroupLabel.move(20, 20);
        GroupLabel.resize(200, 25);
        GroupLabel.setVisible(IsShowGroupName);
        pMailBox = mb;
    }

    bool GetShowGroupName() const { return IsShowGroupName; };
    void SetShowGroupName(bool st) { IsShowGroupName = st; GroupLabel.setVisible(IsShowGroupName); };

    QString GetGroupName() const { return GroupName; }

    //tTestForm() { }
#if 0
    void testButton_Click(QObject sender, EventArgs e) { // Call this to run default Manual test
        if (pMailBox != nullptr) pMailBox.ResultsValid = false;
        pMainWindowForm->StartManualTest(GroupName, "Auto");
        if (pMailBox != nullptr) {
            if (pMailBox->ResultsValid) {
            } else {
            }
        }
    }

    void TestForm_Load(QObject sender, EventArgs e) {

    }
    //FormBorderStyle = BorderStyle.None;
#endif
};
