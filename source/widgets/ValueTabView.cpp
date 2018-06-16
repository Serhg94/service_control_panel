#include "ValueTabView.h"
#include <QFileDialog>
#include <QMessageBox>
#include "AbstractFormatter.h"
#include "LogModel.h"
#include "filterproxymodel.h"

ValueTabView::ValueTabView(const QString & name, QWidget * parent) : controller(parent), formatter(AbstractFormatter::getFormatter()), currentCell(nullptr) {
    Q_UNUSED(name)
    initLayout();
    initFormatter();
}

ValueTabView::~ValueTabView() {
    delete formatter;
    qDebug() << "ValueTabView deleting";
}

void ValueTabView::initFormatter() {
    singleValueFormatterType = new QComboBox;
    singleValueFormatterType->insertItem(0, tr("Plain text"));
    singleValueFormatterType->insertItem(1, tr("JSON"));
    singleValueFormatterType->setCurrentIndex(1);

    formatterLabel = new QLabel;
    formatterLabel->setText(tr("View value as:"));

    connect(singleValueFormatterType, SIGNAL(currentIndexChanged(int)), this,
            SLOT(currentFormatterChanged(int)));
}

void ValueTabView::initLayout() {

    controller->resize(513, 313);
    gridLayout = new QGridLayout(controller);
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(11, 11, 11, 11);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));

    spinner = new WaitingSpinnerWidget;
    gridLayout->addWidget(spinner);

    spinner->start();
}

void ValueTabView::initKeyName() {
    keyNameLabel = new QLabel(controller);
    keyNameLabel->setText(tr(" Logger:"));
    gridLayout->addWidget(keyNameLabel, 0, 0, 1, 1);
}

void ValueTabView::textFilterChanged() {
    QRegExp regExp(filterWidget->text(), filterWidget->caseSensitivity(),
                   filterWidget->patternSyntax());
    this->proxy_model->setFilterRegExp(regExp);
}

void ValueTabView::exportSelected() {
    auto selected = this->keyValue->selectionModel()->selectedRows(0);
    if (selected.isEmpty())
        return;
    QString fileName = QFileDialog::getSaveFileName(
        controller, tr("Export logs to txt"), "", tr("txt Files (*.txt)"));
    if (fileName.isEmpty())
        return;
    QFile outFile(fileName);
    if (outFile.open(QIODevice::WriteOnly)) {
        QTextStream out(&outFile);
        out.setCodec("UTF-8");
        out << "[";
        for (int i = 0; i < selected.size(); ++i) {
            out << proxy_model->data(selected.at(i), LogModel::RawDataRole)
                       .toString()
                << ",";
        }
        out << "]";
        outFile.close();
    } else
        return;
}

void ValueTabView::fullMsgToggle(int state) {
    if (state == Qt::Checked) {
        keyValue->verticalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
    } else {
        keyValue->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        keyValue->verticalHeader()->setDefaultSectionSize(15);
    }
}

void ValueTabView::initKeyValue(LogModel * model) {
    spinner->stop();

    this->model = model;

    singleValue = new QPlainTextEdit(controller);

    if (model->getKeyModelType() == LogModel::KEY_MODEL_TYPE) {
        keyValue = new QTableView(controller);
        keyValue->setEditTriggers(QAbstractItemView::NoEditTriggers);
        keyValue->setWordWrap(true);
        keyValue->setSelectionBehavior(QAbstractItemView::SelectRows);

        keyValue->horizontalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
        keyValue->horizontalHeader()->setStretchLastSection(true);

        keyValue->verticalHeader()->setDefaultSectionSize(15);

        filterWidget = new FilterWidget(controller);
        filterWidget->setText(tr("Filter"));
        connect(filterWidget, &FilterWidget::filterChanged, this,
                &ValueTabView::textFilterChanged);

        singleValue->appendPlainText(tr("Select table cell"));
        singleValue->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        valuesGroup          = new QGroupBox;
        QGridLayout * gridup = new QGridLayout;
        valuesGroup->setFlat(true);
        gridup->setContentsMargins(0, 0, 0, 0);
        gridup->addWidget(filterWidget, 0, 0, 1, 3);
        exportLogs = new QPushButton(tr("Export selected"));
        clearLogs  = new QPushButton(tr("Clear tab"));
        fullMsg    = new QCheckBox(tr("Full message text"));
        gridup->addWidget(fullMsg, 1, 0, 1, 1);
        gridup->addWidget(exportLogs, 1, 1, 1, 1);
        gridup->addWidget(clearLogs, 1, 2, 1, 1);
        gridup->addWidget(keyValue, 2, 0, 1, 3);
        valuesGroup->setLayout(gridup);

        singleValueGroup   = new QGroupBox(tr("Value:"));
        QGridLayout * grid = new QGridLayout;
        grid->addWidget(formatterLabel, 0, 0, 1, 1);
        grid->addWidget(singleValueFormatterType, 0, 1, 1, 1);
        grid->addWidget(singleValue, 1, 0, 1, 2);
        singleValueGroup->setLayout(grid);

        splitter = new QSplitter();
        splitter->setOrientation(Qt::Vertical);
        splitter->addWidget(valuesGroup);
        splitter->addWidget(singleValueGroup);

        gridLayout->addWidget(splitter, 1, 0, 1, 2);

        proxy_model = new FilterProxyModel(this);
        proxy_model->setSourceModel(model);
        keyValue->setModel(proxy_model);
        keyValue->setSortingEnabled(true);
        keyValue->setAlternatingRowColors(true);
        keyValue->horizontalHeader()->resizeSections(
            QHeaderView::ResizeToContents);

        connect(
            keyValue->selectionModel(),
            SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(onSelectedItemChanged(const QModelIndex &,
                                             const QModelIndex &)));
        connect(exportLogs, &QPushButton::clicked, this,
                &ValueTabView::exportSelected);
        connect(clearLogs, &QPushButton::clicked, model, &LogModel::clearModel);
        connect(fullMsg, &QCheckBox::stateChanged, this,
                &ValueTabView::fullMsgToggle);
    }
}

void ValueTabView::onSelectedItemChanged(const QModelIndex & current,
                                         const QModelIndex & previous) {
    Q_UNUSED(previous);
    QModelIndex index = proxy_model->mapToSource(current);
    if (!index.isValid())
        return;
    singleValue->clear();

    formatter->setSource(model->data(index, LogModel::RawDataRole).toString());

    singleValue->appendPlainText(formatter->getFormatted());

    currentCell = &current;
}

void ValueTabView::currentFormatterChanged(int index) {
    AbstractFormatter::FormatterType newFormatterType =
        (AbstractFormatter::FormatterType)index;

    delete formatter;

    formatter = AbstractFormatter::getFormatter(newFormatterType);

    if (currentCell != nullptr) {
        onSelectedItemChanged(*currentCell, *currentCell);
    }
}

const QModelIndex * ValueTabView::getCurrentCell() {
    return currentCell;
}

void ValueTabView::hideSpinner()
{
    spinner->stop();
}
