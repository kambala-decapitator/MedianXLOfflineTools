#ifndef DISENCHANTPREVIEWDIALOG_H
#define DISENCHANTPREVIEWDIALOG_H

#include <QDialog>

#include "showselecteditemdelegate.h"
#include "structs.h"


class DisenchantPreviewModel;
class QSortFilterProxyModel;
class QModelIndex;

class QLabel;
class QTreeView;
class QDialogButtonBox;
class QCheckBox;

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

    DisenchantPreviewDialog(const ItemsList &items, QWidget *parent = 0);
    virtual ~DisenchantPreviewDialog() {}

    virtual bool eventFilter(QObject *obj, QEvent *e);

    virtual ItemInfo *itemForCurrentTreeItem();
    ItemsList selectedItems() const;

public slots:
    virtual void done(int r) { saveSettings(); QDialog::done(r); }

protected:
    virtual void showEvent(QShowEvent *e);

private slots:
    void showTreeViewContextMenu(const QPoint &pos);
    void changeSelectedItemsCheckState();

private:
    QLabel *_label;
    QTreeView *_itemsTreeView;
    QDialogButtonBox *_buttonBox;

    DisenchantPreviewModel *_itemsTreeModel;
    QSortFilterProxyModel *_proxyModel;

    void updateLabelTextAndOkButtonState();

    void loadSettings();
    void saveSettings();
};

#endif // DISENCHANTPREVIEWDIALOG_H
