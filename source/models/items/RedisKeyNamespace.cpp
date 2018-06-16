#include "RedisKeyNamespace.h"

RedisKeyNamespace::RedisKeyNamespace(QString name, const QIcon &icon)
    : QStandardItem(icon, name)

{
  setEditable(false);
}
