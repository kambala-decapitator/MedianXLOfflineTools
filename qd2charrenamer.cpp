#include "qd2charrenamer.h"
#include "colorsmanager.hpp"
#include "helpers.h"

#include <QMenu>


// statics

const int QD2CharRenamer::kMaxNameLength = 15;

void QD2CharRenamer::updateNamePreview(QTextEdit *previewTextEdit, const QString &name)
{
    QString htmlName = QString("<html><body bgcolor=\"black\"><font color = \"#ffffff\">%1</font></body></html>").arg(name); // white by default
    for (int i = 0; i < ColorsManager::correctColorsNum(); i++) // replace color codes with their hex values for HTML
        htmlName.replace(QString("%1%2").arg(ColorsManager::unicodeColorHeader()).arg(ColorsManager::colorCodes().at(i)), QString("</font><font color = \"%1\">").arg(ColorsManager::colors().at(i).name()));
    previewTextEdit->setHtml(htmlName);
    previewTextEdit->setStatusTip(name);
}


// ctor

QD2CharRenamer::QD2CharRenamer(const QString &originalName, bool shouldWarn, QWidget *parent) : QDialog(parent), _originalCharName(originalName), _shouldWarn(shouldWarn),
    colorNames(QStringList() << tr("white") << tr("red") << tr("green") << tr("blue") << tr("gold") << tr("dark gray") << tr("tan") << tr("orange") << tr("yellow") << "foo" << tr("violet"))
{
    ui.setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(size());


    createColorMenu();
    ui.charNameLineEdit->setMaxLength(kMaxNameLength);

    connect(ui.charNameLineEdit, SIGNAL(textChanged(const QString &)), SLOT(nameChanged(const QString &)));
    connect(ui.buttonBox, SIGNAL(accepted()), SLOT(saveName()));

    ui.charNameLineEdit->setText(_originalCharName);
}


// slots

void QD2CharRenamer::saveName()
{
    QString badSymbols = "?*<>.|:\"/\\";
    QList<ushort> goodUpperHalfCodes = QList<ushort>() << 145 << 146;
    for (ushort i = 160; i < 192; ++i)
        goodUpperHalfCodes << i;

    QString newName = ui.charNameLineEdit->text();
    bool isBadName = newName.startsWith('_') || newName.endsWith('-');
    if (!isBadName)
    {
        QString nameToCheck = newName;
        nameToCheck.remove(ColorsManager::unicodeColorHeader());
        for (int i = 0; i < nameToCheck.length(); ++i)
        {
            QChar c = nameToCheck.at(i);
            ushort code = c.unicode();
            if (!((code >= 0x20 && code <= 0x7F && !badSymbols.contains(c)) || goodUpperHalfCodes.contains(code)))
            {
                isBadName = true;
                break;
            }
        }
    }
    if (isBadName)
        ERROR_BOX(ui.charNameLineEdit->toolTip());
    else
    {
        bool hasColor = false;
        for (int i = 0; i < ColorsManager::colorCodes().size(); ++i)
        {
            if (newName.contains(ColorsManager::unicodeColorHeader() + ColorsManager::colorCodes().at(i)))
            {
                hasColor = true;
                break;
            }
        }
        if (hasColor && _shouldWarn && QUESTION_BOX_YESNO(tr("Character with colored name can't join multiplayer games. Are you sure you want to continue?"), QMessageBox::Yes) == QMessageBox::No)
            return;

        _originalCharName = newName;
        accept();
    }
}

void QD2CharRenamer::nameChanged(const QString &newName)
{
    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(newName != _originalCharName && newName.length() > 1);
    updateNamePreview(ui.charNamePreview, ui.charNameLineEdit->text());
    setWindowTitle(tr("Rename (%1/15)", "param is the number of characters in the name").arg(ui.charNameLineEdit->text().length()));
}

void QD2CharRenamer::insertColor()
{
    if (ui.charNameLineEdit->text().length() + 3 <= kMaxNameLength)
    {
        QAction *menuItem = qobject_cast<QAction *>(sender());
        QString codeToInsert = menuItem->text().left(3);
        // don't insert white in the beginning because it's a waste of characters
        if (!(ui.charNameLineEdit->cursorPosition() == 0 && codeToInsert == ColorsManager::unicodeColorHeader() + ColorsManager::colorCodes().at(0)))
            ui.charNameLineEdit->insert(codeToInsert);
        else
            qApp->beep();
        ui.charNameLineEdit->setFocus();
    }
    else
        qApp->beep();
}


// private methods

void QD2CharRenamer::createColorMenu()
{
    QMenu *colorMenu = new QMenu(ui.colorButton);
    QPixmap pix(24, 24);
    for (int i = 0; i < ColorsManager::correctColorsNum(); ++i)
    {
        if (i == ColorsManager::DarkGreen) // ':' can't be used for filename
            continue;

        pix.fill(ColorsManager::colors().at(i));
        QAction *colorAction = new QAction(QIcon(pix), QString("%1 (%2)").arg(ColorsManager::unicodeColorHeader() + ColorsManager::colorCodes().at(i), colorNames.at(i)), this);
        colorAction->setIconVisibleInMenu(true); // explicitly show color icon on Mac OS X
        connect(colorAction, SIGNAL(triggered()), SLOT(insertColor()));

        colorMenu->addAction(colorAction);
    }

    QAction *infoAction = colorMenu->addAction(tr("\"Dynamic\" colors below"));
    infoAction->setEnabled(false);

    for (int i = ColorsManager::correctColorsNum(); i < ColorsManager::colorCodes().size(); ++i)
        colorMenu->addAction(ColorsManager::unicodeColorHeader() + ColorsManager::colorCodes().at(i), this, SLOT(insertColor()));
    ui.colorButton->setMenu(colorMenu);
}
