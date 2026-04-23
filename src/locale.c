#include <libraries/locale.h>
#include <proto/locale.h>
#include <proto/exec.h>

#include "locale.h"

#define CATCOMP_ARRAY
#include "strings.h"

struct LocaleBase *LocaleBase = NULL;
struct Catalog  *catalog = NULL;

BOOL InitLocale()
{
    LocaleBase = (struct LocaleBase *)OpenLibrary("locale.library", 38);
    if (LocaleBase)
        catalog = OpenCatalogA(NULL, "UniTool.catalog", NULL);
}

void CleanupLocale()
{
    if (catalog)   CloseCatalog(catalog);
    if (LocaleBase) CloseLibrary((struct Library *)LocaleBase);
}

const char *_(ULONG id)
{
    ULONG idx = 0;
    STRPTR defstr = NULL;

    do {
        if (CATCOMPARRAY[idx].cca_ID == id) {
            defstr = CATCOMPARRAY[idx].cca_Str;
            break;
        }
    } while (CATCOMPARRAY[++idx].cca_Str != NULL);

    if (defstr) {
        if (catalog) {
            return GetCatalogStr(catalog, id, defstr);
        }
        else return defstr;
    }

    return "???";
}
