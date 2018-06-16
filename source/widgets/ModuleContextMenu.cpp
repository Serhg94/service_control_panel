#include "ModuleContextMenu.h"
#include "ModuleItem.h"

ModuleContextMenu::ModuleContextMenu(QWidget * parent) {
    addAction(QIcon(":/images/ok.png"), tr("Start service"), parent,
              SLOT(OnStartService()));
    addAction(QIcon(":/images/delete.png"), tr("Stop service"), parent,
              SLOT(OnStopService()));
    addAction(QIcon(":/images/refresh.png"), tr("Restart service"), parent,
              SLOT(OnRestartService()));
    addAction(QIcon(":/images/add.png"), tr("Install service"), parent,
              SLOT(OnInstallService()));
    addAction(QIcon(":/images/alert.png"), tr("Uninstall service"), parent,
              SLOT(OnUninstallService()));
    addSeparator();
    addAction(QIcon(":/images/editdb.png"), tr("Edit"), parent,
              SLOT(OnEditModule()));
    addAction(QIcon(":/images/clear.png"), tr("Delete"), parent,
              SLOT(OnRemoveModuleFromTree()));
//    addAction(QIcon(":/images/clear.png"), tr("gettest"), parent,
//              SLOT(OnGetService()));
}

void ModuleContextMenu::setCurrentModule(ModuleItem * curr) {
    switch (curr->getServiceState()) {
        case ModuleItem::Pending:
            this->actions().at(0)->setVisible(false);
            this->actions().at(1)->setVisible(false);
            this->actions().at(2)->setVisible(false);
            this->actions().at(3)->setVisible(false);
            this->actions().at(4)->setVisible(false);
            break;
        case ModuleItem::Running:
            this->actions().at(0)->setVisible(false);
            this->actions().at(1)->setVisible(true);
            this->actions().at(2)->setVisible(true);
            this->actions().at(3)->setVisible(false);
            this->actions().at(4)->setVisible(false);
            break;
        case ModuleItem::NotInstalled:
            this->actions().at(0)->setVisible(false);
            this->actions().at(1)->setVisible(false);
            this->actions().at(2)->setVisible(false);
            this->actions().at(3)->setVisible(true);
            this->actions().at(4)->setVisible(false);
            break;
        case ModuleItem::NotRunning:
            this->actions().at(0)->setVisible(true);
            this->actions().at(1)->setVisible(false);
            this->actions().at(2)->setVisible(false);
            this->actions().at(3)->setVisible(false);
            this->actions().at(4)->setVisible(true);
            break;
    }
}
