#ifndef ITEMNAMESTREEWIDGET_H
#define ITEMNAMESTREEWIDGET_H

#include <QTreeWidget>

class ItemNamesTreeWidget : public QTreeWidget
{
public:
    ItemNamesTreeWidget(QWidget *parent = 0) : QTreeWidget(parent)
    {
        setStyleSheet("QTreeWidget                { background-color: black; }"
                      "QTreeWidget::item          { selection-color: red; }"
                      "QTreeWidget::item:hover    { border: 1px solid #bfcde4; }"
                      "QTreeWidget::item:selected { border: 1px solid #567dbc; }"
                     );

        setColumnCount(2);
        setHeaderLabels(QStringList() << tr("Base name") << tr("Special name"));
        setEditTriggers(QAbstractItemView::NoEditTriggers);
        setDropIndicatorShown(false);
    }

    virtual ~ItemNamesTreeWidget() {}
};

#endif // ITEMNAMESTREEWIDGET_H
