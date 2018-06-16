#pragma once

#include <QDialog>
#include <QFileDialog>
#include "ui_module.h"
#include "lineeditwithbutton.h"

class ModuleItem;
class HostItem;

class ModuleWindow : public QDialog {
    Q_OBJECT

  public:
    ModuleWindow(HostItem * srv, QWidget * parent = 0,
                 ModuleItem * c = nullptr);

  private:
    Ui::moduleDialog ui;
    HostItem * host;
    ModuleItem * module;
    bool inEditMode;

    void loadValuesFromModule(ModuleItem *);

  private slots:
    void OnOkButtonClick();
    void DirChoose();
    void FileChoose();
};
