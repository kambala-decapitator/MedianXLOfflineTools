#ifndef REVERSEBITWRITER_H
#define REVERSEBITWRITER_H

#include "enums.h"


class QString;

class ReverseBitWriter
{
public:
    static QString &replaceValueInBitString(QString &bitString, int offset, int newValue);
};

#endif // REVERSEBITWRITER_H
