#ifndef REVERSEBITWRITER_H
#define REVERSEBITWRITER_H


class QString;
class ItemInfo;

class ReverseBitWriter
{
public:
    static QString &replaceValueInBitString(QString &bitString, int offset, int newValue, int length = -1);
    static QString &updateItemRow(ItemInfo *item);
    static QString &updateItemColumn(ItemInfo *item);

    static QString &remove(QString &bitString, int offset, int length);
    static QString &insert(QString &bitString, int offsetWithoutJM, const QString &bitStringToInsert);
    static QString &byteAlignBits(QString &bitString);

private:
    static int startOffset(const QString &bitString, int offsetWithoutJM, int length, bool isItemHeaderSkipped = true);
};

#endif // REVERSEBITWRITER_H
