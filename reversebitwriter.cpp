#include "reversebitwriter.h"

#include <QString>


QString &ReverseBitWriter::replaceValueInBitString(QString &bitString, int pos, int length, int newValue)
{
	// 16 is JM offset which is not stored in the bitString
	return bitString.replace(bitString.length() - (pos - 16) - length, length, QString::number(newValue, 2));
}
