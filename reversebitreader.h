#ifndef REVERSEBITREADER_H
#define REVERSEBITREADER_H

#include <QString>


class ReverseBitReader
{
public:
    ReverseBitReader(const QString &bitString) : _bitString(bitString), _pos(bitString.length()) {}

    inline bool readBool(bool *ok = 0) { return static_cast<bool>(readNumber(1, ok)); }
    qint64 readNumber(int length, bool *ok = 0);

    int pos() const { return _bitString.length() - _pos; }
    int setPos(int newPos);
    void skip(int length = 1);

    QString notReadBits() const { return _bitString.left(_pos); }
    QChar at(int pos) const { return _bitString.at(_bitString.length() - pos); }

private:
    QString _bitString;
    int _pos;
};

#endif // REVERSEBITREADER_H
