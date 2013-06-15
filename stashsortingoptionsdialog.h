#ifndef STASHSORTINGOPTIONSDIALOG_H
#define STASHSORTINGOPTIONSDIALOG_H

#include <QDialog>


namespace Ui { class StashSortingOptionsDialog; }

class StashSortingOptionsDialog : public QDialog
{
public:
    StashSortingOptionsDialog(QWidget *parent = 0);
    virtual ~StashSortingOptionsDialog();

private:
    Ui::StashSortingOptionsDialog *ui;
};

#endif // STASHSORTINGOPTIONSDIALOG_H
