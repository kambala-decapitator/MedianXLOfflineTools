#ifndef QD2CHARRENAMER_H
#define QD2CHARRENAMER_H

#include "ui_qd2charrenamer.h"

#include <QDialog>


class QTextEdit;

class QD2CharRenamer : public QDialog
{
	Q_OBJECT

public:
	static const int maxNameLength;
    static void updateNamePreview(QTextEdit *previewTextEdit, const QString &name);

	QD2CharRenamer(QWidget *parent, const QString &originalName);

	QString name() const { return _originalCharName; }

private slots:
	void nameChanged(const QString &newName);
	void insertColor();
	void saveName();

private:
	Ui::QD2CharRenamerClass ui;
	QString _originalCharName;

	void createColorMenu();
};

#endif // QD2CHARRENAMER_H
