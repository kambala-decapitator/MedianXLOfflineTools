#ifndef REVERSEBITWRITER_H
#define REVERSEBITWRITER_H

#include "enums.h"


class QString;

class ReverseBitWriter
{
public:
    static QString &replaceValueInBitString(QString &bitString, Enums::ItemOffsets::ItemOffsetsEnum offset, int newValue);
};

#endif // REVERSEBITWRITER_H
