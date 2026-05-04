#pragma once

#include "../PetrelTT/tPanPetrel.h"
#include <QLabel>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <QSerialPortInfo>

//#include "tDeviceConfigDialog.h"

class tPanDevCfg : public tPanPetrel {
    //Q_OBJECT
public:
    tPanDevCfg(QWidget* parent, int id, QString caption = "") 
        : tPanPetrel(parent, id, caption) 
    {
    }

};

class tPanDevCfg_SingleComPort : public tPanDevCfg {
public:
    QLabel labComPort = QLabel("Com port");
    QComboBox cbComPort;
    QPushButton btnAuto = QPushButton("Autodetect");

    tPanDevCfg_SingleComPort(QWidget* parent, int id, QString caption = "")
        : tPanDevCfg(parent, id, caption) {
        AddWidget(&labComPort);
        AddWidget(&cbComPort);
        AddWidget(&btnAuto);

        AddLayout(0, 0, tPanelStyle(eBorderStyle::None, true), StdH);
        SetLayout(1);

        auto lo = GetCurLayoutPtr();
        int y = PaddingY;
        int dy = LabH * 1.5;
        lo->AddWidget(&labComPort, X_1_3, y, W_1_3, LabH);
        lo->AddWidget(&cbComPort, X_2_3, y, W_1_3, CbH);
        //lo->AddWidget(&btnAuto,    X_3_3, y, W_1_3,  CbH);

        Rescan();
    }

    QString GetComPort() const {
        return cbComPort.currentText();
    }

    void Rescan(QString lastComPort = "") {
        cbComPort.clear();
        const auto serialPortInfos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo& portInfo : serialPortInfos) {
            cbComPort.addItem(portInfo.portName());
        }

        if (!lastComPort.isEmpty()) { // Try to select it
            cbComPort.setCurrentText(lastComPort);
            if (cbComPort.currentIndex() == -1) // Not found
                cbComPort.setCurrentIndex(cbComPort.count() - 1);
        }
    }
    //void AutoDetect() {}
};