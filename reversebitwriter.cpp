#include "reversebitwriter.h"
#include "helpers.h"

#include <QString>


QString &ReverseBitWriter::replaceValueInBitString(QString &bitString, int pos, int length, int newValue)
{
    // 16 is JM offset which is not stored in the bitString
    return bitString.replace(bitString.length() - (pos - 16) - length, length, QString("%1").arg(newValue, length, 2, zeroChar));
}
