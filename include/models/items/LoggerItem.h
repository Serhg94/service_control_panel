#ifndef REDISKEYITEM_H
#define REDISKEYITEM_H

#include <QStandardItem>
#include "ConnectionBridge.h"

class ModuleItem;
class LogModel;
class ConnectionBridge;

class LoggerItem : public QObject, public QStandardItem {
    Q_OBJECT
  public:
    const static int TYPE = 2200;

    LoggerItem();
    LoggerItem(QString name, ModuleItem * db, const QIcon &);
    void init(QString name, ModuleItem * db, const QIcon &);

    QString getTabLabelText();

    int virtual type() const;
    LogModel * getKeyModel(const QString &);
    Command getTypeCommand();

    ConnectionBridge * getConnection();

    void remove();

    ModuleItem * getDbItem();

  private:
    ModuleItem * db;
    void loadedDeleteStatus(Response result);
};

#endif  // REDISKEYITEM_H
