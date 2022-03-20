#import <Foundation/Foundation.h>

#include <QString>


NSString *NSStringFromQString(const QString &s)
{
#if IS_QT5
    return s.toNSString();
#else
    return [NSString stringWithCharacters:(const unichar *)s.unicode() length:s.length()];
#endif
}
