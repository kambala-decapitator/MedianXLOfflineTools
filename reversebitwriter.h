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
};

#endif // REVERSEBITWRITER_H
