#include "findresultsdialog.h"
#include "findresultstreemodel.h"
#include "findresultstreeview.h"

#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include <QSettings>


FindResultsDialog::FindResultsDialog(ItemsList *items, QWidget *parent) : QDialog(parent), _resultsTreeModel(new FindResultsTreeModel(this)), _resultsTreeView(new FindResultsTreeView(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() | Qt::Tool);

    _resultsTreeModel->updateItems(items);
    _resultsTreeView->setModel(_resultsTreeModel);

    QPushButton *expandAllButton = new QPushButton(tr("Expand all"), this), *collapseAllButton = new QPushButton(tr("Collapse all"), this);
    connect(expandAllButton, SIGNAL(clicked()), _resultsTreeView, SLOT(expandAll()));
    connect(collapseAllButton, SIGNAL(clicked()), _resultsTreeView, SLOT(collapseAll()));

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->addWidget(expandAllButton);
    hboxLayout->addStretch();
    hboxLayout->addWidget(collapseAllButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_resultsTreeView);
    mainLayout->addLayout(hboxLayout);

    restoreGeometry(QSettings().value("findResultsGeometry").toByteArray());
}

void FindResultsDialog::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}

void FindResultsDialog::saveSettings()
{
    QSettings().setValue("findResultsGeometry", saveGeometry());
}
