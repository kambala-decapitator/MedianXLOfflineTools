#ifndef QD2CHARRENAMER_H
#define QD2CHARRENAMER_H

#include <QDialog>


namespace Ui { class QD2CharRenamerClass; }
class QLabel;

class QD2CharRenamer : public QDialog
{
    Q_OBJECT

public:
    static const int kMaxNameLength;
    static void customizeNamePreviewLabel(QLabel *previewLabel);
    static void    updateNamePreviewLabel(QLabel *previewLabel, const QString &name);

    QD2CharRenamer(const QString &originalName, bool shouldWarn, QWidget *parent = 0, bool areColorsAllowed = true);
    virtual ~QD2CharRenamer();

    QString name()              const { return _originalCharName; }
    bool shouldWarnAboutColor() const { return _shouldWarnAboutColor; }

    void setLineToolTip(const QString &toolTip);

private slots:
    void nameChanged(const QString &newName);
    void insertColor();
    void saveName();

private:
    Ui::QD2CharRenamerClass *ui;
    QString _originalCharName;
    bool _shouldWarnAboutColor, _areColorsAllowed;
    const QStringList colorNames;

    void createColorMenu();
};

#endif // QD2CHARRENAMER_H
