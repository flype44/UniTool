#ifndef _PRESETS_H
#define _PRESETS_H

#include <exec/types.h>
#include <stdint.h>

struct Preset {
    UWORD pr_Width;
    UWORD pr_Height;
    UWORD pr_DX;
    UWORD pr_DY;
    UWORD pr_B;
    UWORD pr_C;
    UWORD pr_Aspect;
    UBYTE pr_Phase;
    UBYTE pr_Smooth;
    UBYTE pr_Integer;
    UBYTE pr_Scanlines;
    UBYTE pr_ScanlinesLaced;
};

int SavePreset(struct Preset * preset, char *name, char *path);
int LoadPreset(struct Preset * preset, char *name, char *path);

#endif /* _PRESETS_H */
