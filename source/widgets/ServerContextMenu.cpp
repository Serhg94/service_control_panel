#include "ServerContextMenu.h"
#include "HostItem.h"

ServerContextMenu::ServerContextMenu(QWidget * parent) {
    addAction(QIcon(":/images/ok.png"), tr("Start all services"), parent,
              SLOT(OnStartAllService()));
    addAction(QIcon(":/images/delete.png"), tr("Stop all services"), parent,
              SLOT(OnStopAllService()));
    addAction(QIcon(":/images/refresh.png"), tr("Restart all services"), parent,
              SLOT(OnRestartAllService()));
    addSeparator();
    addAction(QIcon(":/images/add.png"), tr("Add new module"), parent,
              SLOT(OnAddModule()));
    addSeparator();
    addAction(QIcon(":/images/terminal.png"), tr("Console"), parent,
              SLOT(OnConsoleOpen()));
    addSeparator();
    addAction(QIcon(":/images/editdb.png"), tr("Edit"), parent,
              SLOT(OnEditConnection()));
    addAction(QIcon(":/images/delete.png"), tr("Delete"), parent,
              SLOT(OnRemoveConnectionFromTree()));
}

void ServerContextMenu::setCurrentHost(HostItem * curr) {
    bool all_not_installed = true;
    for (int i = 0; i < curr->rowCount(); ++i) {
        QStandardItem * child = curr->child(i);
        ModuleItem * item     = dynamic_cast<ModuleItem *>(child);
        if (item == nullptr)
            return;
        if (item->getServiceState() != ModuleItem::NotInstalled) {
            all_not_installed = false;
            break;
        }
    }
    if (all_not_installed) {
        this->actions().at(0)->setVisible(false);
        this->actions().at(1)->setVisible(false);
        this->actions().at(2)->setVisible(false);
    } else {
        this->actions().at(0)->setVisible(true);
        this->actions().at(1)->setVisible(true);
        this->actions().at(2)->setVisible(true);
    }
}
