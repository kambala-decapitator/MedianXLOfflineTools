#ifndef ITEMSTORAGETABLEVIEW_H
#define ITEMSTORAGETABLEVIEW_H

#include <QTableView>


struct ItemInfo;

class QTimer;

class ItemStorageTableView : public QTableView
{
    Q_OBJECT

public:
    explicit ItemStorageTableView(QWidget *parent = 0);
    virtual ~ItemStorageTableView() {}

    void setCellSpanForItem(ItemInfo *item);

protected:
    virtual void keyPressEvent(QKeyEvent *event);

    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
//    void itemClicked(const QModelIndex &index);
    void checkIfStillDragging();

private:
    QTimer *_dragLeaveTimer;

    QModelIndex originIndexInRectOfIndex(const QModelIndex &index) { return indexAt(visualRect(index).topLeft()); }
    QModelIndex actualIndexAt(const QPoint &p) { return model()->index(rowAt(p.y()), columnAt(p.x())); }
    QModelIndex indexForDragDropEvent(QDropEvent *event);

    void dragStopped();
};

#endif // ITEMSTORAGETABLEVIEW_H
