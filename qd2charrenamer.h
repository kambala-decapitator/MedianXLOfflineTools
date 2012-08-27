#ifndef QD2CHARRENAMER_H
#define QD2CHARRENAMER_H

#include <QDialog>


namespace Ui { class QD2CharRenamerClass; }
class QTextEdit;

class QD2CharRenamer : public QDialog
{
    Q_OBJECT

public:
    static const int kMaxNameLength;
    static void updateNamePreview(QTextEdit *previewTextEdit, const QString &name);

    QD2CharRenamer(const QString &originalName, bool shouldWarn, QWidget *parent = 0);
    virtual ~QD2CharRenamer() { delete ui; }

    QString name() const { return _originalCharName; }

private slots:
    void nameChanged(const QString &newName);
    void insertColor();
    void saveName();

private:
    Ui::QD2CharRenamerClass *ui;
    QString _originalCharName;
    bool _shouldWarn;
    const QStringList colorNames;

    void createColorMenu();
};

#endif // QD2CHARRENAMER_H
