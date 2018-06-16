#pragma once

#include <QStandardItemModel>
#include <QThread>
#include <QtCore>
#include "ConnectionBridge.h"

class HostItem;

class HostsManager : public QStandardItemModel {
    Q_OBJECT

    friend class TestRedisConnectionsManager;

  public:
    HostsManager(QString, QObject *);
    virtual ~HostsManager(void);

    HostItem * AddConnection(ConnectionBridge *);
    bool RemoveConnection(HostItem *);
    bool ImportConnections(QString &);
    bool SaveConnectionsConfigToFile(QString);

    void connectionChanged();

  private:
    QString configPath;
    bool connectionSettingsChanged;
    QThread connectionsThread;
    QList<ConnectionBridge *> connections;

  protected:
    bool LoadConnectionsConfigFromFile(QString & config,
                                       bool saveChangesToFile = false);
};
