#include "tSerialNumberDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

tSerialNumberDialog::tSerialNumberDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Enter Serial Number"));
    setModal(true);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    m_serialEdit = new QLineEdit(this);
    m_serialEdit->setPlaceholderText(tr("Serial number"));
    m_serialEdit->setClearButtonEnabled(true);
    m_serialEdit->setMinimumWidth(250);

    auto *form = new QFormLayout;
    form->addRow(tr("&Serial Number:"), m_serialEdit);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_serialEdit, &QLineEdit::textChanged, this, &tSerialNumberDialog::slotUpdateOkButton);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_buttons);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    slotUpdateOkButton();
}

QString tSerialNumberDialog::SerialNumber() const {
    return m_serialEdit->text().trimmed();
}

void tSerialNumberDialog::SetSerialNumber(const QString &serialNumber) {
    m_serialEdit->setText(serialNumber);
    m_serialEdit->selectAll();
}

int tSerialNumberDialog::GetSerialNumber(QWidget *parent, QString &serialNumber) {
    tSerialNumberDialog dialog(parent);
    dialog.SetSerialNumber(serialNumber);
    int result = dialog.exec();
    if (result == QDialog::Accepted)
        serialNumber = dialog.SerialNumber();
    else
        serialNumber = "";
    return result;
}

void tSerialNumberDialog::slotUpdateOkButton() {
    // OK is only enabled once a non-blank serial number has been entered.
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(!SerialNumber().isEmpty());
}
