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

    QVBoxLayout *vbl = new QVBoxLayout(ui->initialSortGroupBox);
    vbl->addWidget(ui->qualityRadioButton);
    vbl->addWidget(ui->typeRadioButton);

    vbl = new QVBoxLayout(ui->itemQualityOrderingGroupBox);
    vbl->addWidget(ui->descQualityRadioButton);
    vbl->addWidget(ui->ascQualityRadioButton);

    QFormLayout *fl = new QFormLayout(ui->pageRangeGroupBox);
    fl->addRow(ui->firstLabel, ui->firstPageSpinBox);
    fl->addRow(ui->lastLabel, ui->lastPageSpinBox);

    QHBoxLayout *hbl = new QHBoxLayout;
    hbl->addWidget(ui->blankPagesLabel);
    hbl->addWidget(ui->blankPagesSpinBox);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(ui->initialSortGroupBox, 0, 0);
    grid->addWidget(ui->itemQualityOrderingGroupBox, 0, 1);
    grid->addLayout(hbl, 1, 0, 1, 2);
    grid->addWidget(ui->pageRangeGroupBox, 0, 2, 2, 1);

    hbl = new QHBoxLayout;
    hbl->addWidget(ui->separateEthCheckBox);
    hbl->addWidget(ui->separateCotwCheckBox);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(grid);
    mainLayout->addLayout(hbl);
    mainLayout->addWidget(ui->buttonBox);

    adjustSize();
}

StashSortingOptionsDialog::~StashSortingOptionsDialog()
{
    delete ui;
}
