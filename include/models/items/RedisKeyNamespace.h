#ifndef REDISKEYNAMESPACE_H
#define REDISKEYNAMESPACE_H

#include <QStandardItem>

class RedisKeyNamespace : public QStandardItem {
public:
  const static int TYPE = 2300;

  RedisKeyNamespace(QString name, const QIcon &icon);

  int virtual type() const { return TYPE; }

private:
};

#endif // REDISKEYNAMESPACE_H
