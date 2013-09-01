#include    "stashsortingoptionsdialog.h"
#include "ui_stashsortingoptionsdialog.h"
#include "helpers.h"
#include "helpwindowdisplaymanager.h"
#include "resourcepathmanager.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>

#include <QSettings>
#include <QUrl>


StashSortingOptionsDialog::StashSortingOptionsDialog(quint32 lastPage, QWidget *parent /*= 0*/) : QDialog(parent), ui(new Ui::StashSortingOptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    ui->buttonGroup->setId(ui->descQualityRadioButton, 0);
    ui->buttonGroup->setId(ui->ascQualityRadioButton,  1);

    createLayout();
    loadSettings();

    ui->firstPageSpinBox->setMaximum(lastPage);
    ui->lastPageSpinBox->setMaximum(lastPage);
    ui->lastPageSpinBox->setValue(lastPage);

    QStringList qualitiesList = QStringList() << tr("Misc") << tr("Quest") << tr("Low Quality") << tr("Normal") << tr("Superior") << tr("RW") << tr("Magic") << tr("Honorific") << tr("Rare") << tr("Crafted") << tr("Unique") << tr("Set");
    ui->ascQualityRadioButton->setToolTip(qualitiesList.join(",\n"));

    std::reverse(qualitiesList.begin(), qualitiesList.end());
    ui->descQualityRadioButton->setToolTip(qualitiesList.join(",\n"));

    ui->buttonBox->addButton(tr("Sort"), QDialogButtonBox::AcceptRole);
    ui->buttonBox->button(QDialogButtonBox::Help)->setShortcut(QKeySequence::HelpContents);

    connect(ui->firstPageSpinBox, SIGNAL(valueChanged(double)), SLOT(firstPageChanged(double)));
    connect(ui->lastPageSpinBox,  SIGNAL(valueChanged(double)), SLOT(lastPageChanged(double)));

    QString gearFileName("<b>gear.txt</b>"), miscFileName("<b>misc.txt</b>"), setsFileName("<b>sets.txt</b>");
    HelpWindowDisplayManager *helpDislplayManager = new HelpWindowDisplayManager(tr("Sort help"),
        // last param is %17
        tr(
        "<h2>Item sort order</h2>"
        "<p>The order is mostly softcoded and can be modified with your favorite text editor. Go to %1 folder and edit any of the 3 files except '%17' (it's just a reference):"
        "<ul><li>%14: non-set weapons, armor, arrows and jewelry;</li><li>%15: everything else;</li><li>%16: sets, obviously.</li></ul>"
        "You can change order of the item types, remove item types or even add new ones. All other items not listed in the above files are stored in the very end of the selected page range.</p>"
        "<h2>Options</h2>"
        // item quality ordering
        "<h3><i>%2</i></h3>"
        "<p>Set the order of item qualities. Hover mouse pointer over an option to see the order.</p>"
        // page range
        "<h3><i>%3</i></h3>"
        "<p>Sort items only on pages within range. If sorted items occupy more pages than the range, all subsequent pages are shifted.</p>"
        // blank pages
        "<h3><i>%4</i></h3>"
        "<p>Specify amount of blank pages to insert.</p>"
        // separation
        "<h3><i>%5</i></h3>"
        "<ul>"
        "<li><i>%7:</i> does exactly what the option says :) 'Item type' is a line in config file. Doesn't apply to %16.</li>"
          "<li><i>%8:</i> 'misc' types are defined in %15. 'Similar' are items of one type.</li>"
          "<li><i>%9:</i> if checked, all sacred items of each type are stored first, then all tiered items (within one item quality); otherwise, tiered and sacred items are placed together.</li>"
          "<li><i>%10:</i> if checked, ethereal items are placed in the end of each tier; otherwise, normal and ethereal items are mixed.</li>"
        "</ul>"
        // new row
        "<h3><i>%6</i></h3>"
        "<ul>"
          "<li><i>%11:</i> applies only to item types in %14.</li>"
          "<li><i>%12:</i> 'Cornerstone of the World' is a unique jewel that grants any non-passive character skill as oskill.</li>"
          "<li><i>%13:</i> e.g., Shrines in Ultimative, Charms, Gems, Runes, etc.</li>"
        "</ul>",
        "first param is link to sort folder, 14-17 are file names, others are titles of the UI components").arg(QString("<a href=\"%1\">resources/data/sorting</a>").arg(QUrl::fromLocalFile(ResourcePathManager::dataPathForFileName("sorting/")).toString()))
        .arg(ui->itemQualityOrderingGroupBox->title(), ui->pageRangeGroupBox->title(), ui->blankPagesGroupBox->title().remove(':'), ui->separationBox->title(), ui->newRowGroupBox->title().remove(':'))
        .arg(ui->eachTypeFromNewPageCheckBox->text(), ui->similarMiscItemsOnOnePageCheckBox->text(), ui->separateSacredCheckBox->text(), ui->separateEthCheckBox->text())
        .arg(ui->newRowTierCheckBox->text(), ui->newRowCotwCheckBox->text(), ui->newRowVisuallyDifferentMiscCheckBox->text())
        .arg(gearFileName, miscFileName, setsFileName, "thng_list.txt"), this);
    connect(ui->buttonBox, SIGNAL(helpRequested()), helpDislplayManager, SLOT(showHelp()));
}

StashSortingOptionsDialog::~StashSortingOptionsDialog()
{
    delete ui;
}


StashSortOptions StashSortingOptionsDialog::sortOptions() const
{
    StashSortOptions options;
    options.isQualityOrderAscending = ui->ascQualityRadioButton->isChecked();
    options.isEachTypeFromNewPage = ui->eachTypeFromNewPageCheckBox->isChecked();
    options.shouldPlaceSimilarMiscItemsOnOnePage = ui->similarMiscItemsOnOnePageCheckBox->isChecked();
    options.shouldSeparateSacred = ui->separateSacredCheckBox->isChecked();
    options.shouldSeparateEth = ui->separateEthCheckBox->isChecked();
    options.isNewRowTier = ui->newRowTierCheckBox->isChecked();
    options.isNewRowCotw = ui->newRowCotwCheckBox->isChecked();
    options.isNewRowVisuallyDifferentMisc = ui->newRowVisuallyDifferentMiscCheckBox->isChecked();
    options.firstPage = ui->firstPageSpinBox->value();
    options.lastPage = ui->lastPageSpinBox->value();
    options.diffQualitiesBlankPages = ui->diffQualitiesSpinBox->value();
    options.diffTypesBlankPages = ui->diffTypesSpinBox->value();
    return options;
}


void StashSortingOptionsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}

void StashSortingOptionsDialog::on_eachTypeFromNewPageCheckBox_toggled(bool isChecked)
{
    ui->similarMiscItemsOnOnePageCheckBox->setEnabled(isChecked);
    ui->newRowTierCheckBox->setEnabled(isChecked);

    ui->newRowTierCheckBox->setChecked(isChecked);
    if (!isChecked)
        ui->similarMiscItemsOnOnePageCheckBox->setChecked(true);
}

void StashSortingOptionsDialog::on_similarMiscItemsOnOnePageCheckBox_toggled(bool isChecked)
{
    ui->newRowVisuallyDifferentMiscCheckBox->setEnabled(isChecked);
    if (!isChecked)
        ui->newRowVisuallyDifferentMiscCheckBox->setChecked(false);
}

void StashSortingOptionsDialog::firstPageChanged(double newPage)
{
    ui->lastPageSpinBox->setMinimum(newPage);
}

void StashSortingOptionsDialog::lastPageChanged(double newPage)
{
    ui->firstPageSpinBox->setMaximum(newPage);
}


void StashSortingOptionsDialog::createLayout()
{
    QVBoxLayout *vbl = new QVBoxLayout(ui->itemQualityOrderingGroupBox);
    vbl->addWidget(ui->descQualityRadioButton);
    vbl->addWidget(ui->ascQualityRadioButton);

    QFormLayout *fl = new QFormLayout(ui->pageRangeGroupBox);
    fl->addRow(ui->firstPageLabel, ui->firstPageSpinBox);
    fl->addRow(ui->lastPageLabel, ui->lastPageSpinBox);

    fl = new QFormLayout(ui->blankPagesGroupBox);
    fl->addRow(ui->diffQualitiesLabel, ui->diffQualitiesSpinBox);
    fl->addRow(ui->diffTypesLabel, ui->diffTypesSpinBox);

    vbl = new QVBoxLayout(ui->separationBox);
    vbl->addWidget(ui->eachTypeFromNewPageCheckBox);
    vbl->addWidget(ui->similarMiscItemsOnOnePageCheckBox);
    vbl->addWidget(ui->separateSacredCheckBox);
    vbl->addWidget(ui->separateEthCheckBox);

    vbl = new QVBoxLayout(ui->newRowGroupBox);
    vbl->addWidget(ui->newRowTierCheckBox);
    vbl->addWidget(ui->newRowCotwCheckBox);
    vbl->addWidget(ui->newRowVisuallyDifferentMiscCheckBox);

    vbl = new QVBoxLayout;
    vbl->addWidget(ui->newRowGroupBox);
    vbl->addWidget(ui->buttonBox);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    QHBoxLayout *hbl = new QHBoxLayout;
    hbl->addWidget(ui->itemQualityOrderingGroupBox);
    hbl->addStretch();
    hbl->addWidget(ui->pageRangeGroupBox);
    hbl->addStretch();
    hbl->addWidget(ui->blankPagesGroupBox);
    mainLayout->addLayout(hbl);

    hbl = new QHBoxLayout;
    hbl->addWidget(ui->separationBox);
    hbl->addLayout(vbl);
    mainLayout->addLayout(hbl);
}

void StashSortingOptionsDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup("sortDialog");
    ui->buttonGroup->button(settings.value("qualityOrdering", 0).toInt())->setChecked(true);

    settings.beginGroup("newRow");
    ui->newRowTierCheckBox->setChecked(settings.value("tier", true).toBool());
    ui->newRowCotwCheckBox->setChecked(settings.value("cotw", false).toBool());
    ui->newRowVisuallyDifferentMiscCheckBox->setChecked(settings.value("visuallyDifferentMisc", false).toBool());
    settings.endGroup();

    settings.beginGroup("blankPages");
    ui->diffQualitiesSpinBox->setValue(settings.value("qualities", 0).toInt());
    ui->diffTypesSpinBox->setValue(settings.value("types", 0).toInt());
    settings.endGroup();

    settings.beginGroup("separation");
    ui->eachTypeFromNewPageCheckBox->setChecked(settings.value("eachTypeFromNewPage", true).toBool());
    ui->similarMiscItemsOnOnePageCheckBox->setChecked(settings.value("similarMiscItemsOnOnePage", true).toBool());
    ui->separateSacredCheckBox->setChecked(settings.value("sacred", true).toBool());
    ui->separateEthCheckBox->setChecked(settings.value("eth", true).toBool());
    settings.endGroup();

    settings.endGroup();
}

void StashSortingOptionsDialog::saveSettings()
{
    QSettings settings;
    settings.beginGroup("sortDialog");
    settings.setValue("qualityOrdering", ui->buttonGroup->checkedId());

    settings.beginGroup("newRow");
    settings.setValue("tier", ui->newRowTierCheckBox->isChecked());
    settings.setValue("cotw", ui->newRowCotwCheckBox->isChecked());
    settings.setValue("visuallyDifferentMisc", ui->newRowVisuallyDifferentMiscCheckBox->isChecked());
    settings.endGroup();

    settings.beginGroup("blankPages");
    settings.setValue("qualities", ui->diffQualitiesSpinBox->value());
    settings.setValue("types", ui->diffTypesSpinBox->value());
    settings.endGroup();

    settings.beginGroup("separation");
    settings.setValue("eachTypeFromNewPage", ui->eachTypeFromNewPageCheckBox->isChecked());
    settings.setValue("similarMiscItemsOnOnePage", ui->similarMiscItemsOnOnePageCheckBox->isChecked());
    settings.setValue("sacred", ui->separateSacredCheckBox->isChecked());
    settings.setValue("eth", ui->separateEthCheckBox->isChecked());
    settings.endGroup();

    settings.endGroup();
}
