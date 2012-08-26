#ifndef DISENCHANTPREVIEWDIALOG_H
#define DISENCHANTPREVIEWDIALOG_H

#include <QDialog>

#include "structs.h"


class ItemNamesTreeWidget;

class DisenchantPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    DisenchantPreviewDialog(const ItemsList &items, QWidget *parent = 0);
    virtual ~DisenchantPreviewDialog() {}

private:
    ItemNamesTreeWidget *_itemsTreeWidget;
};

#endif // DISENCHANTPREVIEWDIALOG_H
