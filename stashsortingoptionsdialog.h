#ifndef STASHSORTINGOPTIONSDIALOG_H
#define STASHSORTINGOPTIONSDIALOG_H

#include <QDialog>


namespace Ui { class StashSortingOptionsDialog; }

class StashSortingOptionsDialog : public QDialog
{
public:
    StashSortingOptionsDialog(QWidget *parent = 0);
    virtual ~StashSortingOptionsDialog();

public slots:
    virtual void accept();

private:
    Ui::StashSortingOptionsDialog *ui;

    void createLayout();

    void loadSettings();
    void saveSettings();
};

#endif // STASHSORTINGOPTIONSDIALOG_H
