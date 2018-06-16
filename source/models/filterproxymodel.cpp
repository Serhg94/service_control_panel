#include "filterproxymodel.h"
#include "LogModel.h"

FilterProxyModel::FilterProxyModel(QObject * parent) {
    setFilterRole(LogModel::FilterRole);
    setSortRole(LogModel::SortRole);
    Q_UNUSED(parent)
}
