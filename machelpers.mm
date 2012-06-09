#import <Foundation/Foundation.h>

#include <QString>


NSString *NSStringFromQString(const QString &s)
{
    return [NSString stringWithCharacters:(const unichar *)s.unicode() length:s.length()];
}
