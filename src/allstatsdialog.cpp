#include    "allstatsdialog.h"
#include "ui_allstatsdialog.h"
#include "propertiesdisplaymanager.h"
#include "itemdatabase.h"
#include "characterinfo.hpp"

#include <QPushButton>

#include <QTimer>
#include <QSettings>


AllStatsDialog::AllStatsDialog(QWidget *parent /*= 0*/) : QDialog(parent), ui(new Ui::AllStatsDialog), _searchTimer(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->buttonBox->button(QDialogButtonBox::Close)->setAutoDefault(false);
    ui->lineEdit->setFocus();

    connect(ui->lineEdit, SIGNAL(textChanged(QString)), SLOT(searchTextChanged(QString)));

    QVariant savedGeometry = QSettings().value("allStatsDialogGeometry");
    if (savedGeometry.isValid())
        restoreGeometry(savedGeometry.toByteArray());

    // fuck consts and enums!
    const QSet<int> kIgnoreProps = QSet<int>() << 16 << 17 << 75 << 91 << 92 << 125 << 152 << 185 << 186 << 219 << 243 << 252 << 253 << 254 << 263 << 276 << 288 << 296 << 299 << 300 << 372 << 418 << 419 << 421 << 422 << 423 << 424 << 425 << 427 << 442 << 443 << 473 << 474 << 491 << 501;

    PropertiesMap allProps;
    QMultiHash<QByteArray, int> setItemsHash;
    foreach (ItemInfo *item, CharacterInfo::instance().items.character) //-V807
    {
        if (ItemDataBase::doesItemGrantBonus(item))
        {
            PropertiesDisplayManager::addProperties(&allProps, item->props,   &kIgnoreProps);
            PropertiesDisplayManager::addProperties(&allProps, item->rwProps, &kIgnoreProps);

            ItemBase *itemBase = ItemDataBase::Items()->value(item->itemType);
            foreach (ItemInfo *socketableItem, item->socketablesInfo)
            {
                PropertiesMap socketableProps = PropertiesDisplayManager::socketableProperties(socketableItem, itemBase->socketableType);
                PropertiesDisplayManager::addProperties(&allProps, socketableProps, &kIgnoreProps);
                if (socketableProps != socketableItem->props)
                    qDeleteAll(socketableProps);
            }

            if (item->quality == Enums::ItemQuality::Set)
                if (SetItemInfo *setItem = ItemDataBase::Sets()->value(item->setOrUniqueId))
                    setItemsHash.insert(setItem->key, item->setOrUniqueId);
        }
    }

    foreach (const QByteArray &setKey, setItemsHash.uniqueKeys())
    {
        QList<int> setItemIds = setItemsHash.values(setKey);
        if (quint8 partialPropsNumber = (setItemIds.size() - 1) * 2)
        {
            foreach (int setId, setItemIds)
                PropertiesDisplayManager::addTemporaryPropertiesAndDelete(&allProps, PropertiesDisplayManager::collectSetFixedProps(ItemDataBase::Sets()->value(setId)->fixedProperties, partialPropsNumber), &kIgnoreProps);

            const FullSetInfo fullSetInfo = ItemDataBase::fullSetInfoForKey(setKey);
            PropertiesDisplayManager::addTemporaryPropertiesAndDelete(&allProps, PropertiesDisplayManager::collectSetFixedProps(fullSetInfo.partialSetProperties, partialPropsNumber), &kIgnoreProps);

            if (setItemIds.size() == fullSetInfo.itemNames.size())
                PropertiesDisplayManager::addTemporaryPropertiesAndDelete(&allProps, PropertiesDisplayManager::collectSetFixedProps(fullSetInfo.fullSetProperties), &kIgnoreProps);
        }
    }

    _propsText = QString("<html><body bgcolor=\"black\" align=\"center\">%1</body></html>").arg(PropertiesDisplayManager::propertiesToHtml(allProps));
    qDeleteAll(allProps);
    ui->textEdit->setHtml(_propsText);
}

AllStatsDialog::~AllStatsDialog()
{
    delete ui;
}

void AllStatsDialog::reject()
{
    QSettings().setValue("allStatsDialogGeometry", saveGeometry());
    QDialog::reject();
}

void AllStatsDialog::searchTextChanged(const QString &text)
{
    ui->lineEdit->setStyleSheet("QLineEdit { background-color: white }");

    if (text.isEmpty())
        ui->textEdit->setHtml(_propsText);
    else
    {
        if (!_searchTimer)
        {
            _searchTimer = new QTimer;
            _searchTimer->setSingleShot(true);
            connect(_searchTimer, SIGNAL(timeout()), SLOT(highlightSearchText()));
        }
        _searchTimer->start(qApp->keyboardInputInterval());
    }
}

void AllStatsDialog::highlightSearchText()
{
    ui->textEdit->setHtml(_propsText);

    QTextDocument *doc = ui->textEdit->document();
    QTextCursor cursor;
    QString searchText = ui->lineEdit->text();
    bool wasFound = false;
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(Qt::green);

    while (!(cursor = doc->find(searchText, cursor)).isNull())
    {
        cursor.setCharFormat(highlightFormat);
        if (!wasFound)
        {
            // scroll to first match
            cursor.clearSelection();
            ui->textEdit->setTextCursor(cursor);
            wasFound = true;
        }
    }

    if (!wasFound)
        ui->lineEdit->setStyleSheet("QLineEdit { background-color: red }");

    delete _searchTimer; _searchTimer = 0;
}
