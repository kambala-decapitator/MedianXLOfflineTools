#ifndef ALLSTATSDIALOG_H
#define ALLSTATSDIALOG_H

#include <QDialog>


namespace Ui { class AllStatsDialog; };

class AllStatsDialog : public QDialog
{
    Q_OBJECT

public:
    AllStatsDialog(QWidget *parent = 0);
    virtual ~AllStatsDialog();

public slots:
    virtual void reject();

private slots:
    void searchTextChanged(const QString &text);
    void highlightSearchText();

private:
    Ui::AllStatsDialog *ui;
    QString _propsText;
    QTimer *_searchTimer;
};

#endif // ALLSTATSDIALOG_H
