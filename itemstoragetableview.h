#ifndef ITEMSTORAGETABLEVIEW_H
#define ITEMSTORAGETABLEVIEW_H

#include <QTableView>


class ItemStorageTableView : public QTableView
{
    Q_OBJECT

public:
    explicit ItemStorageTableView(QWidget *parent = 0) : QTableView(parent) {}

protected:
    virtual void keyPressEvent(QKeyEvent *event);
};

#endif // ITEMSTORAGETABLEVIEW_H
