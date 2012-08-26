#include "disenchantpreviewdialog.h"
#include "itemnamestreewidget.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QCheckBox>


DisenchantPreviewDialog::DisenchantPreviewDialog(const ItemsList &items, QWidget *parent) : QDialog(parent), _itemsTreeWidget(new ItemNamesTreeWidget(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Disenchant preview"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Select items to disenchant:"), this));
    layout->addWidget(_itemsTreeWidget);
    layout->addWidget(buttonBox);

    _itemsTreeWidget->setRootIsDecorated(false);
    _itemsTreeWidget->addTopLevelItems(treeItemsForItems(items));
    _itemsTreeWidget->setColumnWidth(0, width() / 2);
    // TODO: this and load/save settings
    //for (int i = 0, n = _itemsTreeWidget->topLevelItemCount(); i < n; ++i)
    //    _itemsTreeWidget->setItemWidget(_itemsTreeWidget->topLevelItem(i), 0, new QCheckBox(_itemsTreeWidget));

    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
}
