#pragma once

#include <QMutex>
#include <QStandardItemModel>
#include <QtConcurrent>
#include "ConnectionBridge.h"

struct LogElem {
    LogElem() {
    }
    LogElem(QString msg, QDateTime time, QString level, QString raw)
        : msg(msg), time(time), raw(raw), level(level) {
    }
    QString msg;
    QDateTime time;
    QString raw;
    QString level;
};

class LogModel : public QAbstractItemModel {
    Q_OBJECT

  public:
    enum Roles {
        SortRole    = Qt::UserRole + 2,
        FilterRole  = Qt::UserRole + 1,
        RawDataRole = Qt::UserRole
    };

    LogModel(ConnectionBridge * db, const QString & keyName, int dbIndex,
             int max_records);
    virtual ~LogModel();

    void loadValue();

    QString getKeyName();

    const static int KEY_MODEL_TYPE      = 1;
    const static int KEY_VALUE_TYPE_ROLE = 7000;

    inline int getKeyModelType() {
        return KEY_MODEL_TYPE;
    }

    QModelIndex index(int row, int column,
                      const QModelIndex & parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
  signals:
    void valueLoaded();
  public slots:
    void clearModel();

  protected slots:
    void loadedValue(Response);
    void processEvent(int db, QString key, QString operation);
    void pollNewRecords();

  protected:
    QFutureWatcher<void> logsLoadingWatcher;
    QFuture<void> logsLoadingResult;
    QSharedPointer<QVector<LogElem>> logs;
    ConnectionBridge * db;
    QString keyName;
    bool loading;
    int dbIndex;
    int added_count;
    int deleted_count;
    int max_records;
    QMutex model_change_mutex;
    void initModel(const QVariant &, bool isNewModel = true);
    void addElems(const QVariant &);
};
