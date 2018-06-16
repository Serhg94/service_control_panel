#pragma once

#include <QObject>
#include "qredisclient/redisclient.h"

using namespace RedisClient;

class ConnectionBridge : public QObject {
    Q_OBJECT

  public:
    ConnectionBridge(const ConnectionConfig & c);

    void initWorker();

    void addCommand(const Command & cmd);

    QString getLastError();

    ConnectionConfig getConfig();
    void setConnectionConfig(ConnectionConfig &);

    void setRedisRefrashTimeout(int t);
    void setStatesRefrashTimeout(int t);
    int getRedisRefrashTimeout() {
        return redis_refrash_timeout;
    }
    int getStatesRefrashTimeout() {
        return states_refrash_timeout;
    }

  signals:
    void loadDatabasesList();
    void dbListLoaded(DatabaseList);
    void error(QString);
    void console(QString);
    void operationProgress(int percents, QObject *);
    void connected();
    void disconnected();
    void changeEvent(int db, QString key, QString operation);
    void syncronize();
    void stateTimeoutChanged(int);

  private slots:
    void getDatabases();
    void subscribe();
    void reciveMsg(Response resp, QString err);
    void tryConnect();

  protected:
    ConnectionConfig config;

  private:
    int redis_refrash_timeout;
    int states_refrash_timeout;
    void initSlots(QSharedPointer<Connection> conn);
    QSharedPointer<Connection> worker;
    QSharedPointer<Connection> poller;
    QTimer poll_timer;
};
