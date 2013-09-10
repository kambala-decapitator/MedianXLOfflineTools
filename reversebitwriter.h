#ifndef REVERSEBITWRITER_H
#define REVERSEBITWRITER_H


class QString;
class ItemInfo;

class ReverseBitWriter
{
public:
    static QString &replaceValueInBitString(QString &bitString, int offset, int newValue);
    static QString &updateItemRow(ItemInfo *item);
    static QString &updateItemColumn(ItemInfo *item);

    static QString &remove(QString &bitString, int offset, int length);
    static QString &byteAlignBits(QString &bitString);

private:
    static int startOffset(const QString &bitString, int offsetWithoutJM, int length, bool isItemHeaderSkilled = true);
};

#endif // REVERSEBITWRITER_H
