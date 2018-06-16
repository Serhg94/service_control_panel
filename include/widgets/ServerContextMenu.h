#pragma once

#include <QMenu>

class HostItem;

class ServerContextMenu : public QMenu {
    Q_OBJECT
  public:
    ServerContextMenu(QWidget * parent);
    void setCurrentHost(HostItem * curr);
};
