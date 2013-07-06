#ifndef STASHSORTINGOPTIONSDIALOG_H
#define STASHSORTINGOPTIONSDIALOG_H

#include <QDialog>

#include "structs.h"


namespace Ui { class StashSortingOptionsDialog; }

class StashSortingOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    StashSortingOptionsDialog(quint32 lastPage, QWidget *parent = 0);
    virtual ~StashSortingOptionsDialog();

    StashSortOptions sortOptions() const;

public slots:
    virtual void accept();

private slots:
    void on_eachTypeFromNewPageCheckBox_toggled(bool isChecked);
    void on_similarMiscItemsOnOnePageCheckBox_toggled(bool isChecked);

    void firstPageChanged(double newPage);
    void lastPageChanged(double newPage);

    void showHelp();

private:
    Ui::StashSortingOptionsDialog *ui;

    void createLayout();

    void loadSettings();
    void saveSettings();
};

#endif // STASHSORTINGOPTIONSDIALOG_H
