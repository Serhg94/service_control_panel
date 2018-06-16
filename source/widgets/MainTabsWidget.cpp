#include "MainTabsWidget.h"
#include <QMessageBox>
#include <QTabBar>
#include "valueTab.h"
#include "ModuleItem.h"

MainTabsWidget::MainTabsWidget(QWidget * parent) : QTabWidget(parent) {
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(OnTabClose(int)));
}

int MainTabsWidget::addTab(QWidget * w, const QIcon & i,
                           const QString & label) {
    return QTabWidget::addTab(w, i, label);
}

int MainTabsWidget::addTab(QWidget * w, const QString & label) {
    return QTabWidget::addTab(w, label);
}

int MainTabsWidget::addTab(QString & tabName, QWidget * tab, QString icon) {
    int currIndex;

    if (icon.isEmpty()) {
        currIndex = QTabWidget::addTab(tab, tabName);
    } else {
        currIndex = QTabWidget::addTab(tab, QIcon(icon), tabName);
    }

    setCurrentIndex(currIndex);

    return currIndex;
}

void MainTabsWidget::openKeyTab(LoggerItem * key, bool inNewTab) {
    if (key == nullptr)
        return;
    QString keyFullName = key->getTabLabelText();
    // logger may be destroyed while records loads, use QPointer
    ValueTab * viewTab = new ValueTab(QPointer<LoggerItem>(key));

    connect(viewTab, SIGNAL(error(const QString &)), this,
            SLOT(OnError(const QString &)));

    if (inNewTab) {
        addTab(keyFullName, viewTab, QString());
    } else {
        addTab(keyFullName, viewTab);
    }
    viewTab->init();
}

int MainTabsWidget::getTabIndex(QString & name) {
    for (int i = 0; i < count(); ++i) {
        if (name == tabText(i)) {
            return i;
        }
    }
    return -1;
}

bool MainTabsWidget::closeCurrentTabWithValue() {
    int currIndex = currentIndex();
    if (currIndex == -1)
        return true;
    // if tab loading has been completed - close it, else return false
    BaseTab * tab = qobject_cast<BaseTab *>(widget(currIndex));

    if (tab->shouldBeReplaced()) {
        closeTab(currIndex);
        return true;
    }
    return false;
}

void MainTabsWidget::closeTab(unsigned int index) {
    BaseTab * tab = qobject_cast<BaseTab *>(widget(index));
    removeTab(index);
    tab->close();
}

void MainTabsWidget::OnTabClose(int index) {
    closeTab((unsigned int)index);
}

void MainTabsWidget::OnError(const QString & error) {
    QMessageBox::warning(this, "Error", error);
}
