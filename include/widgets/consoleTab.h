#pragma once

#include <QThread>
#include "BaseTab.h"
#include "ConsoleConnectionWrapper.h"
#include "qconsole.h"

class ConsoleConnectionWrapper;

class ConsoleTab : public QConsole {
    Q_OBJECT

  public:
    ConsoleTab(ConnectionConfig &);
    virtual ~ConsoleTab(void);
  signals:
    void disconnect();
  public slots:
    void setPrompt(const QString &, bool);

  private:
    ConsoleConnectionWrapper * connection;
    QThread connectionThread;
};
