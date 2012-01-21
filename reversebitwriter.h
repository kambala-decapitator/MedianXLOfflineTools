#ifndef REVERSEBITWRITER_H
#define REVERSEBITWRITER_H


class QString;

class ReverseBitWriter
{
public:
	static QString &replaceValueInBitString(QString &bitString, int pos, int length, int newValue);
};

#endif // REVERSEBITWRITER_H
