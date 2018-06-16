#pragma once

#include <QMenu>

class ModuleItem;

class ModuleContextMenu : public QMenu {
    Q_OBJECT
  public:
    ModuleContextMenu(QWidget * parent);
    void setCurrentModule(ModuleItem * curr);
};
