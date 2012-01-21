#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include "structs.h"

#include <QCoreApplication>


class QModelIndex;

class ItemManager
{
    Q_DECLARE_TR_FUNCTIONS(ItemManager)

public:
    virtual ItemInfo *itemAt(const QModelIndex &modelIndex) const = 0;
    virtual void setItems(const ItemsList &newItems) = 0;
    QVariant modelData(const QModelIndex &index, int role) const;
};

#endif // ITEMMANAGER_H
