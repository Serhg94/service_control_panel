#include "LogModel.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QVariant>
#include "ConnectionBridge.h"

LogModel::LogModel(ConnectionBridge * db, const QString & keyName, int dbIndex,
                   int max_records)
    : db(db), keyName(keyName), dbIndex(dbIndex), max_records(max_records) {
    logs = QSharedPointer<QVector<LogElem>>(new QVector<LogElem>);
    connect(db, &ConnectionBridge::changeEvent, this, &LogModel::processEvent);
    connect(db, &ConnectionBridge::syncronize, this, &LogModel::pollNewRecords);
    connect(&logsLoadingWatcher, SIGNAL(finished()), this,
            SIGNAL(valueLoaded()));
    added_count   = 0;
    deleted_count = 0;
    loading       = false;
}

LogModel::~LogModel() {
    qDebug() << "LogModel " << keyName << " deleted";
}

QString LogModel::getKeyName() {
    return keyName;
}

void LogModel::loadedValue(Response value) {
    logsLoadingResult = QFuture<void>();
    logsLoadingResult =
        QtConcurrent::run(this, &LogModel::initModel, value.getValue(), true);
    logsLoadingWatcher.setFuture(logsLoadingResult);
}

void LogModel::processEvent(int db, QString key, QString operation) {
    if (key == keyName && db == dbIndex) {
        if (operation == "rpush")
            added_count++;
        // так нельзя, т.к. евент по триму генерится даже если 0 строк было
        // удалено
        // else if (operation == "ltrim")
        //    deleted_count++;
    }
}

void LogModel::pollNewRecords() {
    if (added_count > 0 && !loading) {
        QList<QByteArray> command;
        command.append("LRANGE");
        command.append(keyName.toUtf8());
        command.append(QString("-%0").arg(added_count * 2).toUtf8());
        command.append("-1");
        db->addCommand(Command(command, QPointer<LogModel>(this),
                               [this](Response r, QString error) {
                                   this->initModel(r.getValue(), false);
                               },
                               dbIndex));
        added_count = 0;
    }
    deleted_count = logs.data()->size() - max_records;
    if (deleted_count > 0 && max_records > 0) {
        model_change_mutex.lock();
        this->beginRemoveRows(QModelIndex(), 0, deleted_count - 1);
        logs.data()->remove(0, deleted_count);
        this->endRemoveRows();
        deleted_count = 0;
        model_change_mutex.unlock();
    }
}

void LogModel::loadValue() {
    loading = true;
    QList<QByteArray> command;
    command.append("LRANGE");
    command.append(keyName.toUtf8());
    command.append("0");
    command.append("-1");

    db->addCommand(Command(
        command, QPointer<LogModel>(this),
        [this](Response r, QString error) { this->loadedValue(r); }, dbIndex));
}

void LogModel::initModel(const QVariant & val, bool isNewModel) {
    QStringList list;
    if (!val.canConvert<QStringList>())
        return;
    list = val.value<QStringList>();
    QScopedPointer<QJsonParseError> parsingError(new QJsonParseError);
    bool end_finded = false;
    model_change_mutex.lock();
    for (int row = 0; row < list.size(); row++) {
        if (!isNewModel && !end_finded) {
            if (logs.data()->isEmpty() ||
                list.at(row) == logs.data()->last().raw) {
                if (row == list.size() - 1)
                    break;
                end_finded = true;
                this->beginInsertRows(
                    QModelIndex(), logs.data()->size(),
                    logs.data()->size() + list.size() - row - 2);
            }
            continue;
        }
        QJsonDocument document =
            QJsonDocument::fromJson(list.at(row).toUtf8(), parsingError.data());
        if (parsingError->error == QJsonParseError::NoError)
            logs.data()->append(LogElem(
                document.object().value("msg").toString(),
                QDateTime::fromMSecsSinceEpoch(
                    document.object().value("created").toDouble() * 1000),
                document.object().value("levelname").toString(), list.at(row)));
        else
            logs.data()->append(LogElem("-", QDateTime(), "-", list.at(row)));
    }
    if (!isNewModel && end_finded)
        this->endInsertRows();
    model_change_mutex.unlock();
    loading = false;
    // If we do not find the last array we have in the incoming array of records
    // - there is a possibility that we lost something, so we ask twice more
    // records
    if (!isNewModel && !end_finded)
        added_count = list.size() * 2;
}

QModelIndex LogModel::index(int row, int column,
                            const QModelIndex & parent) const {
    Q_UNUSED(parent)
    if (row < logs.data()->size() && column < 3)
        return createIndex(row, column, nullptr);
    return QModelIndex();
}

QModelIndex LogModel::parent(const QModelIndex & index) const {
    Q_UNUSED(index)
    return QModelIndex();
}

int LogModel::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent)
    return logs.data()->size();
}

int LogModel::columnCount(const QModelIndex & parent) const {
    Q_UNUSED(parent)
    return 3;
}

QVariant LogModel::data(const QModelIndex & index, int role) const {
    // if (role = KeyModel::KEY_VALUE_TYPE_ROLE)
    if (role == FilterRole)
        return QVariant(
            logs->at(index.row()).time.toString("dd-MM-yyyy HH:mm:ss,zzz") +
            logs->at(index.row()).level + logs->at(index.row()).msg);
    if (role == RawDataRole)
        return QVariant(logs->at(index.row()).raw);
    if (role != Qt::DisplayRole && role != SortRole)
        return QVariant();
    switch (index.column()) {
        case 0:
            if (role == SortRole)
                return QVariant(logs->at(index.row()).time);
            else
                return QVariant(logs->at(index.row())
                                    .time.toString("dd-MM-yyyy HH:mm:ss,zzz"));
        case 1:
            return QVariant(logs->at(index.row()).level);
        case 2:
            return QVariant(logs->at(index.row()).msg);
    }
    return QVariant();
}

QVariant LogModel::headerData(int section, Qt::Orientation orientation,
                              int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation != Qt::Horizontal)
        return QVariant();
    switch (section) {
        case 0:
            return QVariant(QObject::tr("Time"));
        case 1:
            return QVariant(QObject::tr("Level"));
        case 2:
            return QVariant(QObject::tr("Message"));
    }
    return QVariant();
}

void LogModel::clearModel() {
    if (this->logs.data()->isEmpty())
        return;
    model_change_mutex.lock();
    this->beginRemoveRows(QModelIndex(), 0, logs.data()->size() - 1);
    logs.data()->clear();
    this->endRemoveRows();
    deleted_count = 0;
    model_change_mutex.unlock();
}
