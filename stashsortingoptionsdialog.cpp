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
    options.separateSacredItems = ui->separateSacredsCheckBox->isChecked();
    options.separateEtherealItems = ui->separateEthCheckBox->isChecked();
    options.separateCotw = ui->separateCotwCheckBox->isChecked();
    options.placeSimilarItemsOnOnePage = ui->similarItemsOnOnePageCheckBox->isChecked();
    options.newRowTier = ui->newRowTierCheckBox->isChecked();
    options.newRowCotw = ui->newRowCotwCheckBox->isChecked();
    options.newRowGemQuality = ui->newRowGemCheckBox->isChecked();
    options.newRowVisuallyDifferent = ui->newRowVisuallyDifferentCheckBox->isChecked();
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

    vbl = new QVBoxLayout(ui->additionalNewPageGroupBox);
    vbl->addWidget(ui->separateSacredsCheckBox);
    vbl->addWidget(ui->separateEthCheckBox);
    vbl->addWidget(ui->separateCotwCheckBox);
    vbl->addWidget(ui->lineNewPageBox);
    vbl->addWidget(ui->similarItemsOnOnePageCheckBox);

    vbl = new QVBoxLayout(ui->newRowGroupBox);
    vbl->addWidget(ui->newRowTierCheckBox);
    vbl->addWidget(ui->newRowCotwCheckBox);
    vbl->addWidget(ui->newRowGemCheckBox);
    vbl->addWidget(ui->lineNewRowBox);
    vbl->addWidget(ui->newRowVisuallyDifferentCheckBox);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *hbl = new QHBoxLayout;
    hbl->addWidget(ui->itemQualityOrderingGroupBox);
    hbl->addWidget(ui->pageRangeGroupBox);
    hbl->addWidget(ui->blankPagesGroupBox);
    mainLayout->addLayout(hbl);

    hbl = new QHBoxLayout;
    hbl->addWidget(ui->additionalNewPageGroupBox);
    hbl->addWidget(ui->newRowGroupBox);
    mainLayout->addLayout(hbl);

    hbl = new QHBoxLayout;
    hbl->addWidget(ui->eachTypeFromNewPageCheckBox);
    hbl->addWidget(ui->buttonBox);
    mainLayout->addLayout(hbl);

    adjustSize();
    setFixedSize(size());
}

void StashSortingOptionsDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup("sortDialog");
    ui->buttonGroup->button(settings.value("qualityOrdering", 0).toInt())->setChecked(true);
    ui->eachTypeFromNewPageCheckBox->setChecked(settings.value("eachTypeFromNewPage", true).toBool());

    settings.beginGroup("blankPages");
    ui->diffQualitiesSpinBox->setValue(settings.value("qualities", 0).toInt());
    ui->diffTypesSpinBox->setValue(settings.value("types", 0).toInt());
    settings.endGroup();

    settings.beginGroup("newPage");
    ui->separateSacredsCheckBox->setChecked(settings.value("sacred", true).toBool());
    ui->separateEthCheckBox->setChecked(settings.value("eth", false).toBool());
    ui->separateCotwCheckBox->setChecked(settings.value("cotw", true).toBool());
    ui->similarItemsOnOnePageCheckBox->setChecked(settings.value("similarItemsOnOnePage", true).toBool());
    settings.endGroup();

    settings.beginGroup("newRow");
    ui->newRowTierCheckBox->setChecked(settings.value("tier", true).toBool());
    ui->newRowCotwCheckBox->setChecked(settings.value("cotw", false).toBool());
    ui->newRowGemCheckBox->setChecked(settings.value("gem", false).toBool());
    ui->newRowVisuallyDifferentCheckBox->setChecked(settings.value("visuallyDifferent", false).toBool());
    settings.endGroup();

    settings.endGroup();
}

void StashSortingOptionsDialog::saveSettings()
{
    QSettings settings;
    settings.beginGroup("sortDialog");
    settings.setValue("qualityOrdering", ui->buttonGroup->checkedId());
    settings.setValue("eachTypeFromNewPage", ui->eachTypeFromNewPageCheckBox->isChecked());

    settings.beginGroup("blankPages");
    settings.setValue("qualities", ui->diffQualitiesSpinBox->value());
    settings.setValue("types", ui->diffTypesSpinBox->value());
    settings.endGroup();

    settings.beginGroup("newPage");
    settings.setValue("sacred", ui->separateSacredsCheckBox->isChecked());
    settings.setValue("eth", ui->separateEthCheckBox->isChecked());
    settings.setValue("cotw", ui->separateCotwCheckBox->isChecked());
    settings.setValue("similarItemsOnOnePage", ui->similarItemsOnOnePageCheckBox->isChecked());
    settings.endGroup();

    settings.beginGroup("newRow");
    settings.setValue("tier", ui->newRowTierCheckBox->isChecked());
    settings.setValue("cotw", ui->newRowCotwCheckBox->isChecked());
    settings.setValue("gem", ui->newRowGemCheckBox->isChecked());
    settings.setValue("visuallyDifferent", ui->newRowVisuallyDifferentCheckBox->isChecked());
    settings.endGroup();

    settings.endGroup();
}
