#include "qd2charrenamer.h"
#include "colors.hpp"
#include "helpers.h"

#include <QMenu>


// statics

const int QD2CharRenamer::maxNameLength = 15;

void QD2CharRenamer::updateNamePreview(QTextEdit *previewTextEdit, const QString &name)
{
    previewTextEdit->setHtml(QString("<html><body bgcolor=\"black\">%1</body></html>").arg(htmlStringFromDiabloColorString(name)));
    previewTextEdit->setStatusTip(name);
}


// ctor

QD2CharRenamer::QD2CharRenamer(QWidget *parent, const QString &originalName) : QDialog(parent), _originalCharName(originalName)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::WindowModal);
    setFixedSize(size());

    createColorMenu();
    ui.charNameLineEdit->setMaxLength(maxNameLength);

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
        nameToCheck.remove(unicodeColorHeader);
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
        _originalCharName = newName;
        accept();
    }
}

void QD2CharRenamer::nameChanged(const QString &newName)
{
    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(newName != _originalCharName && newName.length() > 1);
    updateNamePreview(ui.charNamePreview, ui.charNameLineEdit->text());
}

void QD2CharRenamer::insertColor()
{
    if (ui.charNameLineEdit->text().length() + 3 <= maxNameLength)
    {
        QAction *menuItem = qobject_cast<QAction *>(sender());
        QString codeToInsert = menuItem->text().left(3);
        // don't insert white in the beginning because it's a waste of characters
        if (!(ui.charNameLineEdit->cursorPosition() == 0 && codeToInsert == unicodeColorHeader + colorCodes.at(0)))
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
    QStringList colorNames = QStringList() // had to move it here because strings aren't translated otherwise
                               << tr("white")
                               << tr("red")
                               << tr("green")
                               << tr("blue")
                               << tr("gold")
                               << tr("dark gray")
                                  // black was here
                               << tr("tan")
                               << tr("orange")
                               << tr("yellow")
                               << "foo" // to skip dark green
                               << tr("violet")
                                  ;

    QMenu *colorMenu = new QMenu(ui.colorButton);
    QPixmap pix(24, 24);
    for (int i = 0; i < correctColorsNum; ++i)
    {
        if (i == DarkGreen) // ':' can't be used for filename
            continue;
        pix.fill(colors.at(i));
        colorMenu->addAction(QIcon(pix), QString("%1 (%2)").arg(unicodeColorHeader + colorCodes.at(i), colorNames.at(i)), this, SLOT(insertColor()));
    }

    QAction *infoAction = colorMenu->addAction(tr("\"Dynamic\" colors below"));
    infoAction->setEnabled(false);

    for (int i = correctColorsNum; i < colorCodes.size(); ++i)
        colorMenu->addAction(unicodeColorHeader + colorCodes.at(i), this, SLOT(insertColor()));
    ui.colorButton->setMenu(colorMenu);
}
