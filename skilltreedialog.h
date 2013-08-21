#ifndef SKILLTREEDIALOG_H
#define SKILLTREEDIALOG_H

#include <QDialog>

#include "structs.h"


class QTabWidget;

class SkillTreeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SkillTreeDialog(const QList<int> &skillsVisualOrder, QWidget *parent = 0);
    virtual ~SkillTreeDialog() {}

private:
    QTabWidget *_tabWidget;
};

#endif // SKILLTREEDIALOG_H
