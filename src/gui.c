#include <graphics/gfxbase.h>
#include <graphics/displayinfo.h>
#include <libraries/mui.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/unicam.h>

#include <clib/alib_protos.h>

#include "messages.h"
#include "rga_common.h"
#include "rga_host.h"

struct GfxBase *        GfxBase         = NULL;
struct IntuitionBase *  IntuitionBase   = NULL;
struct Library *        MUIMasterBase   = NULL;
Object *                app             = NULL;
Object *                win             = NULL;
Object *                slCropX         = NULL;
Object *                slCropY         = NULL;
Object *                slCropW         = NULL;
Object *                slCropH         = NULL;
Object *                slAspect        = NULL;
Object *                chInteger       = NULL;
Object *                slScanlines     = NULL;
Object *                slScanlinesLaced= NULL;
Object *                chSmooth        = NULL;
Object *                slB             = NULL;
Object *                slC             = NULL;
Object *                slPhase         = NULL;
Object *                menuQuit        = NULL;
Object *                menuDefaults    = NULL;

struct Hook kernelHook;
struct Hook cropSizeHook;
struct Hook cropOffsetHook;
struct Hook aspectHook;
struct Hook configHook;
struct Hook scanlineHook;
struct Window *backdrop;
struct Screen *screen;

ULONG def_x, def_y, def_w, def_h, def_aspect;
ULONG def_b, def_c, def_phase, def_scan, def_scanl;
ULONG def_integer, def_smooth;

void OpenPALScreen(void)
{
    static UWORD pens[] = { -1 };

    screen = OpenScreenTags(NULL,
        SA_DisplayID,   PAL_MONITOR_ID | HIRESLACE_KEY,
        SA_Depth,       2,
        SA_Title,       (ULONG)"UniTool GUI",
        SA_ShowTitle,   FALSE,
        SA_Quiet,       TRUE,
        SA_Type,        CUSTOMSCREEN,
        SA_Pens,        (ULONG)pens,
        SA_FullPalette, TRUE,
        TAG_DONE);

    backdrop = OpenWindowTags(NULL,
        WA_CustomScreen,  (ULONG)screen,
        WA_Left,          0,
        WA_Top,           0,
        WA_Width,         screen->Width,
        WA_Height,        screen->Height,
        WA_Borderless,    TRUE,
        WA_Backdrop,      TRUE,
        WA_Activate,      FALSE,   /* don't steal focus from MUI window */
        WA_NoCareRefresh, TRUE,    /* we handle our own drawing */
        WA_SmartRefresh,  TRUE,
        WA_RMBTrap,       TRUE,
        TAG_DONE);

    struct RastPort *rp = backdrop->RPort;
    LONG sw = backdrop->Width;   /* 640 */
    LONG sh = backdrop->Height;  /* 512 */
    LONG arm = sw / 32;

    LONG gridCols = 20;
    LONG gridRows = 16;

    LONG x0 = sw * 1 / gridCols + 1;
    LONG y0 = sh * 1 / gridRows + 1;
    LONG x1 = sw * 3 / gridCols - 1;
    LONG y1 = sh * 3 / gridRows - 1;

    static UWORD pattern[] = {
        0xAAAA,
        0x5555,
    };

    rp->AreaPtrn  = pattern;
    rp->AreaPtSz  = 1;        /* 2^1 = 2 rows */

    SetAPen(rp, 1);
    SetBPen(rp, 0);
    SetDrMd(rp, JAM2);

    RectFill(rp, x0, y0, x1, y1);

    x0 = sw * 17 / gridCols + 1;
    y0 = sh * 1 / gridRows + 1;
    x1 = sw * 19 / gridCols - 1;
    y1 = sh * 3 / gridRows - 1;

    SetAPen(rp, 2);

    RectFill(rp, x0, y0, x1, y1);

    x0 = sw * 17 / gridCols + 1;
    y0 = sh * 13 / gridRows + 1;
    x1 = sw * 19 / gridCols - 1;
    y1 = sh * 15 / gridRows - 1;

    SetAPen(rp, 3);

    RectFill(rp, x0, y0, x1, y1);

    x0 = sw * 1 / gridCols + 1;
    y0 = sh * 13 / gridRows + 1;
    x1 = sw * 3 / gridCols - 1;
    y1 = sh * 15 / gridRows - 1;

    SetAPen(rp, 1);
    SetBPen(rp, 2);

    RectFill(rp, x0, y0, x1, y1);

    rp->AreaPtrn = NULL;
    rp->AreaPtSz = 0;
    
    /* --- grid --- */
    SetAPen(rp, 1);

    for (LONG i = 1; i < gridCols; i++)
    {
        LONG x = sw * i / gridCols;
        Move(rp, x, 0);
        Draw(rp, x, sh - 1);
    }
    for (LONG i = 1; i < gridRows; i++)
    {
        LONG y = sh * i / gridRows;
        Move(rp, 0, y);
        Draw(rp, sw - 1, y);
    }

    DrawEllipse(rp, 320, 256, 250, 250);
    DrawEllipse(rp, 320, 256, 150, 150);

    DrawEllipse(rp, 64, 64, 48, 48);
    DrawEllipse(rp, 640-64, 64, 48, 48);
    DrawEllipse(rp, 64, 512 - 64, 48, 48);
    DrawEllipse(rp, 640 - 64, 512 - 64, 48, 48);

        /* --- corner markers at screen edges --- */
    SetAPen(rp, 2);

    /* top-left */
    Move(rp, 0, arm); Draw(rp, 0, 0); Draw(rp, arm, 0);
    /* top-right */
    Move(rp, sw - arm - 1, 0); Draw(rp, sw - 1, 0); Draw(rp, sw - 1, arm);
    /* bottom-left */
    Move(rp, arm, sh - 1); Draw(rp, 0, sh - 1); Draw(rp, 0, sh - arm - 1);
    /* bottom-right */
    Move(rp, sw - arm, sh - 1); Draw(rp, sw - 1, sh - 1); Draw(rp, sw - 1, sh - arm - 1);

    /* bottom-left */
    Move(rp, arm, 400 - 1); Draw(rp, 0, 400 - 1); Draw(rp, 0, 400 - arm - 1);
    /* bottom-right */
    Move(rp, sw - arm, 400 - 1); Draw(rp, sw - 1, 400 - 1); Draw(rp, sw - 1, 400 - arm - 1);
}

void CloneWorkbenchPalette(struct Screen *destScreen)
{
    struct Screen *wb = LockPubScreen("Workbench");
    if (!wb) return;

    struct ColorMap *cm = wb->ViewPort.ColorMap;
    UWORD numColors = cm->Count;

    /* GetRGB32 wants a buffer of (numColors * 3) + 2 LONGs
       format: [count<<16 | firstcolor, R, G, B, R, G, B, ..., 0] */
    ULONG *palBuf = AllocVec((numColors * 3 + 2) * sizeof(ULONG), MEMF_ANY);
    if (palBuf)
    {
        GetRGB32(cm, 0, numColors, palBuf + 1);
        palBuf[0] = (ULONG)numColors << 16 | 0;  /* count | first */
        palBuf[numColors * 3 + 1] = 0;            /* terminator */

        LoadRGB32(&destScreen->ViewPort, palBuf);

        FreeVec(palBuf);
    }

    UnlockPubScreen(NULL, wb);
}

void UpdateDList()
{
    struct MsgPort * vc4Port;
    struct MsgPort * replyPort;

    vc4Port = FindPort("VideoCore");
    if (vc4Port)
    {
        struct VC4Msg cmd;
        replyPort = CreateMsgPort();

        cmd.msg.mn_ReplyPort = replyPort;
        cmd.msg.mn_Length = sizeof(struct VC4Msg);

        cmd.cmd = VCMD_UPDATE_UNICAM_DL;
        PutMsg(vc4Port, &cmd.msg);
        WaitPort(replyPort);
        GetMsg(replyPort);

        DeleteMsgPort(replyPort);
    }
}

ULONG CropOffsetHookFunc(struct Hook *hook, Object *app, void **params)
{
    extern APTR UnicamBase;
    ULONG dx,dy;

    get(slCropX, MUIA_Numeric_Value, &dx);
    get(slCropY, MUIA_Numeric_Value, &dy);

    UnicamSetCropOffset(dx, dy);

    UpdateDList();

    return 0;
}

ULONG CropSizeHookFunc(struct Hook *hook, Object *app, void **params)
{
    extern APTR UnicamBase;
    ULONG width,height;

    get(slCropW, MUIA_Numeric_Value, &width);
    get(slCropH, MUIA_Numeric_Value, &height);

    UnicamSetCropSize(width, height);

    set(slCropX, MUIA_Slider_Max, 720 - width);
    set(slCropY, MUIA_Slider_Max, 576 - height);

    UpdateDList();

    return 0;
}

ULONG AspectHookFunc(struct Hook *hook, Object *app, void **params)
{
    extern APTR UnicamBase;
    ULONG aspect;

    get(slAspect, MUIA_Numeric_Value, &aspect);

    UnicamSetAspect(aspect);

    UpdateDList();

    return 0;
}

ULONG KernelHookFunc(struct Hook *hook, Object *app, void **params)
{
    extern APTR UnicamBase;
    ULONG b,c;

    get(slB, MUIA_Numeric_Value, &b);
    get(slC, MUIA_Numeric_Value, &c);

    UnicamSetKernel(b, c);

    UpdateDList();

    return 0;
}

ULONG ConfigHookFunc(struct Hook *hook, Object *app, void **params)
{
    extern APTR UnicamBase;
    ULONG cfg;
    ULONG integer;
    ULONG phase;
    ULONG smooth;

    cfg = UnicamGetConfig();

    get(chInteger, MUIA_Selected, &integer);
    get(chSmooth, MUIA_Selected, &smooth);
    get(slPhase, MUIA_Numeric_Value, &phase);

    cfg = cfg & ~(UNICAMF_PHASE | UNICAMF_INTEGER | UNICAMF_SMOOTHING);
    
    cfg |= (phase << UNICAMB_PHASE) & UNICAMF_PHASE;
    
    if (integer)
        cfg |= UNICAMF_INTEGER;

    if (smooth)
        cfg |= UNICAMF_SMOOTHING;

    UnicamSetConfig(cfg);

    UpdateDList();

    return 0;
}

ULONG ScanlineHookFunc(struct Hook *hook, Object *app, void **params)
{
    ULONG sc, scl;

    get(slScanlines, MUIA_Numeric_Value, &sc);
    get(slScanlinesLaced, MUIA_Numeric_Value, &scl);

    rga_set_scanlines(sc, scl);

    return 0;
}

BOOL BuildGUI(struct Screen * myScreen)
{
    extern APTR UnicamBase;

    app = ApplicationObject,
        MUIA_Application_Title,       "UniTool",
        MUIA_Application_Version,     "$VER: " VERSION_STRING,
        MUIA_Application_Base,        "UNITOOL",

        MUIA_Application_Menustrip, MenustripObject,
            MUIA_Family_Child, MenuObject,
                MUIA_Menu_Title, (ULONG)"Project",
                MUIA_Family_Child, menuQuit = MenuitemObject,
                    MUIA_Menuitem_Title, (ULONG)"Quit",
                End,
            End,
            MUIA_Family_Child, MenuObject,
                MUIA_Menu_Title, (ULONG)"Edit",
                MUIA_Family_Child, menuDefaults = MenuitemObject,
                    MUIA_Menuitem_Title, (ULONG)"Reset to Defaults",
                End,
            End,
        End,

        SubWindow, win = WindowObject,
            MUIA_Window_Screen,       myScreen,
            MUIA_Window_LeftEdge,     MUIV_Window_LeftEdge_Centered,
            MUIA_Window_TopEdge,      MUIV_Window_TopEdge_Centered,
            MUIA_Window_Borderless,   TRUE,
            MUIA_Window_Title,        NULL,
            MUIA_Window_CloseGadget,  FALSE,
            MUIA_Window_DragBar,      FALSE,
            MUIA_Window_DepthGadget,  FALSE,
            MUIA_Window_SizeGadget,   FALSE,

            
            WindowContents, VGroup,
                /* ---- Crop ---- */
                Child, VGroup, GroupFrameT("Image Crop and Size"),
                    Child, ColGroup(2),
                        Child, LLabel1("Width"),
                        Child, slCropW = SliderObject,
                            MUIA_Slider_Min, 1, MUIA_Slider_Max, 720,
                            MUIA_Slider_Level, 720,
                            MUIA_Window_ActiveObject, TRUE,
                            MUIA_MinWidth, 100,
                        End,

                        Child, LLabel1("Height"),
                        Child, slCropH = SliderObject,
                            MUIA_Slider_Min, 1, MUIA_Slider_Max, 576,
                            MUIA_Slider_Level, 576,
                        End,

                        Child, LLabel1("Offset X"),
                        Child, slCropX = SliderObject,
                            MUIA_Slider_Min, 0, MUIA_Slider_Max, 719,
                            MUIA_Slider_Level, 0,
                        End,

                        Child, LLabel1("Offset Y"),
                        Child, slCropY = SliderObject,
                            MUIA_Slider_Min, 0, MUIA_Slider_Max, 575,
                            MUIA_Slider_Level, 0,
                        End,

                        Child, LLabel1("Aspect"),
                        Child, slAspect = SliderObject,
                            MUIA_Slider_Min,   300,
                            MUIA_Slider_Max,   3000,
                            MUIA_Slider_Level, 1000,
                        End,
                    End,
                End,
                /* ---- Scale ---- */
                Child, VGroup, GroupFrameT("Scaling and Smoothing"),
                    Child, ColGroup(2),
                        Child, LLabel1("Integer only"),
                        Child, HGroup,
                            Child, chInteger = CheckMark(FALSE),
                            Child, HSpace(0),
                            Child, LLabel1("Smoothing"),
                            Child, HGroup,
                                Child, chSmooth = CheckMark(FALSE),
                                Child, HSpace(0),
                            End,
                        End,

                        Child, LLabel1("Kernel B"),
                        Child, slB = SliderObject,
                            MUIA_Slider_Min,   0,
                            MUIA_Slider_Max,   1000,
                            MUIA_Slider_Level, 333,
                            MUIA_MinWidth,     100,
                        End,

                        Child, LLabel1("Kernel C"),
                        Child, slC = SliderObject,
                            MUIA_Slider_Min,   0,
                            MUIA_Slider_Max,   1000,
                            MUIA_Slider_Level, 333,
                        End,
                        
                        Child, LLabel1("Phase"),
                        Child, slPhase = SliderObject,
                            MUIA_Slider_Min,   0,
                            MUIA_Slider_Max,   255,
                            MUIA_Slider_Level, 64,
                        End,
                    End,
                End,
                /* ---- Image ---- */
                Child, VGroup, GroupFrameT("Scanlines"),
                    Child, ColGroup(2),
                        Child, LLabel1("Scanlines Normal"),
                        Child, slScanlines = SliderObject,
                            MUIA_Slider_Min,   0,
                            MUIA_Slider_Max,   4,
                            MUIA_Slider_Level, 0,
                            MUIA_MinWidth, 40,
                        End,

                        Child, LLabel1("Scanlines Laced"),
                        Child, slScanlinesLaced = SliderObject,
                            MUIA_Slider_Min,   0,
                            MUIA_Slider_Max,   4,
                            MUIA_Slider_Level, 0,
                        End,
                    End,
                End,

                
                
            End, // VGroup
        End, // WindowObject
    End; // ApplicationObject

    RGA_VideoStatus vstat;

    if (rga_get_video_status(&vstat)) {
        set(slScanlines, MUIA_Numeric_Value, vstat.scanline_level);
        set(slScanlinesLaced, MUIA_Numeric_Value, vstat.scanline_level_laced);
    }

    DoMethod(win, MUIM_Window_SetCycleChain,
        slCropW, slCropH, slCropX, slCropY, slAspect,
        chInteger, chSmooth, slB, slC, slPhase,
        slScanlines, slScanlinesLaced,
        NULL);

    ULONG val = UnicamGetCropOffset();
    def_x = val >> 16;
    def_y = val & 0xffff;
    
    val = UnicamGetCropSize();
    def_w = val >> 16;
    def_h = val & 0xffff;
    
    def_aspect = UnicamGetAspect();

    val = UnicamGetKernel();
    def_b = val >> 16;
    def_c = val & 0xffff;

    val = UnicamGetConfig();

    def_integer = !!(val & UNICAMF_INTEGER);
    def_smooth = !!(val & UNICAMF_SMOOTHING);
    def_phase = (val & UNICAMF_PHASE) >> UNICAMB_PHASE;

    set(slCropX, MUIA_Numeric_Value, def_x);
    set(slCropY, MUIA_Numeric_Value, def_y);
    set(slCropW, MUIA_Numeric_Value, def_w);
    set(slCropH, MUIA_Numeric_Value, def_h);
    set(slAspect, MUIA_Numeric_Value, def_aspect);
    set(slB, MUIA_Numeric_Value, def_b);
    set(slC, MUIA_Numeric_Value, def_c);
    set(slPhase, MUIA_Numeric_Value, def_phase);
    if (def_integer)
        set(chInteger, MUIA_Selected, TRUE);
    if (def_smooth)
        set(chSmooth, MUIA_Selected, TRUE);

    cropOffsetHook.h_Entry = (HOOKFUNC)CropOffsetHookFunc;
    cropSizeHook.h_Entry   = (HOOKFUNC)CropSizeHookFunc;
    aspectHook.h_Entry     = (HOOKFUNC)AspectHookFunc;
    kernelHook.h_Entry     = (HOOKFUNC)KernelHookFunc;
    configHook.h_Entry     = (HOOKFUNC)ConfigHookFunc;
    scanlineHook.h_Entry   = (HOOKFUNC)ScanlineHookFunc;

    DoMethod(slCropX, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &cropOffsetHook);

    DoMethod(slCropY, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &cropOffsetHook);

    DoMethod(slCropW, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &cropSizeHook);

    DoMethod(slCropH, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &cropSizeHook);

    DoMethod(slAspect, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &aspectHook);

    DoMethod(slB, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &kernelHook);

    DoMethod(slC, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &kernelHook);

    DoMethod(slPhase, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &configHook);

    DoMethod(chInteger, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &configHook);

    DoMethod(chSmooth, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &configHook);

    DoMethod(slScanlines, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &scanlineHook);

    DoMethod(slScanlinesLaced, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
        app, 2, MUIM_CallHook, &scanlineHook);

    DoMethod(menuQuit, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime,
        (ULONG)app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

    DoMethod(win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
        app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

    return app != NULL;
}

int StartGUI()
{
    UnicamBase = OpenResource("unicam.resource");
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 39);

    if (GfxBase)
    {
        IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39);

        if (IntuitionBase)
        {
            MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN);

            if (MUIMasterBase)
            {
                OpenPALScreen();
                if (screen)
                {
                    //CloneWorkbenchPalette(screen);

                    if (BuildGUI(screen))
                    {
                        set(win, MUIA_Window_Open, TRUE);
                       
                        ULONG sigs = 0;
                        BOOL running = TRUE;
                        while (running)
                        {
                            ULONG ret = DoMethod(app, MUIM_Application_NewInput, &sigs);
                            if (ret == MUIV_Application_ReturnID_Quit)
                                running = FALSE;

                            if (running && sigs)
                                sigs = Wait(sigs | SIGBREAKF_CTRL_C);

                            if (sigs & SIGBREAKF_CTRL_C)
                                running = FALSE;
                        }

                        set(win, MUIA_Window_Open, FALSE);

                        MUI_DisposeObject(app);
                    }
                    
                    CloseWindow(backdrop);
                    CloseScreen(screen);
                }
            }
            else
            {
                Printf("Failed to open %s version %ld\n", (ULONG)MUIMASTER_NAME, MUIMASTER_VMIN);
            }

            CloseLibrary((struct Library *)IntuitionBase);
        }
        else
        {
            Printf("Failed to open %s version %ld\n", (ULONG)"intuition.library", 39);
        }

        CloseLibrary((struct Library *)GfxBase);
    }
    else
    {
        Printf("Failed to open %s version %ld\n", (ULONG)"graphics.library", 39);
    }

    
    
}
