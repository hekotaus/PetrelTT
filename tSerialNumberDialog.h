#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QDialogButtonBox;

class tSerialNumberDialog : public QDialog {
    Q_OBJECT

public:
    explicit tSerialNumberDialog(QWidget *parent = nullptr);

    QString SerialNumber() const;
    void SetSerialNumber(const QString &serialNumber);

    // Convenience: shows the dialog modally. Returns true if the user pressed OK,
    // in which case serialNumber receives the entered value.
    static int GetSerialNumber(QWidget *parent, QString &serialNumber);

private slots:
    void slotUpdateOkButton();

private:
    QLineEdit *m_serialEdit = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
};
