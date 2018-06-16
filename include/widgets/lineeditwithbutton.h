#ifndef LINEEDITWITHBUTTON_H
#define LINEEDITWITHBUTTON_H

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

class LineEditWithButton : public QLineEdit
{
    Q_OBJECT
public:
    LineEditWithButton(QWidget *parent = 0);
    QPushButton * dirChooseButton;

protected:
    void resizeEvent(QResizeEvent *);

public slots:
    void InternalButtonClicked();

signals:
    void ButtonClick();
};

#endif // LINEEDITWITHBUTTON_H
