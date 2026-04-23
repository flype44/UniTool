#ifndef __LOCALE_H
#define __LOCALE_H

#include <exec/types.h>

BOOL InitLocale();
void CleanupLocale();

const char *_(ULONG id);

#endif /* __LOCALE_H */
