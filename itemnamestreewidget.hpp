#ifndef ITEMNAMESTREEWIDGET_HPP
#define ITEMNAMESTREEWIDGET_HPP

#include <QTreeWidget>

#include "helpers.h"


class ItemNamesTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    ItemNamesTreeWidget(QWidget *parent = 0) : QTreeWidget(parent)
    {
        setColumnCount(2);
        setHeaderLabels(headerLabels());
        customizeItemsTreeView(this);
    }

    virtual ~ItemNamesTreeWidget() {}

    static const QStringList &headerLabels()
    {
        static const QStringList labels = QStringList() << tr("Base name") << tr("Special name");
        return labels;
    }
};

#endif // ITEMNAMESTREEWIDGET_HPP
