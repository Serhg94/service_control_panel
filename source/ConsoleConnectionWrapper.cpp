#include "ConsoleConnectionWrapper.h"
#include "consoleTab.h"

ConsoleConnectionWrapper::ConsoleConnectionWrapper(ConnectionConfig & config)
    : config(config), connectionValid(false) {
}

void ConsoleConnectionWrapper::init() {
    if (config.isNull()) {
        emit addOutput("Invalid config. Can't create connection.");
        return;
    }

    connection = new Connection(config, false);
    if (!connection->connect()) {
        emit addOutput("Can't connect.");
        return;
    }

    connectionValid = true;

    emit addOutput("Connected.\n");
    emit changePrompt(QString("%1:0>").arg(config.name()), true);
}

void ConsoleConnectionWrapper::disconnect() {
    if (connection)
        connection->disconnect();
}

void ConsoleConnectionWrapper::executeCommand(const QString & cmd) {
    if (!connectionValid) {
        emit addOutput("Invalid config. Can't create connection.");
        return;
    }

    if (!connection->isConnected() && !connection->connect()) {
        emit addOutput("Connection error. Check network connection");
        return;
    }
    QList<QByteArray> cmda;
    for (QString part : cmd.split(" ")) {
        cmda.append(part.toUtf8());
    }
    // QVariant result = connection->command(cmda).getValue();

    connection->command(Command(cmda, this, [this](Response r, QString error) {
        QVariant result = r.getValue();
        if (result.canConvert(QMetaType::QStringList)) {
            QStringListIterator iterator(result.toStringList());
            QString output;
            while (iterator.hasNext()) {
                output.append(iterator.next());
                output.append('\n');
            }
            emit addOutput(output);
        } else
            emit addOutput(result.toString());

    }));

    //    QRegExp selectDbRegex("^( )*select( )+(\\d)+");

    //    bool isSelectCommand = selectDbRegex.indexIn(cmd) > -1;

    //    if (isSelectCommand) {
    //        emit changePrompt(QString("%1:%2>")
    //                              .arg(connection->getConfig().name())
    //                              .arg(selectDbRegex.cap(3)),
    //                          false);
    //    }
}
