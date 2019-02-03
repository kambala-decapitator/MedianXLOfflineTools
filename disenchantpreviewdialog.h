#ifndef DISENCHANTPREVIEWDIALOG_H
#define DISENCHANTPREVIEWDIALOG_H

#include <QDialog>

#include "showselecteditemdelegate.h"
#include "structs.h"


class DisenchantPreviewModel;
class CheckboxSortFilterProxyModel;

class QLabel;
class QTreeView;
class QDialogButtonBox;
class QCheckBox;
class QModelIndex;

class DisenchantPreviewDialog : public QDialog, public ShowSelectedItemInterface
{
    Q_OBJECT

public:
    enum CheckAction
    {
        Uncheck = 0,
        Check,
        Invert
    };

    DisenchantPreviewDialog(const ItemsList &items, bool areItemsFromSamePage, QWidget *parent = 0);
    virtual ~DisenchantPreviewDialog() {}

    virtual bool eventFilter(QObject *obj, QEvent *e);

    virtual ItemInfo *itemForCurrentTreeItem() const;
    ItemInfo *itemAtRow(int row) const;
    ItemsList selectedItems() const;

public slots:
    virtual void done(int r) { saveSettings(); QDialog::done(r); }

protected:
    virtual void showEvent(QShowEvent *e);

private slots:
    void showTreeViewContextMenu(const QPoint &pos);
    void changeSelectedItemsCheckState();
    void updateLabelTextAndOkButtonState();

private:
    QLabel *_label;
    QTreeView *_itemsTreeView;
    QDialogButtonBox *_buttonBox;

    DisenchantPreviewModel *_itemsTreeModel;
    CheckboxSortFilterProxyModel *_proxyModel;

    bool isRowChecked(int row) const;
    bool isRowChecked(const QModelIndex &index) const;

    void loadSettings();
    void saveSettings();
};

#endif // DISENCHANTPREVIEWDIALOG_H
