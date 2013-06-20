#ifndef REVERSEBITWRITER_H
#define REVERSEBITWRITER_H

#include "structs.h"


class QString;

class ReverseBitWriter
{
public:
    static QString &replaceValueInBitString(QString &bitString, int offset, int newValue);
    static QString &updateItemRow(ItemInfo *item)    { return replaceValueInBitString(item->bitString, Enums::ItemOffsets::Row,    item->row); }
    static QString &updateItemColumn(ItemInfo *item) { return replaceValueInBitString(item->bitString, Enums::ItemOffsets::Column, item->column); }
};

#endif // REVERSEBITWRITER_H
