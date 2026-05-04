#pragma once

#include <qtextedit.h>
#include <QMouseEvent>
// WTF this is not working in a separate file, but working from tPanReport???
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
