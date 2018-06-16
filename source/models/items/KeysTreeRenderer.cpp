#include "KeysTreeRenderer.h"
#include "LoggerItem.h"
#include "RedisKeyNamespace.h"

QList<QStandardItem *>
KeysTreeRenderer::renderKeys(ModuleItem *db, QStringList rawKeys,
                             const ModuleItem::Icons &icons,
                             QString namespaceSeparator) {
  rawKeys.sort();

  QList<QStandardItem *> result;

  for (QString rawKey : rawKeys) {
    renderNamaspacedKey(result, nullptr, rawKey, rawKey, icons.namespaceIcon,
                        icons.keyIcon, db, namespaceSeparator);
  }

  return result;
}

void KeysTreeRenderer::renderNamaspacedKey(
    QList<QStandardItem *> &root, QStandardItem *currItem,
    QString notProcessedKeyPart, QString fullKey, const QIcon &namespaceIcon,
    const QIcon &keyIcon, ModuleItem *db, QString namespaceSeparator) {
  if (!notProcessedKeyPart.contains(namespaceSeparator)) {
    LoggerItem *newKey = new LoggerItem();
    newKey->init(fullKey, db, keyIcon);

    if (currItem == nullptr) {
      root.push_back(newKey);
    } else {
      currItem->appendRow(newKey);
    }
    return;
  }

  int indexOfNaspaceSeparator = notProcessedKeyPart.indexOf(namespaceSeparator);

  QString firstNamespaceName =
      notProcessedKeyPart.mid(0, indexOfNaspaceSeparator);

  QStandardItem *namespaceItem = nullptr;

  if (currItem == nullptr) {
    for (int i = 0; i < root.size(); ++i) {
      QStandardItem *child = root[i];

      if (child->type() == RedisKeyNamespace::TYPE &&
          child->text() == firstNamespaceName) {
        namespaceItem = child;
        break;
      }
    }
  } else {
    for (int i = 0; i < currItem->rowCount(); ++i) {
      QStandardItem *child = currItem->child(i);

      if (child->type() == RedisKeyNamespace::TYPE &&
          child->text() == firstNamespaceName) {
        namespaceItem = child;
        break;
      }
    }
  }

  if (namespaceItem == nullptr) {
    namespaceItem = new RedisKeyNamespace(firstNamespaceName, namespaceIcon);

    if (currItem == nullptr) {
      root.push_back(namespaceItem);
    } else {
      currItem->appendRow(namespaceItem);
    }
  }

  renderNamaspacedKey(root, namespaceItem,
                      notProcessedKeyPart.mid(indexOfNaspaceSeparator + 1),
                      fullKey, namespaceIcon, keyIcon, db, namespaceSeparator);
}
