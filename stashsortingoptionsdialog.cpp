#include    "stashsortingoptionsdialog.h"
#include "ui_stashsortingoptionsdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>


StashSortingOptionsDialog::StashSortingOptionsDialog(QWidget *parent /*= 0*/) : QDialog(parent), ui(new Ui::StashSortingOptionsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    createLayout();
    loadSettings();

    QStringList qualitiesList = QStringList() << tr("Low Quality") << tr("Normal") << tr("Superior") << tr("RW") << tr("Magic") << tr("Honorific") << tr("Rare") << tr("Crafted") << tr("Unique") << tr("Set");
    ui->ascQualityRadioButton->setToolTip(qualitiesList.join("\n"));

    std::reverse(qualitiesList.begin(), qualitiesList.end());
    ui->descQualityRadioButton->setToolTip(qualitiesList.join("\n"));
}

void StashSortingOptionsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}


StashSortingOptionsDialog::~StashSortingOptionsDialog()
{
    delete ui;
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

    //QGridLayout *mainLayout = new QGridLayout(this);
    //mainLayout->addWidget(ui->primarySortKeyGroupBox, 0, 0);
    //mainLayout->addWidget(ui->blankPagesGroupBox, 0, 1);
    //mainLayout->addWidget(ui->pageRangeGroupBox, 0, 2);
    //mainLayout->addWidget(ui->itemQualityOrderingGroupBox, 1, 0);
    //mainLayout->addWidget(ui->additionalNewPageGroupBox, 1, 1);
    //mainLayout->addWidget(ui->buttonBox, 1, 2, Qt::AlignBottom | Qt::AlignRight);

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
    ui->buttonBox->setOrientation(Qt::Horizontal);

    adjustSize();
}

void StashSortingOptionsDialog::loadSettings()
{

}

void StashSortingOptionsDialog::saveSettings()
{

}
