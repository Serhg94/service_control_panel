#pragma once

#include <QObject>
#include "qredisclient/redisclient.h"

using namespace RedisClient;

class ConsoleConnectionWrapper : public QObject {
    Q_OBJECT

  public:
    ConsoleConnectionWrapper(ConnectionConfig &);

  public slots:
    void init();
    void executeCommand(const QString &);
    void disconnect();

  signals:
    void changePrompt(const QString &, bool);
    void addOutput(const QString &);

  private:
    Connection * connection;
    ConnectionConfig config;
    bool connectionValid;
};
