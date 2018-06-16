#include "AbstractFormatter.h"
#include "JsonFormatter.h"
#include "PlainFormatter.h"

void AbstractFormatter::setSource(const QString & r) {
    rawValue = r;
}

AbstractFormatter * AbstractFormatter::getFormatter(FormatterType type) {
    switch (type) {
        case AbstractFormatter::Json:
            return new JsonFormatter;

        case AbstractFormatter::Plain:
        default:
            return new PlainFormatter;
    }
}
