#pragma once

#include "../PetrelTT/tPanPetrel.h"
#include <QLabel>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <QSerialPortInfo>
#include <qthread.h>
#include <qapplication.h>
//#include <Windows.h>
//#include "tDeviceConfigDialog.h"

class tPanDevCfg : public tPanPetrel {
    //Q_OBJECT
    //QLabel labName = QLabel("Name");
public:
    tPanDevCfg(QWidget* parent, int id, QString caption = "") 
        : tPanPetrel(parent, id, caption) {
        //AddWidget(&labName);
        AddLayout(0, 0, tPanelStyle(eBorderStyle::None, true), StdH);
        SetLayout(1);
        auto lo = GetCurLayoutPtr();
        int y = PaddingY;
        int dy = LabH * 1.5;
        //lo->AddWidget(&labName, X_1_3, y, W_1_3, LabH);
    }
};

class tPanDevCfg_SingleComPort : public tPanDevCfg {
    void ExitAutoDetect() {
        labMessage.setText("");
        btnCancel.setEnabled(false);
        pbProg.setEnabled(false);
        pbProg.setValue(0);
    }

public:
    QLabel labComPort = QLabel("Com port");
    QComboBox cbComPort;
    QPushButton btnAuto = QPushButton("Autodetect");
    QProgressBar pbProg = QProgressBar();
    QPushButton btnCancel = QPushButton("Cancel");
    QLabel labMessage = QLabel();
    bool Cancelled = false;
    tPanDevCfg_SingleComPort(QWidget* parent, int id, QString caption = "")
        : tPanDevCfg(parent, id, caption) {

        qDebug() << "Cancel button enabled" << btnCancel.isEnabled();

        AddWidget(&labComPort);
        AddWidget(&cbComPort);
        AddWidget(&btnAuto);
        AddWidget(&pbProg);
        AddWidget(&btnCancel);
        AddWidget(&labMessage);

        AddLayout(0, 0, tPanelStyle(eBorderStyle::None, true), StdH);
        SetLayout(1);

        auto lo = GetCurLayoutPtr();
        int y = PaddingY;
        int dy = LabH * 1.5;
        lo->AddWidget(&labComPort, X_1_3, y, W_1_3, LabH);
        lo->AddWidget(&cbComPort, X_2_3, y, W_1_3, CbH);
        lo->AddWidget(&btnAuto,    X_3_3, y, W_1_3,  CbH);
        y += FullH;
        lo->AddWidget(&labMessage, X_1_1, y, W_1_1, LabH);
        y += FullH;
        lo->AddWidget(&pbProg, X_1_3, y, W_2_3, LabH);
        lo->AddWidget(&btnCancel, X_3_3, y, W_1_3, LabH);

        //QApplication::processEvents();
        connect(&btnAuto, &QPushButton::clicked, this, &tPanDevCfg_SingleComPort::slotAutoDetect);
        connect(&btnCancel, &QPushButton::clicked, this, &tPanDevCfg_SingleComPort::slotCancel);
        Rescan();

        this->Layouts[0]->Elements[&btnCancel].Enabled = false;
        this->Layouts[0]->Elements[&pbProg].Enabled = false;

        qDebug() << "Cancel button enabled" << btnCancel.isEnabled();
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

public slots:
    void slotCancel() {
        Cancelled = true;
        qDebug() << "Cancel button enabled" << btnCancel.isEnabled();
    }

    void slotAutoDetect() {
        // instruct user to unplug and plug USB adapter
        // Check, which port appeared
        labMessage.setText("Unplug and plug the device");
        btnCancel.setEnabled(true);
        Cancelled = false;
        bool detected = false;
        int timeout = 100; // 0.1sec
        auto oldPorts = QSerialPortInfo::availablePorts();
        auto newPorts = QSerialPortInfo::availablePorts();
        do {
            detected = ((oldPorts.size() + 1) == newPorts.size());
            if (!detected) {
                QThread::msleep(200);
                QApplication::processEvents();
                oldPorts = newPorts;
                newPorts = QSerialPortInfo::availablePorts();
                timeout--;
                pbProg.setValue(timeout);
                qDebug() << "New ports size" << newPorts.size();
            }
        } while (!detected && (timeout > 0) && !Cancelled);
        if (!detected) {
            ExitAutoDetect();
            return;
        }

        // Old ports contains one item less
        // remove all items of last from cur
        // Convert to QSet
        QSet<QString> setPorts;
        for (const auto& port : newPorts)
            setPorts.insert(port.portName());

        for (const auto& port : oldPorts)
            setPorts.remove(port.portName());

        if (setPorts.size() != 1) {
            ExitAutoDetect();
            return; // Ambigous situation
        }
        Rescan();
        cbComPort.setCurrentText(*setPorts.begin());
        ExitAutoDetect();
        //return *setPorts.begin();
    }
};