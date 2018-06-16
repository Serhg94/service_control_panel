#pragma once

#include <QObject>
#include <QStandardItem>

#include "ModuleItem.h"

class KeysTreeRenderer : public QObject {
  Q_OBJECT

public:
  static QList<QStandardItem *> renderKeys(ModuleItem *, QStringList,
                                           const ModuleItem::Icons &,
                                           QString namespaceSeparator);

  static void renderNamaspacedKey(QList<QStandardItem *> &, QStandardItem *,
                                  QString, QString, const QIcon &,
                                  const QIcon &, ModuleItem *,
                                  QString namespaceSeparator);
};
