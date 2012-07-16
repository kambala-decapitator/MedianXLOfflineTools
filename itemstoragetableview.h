#ifndef ITEMSTORAGETABLEVIEW_H
#define ITEMSTORAGETABLEVIEW_H

#include <QTableView>


struct ItemInfo;
class ItemStorageTableModel;

class QTimer;

class ItemStorageTableView : public QTableView
{
    Q_OBJECT

public:
    explicit ItemStorageTableView(QWidget *parent = 0);
    virtual ~ItemStorageTableView() {}

    ItemStorageTableModel *model() const;

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
    ItemInfo *_draggedItem;

    QModelIndex originIndexInRectOfIndex(const QModelIndex &index) const { return indexAt(visualRect(index).topLeft()); }
    QModelIndex actualIndexAt(const QPoint &p) const;
    QModelIndex indexForDragDropEvent(QDropEvent *event) const;

    void updateHighlightIndexesForOriginIndex(const QModelIndex &originIndex) const;

    void dragStopped();
};

#endif // ITEMSTORAGETABLEVIEW_H
