#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QLockFile>
#include <QMessageBox>
#include <QTranslator>

#include "application.h"

int main(int argc, char * argv[]) {
    QApplication a(argc, argv);
    QApplication::setApplicationName("Service Control Panel");
    QTranslator myTranslator;
    qDebug() << "scp_" + QLocale::system().name();
    myTranslator.load("scp_" + QLocale::system().name());
    a.installTranslator(&myTranslator);

    MainWin w;
    QLockFile lockFile(QDir::temp().absoluteFilePath("running.lock"));

    if (!lockFile.tryLock(100)) {
        w.ShowTrayInfo();
        return 1;
    }

    w.show();
    return a.exec();
}
