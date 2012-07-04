#ifndef ITEMSTORAGETABLEVIEW_H
#define ITEMSTORAGETABLEVIEW_H

#include <QTableView>


class ItemStorageTableView : public QTableView
{
    Q_OBJECT

public:
    explicit ItemStorageTableView(QWidget *parent = 0);
    virtual ~ItemStorageTableView() {}

protected:
    virtual void keyPressEvent(QKeyEvent *event);

    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
//    void itemClicked(const QModelIndex &index);

private:
    QModelIndex originIndexInRectOfIndex(const QModelIndex &index) { return indexAt(visualRect(index).topLeft()); }
    QModelIndex actualIndexAt(const QPoint &p) { return model()->index(rowAt(p.y()), columnAt(p.x())); }
};

#endif // ITEMSTORAGETABLEVIEW_H
