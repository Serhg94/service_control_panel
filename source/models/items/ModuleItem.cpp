#include "ModuleItem.h"
#include <QPointer>
#include <QSettings>
#include "HostItem.h"
#include "KeysTreeRenderer.h"
#include "LoggerItem.h"
#include "QMessageBox"
#include "RedisKeyNamespace.h"

ModuleItem::ModuleItem(int dbIndex, QString exec_path, int keysCount,
                       HostItem * parent)
    : server(parent),
      loading_in_progress(false),
      exec_path(exec_path),
      dbIndex(dbIndex),
      keysCount(keysCount),
      currentKeysPoolPosition(0),
      iconStorage(QIcon(":/images/logs.png"), QIcon(":/images/namespace.png")) {
    setNormalIcon();
    setEditable(true);

    connect(&keysLoadingWatcher, SIGNAL(finished()), this,
            SLOT(keysLoadingFinished()));
    connect(&serviceStateWatcher, SIGNAL(finished()), this,
            SLOT(serviceStateRecived()));
    connect(&serviceControlStarter, SIGNAL(timeout()), this,
            SLOT(serviceCheck()));
    connect(parent->getConnection(), SIGNAL(connected()), this,
            SLOT(loadKeys()));
    connect(parent->getConnection(), SIGNAL(stateTimeoutChanged(int)), this,
            SLOT(timeoutChanged(int)));
    connect(this, &ModuleItem::serviceReady, this,
            &ModuleItem::ShowServiceResult);
    serviceControlStarter.start(
        server->getConnection()->getStatesRefrashTimeout());
    service_state = NotInstalled;
    loadKeys();
}

bool ModuleItem::loadKeys() {
    if (loading_in_progress || !server->isConnected())
        return false;
    setBusyIcon();

    connect(server->connection, SIGNAL(error(QString)), this,
            SLOT(proccessError(QString)));
    connect(server->connection, SIGNAL(operationProgress(int, QObject *)), this,
            SLOT(keysLoadingStatusChanged(int, QObject *)));
    QList<QByteArray> cmd;
    cmd.append("keys");
    cmd.append("*");
    loading_in_progress = true;
    server->connection->addCommand(Command(
        cmd, QPointer<ModuleItem>(this),
        [this](Response r, QString error) { this->keysLoaded(r, error); },
        dbIndex));

    return true;
}

void ModuleItem::keysLoaded(Response resp, QString err) {
    loading_in_progress = false;
    disconnect(server->connection, SIGNAL(error(QString)), this,
               SLOT(proccessError(QString)));
    disconnect(server->connection, SIGNAL(operationProgress(int, QObject *)),
               this, SLOT(keysLoadingStatusChanged(int, QObject *)));
    if (!resp.isValid()) {
        proccessError(err);
        return;
    }
    removeRows(0, rowCount());
    QVariant keys = resp.getValue();
    rawKeys       = keys.toStringList();
    keysCount     = rawKeys.size();

    if (keysCount == 0) {
        server->unlockUI();
        setNormalIcon();
        return;
    }

    if (keysCount < keysCount) {
        server->error(tr("Loaded loggers: %2 of %3. Error - %4")
                          .arg(keysCount)
                          .arg(keysCount)
                          .arg(server->connection->getLastError()));
    }
    renderKeys(rawKeys);

    setNormalIcon();
}

void ModuleItem::renameModule(QString new_name) {
    setText(new_name);
    service_state = -1;
    setNormalIcon();
    serviceCheck();
    this->name = new_name;
}
void ModuleItem::setExecPath(QString path) {
    exec_path = path;
}

void ModuleItem::setAppParams(QString par) {
    app_param = par;
}

void ModuleItem::setWorkDir(QString dir) {
    work_dir = dir;
}
void ModuleItem::setDbIndex(int id) {
    dbIndex = id;
}

void ModuleItem::setMaxRecordsCount(int c) {
    max_records = c;
}

QString ModuleItem::execCmd(QString cmd) {
    if (this->text().isEmpty())
        return "";
    if (serviceStateWatcher.isRunning())
        return "";
    QStringList params;
    params << cmd << this->getServiceName();
    if (cmd == "get") {
        params << "name";
    } else {
        params << "confirm";
    }
    auto future = QtConcurrent::run(ModuleItem::execCmdFuture, params, this);

    return future.result();
}

QString ModuleItem::execCmdFuture(QStringList params, ModuleItem * receiver) {
    QProcess proc;
    proc.setReadChannelMode(QProcess::MergedChannels);
    proc.start("nssm", params);
    proc.waitForFinished();
    QString output = proc.readAll();
    qDebug() << "Service cmd: " << params
             << " executed with result: " << output;

    emit receiver->serviceReady(output);
    return output;
}

void ModuleItem::startSrv() {
    execCmd("start");
    serviceCheck();
}

void ModuleItem::stopSrv() {
    execCmd("stop");
    serviceCheck();
}

void ModuleItem::restartSrv() {
    execCmd("restart");
    serviceCheck();
}

void ModuleItem::installSrv() {
    QProcess proc;
    proc.setReadChannelMode(QProcess::MergedChannels);
    QStringList params;
    params << "install" << server->text() + "." + this->text() << exec_path
           << work_dir << app_param;
    proc.start("nssm", params);
    proc.waitForFinished();
    QString output = proc.readAll();
    qDebug() << "Service installed: " << output;
    serviceCheck();
}

void ModuleItem::uninstallSrv() {
    execCmd("remove");
    serviceCheck();
}

void ModuleItem::gettestsrv() {
    execCmd("get");
}

int ModuleItem::getServiceState(QString srvname) {
    QProcess proc;
    proc.setReadChannelMode(QProcess::MergedChannels);
    QStringList params;
    // params << "\\\\" + host << "query" << name;
    params << "status" << srvname;
    proc.start("nssm", params);
    proc.waitForFinished();
    QString output = proc.readAll();
    if (output.contains("STOPPED"))
        return NotRunning;
    if (output.contains("RUNNING"))
        return Running;
    if (output.contains("PENDING"))
        return Pending;
    return NotInstalled;
}

void ModuleItem::proccessError(QString srcError) {
    setNormalIcon();
    QString message = QString("Can not load keys. %1").arg(srcError);
    emit server->error(message);
}

void ModuleItem::renderKeys(QStringList & rawKeys) {
    if (rawKeys.size() == 0)
        return;

    keysLoadingResult = QFuture<QList<QStandardItem *>>();
    keysLoadingResult = QtConcurrent::run(KeysTreeRenderer::renderKeys, this,
                                          rawKeys, iconStorage, QString(":"));
    keysLoadingWatcher.setFuture(keysLoadingResult);

    server->statusMessage(tr("Keys rendering ..."));
}

void ModuleItem::serviceCheck() {
    if (this->text().isEmpty())
        return;
    if (serviceStateWatcher.isRunning())
        return;
    serviceStateResult = QFuture<int>();
    serviceStateResult =
        QtConcurrent::run(ModuleItem::getServiceState, this->service_name);
    serviceStateWatcher.setFuture(serviceStateResult);
}

void ModuleItem::timeoutChanged(int new_t) {
    serviceControlStarter.setInterval(new_t);
}

void ModuleItem::setBusyIcon() {
    setIcon(QIcon(":/images/wait.png"));
}

void ModuleItem::setNormalIcon() {
    switch (service_state) {
        case 2:
            this->setToolTip(tr("pending"));
            setIcon(QIcon(":/images/pending.png"));
            break;
        case 1:
            this->setToolTip(tr("running"));
            setIcon(QIcon(":/images/running.png"));
            break;
        case -1:
            this->setToolTip(tr("not installed"));
            setIcon(QIcon(":/images/not_installed.png"));
            break;
        case 0:
            this->setToolTip(tr("not running"));
            setIcon(QIcon(":/images/not_running.png"));
            break;
        default:
            this->setToolTip(tr("state refrashing"));
            setIcon(QIcon(":/images/wait.png"));
    }
}

int ModuleItem::type() const {
    return TYPE;
}

bool ModuleItem::operator<(const QStandardItem & other) const {
    return this->text() < other.text();
}

void ModuleItem::setServiceName(QString srvname) {
    this->service_name = srvname;
}

int ModuleItem::getDbIndex() const {
    return dbIndex;
}

int ModuleItem::getMaxRecordsCount() const {
    return max_records;
}
QString ModuleItem::getExecPath() const {
    return exec_path;
}
QString ModuleItem::getAppParams() const {
    return app_param;
}
QString ModuleItem::getWorkDir() const {
    return work_dir;
}

QString ModuleItem::getServiceName() const {
    return this->service_name;
}

QString ModuleItem::getName() const {
    return this->name;
}

HostItem * ModuleItem::getServer() const {
    return this->server;
}

void ModuleItem::keysLoadingStatusChanged(int progressValue, QObject * owner) {
    if (owner != this) {
        return;
    }
    server->statusMessage(tr("Downloading loggers list from database: %1 / %2 ")
                              .arg(progressValue)
                              .arg(keysCount));
}

void ModuleItem::keysLoadingFinished() {
    appendRows(keysLoadingResult.result());
    server->statusMessage(tr("Loggers rendering done"));
    server->unlockUI();
}

void ModuleItem::serviceStateRecived() {
    service_state = serviceStateResult.result();
    setNormalIcon();
}

void ModuleItem::decreaseKeyCounter() {
    keysCount--;
}

void ModuleItem::loadedDeleteStatus(Response result, int row) {
    if (result.isErrorMessage()) {
        server->connection->error(result.getValue().toString());
    } else {
        decreaseKeyCounter();
        removeRow(row);
    }
}

void ModuleItem::removeLogger(int row) {
    QList<QByteArray> deleteCommand;

    deleteCommand.append("DEL");
    deleteCommand.append(this->child(row)->text().toUtf8());

    server->connection->addCommand(Command(
        deleteCommand, QPointer<ModuleItem>(this),
        [this, row](Response r, QString error) { loadedDeleteStatus(r, row); },
        dbIndex));
}

void ModuleItem::ShowServiceResult(QString result) {
    QMessageBox::information(nullptr, tr("Info"), result);
}

ModuleItem::~ModuleItem() {
}
