#include "reversebitreader.h"


qint64 ReverseBitReader::readNumber(int length, bool *ok /*= 0*/)
{
    if (_pos - length >= 0)
    {
        if (ok)
            *ok = true;

        _pos -= length;
        return _bitString.mid(_pos, length).toLongLong(ok, 2);
    }
    else
    {
        if (ok)
            *ok = false;

        _pos = _bitString.length() + 1;
        qWarning("attempt to read past bitstring length");
        throw 1;
        return 0;
    }
}

int ReverseBitReader::setPos(int newPos)
{
    if (newPos >= 0 && newPos < _bitString.length())
    {
        _pos = _bitString.length() - newPos;
        return _pos;
    }
    else
    {
        qWarning("attempt to set new position past bitstring length");
        throw 2;
        return -1;
    }
}

void ReverseBitReader::skip(int length)
{
    if (_pos - length > 0 && _pos - length <= _bitString.length())
        _pos -= length;
    else
    {
        qWarning("attempt to skip past bitstring length");
        throw 3;
    }
}
