#include "lineeditwithbutton.h"
#include <QFileDialog>
#include <QStyle>

LineEditWithButton::LineEditWithButton(QWidget * parent) : QLineEdit(parent)
{
    dirChooseButton = new QPushButton("...", this);
//    QPixmap pixmap("fileclose.png");
//    dirChooseButton->setIcon(QIcon(pixmap));
//    dirChooseButton->setIconSize(pixmap.size());
    dirChooseButton->setCursor(Qt::PointingHandCursor);
    dirChooseButton->setStyleSheet("QPushButton { border: none; padding: 0px; color: rgba(0, 0, 0, 80%) }");
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(dirChooseButton->sizeHint().width() + frameWidth + 1));
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), dirChooseButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), dirChooseButton->sizeHint().height() + frameWidth * 2 + 2));

    connect(dirChooseButton, &QPushButton::clicked, this, &LineEditWithButton::InternalButtonClicked);
}

void LineEditWithButton::resizeEvent(QResizeEvent *)
{
    QSize sz = dirChooseButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    dirChooseButton->move(rect().right() - frameWidth - sz.width(),
                      (rect().bottom() + 1 - sz.height())/2);
}

void LineEditWithButton::InternalButtonClicked()
{
    emit ButtonClick();
}
