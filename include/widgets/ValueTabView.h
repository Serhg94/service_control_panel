#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QMovie>
#include <QPushButton>
#include <QSplitter>
#include <QtCore>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>
#include "filterwidget.h"
#include "waitingspinnerwidget.h"

class LogModel;
class FilterProxyModel;
class AbstractFormatter;

class ValueTabView : public QObject {
    Q_OBJECT

  public:
    ValueTabView(const QString & name, QWidget * parent);
    virtual ~ValueTabView();

    void initKeyValue(LogModel *);

    const QModelIndex * getCurrentCell();

    void hideSpinner();

  protected:
    QWidget * controller;
    LogModel * model;
    FilterProxyModel * proxy_model;
    AbstractFormatter * formatter;
    const QModelIndex * currentCell;

    // UI items
    QTableView * keyValue;
    FilterWidget * filterWidget;
    QPlainTextEdit * singleValue;
    QGroupBox * valuesGroup;
    QPushButton * exportLogs;
    QPushButton * clearLogs;
    QCheckBox * fullMsg;
    QGroupBox * singleValueGroup;
    QComboBox * singleValueFormatterType;
    QSplitter * splitter;
    QLabel * keyNameLabel;
    QLabel * keyValueLabel;
    QLabel * formatterLabel;
    QGridLayout * gridLayout;
    WaitingSpinnerWidget * spinner;

    void initLayout();
    void initKeyName();
    void initFormatter();

  protected slots:
    void onSelectedItemChanged(const QModelIndex & current,
                               const QModelIndex & previous);
    void currentFormatterChanged(int index);
    void textFilterChanged();
    void exportSelected();
    void fullMsgToggle(int state);

  signals:
    void saveChangedValue(const QString & value, const QModelIndex * currCell);
};
