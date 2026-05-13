#pragma once
#include <qstring.h>
#include <qlabel.h>

class tTestDialog : public QWidget {
//class tTestDialog : public tPanPetrel {
    QLabel Caption;
public:
    tTestDialog() {
        Caption.move(10, 10);
        //Caption.setFont();
        //Caption.setParent(this);
    }

    void SetCaption(const QString& caption) {
        Caption.setText(caption);
    }
};
