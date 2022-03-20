#include "itemspropertiessplitter.h"
#include "propertiesviewerwidget.h"
#include "itemdatabase.h"
#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "itemparser.h"
#include "resourcepathmanager.hpp"
#include "itemsviewerdialog.h"
#include "reversebitwriter.h"
#include "characterinfo.hpp"
#include "progressbarmodal.hpp"
#include "qd2charrenamer.h"

#include <QMenu>
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QPushButton>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

#ifdef DUMP_INFO_ACTION
#include <QTextCodec>
#endif


static const int kShardsPerCrystal = 5;
static const QRegExp kRuneRegExp("r(\\d\\d)");
static const quint8 HighestRuneKey = 33, kPerfectGrade = 4;


ItemsPropertiesSplitter::ItemsPropertiesSplitter(ItemStorageTableView *itemsView, QWidget *parent /*= 0*/) : QSplitter(Qt::Horizontal, parent), _itemsView(itemsView), _propertiesWidget(new PropertiesViewerWidget(parent))
{
    QWidget *leftWidget = new QWidget(this);

    QPushButton *copyBBCodesButton = new QPushButton(tr("Copy BBCode of items here"), leftWidget);
    QMenu *copyBBCodesMenu = createCopyBBCodeMenu(false);
    copyBBCodesButton->setMenu(copyBBCodesMenu);

    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->addWidget(_itemsView);
    leftLayout->addWidget(copyBBCodesButton);

    addWidget(leftWidget);
    addWidget(_propertiesWidget);

    createItemActions();

    setChildrenCollapsible(false);
    setStretchFactor(1, 5);

    connect(_itemsView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));
    connect(_itemsView, SIGNAL(pressed(QModelIndex)), SLOT(moveBetweenStashes()));
    connect(copyBBCodesMenu, SIGNAL(triggered(QAction*)), SLOT(copyAllItemsBBCode(QAction*)));
}

void ItemsPropertiesSplitter::setModel(ItemStorageTableModel *model)
{
    _itemsModel = model;
    _itemsView->setModel(model);
    // TODO: [0.5] change signal to selectionChanged
    connect(_itemsView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(itemSelected(const QModelIndex &)));
    connect(_itemsModel, SIGNAL(itemMoved(const QModelIndex &, const QModelIndex &)), SLOT(moveItem(const QModelIndex &, const QModelIndex &)));
}

void ItemsPropertiesSplitter::itemSelected(const QModelIndex &index, bool display /*= true*/)
{
    ItemInfo *item = _itemsModel->itemAtIndex(index);
    if (display)
        _propertiesWidget->showItem(item);

    // correctly disable hotkeys
    _itemActions[DisenchantShards]->setEnabled(ItemDataBase::canDisenchantIntoArcaneShards(item));
    _itemActions[DisenchantSignet]->setEnabled(ItemDataBase::canDisenchantIntoSignetOfLearning(item));
    _itemActions[RemoveMO]->setEnabled(_propertiesWidget->hasMysticOrbs() && !ItemDataBase::isUberCharm(item));

    // item bbcode
    QAction *bbcodeAction = _itemActions[CopyItemBBCode];
    bbcodeAction->setObjectName(itemNameBBCode(item));
    foreach (QAction *action, bbcodeAction->menu()->actions() + QList<QAction *>() << bbcodeAction)
        action->setDisabled(bbcodeAction->objectName().isEmpty());

    // eat signet of learning
    quint8 statsFromSignet = 0;
    if (item)
    {
        QRegExp customSignetRegExp("(\\d\\d)\\^");
        if (isSignetOfLearning(item))
            statsFromSignet = 1;
        else if (customSignetRegExp.exactMatch(item->itemType))
            statsFromSignet = customSignetRegExp.cap(1).toUShort();
        else if (item->itemType == "zk#")
            statsFromSignet = 5;
        else if (item->itemType == "zke" || item->itemType == "zky")
            statsFromSignet = 25;
    }
    QAction *eatSignetsAction = _itemActions[EatSignetOfLearning];
    eatSignetsAction->setData(statsFromSignet);
    eatSignetsAction->setText(tr("Eat signet [%n free stat(s)]", 0, statsFromSignet));
    eatSignetsAction->setEnabled(statsFromSignet > 0 && CharacterInfo::instance().valueOfStatistic(Enums::CharacterStats::SignetsOfLearningEaten) < Enums::CharacterStats::SignetsOfLearningMax);
}

void ItemsPropertiesSplitter::moveItem(const QModelIndex &newIndex, const QModelIndex &oldIndex)
{
    ItemInfo *item = _itemsModel->itemAtIndex(newIndex);
    ReverseBitWriter::updateItemRow(item);
    ReverseBitWriter::updateItemColumn(item);
    item->hasChanged = true;

    int oldRow = oldIndex.row(), oldCol = oldIndex.column();
    if (_itemsView->rowSpan(oldRow, oldCol) > 1 || _itemsView->columnSpan(oldRow, oldCol) > 1)
        _itemsView->setSpan(oldRow, oldCol, 1, 1);
    setCellSpanForItem(item);
    _itemsView->setCurrentIndex(newIndex);

    emit itemsChanged();
}

void ItemsPropertiesSplitter::showItem(ItemInfo *item)
{
    if (item)
    {
        _itemsView->setCurrentIndex(_itemsModel->index(item->row, item->column));
        //dumpInfo(item, false);
    }
}

void ItemsPropertiesSplitter::showFirstItem()
{
    _itemsView->setFocus(); // otherwise
    // sometimes item is selected, but there's no visual selection
    if (_itemsView->selectionModel()->selectedIndexes().isEmpty() || !selectedItem(false))
        showItem(_itemsModel->firstItem());
}

void ItemsPropertiesSplitter::setItems(const ItemsList &newItems)
{
    _allItems = newItems;
    updateItems(_allItems);
}

QPair<bool, bool> ItemsPropertiesSplitter::updateDisenchantButtonsState(bool includeUniques, bool includeSets, bool toCrystals, ItemsList *items /*= 0*/)
{
    bool allowShards = false, allowSignets = false;
    int shards = 0;
    foreach (ItemInfo *item, items ? *items : _allItems)
    {
        if (toCrystals)
        {
            if (isArcaneShard(item))
                ++shards;
            else if (isArcaneShard2(item))
                shards += 2;
            else if (isArcaneShard3(item))
                shards += 3;
            else if (isArcaneShard4(item))
                shards += 4;
        }

        if ((includeUniques && item->quality == Enums::ItemQuality::Unique) || (includeSets && item->quality == Enums::ItemQuality::Set))
        {
            if (!allowShards)
                allowShards = ItemDataBase::canDisenchantIntoArcaneShards(item);
            if (!allowSignets)
                allowSignets = ItemDataBase::canDisenchantIntoSignetOfLearning(item);
            if (allowShards && allowSignets)
                break;
        }
    }
    // allow just upgrading shards to crystals
    if (toCrystals && !allowShards && shards >= kShardsPerCrystal)
        allowShards = true;
    return qMakePair(allowShards, allowSignets);
}

bool ItemsPropertiesSplitter::canSocketableMapBeUpgraded(const UpgradableItemsMultiMap &socketableMap)
{
    foreach (quint8 key, socketableMap.uniqueKeys())
        if (socketableMap.count(key) > 1)
            return true;
    return false;
}

QPair<bool, bool> ItemsPropertiesSplitter::updateUpgradeButtonsState(int reserveRunes, ItemsList *pItems /*= 0*/)
{
    const ItemsList &items = pItems ? *pItems : _allItems;

    bool enableGemsButton = false;
    QHash<QByteArray, UpgradableItemsMultiMap> gemsMapsHash = gemsMapsFromItems(items);
    foreach (const UpgradableItemsMultiMap &gemsMap, gemsMapsHash)
        if ((enableGemsButton = canSocketableMapBeUpgraded(gemsMap)))
            break;

    return qMakePair(enableGemsButton, canSocketableMapBeUpgraded(runesMapFromItems(items, reserveRunes)));
}

void ItemsPropertiesSplitter::updateItems(const ItemsList &newItems)
{
    _propertiesWidget->clear();
    _itemsModel->setItems(newItems);

    _itemsView->clearSpans();
    foreach (ItemInfo *item, newItems)
        setCellSpanForItem(item);

    showFirstItem();
}

void ItemsPropertiesSplitter::showContextMenu(const QPoint &pos)
{
    ItemInfo *item = selectedItem(false);
    if (item)
    {
        QList<QAction *> actions;

        if (shouldAddMoveItemAction())
        {
            QAction *moveBetweenStashesAction = new QAction(moveItemActionText() + QString(" (%1)").arg(tr("Alt+Click")), this);
            connect(moveBetweenStashesAction, SIGNAL(triggered()), SLOT(moveBetweenStashes()));
            actions << moveBetweenStashesAction << separatorAction();
        }

        if (_itemActions[CopyItemBBCode]->isEnabled())
            actions << _itemActions[CopyItemBBCode] << separatorAction();

        // TODO: 0.5
        //QMenu *menuExport = new QMenu(tr("Export as"), _itemsView);
        //menuExport->addActions(QList<QAction *>() << _itemActions[ExportBbCode] << _itemActions[ExportHtml]);
        //actions << menuExport->menuAction() << separator();

        if (_itemActions[DisenchantShards]->isEnabled() || _itemActions[DisenchantSignet]->isEnabled())
        {
            QMenu *menuDisenchant = new QMenu(tr("Disenchant into"), _itemsView);
            if (_itemActions[DisenchantSignet]->isEnabled())
                menuDisenchant->addAction(_itemActions[DisenchantSignet]);
            if (_itemActions[DisenchantShards]->isEnabled())
                menuDisenchant->addAction(_itemActions[DisenchantShards]);
            actions << menuDisenchant->menuAction();
        }

        // TODO: 0.5
//        if (item->isSocketed && item->socketablesNumber)
//            actions << _itemActions[Unsocket];

        if (_itemActions[RemoveMO]->isEnabled())
        {
            QAction *actionToAdd = 0;
            if (_propertiesWidget->mysticOrbsTotal() > 1)
            {
                QMenu *menuMO = new QMenu(_itemsView);
                _itemActions[RemoveMO]->setText(tr("All"));
                menuMO->addActions(QList<QAction *>() << _itemActions[RemoveMO] << separatorAction());

                createActionsForMysticOrbs(menuMO, true, item);
                menuMO->addAction(separatorAction());
                createActionsForMysticOrbs(menuMO, false, item);

                actionToAdd = menuMO->menuAction();
            }
            else
                actionToAdd = _itemActions[RemoveMO];

            actionToAdd->setText(tr("Remove Mystic Orbs"));
            actions << actionToAdd;
        }

        // downgrade a rune
        if (kRuneRegExp.exactMatch(item->itemType))
        {
            quint8 runeCode = kRuneRegExp.cap(1).toUShort();
            if (runeCode > 1 && runeCode <= HighestRuneKey)
            {
                QMenu *menuDowngrade = new QMenu(tr("Downgrade to"), _itemsView);
                while (--runeCode)
                {
                    QByteArray runeKey = QString("r%1").arg(runeCode, 2, 10, kZeroChar).toLatin1();
                    ItemBase *base = ItemDataBase::Items()->value(runeKey);
                    QAction *actionRune = new QAction(QIcon(ResourcePathManager::pathForItemImageName(base->imageName)), QString("(%1) %2").arg(base->rlvl).arg(QString(base->name).remove("\\purple;")), _itemsView);
                    actionRune->setData(runeCode);
                    actionRune->setIconVisibleInMenu(true); // explicitly show icon on Mac OS X
                    connect(actionRune, SIGNAL(triggered()), SLOT(downgradeSelectedRune()));
                    menuDowngrade->addAction(actionRune);
                }
                actions << menuDowngrade->menuAction();
            }
        }

        if (_itemActions[EatSignetOfLearning]->data().toUInt() > 0)
            actions << _itemActions[EatSignetOfLearning];

        if (isShrineVessel(item))
        {
            QAction *collectAction = new QAction(tr("Collect Shrines"), this);
            connect(collectAction, SIGNAL(triggered()), SLOT(collectShrinesToVessel()));
            actions << collectAction;

            ItemProperty *prop = item->props.value(Enums::ItemProperties::ShrineVesselCounter);
            if (prop->value > 0)
            {
                QAction *extractAction = new QAction(tr("Extract all Shrines"), this);
                connect(extractAction, SIGNAL(triggered()), SLOT(extractShrinesFromVessel()));
                actions << extractAction;

                if (prop->value > 1)
                {
                    extractAction = new QAction(tr("Extract Shrines..."), this);
                    extractAction->setObjectName("input");
                    connect(extractAction, SIGNAL(triggered()), SLOT(extractShrinesFromVessel()));
                    actions << extractAction;
                }
                else
                    extractAction->setText(tr("Extract the only Shrine"));
            }
            actions << separatorAction();
        }

        if (item->isPersonalized)
        {
            QAction *depersonalizeAction = new QAction(tr("Depersonalize"), this);
            connect(depersonalizeAction, SIGNAL(triggered()), SLOT(depersonalize()));
            actions << depersonalizeAction;
        }
        else if (item->isExtended) // allow personalization of any item
        {
            QAction *personalizeAction = new QAction(tr("Personalize"), this);
            connect(personalizeAction, SIGNAL(triggered()), SLOT(personalize()));
            actions << personalizeAction;

            personalizeAction = new QAction(tr("Personalize with name..."), this);
            personalizeAction->setObjectName("setName");
            connect(personalizeAction, SIGNAL(triggered()), SLOT(personalize()));
            actions << personalizeAction;
        }

        actions << separatorAction() << _itemActions[Delete];
#ifdef DUMP_INFO_ACTION
        QAction *dumpInfoAction = new QAction("Dump info", this);
        connect(dumpInfoAction, SIGNAL(triggered()), SLOT(dumpInfo()));
        actions << separatorAction() << dumpInfoAction;
#endif
        QMenu::exec(actions, _itemsView->mapToGlobal(pos));
    }
}

ItemInfo *ItemsPropertiesSplitter::selectedItem(bool showError /*= true*/)
{
    //QModelIndexList selectedIndexes = _itemsView->selectionModel()->selectedIndexes();
    ItemInfo *item = _itemsModel->itemAtIndex(_itemsView->selectionModel()->currentIndex());
    if (!item && showError)
        ERROR_BOX("TROLOLOL no item selection found");
    return item;
}

bool ItemsPropertiesSplitter::moveBetweenStashes()
{
    ItemInfo *item = selectedItem(false);
    if (sender() == _itemsView && !(item && qApp->mouseButtons() == Qt::LeftButton && qApp->keyboardModifiers() == Qt::AltModifier)) // Alt+Click
        return false;

    removeItemFromModel(item);
    _itemsView->selectionModel()->clearSelection();
    emit itemCountChanged(_itemsModel->itemCount());

    emit itemMovingBetweenStashes(item);
    return true;
}

void ItemsPropertiesSplitter::exportText()
{

}

void ItemsPropertiesSplitter::copyAllItemsBBCode(QAction *action)
{
    QStringList codes;
    foreach (ItemInfo *item, _itemsModel->items())
    {
        QString name = itemNameBBCode(item);
        if (!name.isEmpty())
            codes << itemBBCode(name, action->objectName());
    }
    if (!codes.isEmpty())
        qApp->clipboard()->setText(codes.join(QChar('\n')));
}

void ItemsPropertiesSplitter::copyItemBBCode(QAction *action)
{
    QAction *menuAction = qobject_cast<QMenu *>(sender())->menuAction();
    qApp->clipboard()->setText(itemBBCode(menuAction->objectName(), action->objectName()));
}

void ItemsPropertiesSplitter::disenchantSelectedItem()
{
    QAction *action = qobject_cast<QAction *>(sender());
    ItemInfo *item = selectedItem();
    if (!action || !item)
    {
        ERROR_BOX("TROLOLOL I can't disenchant the item");
        return;
    }

    ItemInfo *newItem = ItemDataBase::loadItemFromFile(action->objectName() == "signet" ? "signet_of_learning" : "arcane_shard");
    ItemInfo *newItemStored = disenchantItemIntoItem(item, newItem);
    delete newItem;

    if (newItemStored) // let's be safe
    {
        _itemsView->setCurrentIndex(_itemsModel->index(newItemStored->row, newItemStored->column));
        itemSelected(_itemsView->currentIndex(), false);
    }
}

void ItemsPropertiesSplitter::downgradeSelectedRune()
{
    QAction *senderAction = qobject_cast<QAction *>(sender());
    if (!senderAction)
    {
        ERROR_BOX("EPIC PHAIL WHEN DOWNGRADING RUNE");
        return;
    }

    quint8 newRuneCode = senderAction->data().toUInt();
    ItemInfo *item = selectedItem();
    item->hasChanged = true;
    item->itemType = QString("r%1").arg(newRuneCode, 2, 10, kZeroChar).toLatin1();
    for (int i = 1; i <= 2; ++i)
        ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Type + i*8, item->itemType.at(i));

    _propertiesWidget->showItem(item);
    emit itemsChanged();
}

void ItemsPropertiesSplitter::eatSelectedSignet()
{
    QAction *senderAction = qobject_cast<QAction *>(sender());
    if (!senderAction)
    {
        ERROR_BOX("EPIC PHAIL WHEN EATING SIGNET");
        return;
    }

    uint signetsToEat = senderAction->data().toUInt(), signetsEaten = CharacterInfo::instance().valueOfStatistic(Enums::CharacterStats::SignetsOfLearningEaten);
    const uint signetsMax = Enums::CharacterStats::SignetsOfLearningMax;
    if (signetsEaten + signetsToEat > signetsMax)
        if (QUESTION_BOX_YESNO(tr("You're going to eat %n signet(s), which is beyond the limit (%1) by %2.\nDo you really want to do it?", 0, signetsToEat).arg(signetsMax).arg(signetsEaten + signetsToEat - signetsMax), QMessageBox::No) == QMessageBox::No)
            return;
    deleteItem(selectedItem());
    emit signetsOfLearningEaten(signetsToEat);
}

void ItemsPropertiesSplitter::collectShrinesToVessel()
{
    ItemInfo *vesselItem = selectedItem();
    QByteArray shrineType = QByteArray::fromRawData(vesselItem->itemType.constData(), vesselItem->itemType.length() - 1);
    int shrines = 0;
    foreach (ItemInfo *item, _allItems)
    {
        if (item->itemType == shrineType)
        {
            ++shrines;
            performDeleteItem(item, false);
        }
    }

    if (shrines)
    {
        ItemProperty *vesselProp = vesselItem->props.value(Enums::ItemProperties::ShrineVesselCounter);
        if (vesselProp->value < 0)
            vesselProp->value = 0;
        vesselProp->value += shrines;

        ItemPropertyTxt *txtProp = ItemDataBase::Properties()->value(Enums::ItemProperties::ShrineVesselCounter);
        ReverseBitWriter::replaceValueInBitString(vesselItem->bitString, vesselProp->bitStringOffset, vesselProp->value + txtProp->add, txtProp->bits);

        vesselItem->hasChanged = true;
        _propertiesWidget->showItem(vesselItem);
        emit itemsChanged();

        INFO_BOX(tr("%n Shrine(s) inserted in the Vessel", 0, shrines));
    }
    else
        INFO_BOX(tr("No Shrines of selected type found"));
}

void ItemsPropertiesSplitter::extractShrinesFromVessel()
{
    ItemInfo *vesselItem = selectedItem();
    ItemProperty *vesselProp = vesselItem->props.value(Enums::ItemProperties::ShrineVesselCounter);
    int n = vesselProp->value;
    if (sender()->objectName() == "input")
    {
        bool ok;
        n = QInputDialog::getInt(this, tr("Shrine Vessel"), tr("Extract Shrines (1-%1)").arg(n), 1, 1, n, 1, &ok);
        if (!ok)
            return;
    }

    ItemInfo *shrine = ItemDataBase::loadItemFromFile("shrine");
    if (shrine->itemType.at(0) != vesselItem->itemType.at(0))
    {
        shrine->itemType = QByteArray(vesselItem->itemType.constData(), vesselItem->itemType.length() - 1); // e.g. B0+S => B0+
        ReverseBitWriter::replaceValueInBitString(shrine->bitString, Enums::ItemOffsets::Type, shrine->itemType.at(0)); // shrines differ only by the first type letter
    }

    bool notEnoughSpace = false;
    int stored = 0;
    for (; stored < n; ++stored)
    {
        ItemInfo *shrineCopy = new ItemInfo(*shrine);
        if (!storeItemInStorage(shrineCopy, vesselItem->storage))
        {
            notEnoughSpace = true;
            delete shrineCopy;
            break;
        }
        setCellSpanForItem(shrineCopy);
    }
    delete shrine;

    if (!stored)
    {
        ERROR_BOX(tr("No free space for Shrines in current storage"));
        return;
    }

    vesselProp->value -= stored;
    ItemPropertyTxt *txtProp = ItemDataBase::Properties()->value(Enums::ItemProperties::ShrineVesselCounter);
    ReverseBitWriter::replaceValueInBitString(vesselItem->bitString, vesselProp->bitStringOffset, vesselProp->value + txtProp->add, txtProp->bits);

    vesselItem->hasChanged = true;
    _propertiesWidget->showItem(vesselItem);
    emit itemsChanged();

    if (notEnoughSpace)
        ERROR_BOX(tr("Not enough space to store %1 Shrines in current storage, extracted only %2 pieces").arg(n, stored));
}

void ItemsPropertiesSplitter::depersonalize()
{
    ItemInfo *item = selectedItem();

    ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::IsPersonalized, 0);
    item->isPersonalized = false;

    ReverseBitWriter::remove(item->bitString, item->inscribedNameOffset, (item->inscribedName.length() + 1) * ItemParser::kInscribedNameCharacterLength); // also remove trailing \0
    item->inscribedName.clear();

    ReverseBitWriter::byteAlignBits(item->bitString);
    item->hasChanged = true;

    _propertiesWidget->showItem(item);
    emit itemsChanged();
}

void ItemsPropertiesSplitter::personalize()
{
    ItemInfo *item = selectedItem();
    QString personalizationName = CharacterInfo::instance().basicInfo.originalName;
    // remove all colors
    for (int i = 0; i < ColorsManager::colorCodes().size(); ++i)
        personalizationName.remove(ColorsManager::unicodeColorHeader() + ColorsManager::colorCodes().at(i));

    if (sender()->objectName() == "setName") // set arbitrary name
    {
        QD2CharRenamer renameWidget(personalizationName, false, this, false);
        renameWidget.setLineToolTip(tr("Colors don't work in personalized name"));
        if (!renameWidget.exec())
            return;
        personalizationName = renameWidget.name();
    }

    ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::IsPersonalized, 1);
    item->isPersonalized = true;

    item->inscribedName = personalizationName.toLatin1();
    const char *personalizationNameCstr = item->inscribedName.constData();
    QString personalizationNameBitString;
    for (quint8 i = 0; i < personalizationName.length() + 1; ++i) // trailing \0 must also be written
        personalizationNameBitString.prepend(binaryStringFromNumber(personalizationNameCstr[i], false, ItemParser::kInscribedNameCharacterLength));
    ReverseBitWriter::insert(item->bitString, item->inscribedNameOffset, personalizationNameBitString);

    ReverseBitWriter::byteAlignBits(item->bitString);
    item->hasChanged = true;

    _propertiesWidget->showItem(item);
    emit itemsChanged();
}

//void ItemsPropertiesSplitter::unsocketItem()
//{

//}

void ItemsPropertiesSplitter::deleteItemTriggered()
{
    ItemInfo *item = selectedItem(false);
    if (item && QUESTION_BOX_YESNO(tr("Are you sure you want to delete this item?"), QMessageBox::Yes) == QMessageBox::Yes)
    {
        bool isCube = ItemDataBase::isCube(item);
        if (isCube && !ItemDataBase::itemsStoredIn(Enums::ItemStorage::Cube).isEmpty())
            if (QUESTION_BOX_YESNO(tr("Cube is not empty. Do you really want to delete it?\nNote: items inside will be preserved. You can recover them by getting new Cube."), QMessageBox::No) == QMessageBox::Yes)
                return;

        deleteItem(item);
        if (isCube)
            emit cubeDeleted();
    }
}

#ifdef DUMP_INFO_ACTION
void ItemsPropertiesSplitter::dumpInfo(ItemInfo *item /*= 0*/, bool shouldShowMsgBox /*= true*/)
{
    if (!item)
        item = selectedItem(false);
    ItemBase *base = ItemDataBase::Items()->value(item->itemType);
    const char *quality = metaEnumFromName<Enums::ItemQuality>("ItemQualityEnum").valueToKey(item->quality);
    bool isSetOrUnique = areBothItemsSetOrUnique(item, item); // hacky code :)

    qDebug() << ItemParser::itemStorageAndCoordinatesString("location %1, row %2, col %3, equipped in %4", item) << "quality" << quality << "code" << item->itemType << "types" << base->types << "image" << base->imageName << "vargfx" << item->variableGraphicIndex << "quest ID" << base->questId;
    if (isSetOrUnique)
        qDebug() << "set/unique ID" << item->setOrUniqueId;
    for (PropertiesMultiMap::const_iterator it = item->props.constBegin(); it != item->props.constEnd(); ++it)
        qDebug() << "prop" << it.key() << it.value()->value << it.value()->param;
    qDebug("--------------------");

    if (shouldShowMsgBox)
    {
        QString types;
        foreach (const QByteArray &type, base->types)
            types += type + ", ";
        INFO_BOX(QString("%1\nquality %2, set/unique ID %3\ncode %4, types: %5\nimage %6, vargfx %7, quest ID %8").arg(ItemParser::itemStorageAndCoordinatesString("location %1, row %2, col %3, equipped in %4", item))
            .arg(quality).arg(isSetOrUnique ? item->setOrUniqueId : 0).arg(QTextCodec::codecForName("Windows-1252")->toUnicode(item->itemType)).arg(types).arg(base->imageName.constData()).arg(item->variableGraphicIndex).arg(base->questId));
    }
}
#endif

void ItemsPropertiesSplitter::setCurrentStorageHasChanged()
{
    // a hack to make stash modified
    if (!_allItems.isEmpty())
        _allItems.first()->hasChanged = true;
}

void ItemsPropertiesSplitter::deleteItem(ItemInfo *item)
{
    performDeleteItem(item);
    showFirstItem();

    emit itemCountChanged(_allItems.size());
    emit itemDeleted();

    setCurrentStorageHasChanged();
}

void ItemsPropertiesSplitter::performDeleteItem(ItemInfo *item, bool emitSignal /*= true*/)
{
    // TODO: [0.5] add option to unsocket at first
    removeItemFromList(item, emitSignal);
    delete item;
}

void ItemsPropertiesSplitter::addItemToList(ItemInfo *item, bool emitSignal /*= true*/)
{
    if (!CharacterInfo::instance().items.character.contains(item))
        CharacterInfo::instance().items.character.append(item);

    if (!_allItems.contains(item))
        _allItems.append(item);
    if (isItemInCurrentStorage(item))
    {
        _itemsModel->addItem(item);
        if (selectedItem(false) == item) // signal is emitted only when single item is disenchanted (through context menu), so we don't need extra parameter for just another name
            _propertiesWidget->showItem(item);
    }

    if (emitSignal)
        emit itemsChanged();
}

void ItemsPropertiesSplitter::removeItemFromList(ItemInfo *item, bool emitSignal /*= true*/)
{
    CharacterInfo::instance().items.character.removeOne(item);

    _allItems.removeOne(item);
    if (selectedItem(false) == item)
        _propertiesWidget->clear();
    if (isItemInCurrentStorage(item))
        removeItemFromModel(item);

    if (emitSignal)
        emit itemsChanged();
}

void ItemsPropertiesSplitter::removeItemFromModel(ItemInfo *item)
{
    _itemsModel->removeItem(item);
    if (_itemsView->rowSpan(item->row, item->column) > 1 || _itemsView->columnSpan(item->row, item->column) > 1) // hides warning in console
        _itemsView->setSpan(item->row, item->column, 1, 1);
}

ItemsList ItemsPropertiesSplitter::disenchantAllItems(bool toShards, bool upgradeToCrystals, bool eatSignets, ItemsList *pItems /*= 0*/)
{
    ProgressBarModal progressBar;
    progressBar.centerInWidget(this);
    progressBar.show();

    ItemsList &items = pItems ? *pItems : _allItems, disenchantedItems;
    quint32 disenchantedItemsNumber = items.size(), signetsEaten = 0, signetsEatenTotal = CharacterInfo::instance().valueOfStatistic(Enums::CharacterStats::SignetsOfLearningEaten);
    ItemInfo *disenchantedItem = ItemDataBase::loadItemFromFile(toShards ? "arcane_shard" : "signet_of_learning");
    foreach (ItemInfo *item, items)
    {
        qApp->processEvents();

        bool shouldDisenchant = true;
        if (!toShards && eatSignets)
        {
            if (signetsEatenTotal + signetsEaten < Enums::CharacterStats::SignetsOfLearningMax)
            {
                removeItemFromList(item, false);
                shouldDisenchant = false;
                ++signetsEaten;
            }
        }
        if (shouldDisenchant)
            disenchantedItems += disenchantItemIntoItem(item, disenchantedItem, false);
    }

    QString text;
    QString baseTextFormat = tr("You've received %1", "number of Arcane Crystals, Arcane Shards, or Signets of Learning");
    if (toShards)
    {
        if (upgradeToCrystals)
        {
            quint32 shards = 0;
            foreach (ItemInfo *item, disenchantedItems)
            {
                qApp->processEvents();
                if (isArcaneShard(item))
                    ++shards;
                else if (isArcaneShard2(item))
                    shards += 2;
                else if (isArcaneShard3(item))
                    shards += 3;
                else if (isArcaneShard4(item))
                    shards += 4;
            }

            quint32 crystals = shards / kShardsPerCrystal;
            if (crystals)
            {
                int storage = disenchantedItems.first()->storage;
                foreach (ItemInfo *item, disenchantedItems)
                {
                    if (isArcaneShard(item) || isArcaneShard2(item) || isArcaneShard3(item) || isArcaneShard4(item))
                    {
                        qApp->processEvents();
                        performDeleteItem(item, false);
                    }
                }

                ItemInfo *crystal = ItemDataBase::loadItemFromFile("arcane_crystal");
                for (quint32 i = 0; i < crystals; ++i)
                {
                    qApp->processEvents();

                    ItemInfo *crystalCopy = new ItemInfo(*crystal);
                    storeItemInStorage(crystalCopy, storage);
                }
                delete crystal;

                quint8 shardsLeft = shards - crystals * kShardsPerCrystal;
                for (int i = 0; i < shardsLeft; ++i)
                {
                    qApp->processEvents();

                    ItemInfo *shard = new ItemInfo(*disenchantedItem);
                    storeItemInStorage(shard, storage);
                }

                QString crystalsText = tr("%n Arcane Crystal(s)", 0, crystals);
                text = baseTextFormat.arg(crystalsText);
                if (shardsLeft)
                    text += QString(" %1 %2").arg(tr("and"), tr("%n Arcane Shard(s)", 0, shardsLeft));

                emit itemCountChanged(_allItems.size());
            }
        }

        if (text.isEmpty())
            text = baseTextFormat.arg(tr("%n Arcane Shard(s)", 0, disenchantedItemsNumber));
    }
    else
    {
        QString signetsText = tr("%n Signet(s) of Learning", 0, disenchantedItemsNumber);
        QString baseSignetsTextFormat = tr("You've eaten %1", "number of Signets of Learning");
        if (eatSignets && signetsEaten)
        {
            emit signetsOfLearningEaten(signetsEaten);

            if (static_cast<quint32>(signetsEaten) == disenchantedItemsNumber)
                text = baseSignetsTextFormat.arg(signetsText);
            else
            {
                signetsText = tr("%n Signet(s) of Learning", 0, signetsEaten);
                if (signetsEaten != Enums::CharacterStats::SignetsOfLearningMax)
                    text = tr("%1 (now you have %2) and received %3").arg(baseSignetsTextFormat.arg(signetsText)).arg(QString::number(Enums::CharacterStats::SignetsOfLearningMax))
                                                                     .arg(tr("%n Signet(s) of Learning", 0, disenchantedItemsNumber - signetsEaten));
                else
                    text = tr("%1 and received %2").arg(baseSignetsTextFormat.arg(signetsText)).arg(disenchantedItemsNumber - signetsEaten);
            }
        }
        else
            text = baseTextFormat.arg(signetsText);
    }
    progressBar.hide();

    delete disenchantedItem;
    emit itemsChanged();
    _itemsView->viewport()->update();

    INFO_BOX(text);
    return disenchantedItems;
}

ItemInfo *ItemsPropertiesSplitter::disenchantItemIntoItem(ItemInfo *oldItem, ItemInfo *newItem, bool emitSignal /*= true*/)
{
    ItemsList items = ItemDataBase::itemsStoredIn(oldItem->storage, oldItem->location, oldItem->plugyPage ? &oldItem->plugyPage : 0);
    items.removeOne(oldItem);
    ItemInfo *newItemCopy = new ItemInfo(*newItem); // it's safe because there're no properties and no socketables
    if (!ItemDataBase::canStoreItemAt(oldItem->row, oldItem->column, newItemCopy->itemType, items, ItemsViewerDialog::rowsInStorageAtIndex(oldItem->storage), ItemsViewerDialog::colsInStorageAtIndex(oldItem->storage)))
    {
        ERROR_BOX("If you see this text (which you shouldn't), please tell me which item you've just tried to disenchant");
        delete newItemCopy;
        return 0;
    }

    newItemCopy->move(oldItem->row, oldItem->column, oldItem->plugyPage);
    newItemCopy->storage = oldItem->storage;
    newItemCopy->whereEquipped = oldItem->whereEquipped;

    // update bits
    ReverseBitWriter::replaceValueInBitString(newItemCopy->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newItemCopy) ? Enums::ItemStorage::Stash : newItemCopy->storage);

    performDeleteItem(oldItem, emitSignal);
    addItemToList(newItemCopy, emitSignal);

    return newItemCopy;
}

bool ItemsPropertiesSplitter::storeItemInStorage(ItemInfo *item, int storage, bool emitSignal /*= false*/)
{
    bool result = ItemDataBase::storeItemIn(item, static_cast<Enums::ItemStorage::ItemStorageEnum>(storage), ItemsViewerDialog::rowsInStorageAtIndex(storage), ItemsViewerDialog::colsInStorageAtIndex(storage));
    if (result)
        addItemToList(item, emitSignal);
    return result;
}

void ItemsPropertiesSplitter::setCellSpanForItem(ItemInfo *item)
{
    _itemsView->setCellSpanForItem(item);
}

bool ItemsPropertiesSplitter::upgradeItemsInMap(UpgradableItemsMultiMap &itemsMap, quint8 maxKey, const QString &itemNameFormat)
{
    QList<quint8> keys = itemsMap.uniqueKeys();
    if (keys.isEmpty())
        return false;

    int currentStorage = itemsMap.value(keys.at(0))->storage;
    for (int i = 0; i < keys.size(); ++i)
    {
        quint8 key = keys.at(i);
        int sameItemsSize = itemsMap.values(key).size(), upgradedItemsSize = sameItemsSize / 2, leftItemsSize = sameItemsSize - upgradedItemsSize * 2;
        if (upgradedItemsSize)
        {
            for (UpgradableItemsMultiMap::iterator iter = itemsMap.begin(); sameItemsSize > leftItemsSize && iter != itemsMap.end();)
            {
                if (iter.key() == key)
                {
                    qApp->processEvents();
                    removeItemFromList(iter.value(), false);
                    --sameItemsSize;
                    iter = itemsMap.erase(iter);
                }
                else
                    ++iter;
            }

            quint8 newKey = key + 1;
            ItemsList higherItems = itemsMap.values(newKey);
            ItemInfo *newItem = higherItems.isEmpty() ? ItemDataBase::loadItemFromFile(itemNameFormat.arg(newKey)) : higherItems.first();
            for (int j = 0; j < upgradedItemsSize; ++j)
            {
                qApp->processEvents();
                ItemInfo *itemCopy = new ItemInfo(*newItem);
                storeItemInStorage(itemCopy, currentStorage);
                itemsMap.insertMulti(newKey, itemCopy);
            }

            if (higherItems.isEmpty())
            {
                delete newItem;

                if (newKey == maxKey)
                    break;
                keys.insert(i + 1, newKey);
            }
        }
    }
    return true;
}

void ItemsPropertiesSplitter::upgradeGems(ItemsList *pItems /*= 0*/)
{
    QHash<QByteArray, UpgradableItemsMultiMap> gemsMapsHash = gemsMapsFromItems(pItems ? *pItems : _allItems);
    for (QHash<QByteArray, UpgradableItemsMultiMap>::iterator iter = gemsMapsHash.begin(); iter != gemsMapsHash.end(); ++iter)
        upgradeItemsInMap(iter.value(), kPerfectGrade, QString("gems/%1%2").arg(iter.key().constData()).arg("%1"));

    emit itemsChanged();
    emit itemCountChanged(_allItems.size());
}

void ItemsPropertiesSplitter::upgradeRunes(int reserveRunes, ItemsList *pItems /*= 0*/)
{
    UpgradableItemsMultiMap runesMap = runesMapFromItems(pItems ? *pItems : _allItems, reserveRunes);
    if (upgradeItemsInMap(runesMap, HighestRuneKey, "runes/r%1"))
    {
        emit itemsChanged();
        emit itemCountChanged(_allItems.size());
    }
}

QHash<QByteArray, UpgradableItemsMultiMap> ItemsPropertiesSplitter::gemsMapsFromItems(const ItemsList &items)
{
    const QByteArray kPerfectGradeBytes = QByteArray::number(kPerfectGrade);

    QMultiHash<QByteArray, ItemInfo *> allGems;
    foreach (ItemInfo *item, items)
    {
        QList<QByteArray> types = ItemDataBase::Items()->value(item->itemType)->types;  // first element is gem type, second element is gem grade
        if (types.at(0).startsWith("gem") && !types.at(1).endsWith(kPerfectGradeBytes)) // exclude prefect gems
            allGems.insertMulti(types.at(0), item);
    }

    QHash<QByteArray, UpgradableItemsMultiMap> gemsMapsHash;
    foreach (const QByteArray &gemType, allGems.uniqueKeys())
    {
        UpgradableItemsMultiMap gemsMap;
        foreach (ItemInfo *gem, allGems.values(gemType))
            gemsMap.insertMulti(ItemDataBase::Items()->value(gem->itemType)->types.at(1).right(1).toUShort(), gem);
        gemsMapsHash[gemType] = gemsMap;
    }
    return gemsMapsHash;
}

UpgradableItemsMultiMap ItemsPropertiesSplitter::runesMapFromItems(const ItemsList &items, int reserveRunes)
{
    UpgradableItemsMultiMap runesMap;
    QHash<quint8, quint8> reserveHash;
    foreach (ItemInfo *item, items)
    {
        if (kRuneRegExp.exactMatch(item->itemType))
        {
            quint8 runeKey = kRuneRegExp.cap(1).toUShort();
            if (runeKey < HighestRuneKey) // don't include 'On' rune, Great runes and Ultimative runes
            {
                quint8 &reserve = reserveHash[runeKey];
                if (reserve < reserveRunes)
                    ++reserve;
                else
                    runesMap.insertMulti(runeKey, item);
            }
        }
    }
    return runesMap;
}

QString ItemsPropertiesSplitter::itemBBCode(const QString &name, const QString &codeType)
{
    return QString("[item%1]%2[/item]").arg(codeType.isEmpty() ? QString() : (QLatin1String("=") + codeType)).arg(name);
}

QString ItemsPropertiesSplitter::itemNameBBCode(ItemInfo *item)
{
    if (!item)
        return QString();

    QString name;
    if (item->quality == Enums::ItemQuality::Set)
    {
        if (SetItemInfo *setItem = ItemDataBase::Sets()->value(item->setOrUniqueId))
            name = setItem->itemName;
        else
            return QString();
    }
	else if (item->quality == Enums::ItemQuality::Unique)
	{
		if (UniqueItemInfo *uniqueInfo = ItemDataBase::Uniques()->value(item->setOrUniqueId))
			name = uniqueInfo->name;
		else
			return QString();
	}
    else if (item->isRW)
    {
        name = item->rwName;
        foreach (ItemInfo *socketable, item->socketablesInfo)
        {
            if (socketable->itemType.startsWith("rx"))
            {
                name += QLatin1String(" (Xis)");
                break;
            }
        }
    }
    else
        return QString();

    ItemDataBase::removeColorCodesFromString(name);
    return name.split(QLatin1String("\\n"), QString::SkipEmptyParts).last();
}

QMenu *ItemsPropertiesSplitter::createCopyBBCodeMenu(bool addShortcut)
{
    QMenu *menu = new QMenu(tr("Copy item BBCode"), _itemsView);

    QAction *actionBBCodeText = new QAction(tr("Text", "BBCode type"), menu);
    if (addShortcut)
        actionBBCodeText->setShortcut(QKeySequence("Ctrl+Alt+C"));

    QAction *actionBBCodeImage = new QAction(tr("Image", "BBCode type"), menu);
    actionBBCodeImage->setObjectName(QLatin1String("image"));

    QAction *actionBBCodeFull = new QAction(tr("Full", "BBCode type"), menu);
    actionBBCodeFull->setObjectName(QLatin1String("full"));

    menu->addActions(QList<QAction *>() << actionBBCodeText << actionBBCodeImage << actionBBCodeFull);
    return menu;
}

void ItemsPropertiesSplitter::createItemActions()
{
    //QAction *actionBbCode = new QAction("BBCode", _itemsView);
    //actionBbCode->setShortcut(QKeySequence("Ctrl+E"));
    //actionBbCode->setObjectName("bbcode");
    //connect(actionBbCode, SIGNAL(triggered()), SLOT(exportText()));
    //_itemsView->addAction(actionBbCode);
    //_itemActions[ExportBbCode] = actionBbCode;

    //QAction *actionHtml = new QAction("HTML", _itemsView);
    //actionHtml->setShortcut(QKeySequence("Alt+E"));
    //actionHtml->setObjectName("html");
    //connect(actionHtml, SIGNAL(triggered()), SLOT(exportText()));
    //_itemsView->addAction(actionHtml);
    //_itemActions[ExportHtml] = actionHtml;

    QMenu *menuCopyItemBBCode = createCopyBBCodeMenu(true);
    connect(menuCopyItemBBCode, SIGNAL(triggered(QAction*)), SLOT(copyItemBBCode(QAction*)));
    _itemsView->addActions(menuCopyItemBBCode->actions());
    _itemActions[CopyItemBBCode] = menuCopyItemBBCode->menuAction();

    QAction *actionSol = new QAction(QIcon(ResourcePathManager::pathForItemImageName("sigil1b")), tr("Signet of Learning"), _itemsView);
    actionSol->setShortcut(QKeySequence("Ctrl+D"));
    actionSol->setObjectName("signet");
    actionSol->setIconVisibleInMenu(true); // explicitly show icon on Mac OS X
    connect(actionSol, SIGNAL(triggered()), SLOT(disenchantSelectedItem()));
    _itemsView->addAction(actionSol);
    _itemActions[DisenchantSignet] = actionSol;

    QAction *actionShards = new QAction(QIcon(ResourcePathManager::pathForItemImageName("invfary4")), tr("Arcane Shards"), _itemsView);
    actionShards->setShortcut(QKeySequence("Alt+D"));
    actionShards->setObjectName("shards");
    actionShards->setIconVisibleInMenu(true); // explicitly show icon on Mac OS X
    connect(actionShards, SIGNAL(triggered()), SLOT(disenchantSelectedItem()));
    _itemsView->addAction(actionShards);
    _itemActions[DisenchantShards] = actionShards;

    // TODO: [0.5] unsocket
//    QAction *actionUnsocket = new QAction(tr("Unsocket"), _itemsView);
//    connect(actionUnsocket, SIGNAL(triggered()), SLOT(unsocketItem()));
//    _itemsView->addAction(actionUnsocket);
//    _itemActions[Unsocket] = actionUnsocket;

    QAction *actionRemoveMO = new QAction(_itemsView);
    actionRemoveMO->setShortcut(QKeySequence("Ctrl+M"));
    connect(actionRemoveMO, SIGNAL(triggered()), _propertiesWidget, SLOT(removeAllMysticOrbs()));
    connect(actionRemoveMO, SIGNAL(triggered()), SIGNAL(itemsChanged()));
    _itemsView->addAction(actionRemoveMO);
    _itemActions[RemoveMO] = actionRemoveMO;

    QAction *actionEatSignetOfLearning = new QAction(_itemsView);
    actionEatSignetOfLearning->setShortcut(QKeySequence("Ctrl+L"));
    connect(actionEatSignetOfLearning, SIGNAL(triggered()), SLOT(eatSelectedSignet()));
    _itemsView->addAction(actionEatSignetOfLearning);
    _itemActions[EatSignetOfLearning] = actionEatSignetOfLearning;

    QAction *actionDelete = new QAction(tr("Delete"), _itemsView);
    actionDelete->setShortcut(
#ifdef Q_OS_MAC
                Qt::Key_Backspace
#else
                QKeySequence::Delete
#endif
                );
    connect(actionDelete, SIGNAL(triggered()), SLOT(deleteItemTriggered()));
    _itemsView->addAction(actionDelete);
    _itemActions[Delete] = actionDelete;
}

QAction *ItemsPropertiesSplitter::separatorAction()
{
    QAction *sep = new QAction(_itemsView);
    sep->setSeparator(true);
    return sep;
}

void ItemsPropertiesSplitter::createActionsForMysticOrbs(QMenu *parentMenu, bool isItemMO, ItemInfo *item)
{
    foreach (int moCode, _propertiesWidget->mysticOrbs(isItemMO))
    {
        QAction *moAction = new QAction((isItemMO ? item->props : item->rwProps).value(moCode)->displayString, _itemsView);
        moAction->setProperty("isItemMO", isItemMO);
        moAction->setProperty("moCode", moCode);
        connect(moAction, SIGNAL(triggered()), _propertiesWidget, SLOT(removeMysticOrb()));
        connect(moAction, SIGNAL(triggered()), SIGNAL(itemsChanged()));
        parentMenu->addAction(moAction);
    }
}

bool ItemsPropertiesSplitter::shouldAddMoveItemAction() const { return true; }
QString ItemsPropertiesSplitter::moveItemActionText() const { return tr("Move to shared infinite stash"); }
