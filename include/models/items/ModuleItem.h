#pragma once

#include <QDomNode>
#include <QStandardItem>
#include <QtConcurrent>
#include "qredisclient/redisclient.h"

using namespace RedisClient;

class HostItem;
class LoggerItem;

class ModuleItem : public QObject, public QStandardItem {
    Q_OBJECT

    friend class LoggerItem;

  public:
    enum States { NotInstalled = -1, NotRunning = 0, Running = 1, Pending = 2 };

    ModuleItem(int dbIndex, QString exec_path, int keysCount,
               HostItem * parent);
    virtual ~ModuleItem();

    int virtual type() const;
    const static int TYPE = 2100;

    int getDbIndex() const;
    int getMaxRecordsCount() const;
    QString getExecPath() const;
    QString getAppParams() const;
    QString getWorkDir() const;
    QString getServiceName() const;
    QString getName() const;
    HostItem * getServer() const;

    void removeLogger(int row);

    bool operator<(const QStandardItem & other) const;

    struct Icons {
        Icons(QIcon k, QIcon n) : keyIcon(k), namespaceIcon(n) {
        }

        QIcon keyIcon;
        QIcon namespaceIcon;
    };

    void setServiceName(QString srvname);
    void renameModule(QString new_name);
    void setExecPath(QString path);
    void setAppParams(QString par);
    void setWorkDir(QString dir);
    void setDbIndex(int id);
    void setMaxRecordsCount(int c);
    int getServiceState() {
        return service_state;
    }

    void startSrv();
    void stopSrv();
    void restartSrv();
    void installSrv();
    void uninstallSrv();
    void gettestsrv();

    static int getServiceState(QString srvname);

  protected:
    void decreaseKeyCounter();

  private:
    HostItem * server;
    QString name;
    QString service_name = "";
    int dbIndex;
    int max_records;
    int service_state;
    bool loading_in_progress;
    unsigned int keysCount;
    QString exec_path;
    QString work_dir;
    QString app_param;
    int currentKeysPoolPosition;
    Icons iconStorage;
    QStringList rawKeys;
    QFutureWatcher<QList<QStandardItem *>> keysLoadingWatcher;
    QFuture<QList<QStandardItem *>> keysLoadingResult;
    QTimer serviceControlStarter;
    QFutureWatcher<int> serviceStateWatcher;
    QFuture<int> serviceStateResult;
    LoggerItem * keysPool;
    LoggerItem * originalKeyPool;

    void renderKeys(QStringList &);
    void loadedDeleteStatus(Response result, int row);

    void setNormalIcon();
    void setBusyIcon();
    QString execCmd(QString cmd);
    static QString execCmdFuture(QStringList params,
                                 ModuleItem * receiver = nullptr);

  public slots:
    bool loadKeys();
    void keysLoaded(Response resp, QString err);
  private slots:
    void ShowServiceResult(QString result);
    void proccessError(QString srcError);
    void keysLoadingStatusChanged(int progress, QObject *);
    void keysLoadingFinished();
    void serviceStateRecived();
    void serviceCheck();
    void timeoutChanged(int new_t);
  signals:
    void serviceReady(QString result);
};
