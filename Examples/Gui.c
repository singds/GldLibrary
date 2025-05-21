//===================================================================== INCLUDES
#include "Gui.h"

#include "App.h"
#include "ActAssets.h"

//====================================================================== DEFINES
#define L_FB0_ADDR          EMC_SDRAM_BASE_ADDR
#define L_FB1_ADDR          (L_FB0_ADDR + (4 * 320 * 240))

//=========================================================== PRIVATE PROTOTYPES
static void ThreadGuiRender (void *argument);
static void OpenFonts (void);

//============================================================= STATIC VARIABLES
__align(8) static uint8_t ThreadGuiStack[3000];
static const osThreadAttr_t ThreadGuiRenderAttr =
{
    .name = "GuiRender",
    .stack_mem = &ThreadGuiStack,
    .stack_size = sizeof (ThreadGuiStack),
    .priority = osPriorityNormal,
};

//============================================================= GLOBAL VARIABLES
osThreadId_t gui_ThreadRenderId;
uint32_t gui_MaxFrameRefreshTime;

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
int32_t gui_InitModule (void)
{
    int32_t ret = _STATUS_FAIL_;
    
    //______________________________________________________Init graphic library
    // inizializza qui il display driver se ti serve

    // inizializza qui il touchscreen driver se ti serve
    
    gldrefr_Init ( );
    glddisp_Init (&bkehaldisp_Display);
    gldtouch_Init (&bkehaltouch_Touch);
    
    OpenFonts ( );
    
    //_____________________________________________________Start graphic threads
    gui_ThreadRenderId = osThreadNew (ThreadGuiRender, NULL, &ThreadGuiRenderAttr);
    if (gui_ThreadRenderId)
    {
        ret = _STATUS_OK_;
    }
    return ret;
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
const char *gui_Translate (const char *const *textArray)
{
    const char *traduzione = "UNDEF.";

    if (textArray)
    {
        traduzione = textArray[GUI_LINGUA];
        /* fallback alla lingua inglese se la traduzione e' stringa vuota oppure
        la traduzione non e' definita */
        if (traduzione == NULL || traduzione[0] == '\0')
            traduzione = textArray[1];
        /* fallback alla lingua italiana se la traduzione in inglese ancora e'
        stringa vuota oppure non e' definita */
        if (traduzione == NULL || traduzione[0] == '\0')
            traduzione = textArray[0];
    }
    return traduzione;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
const char *gui_TranslateIn (const char *const *textArray, uint8_t lingua)
{
    const char *traduzione = "UNDEF.";

    if (textArray)
    {
        traduzione = textArray[lingua];
        /* fallback alla lingua inglese se la traduzione e' stringa vuota oppure
        la traduzione non e' definita */
        if (traduzione == NULL || traduzione[0] == '\0')
            traduzione = textArray[1];
        /* fallback alla lingua italiana se la traduzione in inglese ancora e'
        stringa vuota oppure non e' definita */
        if (traduzione == NULL || traduzione[0] == '\0')
            traduzione = textArray[0];
    }
    return traduzione;
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void ThreadGuiRender (void *arg)
{
    gldactivity_Next (&actassets_Page, false);
    
    while(1)
    {
        /*_______________________________________________________ GUI POLLING */
        /*_______________________________________________________ GUI POLLING */
        gldtouch_Task ( );
        gldactivity_Task ( );
        
        swtimer_Time_t startTime = swtimer_GetMsActivity ( );
        
        gldrefr_Task ( );
            
#if (1) // calcolo il massimo tempo impiegato per il refresh di un frame
        swtimer_Time_t elapsedTime = swtimer_GetMsActivity ( ) - startTime;
        if (elapsedTime > gui_MaxFrameRefreshTime)
            gui_MaxFrameRefreshTime = elapsedTime;
#endif
        osThreadYield ( );
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void OpenFonts (void)
{
    guifont_ArialUniSz14 = gldfont_Open (&arialuni14_Font);
}

