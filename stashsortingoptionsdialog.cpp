#include    "stashsortingoptionsdialog.h"
#include "ui_stashsortingoptionsdialog.h"
#include "helpers.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>


StashSortingOptionsDialog::StashSortingOptionsDialog(quint32 lastPage, QWidget *parent /*= 0*/) : QDialog(parent), ui(new Ui::StashSortingOptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    createLayout();
    loadSettings();

    ui->firstPageSpinBox->setMaximum(lastPage);
    ui->lastPageSpinBox->setMaximum(lastPage);

    QStringList qualitiesList = QStringList() << tr("Low Quality") << tr("Normal") << tr("Superior") << tr("RW") << tr("Magic") << tr("Honorific") << tr("Rare") << tr("Crafted") << tr("Unique") << tr("Set");
    ui->ascQualityRadioButton->setToolTip(qualitiesList.join("\n"));

    std::reverse(qualitiesList.begin(), qualitiesList.end());
    ui->descQualityRadioButton->setToolTip(qualitiesList.join("\n"));

    ui->buttonBox->addButton(tr("Sort"), QDialogButtonBox::AcceptRole);
    ui->buttonBox->button(QDialogButtonBox::Help)->setShortcut(QKeySequence::HelpContents);

    connect(ui->buttonBox, SIGNAL(helpRequested()), SLOT(showHelp()));
}

StashSortingOptionsDialog::~StashSortingOptionsDialog()
{
    delete ui;
}


StashSortOptions StashSortingOptionsDialog::sortOptions() const
{
    StashSortOptions options;
    options.primarySortKey = ui->qualityRadioButton->isChecked() ? StashSortOptions::Quality : StashSortOptions::Type;
    options.qualityOrderAscending = ui->ascQualityRadioButton->isChecked();
    options.firstPage = ui->firstPageSpinBox->value();
    options.lastPage = ui->lastPageSpinBox->value();
    options.separateEtherealItems = ui->separateEthCheckBox->isChecked();
    options.separateCotw = ui->separateCotwCheckBox->isChecked();
    options.diffQualitiesBlankPages = ui->diffQualitiesSpinBox->value();
    options.diffTypesBlankPages = ui->diffTypesSpinBox->value();
    return options;
}


void StashSortingOptionsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}

void StashSortingOptionsDialog::showHelp()
{
    INFO_BOX(tr("help"));
}


void StashSortingOptionsDialog::createLayout()
{
    QVBoxLayout *vbl = new QVBoxLayout(ui->primarySortKeyGroupBox);
    vbl->addWidget(ui->qualityRadioButton);
    vbl->addWidget(ui->typeRadioButton);

    vbl = new QVBoxLayout(ui->itemQualityOrderingGroupBox);
    vbl->addWidget(ui->descQualityRadioButton);
    vbl->addWidget(ui->ascQualityRadioButton);

    vbl = new QVBoxLayout(ui->additionalNewPageGroupBox);
    vbl->addWidget(ui->separateEthCheckBox);
    vbl->addWidget(ui->separateCotwCheckBox);

    QFormLayout *fl = new QFormLayout(ui->blankPagesGroupBox);
    fl->addRow(ui->diffQualitiesLabel, ui->diffQualitiesSpinBox);
    fl->addRow(ui->diffTypesLabel, ui->diffTypesSpinBox);

    fl = new QFormLayout(ui->pageRangeGroupBox);
    fl->addRow(ui->firstPageLabel, ui->firstPageSpinBox);
    fl->addRow(ui->lastPageLabel, ui->lastPageSpinBox);

    QHBoxLayout *hbl = new QHBoxLayout;
    hbl->addWidget(ui->primarySortKeyGroupBox);
    hbl->addWidget(ui->itemQualityOrderingGroupBox);
    hbl->addWidget(ui->pageRangeGroupBox);

    QHBoxLayout *hbl1 = new QHBoxLayout;
    hbl1->addWidget(ui->additionalNewPageGroupBox);
    hbl1->addWidget(ui->blankPagesGroupBox);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(hbl);
    mainLayout->addLayout(hbl1);
    mainLayout->addWidget(ui->buttonBox);

    adjustSize();
}

void StashSortingOptionsDialog::loadSettings()
{

}

void StashSortingOptionsDialog::saveSettings()
{

}