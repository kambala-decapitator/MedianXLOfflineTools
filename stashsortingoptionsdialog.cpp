#include    "stashsortingoptionsdialog.h"
#include "ui_stashsortingoptionsdialog.h"
#include "helpers.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>

#include <QSettings>


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

    QStringList qualitiesList = QStringList() << tr("Misc items") << tr("Low Quality") << tr("Normal") << tr("Superior") << tr("RW") << tr("Magic") << tr("Honorific") << tr("Rare") << tr("Crafted") << tr("Unique") << tr("Set");
    ui->ascQualityRadioButton->setToolTip(qualitiesList.join(",\n"));

    std::reverse(qualitiesList.begin(), qualitiesList.end());
    ui->descQualityRadioButton->setToolTip(qualitiesList.join(",\n"));

    ui->buttonBox->addButton(tr("Sort"), QDialogButtonBox::AcceptRole);
    ui->buttonBox->button(QDialogButtonBox::Help)->setShortcut(QKeySequence::HelpContents);

    connect(ui->firstPageSpinBox, SIGNAL(valueChanged(double)), SLOT(firstPageChanged(double)));
    connect(ui->lastPageSpinBox,  SIGNAL(valueChanged(double)), SLOT(lastPageChanged(double)));
    connect(ui->buttonBox, SIGNAL(helpRequested()), SLOT(showHelp()));
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

void StashSortingOptionsDialog::showHelp()
{
    INFO_BOX(tr("help"));
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

    adjustSize();
    setFixedSize(size());
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
