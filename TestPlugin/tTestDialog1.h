#pragma once

#include "PetrelTT/tTestDialog.h"
#include <qlineedit.h>
#include <qlabel.h>
#include "PetrelTT/tTestDevice.h"
//#include "../StepperBoard/Src/StepperSettingsI.h"

class tTestDialog1 : public tTestDialog {
    Q_OBJECT
    char Name = ' ';
//    uint8_t DevId = -1;
    //QLabel Caption;
public:
//    QPushButton btnCheckGap_Home = QPushButton("Check home switch gap", this);
//    QPushButton btnSetGap_Home = QPushButton("Set home switch gap", this);

public:
    tTestDialog1() {
#if 0
        int vgap = 5;
        int hgap = 10;
        //Caption.setText("Motor " + Name);
        //Caption.move(10, 10);
        
        int h = 20;
        int w1 = 50, w2 = 90, w3 = w1, w4 = w2, w5 = 50, w6 = 160;
        int x1 = vgap, x2 = x1 + w1 + hgap, x3 = x2 + w2 + hgap, x4 = x3 + w3 + hgap, x5 = x4 + w4 + hgap, x6 = x5 + w5 + hgap; 
        int y1 = hgap, y2 = y1 + h + vgap, y3 = y2 + h + vgap;

        labMotorStatus.move(x1, y1); labMotorStatus.resize(w1, h);
        leMotorStatus.move(x2, y1); leMotorStatus.resize(w2, h);
        labMotorError.move(x3, y1); labMotorError.resize(w3, h);
        leMotorError.move(x4, y1); leMotorError.resize(w4, h);
        labSwitch.move(x5, y1); labSwitch.resize(w5, h);
        leSwitch.move(x6, y1); leSwitch.resize(w6, h);

        w1 = 150; w2 = w1; w3 = (w5 + w6) / 2; w4 = w3;
        y1 += h + vgap;
        h = 25;
        x2 = x1 + w1 + hgap;
        y2 = y1 + h + vgap;
        x3 = x2 + w2 + hgap;
        x4 = x3 + w3 + hgap;

        btnCheckGap_Home.move(x1, y1); btnCheckGap_Home.resize(w1, h);
        btnSetGap_Home.move(x2, y1); btnSetGap_Home.resize(w2, h);

        btnCheckGap_Limit.move(x1, y2); btnCheckGap_Limit.resize(w1, h);
        btnSetGap_Limit.move(x2, y2); btnSetGap_Limit.resize(w2, h);
        
        btnCheckRange.move(x3, y1); btnCheckRange.resize(w3, h);
        btnTestAxis.move(x4, y1); btnTestAxis.resize(w4, h);
     

        // x3 = x3;
        w1 = 200; w2 = w1;
        w3 = 50; w4 = 50; w5 = 50; w6 = 50;
        hgap = 5;
        x4 = x3 + w3 + hgap;
        x5 = x4 + w4 + hgap;
        x6 = x5 + w5 + hgap;
        btnBackward.move(x3, y2); btnBackward.resize(w3, h);
        btnBrake.move(x4, y2); btnBrake.resize(w4, h);
        btnForward.move(x5, y2); btnForward.resize(w5, h);
        btnStop.move(x6, y2); btnStop.resize(w6, h);

        resize(x4 + w4 + hgap, y1 + h + vgap);
#endif
    }

    void SetName(const char& name) {
        Name = name;
//        DevId = tTestDevice_ESP_Stepper::DevName2DevId(Name);
        //Caption.setText("Motor " + Name);
        //SetCaption(QString("Test motor %1").arg(Name));
        
//        bool isC = (name == 'C');
//        btnCheckGap_Limit.setEnabled(isC);
//        btnSetGap_Limit.setEnabled(isC);
        //show();
    }

    char GetName() const { return Name; }
    //uint8_t GetMotId() const { return MotId; }
//    uint8_t GetDevId() const { return DevId; }

//public slots:
//    void slotChangedStatus(uint8_t devId, uint32_t status, uint32_t error) {
//        const uint32_t mskStatHomeSwitch = 0x00001000; // TODO: remove
//        const uint32_t mskStatLimitSwitch = 0x00002000; // TODO: remove
//        //uint8_t motIdx = devId - 1;
//        if (devId == DevId) {
//            leMotorStatus.setText(QString("0x%1").arg(status, 8, 16, '0'));
//            leMotorError.setText(QString("0x%1").arg(error, 8, 16, '0'));
//            QString swState;
//            swState = (status & mskStatHomeSwitch) ? "Home:ON" : "Home:OFF";
//            if (devId == nsDev::MotorCidx)
//                swState += (status & mskStatLimitSwitch) ? " Limit:ON" : " Limit:OFF";
//
//            leSwitch.setText(swState);
//        }
//    }
};
