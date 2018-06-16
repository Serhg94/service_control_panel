#ifndef FILTERPROXYMODEL_H
#define FILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

//! [0]
class FilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

  public:
    FilterProxyModel(QObject * parent = 0);
};

#endif  // FILTERPROXYMODEL_H
