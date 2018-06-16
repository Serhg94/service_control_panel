QT       += core gui network xml concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scp
TEMPLATE = app

include($$PWD/deps/qredisclient/qredisclient.pri)

SOURCES += \
    $$PWD/source/*.cpp \
    $$PWD/source/widgets/*.cpp \
    $$PWD/source/models/*.cpp \
    $$PWD/source/models/items/*.cpp \
    $$PWD/source/models/value-view-formatters/*.cpp

HEADERS  += \
    $$PWD/include/*.h \
    $$PWD/include/widgets/*.h \
    $$PWD/include/models/*.h \
    $$PWD/include/models/items/*.h \
    $$PWD/include/models/value-view-formatters/*.h

DESTDIR = $$PWD/build/

TRANSLATIONS = $$PWD/scp_ru_RU.ts

CONFIG(release, debug|release):{
    MOC_DIR += $$PWD/GeneratedFiles/release/mocs
    OBJECTS_DIR += $$PWD/GeneratedFiles/release/objs
    UI_DIR += $$PWD/GeneratedFiles/release/uis
    RCC_DIR += $$PWD/GeneratedFiles/release/rccs
}
CONFIG(debug, debug|release): {
    DEFINES += _DEBUG
    MOC_DIR += $$PWD/GeneratedFiles/debug/mocs
    OBJECTS_DIR += $$PWD/GeneratedFiles/debug/objs
    UI_DIR += $$PWD/GeneratedFiles/debug/uis
    RCC_DIR += $$PWD/GeneratedFiles/debug/rccs
}

INCLUDEPATH += $$PWD/source \
    $$PWD/source/models \
    $$PWD/source/models/items \
    $$PWD/source/widgets \
    $$PWD/"include" \
    $$PWD/include/models \
    $$PWD/include/models/items \
    $$PWD/include/models/value-view-formatters \
    $$PWD/include/widgets \

FORMS += \
    $$PWD/forms/*.ui \
    forms/module.ui

RESOURCES += \
    Resources/images.qrc

DISTFILES +=
