#ifndef SKILLTREEDIALOG_H
#define SKILLTREEDIALOG_H

#include <QDialog>

#include "structs.h"


class QTabWidget;

class SkillTreeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SkillTreeDialog(QWidget *parent = 0);
    virtual ~SkillTreeDialog() {}

public slots:
    virtual void reject();

private:
    QTabWidget *_tabWidget;

    qint32 getValueOfPropertyInSetProperties(const QList<SetFixedProperty> &setProps, quint16 propKey, quint8 propsNumber = 0, quint16 param = 0);
};

#endif // SKILLTREEDIALOG_H
