#include "reversebitwriter.h"
#include "helpers.h"

#include <QString>


QString &ReverseBitWriter::replaceValueInBitString(QString &bitString, int offset, int newValue)
{
    int length = Enums::ItemOffsets::offsetLength(offset);
    // 16 is 'JM' offset which is not stored in the bitString
    return bitString.replace(bitString.length() - (offset - 16) - length, length, binaryStringFromNumber(newValue, false, length));
}
