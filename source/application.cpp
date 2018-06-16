#include "application.h"

#include <QFileDialog>
#include <QMenu>
#include <QMovie>
#include <QStatusBar>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>
#include "qredisclient/redisclient.h"

#include "HostItem.h"
#include "HostsManager.h"
#include "ServerContextMenu.h"
#include "connectionwindow.h"
#include "consoleTab.h"
#include "module.h"
#include "valueTab.h"

MainWin::MainWin(QWidget * parent) : QMainWindow(parent) {
    ui.setupUi(this);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/images/scp.ico"));
    trayIcon->setToolTip("Service Control Panel");
    trayIcon->hide();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    QMenu * menu         = new QMenu(this);
    QAction * quitAction = new QAction(tr("Exit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));
    menu->addAction(quitAction);
    trayIcon->setContextMenu(menu);

    initRedisClient();
    initConnectionsTreeView();
    initContextMenus();
    initFormButtons();
    initSystemConsole();

    performanceTimer.invalidate();
    ui.systemConsole->setMaximumBlockCount(1000);
}

void MainWin::ShowTrayInfo() {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("Application is working.\n") + tr("Find my in tray."));
    msgBox.exec();
}

void MainWin::initConnectionsTreeView() {
    // connection manager
    connections = QSharedPointer<HostsManager>(
        new HostsManager(getConfigPath("settings.xml"), this));

    ui.serversTreeView->setModel(connections.data());
    ui.serversTreeView->expandAll();

    connect(ui.serversTreeView, SIGNAL(clicked(const QModelIndex &)), this,
            SLOT(OnConnectionTreeClick(const QModelIndex &)));
    connect(ui.serversTreeView, SIGNAL(wheelClicked(const QModelIndex &)), this,
            SLOT(OnConnectionTreeWheelClick(const QModelIndex &)));

    // setup context menu
    connect(ui.serversTreeView,
            SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(OnTreeViewContextMenu(const QPoint &)));
}

void MainWin::initContextMenus() {
    serverMenu = QSharedPointer<ServerContextMenu>(new ServerContextMenu(this));
    moduleMenu = QSharedPointer<ModuleContextMenu>(new ModuleContextMenu(this));

    keyMenu = QSharedPointer<QMenu>(new QMenu());
    keyMenu->addAction(QIcon(":/images/terminal.png"),
                       tr("Open logger in new tab"), this,
                       SLOT(OnKeyOpenInNewTab()));
    keyMenu->addAction(QIcon(":/images/clear.png"), tr("Delete logger"), this,
                       SLOT(OnDelLogger()));

    connectionsMenu = QSharedPointer<QMenu>(new QMenu());
    connectionsMenu->addAction(QIcon(":/images/import.png"),
                               tr("Import settings"), this,
                               SLOT(OnImportConnectionsClick()));
    connectionsMenu->addAction(QIcon(":/images/export.png"),
                               tr("Export settings"), this,
                               SLOT(OnExportConnectionsClick()));
    connectionsMenu->addSeparator();

    ui.pbImportConnections->setMenu(connectionsMenu.data());
}

void MainWin::initFormButtons() {
    connect(ui.pbAddServer, SIGNAL(clicked()), SLOT(OnAddConnectionClick()));
    connect(ui.pbImportConnections, SIGNAL(clicked()),
            SLOT(OnImportConnectionsClick()));
}

void MainWin::initSystemConsole() {
    QPushButton * systemConsoleActivator =
        new QPushButton(QIcon(":/images/terminal.png"), tr("System console"));

    connect(systemConsoleActivator, SIGNAL(clicked()), this,
            SLOT(OnConsoleStateChanged()));

    ui.systemConsole->hide();
    ui.statusBar->addPermanentWidget(systemConsoleActivator);
}

void MainWin::OnConsoleStateChanged() {
    ui.systemConsole->setVisible(!ui.systemConsole->isVisible());
}

void MainWin::OnStartAllService() {
    HostItem * host = getSelectedHost();
    if (host == nullptr)
        return;
    for (int i = 0; i < host->rowCount(); ++i) {
        QStandardItem * child = host->child(i);
        ModuleItem * module   = dynamic_cast<ModuleItem *>(child);
        if (module == nullptr)
            continue;
        if (module->getServiceState() == ModuleItem::NotRunning)
            module->startSrv();
    }
}

void MainWin::OnStopAllService() {
    HostItem * host = getSelectedHost();
    if (host == nullptr)
        return;
    for (int i = 0; i < host->rowCount(); ++i) {
        QStandardItem * child = host->child(i);
        ModuleItem * module   = dynamic_cast<ModuleItem *>(child);
        if (module == nullptr)
            continue;
        if (module->getServiceState() == ModuleItem::Running)
            module->stopSrv();
    }
}

void MainWin::OnRestartAllService() {
    HostItem * host = getSelectedHost();
    if (host == nullptr)
        return;
    for (int i = 0; i < host->rowCount(); ++i) {
        QStandardItem * child = host->child(i);
        ModuleItem * module   = dynamic_cast<ModuleItem *>(child);
        if (module == nullptr)
            continue;
        if (module->getServiceState() == ModuleItem::Running)
            module->restartSrv();
    }
}

ModuleItem * MainWin::getSelectedModule() {
    QStandardItem * item =
        ui.serversTreeView->getSelectedItem(ModuleItem::TYPE);
    return dynamic_cast<ModuleItem *>(item);
}

HostItem * MainWin::getSelectedHost() {
    QStandardItem * item = ui.serversTreeView->getSelectedItem(HostItem::TYPE);
    return dynamic_cast<HostItem *>(item);
}

void MainWin::closeEvent(QCloseEvent * event) {
    if (this->isVisible()) {
        event->ignore();
        this->hide();
        trayIcon->show();
    }
}

void MainWin::iconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::Trigger:
            /* иначе, если окно видимо, то оно скрывается,
             * и наоборот, если скрыто, то разворачивается на экран
             * */
            if (!this->isVisible()) {
                trayIcon->hide();
                this->show();
            } else {
                this->hide();
            }

            break;
        default:
            break;
    }
}

void MainWin::OnStartService() {
    ModuleItem * module = getSelectedModule();
    if (module == nullptr)
        return;
    module->startSrv();
}

void MainWin::OnStopService() {
    ModuleItem * module = getSelectedModule();
    if (module == nullptr)
        return;
    module->stopSrv();
}

void MainWin::OnRestartService() {
    ModuleItem * module = getSelectedModule();
    if (module == nullptr)
        return;
    module->restartSrv();
}

void MainWin::OnInstallService() {
    ModuleItem * module = getSelectedModule();
    if (module == nullptr)
        return;
    module->installSrv();
}

void MainWin::OnUninstallService() {
    ModuleItem * module = getSelectedModule();
    ;
    if (module == nullptr)
        return;
    module->uninstallSrv();
}

void MainWin::OnGetService() {
    ModuleItem * module = getSelectedModule();
    ;
    if (module == nullptr)
        return;
    module->gettestsrv();
}

QString MainWin::getConfigPath(const QString & configFile) {
    /*
     * Check home user directory
     */
    QString fullHomePath =
        QString("%1/%2").arg(QDir::currentPath()).arg(configFile);
    QFile testConfig(fullHomePath);
    QFileInfo checkPermissions(fullHomePath);

    if (!testConfig.exists() && testConfig.open(QIODevice::WriteOnly))
        testConfig.close();

    if (checkPermissions.isWritable()) {
        return fullHomePath;
    }

    QMessageBox::warning(
        this, "Current directory is not writable",
        "Program can't save connections file to current dir. "
        "Please change permissions or restart this program with "
        "administrative privileges");
    exit(1);
}

void MainWin::OnAddConnectionClick() {
    QScopedPointer<ConnectionWindow> connectionDialog(
        new ConnectionWindow(this));
    connectionDialog->setWindowState(Qt::WindowActive);
    connectionDialog->exec();
}

void MainWin::OnConnectionTreeClick(const QModelIndex & index) {
    if (!index.isValid())
        return;

    QStandardItem * item = connections->itemFromIndex(index);

    int type = item->type();

    switch (type) {
        case HostItem::TYPE: {
            ui.serversTreeView->setExpanded(index, true);
        } break;

        case ModuleItem::TYPE: {
            ModuleItem * db = dynamic_cast<ModuleItem *>(item);
            if (db->loadKeys()) {
                performanceTimer.start();
                statusBar()->showMessage(tr("Loading loggers ..."));
                ui.serversTreeView->setExpanded(index, true);
            }

        } break;

        case LoggerItem::TYPE:

            if (item->isEnabled())
                if (ui.tabWidget->closeCurrentTabWithValue())
                    ui.tabWidget->openKeyTab(dynamic_cast<LoggerItem *>(item));

            break;
    }
}

void MainWin::OnConnectionTreeWheelClick(const QModelIndex & index) {
    if (!index.isValid())
        return;

    QStandardItem * item = connections->itemFromIndex(index);

    if (item->type() == LoggerItem::TYPE) {
        ui.tabWidget->openKeyTab((LoggerItem *)item, true);
    }
}

void MainWin::OnTreeViewContextMenu(const QPoint & point) {
    if (point.isNull())
        return;
    QStandardItem * item =
        connections->itemFromIndex(ui.serversTreeView->indexAt(point));
    QPoint currentPoint = QCursor::pos();
    if (!item || currentPoint.isNull())
        return;
    int type = item->type();
    if (type == HostItem::TYPE) {
        serverMenu.data()->setCurrentHost((HostItem *)item);
        serverMenu.data()->exec(currentPoint);
    } else if (type == LoggerItem::TYPE) {
        keyMenu->exec(currentPoint);
    } else if (type == ModuleItem::TYPE) {
        moduleMenu.data()->setCurrentModule((ModuleItem *)item);
        moduleMenu.data()->exec(currentPoint);
    }
}

void MainWin::OnRemoveConnectionFromTree() {
    QStandardItem * item = ui.serversTreeView->getSelectedItem(HostItem::TYPE);
    if (item == nullptr)
        return;
    QMessageBox::StandardButton reply;
    reply =
        QMessageBox::question(this, tr("Confirm action"),
                              tr("Do you really want to delete connection?"),
                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        HostItem * server = dynamic_cast<HostItem *>(item);
        for (int i = 0; i < item->rowCount(); i++) {
            ModuleItem * m_child = dynamic_cast<ModuleItem *>(item->child(i));
            reply                = QMessageBox::question(
                this, tr("Confirm action"),
                QString(tr("Do you really want to delete service associated "
                           "with module %1?")
                            .arg(m_child->text())),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes)
                m_child->uninstallSrv();
        }

        connections->RemoveConnection(server);
    }
}

void MainWin::OnRemoveModuleFromTree() {
    QStandardItem * item =
        ui.serversTreeView->getSelectedItem(ModuleItem::TYPE);
    if (item == nullptr)
        return;
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Confirm action"),
                                  tr("Do you really want to delete module?"),
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        OnUninstallService();
        item->parent()->removeRow(item->row());
    }
}

void MainWin::OnAddModule() {
    QStandardItem * item = ui.serversTreeView->getSelectedItem(HostItem::TYPE);
    if (item == nullptr)
        return;
    HostItem * host = dynamic_cast<HostItem *>(item);
    QScopedPointer<ModuleWindow> moduleDialog(new ModuleWindow(host, this));
    moduleDialog->exec();
}

void MainWin::OnEditModule() {
    QStandardItem * item =
        ui.serversTreeView->getSelectedItem(ModuleItem::TYPE);
    QStandardItem * itemParent = ui.serversTreeView->getSelectedItemParent();
    if (item == nullptr || itemParent == nullptr)
        return;
    ModuleItem * module = dynamic_cast<ModuleItem *>(item);
    HostItem * host     = dynamic_cast<HostItem *>(itemParent);
    QScopedPointer<ModuleWindow> moduleDialog(
        new ModuleWindow(host, this, module));
    moduleDialog->exec();
    module->loadKeys();
}

void MainWin::OnDelLogger() {
    LoggerItem * item =
        dynamic_cast<LoggerItem *>(ui.serversTreeView->getSelectedItem());
    if (item == nullptr)
        return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Confirm action"),
                                  tr("Do you want to delete this logger?"),
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;

    item->getDbItem()->removeLogger(item->row());
}

void MainWin::OnEditConnection() {
    QStandardItem * item = ui.serversTreeView->getSelectedItem(HostItem::TYPE);
    if (item == nullptr)
        return;
    HostItem * server = dynamic_cast<HostItem *>(item);
    QScopedPointer<ConnectionWindow> connectionDialog(
        new ConnectionWindow(this, server));
    connectionDialog->exec();
}

void MainWin::OnImportConnectionsClick() {
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Import Settings"), "", tr("Xml Files (*.xml)"));

    if (fileName.isEmpty()) {
        return;
    }

    if (connections->ImportConnections(fileName)) {
        QMessageBox::information(this, tr("Settings imported"),
                                 tr("Settings imported from settings file"));
    } else {
        QMessageBox::warning(this, tr("Can't import settings"),
                             tr("Select valid file for import"));
    }
}

void MainWin::OnExportConnectionsClick() {
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Export Settings to xml"), "", tr("Xml Files (*.xml)"));

    if (fileName.isEmpty()) {
        return;
    }

    if (connections->SaveConnectionsConfigToFile(fileName)) {
        QMessageBox::information(this, tr("Settings exported"),
                                 tr("Settings exported in selected file"));
    } else {
        QMessageBox::warning(this, tr("Can't export settings"),
                             tr("Select valid file name for export"));
    }
}

void MainWin::OnConsoleOpen() {
    QStandardItem * item = ui.serversTreeView->getSelectedItem(HostItem::TYPE);

    if (item == nullptr)
        return;

    HostItem * server       = dynamic_cast<HostItem *>(item);
    ConnectionConfig config = server->getConnection()->getConfig();

    BaseTab * tab        = new BaseTab();
    ConsoleTab * console = new ConsoleTab(config);
    console->setSizePolicy(
        QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    QBoxLayout * layout = new QBoxLayout(QBoxLayout::LeftToRight, tab);
    layout->setMargin(0);
    layout->addWidget(console);
    tab->setLayout(layout);
    tab->setSizePolicy(
        QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    console->setParent(tab);

    QString serverName = server->text();

    ui.tabWidget->addTab(serverName, tab, ":/images/terminal.png");

    console->setFocus();
}

void MainWin::OnKeyOpenInNewTab() {
    QStandardItem * item = ui.serversTreeView->getSelectedItem();

    if (item == nullptr || item->type() != LoggerItem::TYPE)
        return;

    ui.tabWidget->openKeyTab((LoggerItem *)item, true);
}

void MainWin::OnError(QString msg) {
    // QMessageBox::warning(this, tr("Error"), msg);
}

void MainWin::OnLogMessage(QString message) {
    ui.systemConsole->appendPlainText(
        QString("[%1] %2").arg(QTime::currentTime().toString()).arg(message));
}

void MainWin::OnUIUnlock() {
    if (performanceTimer.isValid()) {
        statusBar()->showMessage(
            tr("Logs loaded in: %1 ms").arg(performanceTimer.elapsed()));
        performanceTimer.invalidate();
    }
}

void MainWin::OnStatusMessage(QString message) {
    statusBar()->showMessage(message);
}
