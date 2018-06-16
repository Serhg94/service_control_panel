#include "LoggerItem.h"
#include <QList>
#include "HostItem.h"
#include "LogModel.h"
#include "ModuleItem.h"
#include "RedisKeyNamespace.h"

LoggerItem::LoggerItem(QString name, ModuleItem * db, const QIcon & icon)
    : QStandardItem(icon, name), QObject(), db(db) {
    setEditable(false);
}

LoggerItem::LoggerItem() : db(nullptr), QObject() {
    setEditable(false);
}

ModuleItem * LoggerItem::getDbItem() {
    return this->db;
}

void LoggerItem::init(QString name, ModuleItem * db, const QIcon & icon) {
    setText(name);
    setIcon(icon);

    this->db = db;
}

int LoggerItem::type() const {
    return TYPE;
}

QString LoggerItem::getTabLabelText() {
    // int dbIndex           = db->getDbIndex();
    QString dbIndexString = db->text();  // QString::number(dbIndex);
    QString connection    = db->server->connection->getConfig().name();

    return QString("%1:%2>%3")
        .arg(connection)
        .arg(dbIndexString)
        .arg(this->text());
}

LogModel * LoggerItem::getKeyModel(const QString & type) {
    int dbIndex     = db->getDbIndex();
    QString keyName = text();

    if (type == "list")
        return new LogModel(db->server->connection, keyName, dbIndex,
                            this->db->getMaxRecordsCount());
    return nullptr;
}

Command LoggerItem::getTypeCommand() {
    QList<QByteArray> typeCommand;
    typeCommand.append("TYPE");
    typeCommand.append(this->text().toUtf8());

    return Command(typeCommand, db->getDbIndex());
}

ConnectionBridge * LoggerItem::getConnection() {
    if (db == nullptr || db->server == nullptr)
        return nullptr;

    return db->server->connection;
}
