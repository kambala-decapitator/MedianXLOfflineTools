#ifndef SKILLTREEDIALOG_H
#define SKILLTREEDIALOG_H

#include <QDialog>

#include "structs.h"


class QTabWidget;

class SkillTreeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SkillTreeDialog(const SkillsOrderPair &skillsOrderPair, QWidget *parent = 0);
    virtual ~SkillTreeDialog() {}
    
signals:
    
public slots:

private:
    QTabWidget *_tabWidget;
};

#endif // SKILLTREEDIALOG_H
