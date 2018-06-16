#pragma once

#include <QElapsedTimer>
#include <QScopedPointer>
#include <QSystemTrayIcon>
#include <QtWidgets/QMainWindow>
#include "ModuleContextMenu.h"
#include "ServerContextMenu.h"
#include "ui_mainwindow.h"

class LoggerItem;
class ModuleItem;
class HostItem;
class HostsManager;

class MainWin : public QMainWindow {
    Q_OBJECT

  public:
    QSharedPointer<HostsManager> connections;

    MainWin(QWidget * parent = 0);

    static void ShowTrayInfo();

  private:
    Ui::demoClass ui;
    QSystemTrayIcon * trayIcon;
    QElapsedTimer performanceTimer;

    QSharedPointer<ServerContextMenu> serverMenu;
    QSharedPointer<ModuleContextMenu> moduleMenu;
    QSharedPointer<QMenu> keyMenu;
    QSharedPointer<QMenu> connectionsMenu;

    // todo: move to custom Settings class
    QString getConfigPath(const QString &);

    void initFormButtons();
    void initConnectionsTreeView();
    void initContextMenus();
    void initSystemConsole();

    ModuleItem * getSelectedModule();
    HostItem * getSelectedHost();

    void closeEvent(QCloseEvent * event);

  private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void OnAddConnectionClick();
    void OnConnectionTreeClick(const QModelIndex & index);
    void OnConnectionTreeWheelClick(const QModelIndex & index);
    void OnTreeViewContextMenu(const QPoint &);
    void OnRemoveModuleFromTree();
    void OnRemoveConnectionFromTree();
    void OnAddModule();
    void OnEditModule();
    void OnEditConnection();
    void OnDelLogger();
    void OnImportConnectionsClick();
    void OnExportConnectionsClick();
    void OnConsoleOpen();
    void OnError(QString);
    void OnLogMessage(QString);
    void OnUIUnlock();
    void OnStatusMessage(QString);
    void OnKeyOpenInNewTab();
    void OnConsoleStateChanged();

    void OnStartAllService();
    void OnStopAllService();
    void OnRestartAllService();

    void OnStartService();
    void OnStopService();
    void OnRestartService();
    void OnInstallService();
    void OnUninstallService();
    void OnGetService();
};
