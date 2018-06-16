#include "HostItem.h"
#include "qredisclient/connectionconfig.h"

HostItem::HostItem(ConnectionBridge * c) : connection(c) {
    setOfflineIcon();
    getItemNameFromConnection();
    setEditable(false);
    conn_state = false;
    connect(connection, SIGNAL(connected()), this, SLOT(connected()));
    connect(connection, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connection->initWorker();
}

HostItem::~HostItem() {
    delete connection;
}

void HostItem::getItemNameFromConnection() {
    setText(connection->getConfig().name());
}

//ошибка с выводом месседжбокса
void HostItem::proccessError(QString srcError) {
    QString message =
        tr("Can not connect to server %1. Error: %2").arg(text()).arg(srcError);

    emit error(message);
}

void HostItem::connected() {
    conn_state = true;
    setNormalIcon();
}

void HostItem::disconnected() {
    conn_state = false;
    setOfflineIcon();
}

ConnectionBridge * HostItem::getConnection() {
    return connection;
}

bool HostItem::isConnected() {
    return conn_state;
}

void HostItem::setNormalIcon() {
    setIcon(QIcon(":/images/redis_small_icon.png"));
}

void HostItem::setOfflineIcon() {
    setIcon(QIcon(":/images/redis_small_icon_offline.png"));
}

int HostItem::type() const {
    return TYPE;
}
