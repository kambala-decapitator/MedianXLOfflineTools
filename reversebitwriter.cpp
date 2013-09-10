#include "reversebitwriter.h"
#include "helpers.h"
#include "structs.h"

#include <QString>


QString &ReverseBitWriter::replaceValueInBitString(QString &bitString, int offset, int newValue)
{
    int length = Enums::ItemOffsets::offsetLength(offset);
    return bitString.replace(startOffset(bitString, offset, length), length, binaryStringFromNumber(newValue, false, length));
}

QString &ReverseBitWriter::updateItemRow(ItemInfo *item)
{
    return replaceValueInBitString(item->bitString, Enums::ItemOffsets::Row, item->row);
}

QString &ReverseBitWriter::updateItemColumn(ItemInfo *item)
{
    return replaceValueInBitString(item->bitString, Enums::ItemOffsets::Column, item->column);
}

QString &ReverseBitWriter::remove(QString &bitString, int offsetWithoutJM, int length)
{
    return bitString.remove(startOffset(bitString, offsetWithoutJM, length, false) - 1, length);
}

QString &ReverseBitWriter::byteAlignBits(QString &bitString)
{
    const int kBitsInByte = 8;
    int extraBits = bitString.length() % kBitsInByte;
    if (extraBits)
    {
        int zerosBeforeFirst1 = bitString.indexOf('1'), zerosToAppend = kBitsInByte - extraBits;
        if (zerosBeforeFirst1 + zerosToAppend < kBitsInByte)
            bitString.prepend(QString(zerosToAppend, kZeroChar));
        else
            bitString.remove(0, extraBits);
    }
    return bitString;
}


int ReverseBitWriter::startOffset(const QString &bitString, int offset, int length, bool isItemHeaderSkilled/* = true*/)
{
    // 16 is 'JM' offset which is not stored in the bitString
    return bitString.length() - (offset - 16 * isItemHeaderSkilled) - length;
}
