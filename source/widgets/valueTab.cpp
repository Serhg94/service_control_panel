#include "valueTab.h"
#include <QMessageBox>
#include "ConnectionBridge.h"
#include "LogModel.h"
#include "LoggerItem.h"
#include "ValueTabView.h"

ValueTab::ValueTab(QPointer<LoggerItem> key)
    : key(key),
      ui(nullptr),
      isInitialized(false),
      tabMustBeDestroyed(false),
      operationInProgress(true) {
    setObjectName("valueTab");

    if (key.isNull() || key->getDbItem() == nullptr ||
        key->getConnection() == nullptr) {
        return;
    }
    // if module has been deleted, delete this tab
    connect((QObject *)key->getDbItem(), SIGNAL(destroyed(QObject *)), this,
            SLOT(OnClose()));
}

void ValueTab::init() {
    if (key.isNull())
        return;
    ui = QSharedPointer<ValueTabView>(new ValueTabView(key->text(), this));

    Command typeCmd = key->getTypeCommand();
    typeCmd.setCallBack(
        QPointer<ValueTab>(this),
        [this](Response r, QString error) { this->keyTypeLoaded(r, error); });
    key->getConnection()->addCommand(typeCmd);

    connect(this, SIGNAL(error(const QString &)), this,
            SLOT(errorOccurred(const QString &)));

    isInitialized = true;
}

void ValueTab::close() {
    tabMustBeDestroyed = true;

    if (!operationInProgress)
        destroy();
}

void ValueTab::OnClose() {
    close();
}

void ValueTab::destroy() {
    delete this;
}

bool ValueTab::isOperationsAborted() {
    operationInProgress = false;

    return tabMustBeDestroyed;
}

bool ValueTab::shouldBeReplaced() {
    return !operationInProgress;
}

void ValueTab::keyTypeLoaded(Response type, QString err) {
    if (isOperationsAborted() || key.isNull())
        return destroy();

    QString t = type.getValue().toString();
    keyModel  = QSharedPointer<LogModel>(key->getKeyModel(t));

    if (keyModel.isNull()) {
        emit error(
            tr("Can not load key value. Key was removed or redis-server went "
               "away."));
        return;
    }

    connect(keyModel.data(), SIGNAL(valueLoaded()), this, SLOT(valueLoaded()));

    operationInProgress = true;
    keyModel->loadValue();
}

void ValueTab::valueLoaded() {
    if (isOperationsAborted() || key.isNull())
        return destroy();

    ui->initKeyValue(keyModel.data());

    setObjectName("valueTabReady");
}

void ValueTab::errorOccurred(const QString & message) {
    ui->hideSpinner();
    QMessageBox::warning(this, "Error occurred", message);
}
