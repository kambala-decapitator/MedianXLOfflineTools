#include "experienceindicatorgroupbox.h"

#include <QHBoxLayout>


ExperienceIndicatorGroupBox::ExperienceIndicatorGroupBox(QWidget *parent) : QGroupBox(tr("Experience"), parent), _progressBar(new QProgressBar(this))
{
    _progressBar->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    _progressBar->setFormat("%v / %m (%p%)");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(_progressBar);
}
