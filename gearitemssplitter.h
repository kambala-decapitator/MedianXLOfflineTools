#ifndef GEARITEMSSPLITTER_H
#define GEARITEMSSPLITTER_H

#include "itemspropertiessplitter.h"


class GearItemsSplitter : public ItemsPropertiesSplitter
{
    Q_OBJECT

public:
    enum GearNameIndex
    {
        CharacterNameIndex = 0,
        MercenaryNameIndex,
        IronGolemNameIndex
    };

    explicit GearItemsSplitter(ItemStorageTableView *itemsView, QWidget *parent = 0);
    virtual ~GearItemsSplitter() {}

#ifdef Q_CC_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
    virtual void setItems(const ItemsList &newItems, bool isCreatingTab = false);
#ifdef Q_CC_CLANG
#pragma clang diagnostic pop
#endif

    const QString &currentGearTitle() const { return kButtonNames.at(_currentGearButtonNameIndex); }

protected:
    virtual void removeItemFromList(ItemInfo *item, bool emitSignal = true);

private slots:
    void changeGear();

private:
    QPushButton *_button1, *_button2;
    const QStringList kButtonNames;
    GearNameIndex _currentGearButtonNameIndex;
    QHash<GearNameIndex, ItemsList> _gearItems;

    void updateItemsForCurrentGear();
    void changeButtonText(QPushButton *button, GearNameIndex nameIndex);
};

#endif // GEARITEMSSPLITTER_H
