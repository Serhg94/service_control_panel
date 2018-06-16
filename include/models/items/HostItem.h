#pragma once

#include "ConnectionBridge.h"
#include "ModuleItem.h"

class LoggerItem;
class HostsManager;

class HostItem : public QObject, public QStandardItem {
    Q_OBJECT

    friend class ModuleItem;
    friend class LoggerItem;

  public:
    HostItem(ConnectionBridge * c);
    virtual ~HostItem();

    ConnectionBridge * getConnection();
    bool isLocked();
    bool isConnected();
    int virtual type() const;
    const static int TYPE = 2000;

  private:
    ConnectionBridge * connection;
    bool conn_state;

    void setOfflineIcon();
    void setNormalIcon();
    void getItemNameFromConnection();

  private slots:
    void proccessError(QString);
    void connected();
    void disconnected();

  signals:
    void error(QString);
    void statusMessage(QString);
    void databasesLoaded();
    void unlockUI();

  public:
  signals:
    void configChanged();
};
