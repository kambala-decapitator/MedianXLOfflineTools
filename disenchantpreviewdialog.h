#ifndef DISENCHANTPREVIEWDIALOG_H
#define DISENCHANTPREVIEWDIALOG_H

#include <QDialog>

#include "showselecteditemdelegate.h"
#include "structs.h"


class ItemNamesTreeWidget;

class QLabel;
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

    virtual ItemInfo *itemForTreeItem(QTreeWidgetItem *treeItem);
    ItemsList selectedItems() const;

public slots:
    virtual void done(int r) { saveSettings(); QDialog::done(r); }

protected:
    virtual void showEvent(QShowEvent *e);

private slots:
    void checkboxStateChanged(bool checked);
    void showTreeWidgetContextMenu(const QPoint &pos);
    void changeSelectedItemsCheckState();

private:
    QLabel *_label;
    ItemNamesTreeWidget *_itemsTreeWidget;
    QDialogButtonBox *_buttonBox;

    ItemsList _items;
    quint32 _selectedItemsCount;

    void updateLabelText();
    QCheckBox *checkboxOfTreeItem(QTreeWidgetItem *treeItem) const;
    void loadSettings();
    void saveSettings();
};

#endif // DISENCHANTPREVIEWDIALOG_H
