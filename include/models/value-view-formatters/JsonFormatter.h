#pragma once
#include <QJsonDocument>
#include "AbstractFormatter.h"

class JsonFormatter : public AbstractFormatter {
  public:
    JsonFormatter();

    void setSource(const QString &);

    QString getFormatted();

    bool isValid();

    QString getRaw();

  protected:
    bool valid;
    QJsonDocument document;
};
