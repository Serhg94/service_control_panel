#pragma once

#include <QPointer>
#include <QSharedPointer>
#include "BaseTab.h"
#include "LogModel.h"
#include "LoggerItem.h"
#include "qredisclient/redisclient.h"

using namespace RedisClient;

class ValueTabView;

class ValueTab : public BaseTab {
    Q_OBJECT

  public:
    ValueTab(QPointer<LoggerItem> key);
    void init();
    void close();

  protected:
    QPointer<LoggerItem> key;
    QSharedPointer<LogModel> keyModel;
    QSharedPointer<ValueTabView> ui;

    bool isInitialized;

    bool tabMustBeDestroyed;

    bool operationInProgress;

    void destroy();

    bool isOperationsAborted();

    bool shouldBeReplaced();

  protected slots:
    void keyTypeLoaded(Response type, QString err);
    void valueLoaded();
    void errorOccurred(const QString &);
    void OnClose();

  signals:
    void error(const QString &);
};
