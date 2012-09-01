#ifndef PROGRESSBARMODAL_HPP
#define PROGRESSBARMODAL_HPP

#include <QProgressBar>
#include <QCloseEvent>


class ProgressBarModal : public QProgressBar
{
    Q_OBJECT

public:
    ProgressBarModal(QWidget *parent = 0) : QProgressBar(parent)
    {
        setRange(0, 0);
        setTextVisible(false);
        setWindowTitle(tr("Please wait..."));
        setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        setWindowModality(Qt::ApplicationModal);
        adjustSize();
        setFixedSize(size());
    }

    virtual ~ProgressBarModal() {}

    void centerInWidget(QWidget *w) { move(w->mapToGlobal(QPoint((w->size().width() - size().width()) / 2, (w->size().height() - size().height()) / 2))); }

protected:
    virtual void closeEvent(QCloseEvent *e) { e->ignore(); }
};

#endif // PROGRESSBARMODAL_HPP
