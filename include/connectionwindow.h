#pragma once

#include <QDialog>
#include "ConnectionBridge.h"
#include "ui_connection.h"

class HostItem;

class MainWin;

class ConnectionWindow : public QDialog {
    Q_OBJECT
    static const uint DEFAULT_TIMEOUT_UPDATE = 1000;
    static const uint DEFAULT_TIMEOUT_STATES = 3000;

  public:
    ConnectionWindow(QWidget * parent = 0, HostItem * c = nullptr);

  private:
    Ui::connectionDialog ui;
    MainWin * mainForm;
    HostItem * server;
    bool inEditMode;

    bool isFormDataValid();
    bool isConnectionSettingsValid();
    bool isSshSettingsValid();
    bool isAdvancedSettingsValid();
    bool isSshTunnelUsed();
    ConnectionConfig getConectionConfigFromFormData();
    void loadValuesFromConnection(ConnectionBridge *);

  private slots:
    void OnOkButtonClick();
    void OnShowPasswordCheckboxChanged(int);
};
