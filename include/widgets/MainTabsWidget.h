#pragma once

#include <QTabWidget>

class HostItem;
class LoggerItem;

class MainTabsWidget : public QTabWidget {
    Q_OBJECT

  public:
    MainTabsWidget(QWidget * parent = nullptr);

    int addTab(QString &, QWidget *, QString icon = QString());
    int addTab(QWidget *, const QIcon &, const QString &);
    int addTab(QWidget *, const QString &);

    bool closeCurrentTabWithValue();

    void closeTab(unsigned int index);

    void openKeyTab(LoggerItem * key, bool inNewTab = false);

protected:
    /** @return >=0 if exist **/
    int getTabIndex(QString &);

  protected slots:
    void OnError(const QString &);
    void OnTabClose(int i);
};
