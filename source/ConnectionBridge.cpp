#include "ConnectionBridge.h"
#include <QPointer>

ConnectionBridge::ConnectionBridge(const ConnectionConfig & c) : config(c) {
    worker = QSharedPointer<Connection>(new Connection(c));
    poller = QSharedPointer<Connection>(new Connection(c));
    initSlots(worker);
    initSlots(poller);
    connect(poller.data(), SIGNAL(connected()), this, SIGNAL(connected()));
    connect(poller.data(), SIGNAL(disconnected()), this,
            SIGNAL(disconnected()));
    connect(poller.data(), SIGNAL(error(QString)), this,
            SIGNAL(disconnected()));
    connect(this, SIGNAL(loadDatabasesList()), this, SLOT(getDatabases()));
    connect(poller.data(), SIGNAL(connected()), this, SLOT(subscribe()));
    connect(&poll_timer, SIGNAL(timeout()), this, SLOT(tryConnect()));
    redis_refrash_timeout  = 1000;
    states_refrash_timeout = 3000;
}

void ConnectionBridge::initWorker() {
    tryConnect();
    poll_timer.start(redis_refrash_timeout);
}

void ConnectionBridge::addCommand(const Command & cmd) {
    try {
        worker.data()->command(cmd);
    } catch (Connection::Exception & e) {
        emit error(QString(e.what()));
    }
}

QString ConnectionBridge::getLastError() {
    return QString();  // todo : fix it
}

void ConnectionBridge::setConnectionConfig(ConnectionConfig & newConf) {
    config = newConf;
    worker.data()->setConnectionConfig(config);
    poller.data()->setConnectionConfig(config);
}

void ConnectionBridge::setRedisRefrashTimeout(int t) {
    redis_refrash_timeout = t;
    poll_timer.setInterval(t);
}

void ConnectionBridge::setStatesRefrashTimeout(int t) {
    states_refrash_timeout = t;
    emit stateTimeoutChanged(t);
}

void ConnectionBridge::getDatabases() {
    emit dbListLoaded(worker.data()->getKeyspaceInfo());
}

void ConnectionBridge::tryConnect() {
    if (poller.data()->isConnected())
        emit syncronize();
    else
        poller.data()->connect(false);
}

void ConnectionBridge::subscribe() {
    QList<QByteArray> cmda;
    cmda.append("psubscribe");
    cmda.append("__key*__:*");
    try {
        // ConnectionBridge may be destroyed while command running - use
        // QPointer
        poller.data()->command(Command(
            cmda, QPointer<ConnectionBridge>(this),
            [this](Response r, QString error) { this->reciveMsg(r, error); }));
    } catch (Connection::Exception & e) {
        qDebug() << e.what();
    }
}

void ConnectionBridge::reciveMsg(Response resp, QString err) {
    QVariant result = resp.getValue();
    if (result.canConvert(QMetaType::QStringList)) {
        QStringList list = result.toStringList();
        if (list.at(1) != "__key*__:*") {
            qDebug() << "Wrong namespace: " << list.at(1);
            return;
        }
        QRegExp eventRegex("__keyevent@(\\d\\d?)__:(ltrim|rpush)");
        bool isSelectCommand = eventRegex.indexIn(list.at(2)) > -1;
        if (isSelectCommand) {
            emit changeEvent(eventRegex.cap(1).toInt(), list.at(3),
                             eventRegex.cap(2));
        } else
            qDebug() << "Can't parse event: " << list;

    } else
        qDebug() << (result.toString());
}

void ConnectionBridge::initSlots(QSharedPointer<Connection> conn) {
    // Errors output to console, in MessageBoxes it mess user
    connect(conn.data(), SIGNAL(error(QString)), this,
            SIGNAL(console(QString)));
    connect(conn.data(), SIGNAL(authError(QString)), this,
            SIGNAL(error(QString)));
    connect(conn.data(), SIGNAL(log(QString)), this, SIGNAL(console(QString)));
    connect(conn.data(), SIGNAL(reconnectTo(QString, int)), this,
            SIGNAL(console(QString)));
}

ConnectionConfig ConnectionBridge::getConfig() {
    return config;
}
