#ifndef HELPERS_H
#define HELPERS_H

// when building from visual studio
#ifndef IS_QT5
#define IS_QT5 (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#endif

// message boxes
#include <QMessageBox>
#define CUSTOM_BOX(type, message, buttons, defaultButton) QMessageBox::type(this, qApp->applicationName(), message, buttons, defaultButton)
#define QUESTION_BOX_YESNO(message, defaultButton) CUSTOM_BOX(question, message, QMessageBox::Yes | QMessageBox::No, defaultButton)
#define CUSTOM_BOX_OK(type, message) CUSTOM_BOX(type, message, QMessageBox::Ok, QMessageBox::Ok)
#define ERROR_BOX(message) CUSTOM_BOX_OK(critical, message)
#define INFO_BOX(message) CUSTOM_BOX_OK(information, message)
#define WARNING_BOX(message) CUSTOM_BOX_OK(warning, message)
#define ERROR_BOX_NO_PARENT(message) QMessageBox::critical(0, qApp->applicationName(), message)

// string building
static const QChar kZeroChar('0');
static const QString kHtmlLineBreak("<br />");
QString binaryStringFromNumber(quint64 number, bool needsInversion = false, int fieldWidth = 8);
QString coloredText(const QString &text, int colorIndex);

#include "colorsmanager.hpp"
QString htmlStringFromDiabloColorString(const QString &s, ColorsManager::ColorIndex defaultColor = ColorsManager::White);

// QMetaEnum getter. Moving definition to .cpp causes unresolved external symbols, so don't touch it.
#include <QMetaEnum>
template<class T>
QMetaEnum metaEnumFromName(const char *enumName)
{
    const QMetaObject &metaObject = T::staticMetaObject;
    return metaObject.enumerator(metaObject.indexOfEnumerator(enumName));
}

// UI
#include <QList>
class QTreeWidgetItem;
class ItemInfo;
class QTreeView;
extern const QString modName;
QList<QTreeWidgetItem *> treeItemsForItems(const QList<ItemInfo *> &items);
void customizeItemsTreeView(QTreeView *treeView);

qint32 getValueOfPropertyInItem(ItemInfo *item, quint16 propKey, quint16 param = 0);
void writeByteArrayDataWithNull(QDataStream &ds, const QByteArray &ba);
void writeByteArrayDataWithoutNull(QDataStream &ds, const QByteArray &ba);

bool isInExternalStorage(ItemInfo* item);

bool isTiered(ItemInfo *item);
bool isTiered(const QList<QByteArray> &itemTypes);
bool isSacred(ItemInfo *item);
bool isSacred(const QList<QByteArray> &itemTypes);
bool areBothItemsSetOrUnique(ItemInfo *a, ItemInfo *b);

// item conditions for STL algorithms
bool isCubeInCharacterItems(ItemInfo *item);
bool hasChanged(ItemInfo *item);

bool isClassCharm(ItemInfo *item);
bool isCrystallineFlameMedallion(ItemInfo *item);
bool isMoonOfSpider(ItemInfo *item);
bool isLegacyOfBlood(ItemInfo *item);
bool isDrekavacInGear(ItemInfo *item);
bool isVeneficaInGear(ItemInfo *item);
bool isArcaneShard(ItemInfo *item);
bool isSignetOfLearning(ItemInfo *item);
bool isCotw(ItemInfo *item);
// for Ultimative
bool isCharacterOrb(const QByteArray &itemType);
bool isCharacterOrb(ItemInfo *item);
bool isSunstoneOfElements(const QByteArray &itemType);
bool isSunstoneOfElements(ItemInfo *item);
bool isTradersChest(ItemInfo *item);
bool isTradersChest(const QByteArray &itemType);
bool isArcaneShard2(ItemInfo *item);
bool isArcaneShard3(ItemInfo *item);
bool isArcaneShard4(ItemInfo *item);
bool isShrineVessel(ItemInfo *item);

// sorting predicates
bool compareItemsByPlugyPage(ItemInfo *a, ItemInfo *b);
bool compareItemsByRlvl(ItemInfo *a, ItemInfo *b);
bool compareItemsByRlvlAndEthereality(ItemInfo *a, ItemInfo *b);
bool compareItemsByCode(ItemInfo *a, ItemInfo *b);

#endif // HELPERS_H
