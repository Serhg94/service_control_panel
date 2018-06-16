#include "module.h"

#include <QtWidgets/QMessageBox>

#include "ConnectionBridge.h"
#include "HostItem.h"
#include "HostsManager.h"
#include "application.h"

ModuleWindow::ModuleWindow(HostItem * srv, QWidget * parent, ModuleItem * c)
    : QDialog(parent), inEditMode(false) {
    ui.setupUi(this);

    this->setWindowFlags(Qt::Tool);
    this->setModal(true);
    ui.rplEdit->setValue(10000);
    module = c;
    host   = srv;
    // connect slots to signals
    connect(ui.OkButton, &QPushButton::clicked, this,
            &ModuleWindow::OnOkButtonClick);
    connect(ui.CancelButton, &QPushButton::clicked, [&]() { reject(); });
    connect(ui.dirEdit, &LineEditWithButton::ButtonClick, this,
            &ModuleWindow::DirChoose);
    connect(ui.parEdit, &LineEditWithButton::ButtonClick, this,
            &ModuleWindow::FileChoose);

    // edit mode
    if (c != nullptr)
        loadValuesFromModule(module);
}

void ModuleWindow::loadValuesFromModule(ModuleItem * c) {
    inEditMode = true;
    ui.nameEdit->setText(c->text());
    ui.execEdit->setText(c->getExecPath());
    ui.dirEdit->setText(c->getWorkDir());
    ui.parEdit->setText(c->getAppParams());
    ui.dbEdit->setValue(c->getDbIndex());
    ui.rplEdit->setValue(c->getMaxRecordsCount());
}

void ModuleWindow::OnOkButtonClick() {
    QString name = ui.nameEdit->text().trimmed();
    int a        = -1;
    QString new_service_name =
        QString("%1.%2").arg(host->text(), ui.nameEdit->text());
    a = ModuleItem::getServiceState(new_service_name);

    if (name.length() == 0) {
        QMessageBox::information(
            nullptr, tr("Info"),
            QString("Can't create service without a name"));
        return;
    }

    if (name[0] == " ") {  // incorrect name
        QMessageBox::information(
            nullptr, tr("Info"),
            QString(
                tr("Choose a name not staring with whitespace characters")));
        return;
    }

    if (a != -1) {  // check if service already exists
        QMessageBox::information(
            nullptr, tr("Info"),
            QString(tr("Service with name %1 already exists")).arg(name));
        return;
    };
    if (inEditMode) {
        module->renameModule(ui.nameEdit->text().trimmed());
        module->setExecPath(ui.execEdit->text().trimmed());
        module->setWorkDir(ui.dirEdit->text().trimmed());
        module->setAppParams(ui.parEdit->text().trimmed());
        module->setDbIndex(ui.dbEdit->value());
        module->setMaxRecordsCount(ui.rplEdit->value());
    } else {
        ModuleItem * item = new ModuleItem(
            ui.dbEdit->value(), ui.execEdit->text().trimmed(), 0, host);
        item->setWorkDir(ui.dirEdit->text().trimmed());
        item->setText(ui.nameEdit->text().trimmed());
        item->setAppParams(ui.parEdit->text().trimmed());
        item->setMaxRecordsCount(ui.rplEdit->value());
        item->setServiceName(new_service_name);
        host->appendRow(item);
        emit host->configChanged();
    }
    accept();
}

void ModuleWindow::DirChoose() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    QString dirName;
    dirName = QFileDialog::getExistingDirectory(
        this, tr("Choose your directory"),
        QApplication::instance()->applicationDirPath());
    if (dirName != QString("")) {
        ui.dirEdit->setText(dirName);
    }
}

void ModuleWindow::FileChoose() {
    QFileDialog dialog(this);
    //    dialog.setFileMode(QFileDialog::FileName);
    QString dirName;
    dirName = QFileDialog::getOpenFileName(
        this, tr("Choose your directory"),
        QApplication::instance()->applicationDirPath());
    if (dirName != QString("")) {
        ui.parEdit->setText(dirName);
    }
}
