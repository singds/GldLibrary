//===================================================================== INCLUDES
#include "GldSimulator.h"

#include "stdafx.h"
#include <cstdint>
#include <Objbase.h>
#include <d2d1.h>
#include <WindowsX.h>
#include "App.h"


extern "C" {
#include "GldCfg.h"
#include "BkeHalDisp.h"
}

//====================================================================== CLASSES
template <class T> void SafeRelease (T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release ( );
		*ppT = NULL;
	}
}

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES
static ATOM MyRegisterClass (HINSTANCE hInstance);
static LRESULT CALLBACK WinClassProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL InitInstance (HINSTANCE hInstance, int nCmdShow);
static HRESULT CreateGraphicsResources ( );
static void DiscardGraphicsResources ( );
static LRESULT HandleCreate (UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT HandlePaint (UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT HandleDestroy (UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT HandleMouse (UINT message, WPARAM wParam, LPARAM lParam);
static DWORD WINAPI ThreadGraphic (LPVOID lpParam);

//============================================================= STATIC VARIABLES
static const LPCWSTR WinClassName = L"MyWindowClass"; // Nome della classe della window
static const LPCWSTR WinTitle = L"GldSimulator"; // Titolo della finestra
#if ((BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__0) || (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__180))
static const uint32_t WinWidth = BKEHALDISP_FRAME_BUFFER_X_RES; // Dimensione x della finestra (pxl)
static const uint32_t WinHeight = BKEHALDISP_FRAME_BUFFER_Y_RES; // Dimensione y della finestra (pxl)
#endif
#if ((BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__90) || (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__270))
static const uint32_t WinWidth = BKEHALDISP_FRAME_BUFFER_Y_RES; // Dimensione x della finestra (pxl)
static const uint32_t WinHeight = BKEHALDISP_FRAME_BUFFER_X_RES; // Dimensione y della finestra (pxl)
#endif
static const uint8_t FbBytePPxl = BKEHALDISP_BYTE_PER_PXL; // Byte per pixel del framebuffer
static const UINT_PTR TmrIDSwTimer = 0; // Timer id del timer utilizzato per il timer della GUI

static HINSTANCE AppHInst; // istanza corrente dell'applicazione
static ID2D1Factory *D2dFactory; // direct2d factory
static ID2D1HwndRenderTarget *D2dRenderTarget; // direct2d render target
static ID2D1Bitmap *D2dBitmap; // direct2d bitmap
static HWND HWind; // handler di questa finestra
static HANDLE HThreadGraphic;
static DWORD IdThreadGraphic;

struct
{
	bool Touch;
	int16_t XPos;
	int16_t YPos;
} static Touch;

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
int APIENTRY wWinMain (_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPWSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	HRESULT hr; // COM library operation result

	/* Inizializzo la libreria COM. Devo inizializzarla almento una volta per
	   ogni thread che la utilizza. */
	hr = CoInitializeEx (NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED (hr))
	{
		MyRegisterClass (hInstance);

		// Eseguire l'inizializzazione dall'applicazione:
		if (!InitInstance (hInstance, nCmdShow))
		{
			// inizializzazione fallita
			// impossibile creare la finestra
		}

		{
			MSG msg;

			// Ciclo di messaggi principale:
			while (GetMessage (&msg, nullptr, 0, 0))
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
		}
	}

	return FALSE;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
extern "C"
{
void gldsimulator_InvalidateArea (int16_t x0, int16_t y0, int16_t x_size, int16_t y_size)
{
	RECT invalid_rect = {x0, y0, x0 + x_size, y0 + y_size};
	InvalidateRect (HWind, NULL, FALSE);
	//InvalidateRect (PntWindow->HWind, &invalid_rect, FALSE);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
bool gldsimulator_GetTouch (int16_t *x_pos, int16_t *y_pos)
{
	if (Touch.Touch)
	{
		*x_pos = Touch.XPos;
		*y_pos = Touch.YPos;
	}
	return Touch.Touch;
}
}

//============================================================ PRIVATE FUNCTIONS
/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static LRESULT CALLBACK WinClassProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		return HandleCreate (message, wParam, lParam);
	case WM_PAINT:
		return HandlePaint (message, wParam, lParam);
	case WM_DESTROY:
		return HandleDestroy (message, wParam, lParam);
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		return HandleMouse (message, wParam, lParam);
	case WM_TIMER:
		for (uint16_t j = 0; j < 100; j++) {
			// TODO call the timer handler
			// bkehaltimer_Manager1Ms();
		}
		return 0;
	default:
		return DefWindowProc (hWnd, message, wParam, lParam);
	}
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static LRESULT HandleCreate (UINT message, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;
	D2dFactory = NULL;
	D2dRenderTarget = NULL;

	// Create Direct2D factory
	hr = D2D1CreateFactory (
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&D2dFactory);
	if (SUCCEEDED (hr))
	{
		app_InitModule ( );

		HThreadGraphic = CreateThread (
			NULL, // default security attributes
			0, // use default stack size  
			ThreadGraphic, // thread function name
			NULL, // argument to thread function 
			0, // use default creation flags 
			&IdThreadGraphic); // returns the thread identifier
		return 0;
	}
	return -1; // Fail CreateWindowEx.
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static LRESULT HandlePaint (UINT message, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr = CreateGraphicsResources ( );
	if (SUCCEEDED (hr))
	{
		PAINTSTRUCT ps;
		D2D1_RECT_F bmpDestRect;
		D2D1_RECT_U memDestRect;
		BeginPaint (HWind, &ps);

		D2dRenderTarget->BeginDraw ( );

		D2dRenderTarget->Clear (D2D1::ColorF (D2D1::ColorF::Black));
		bmpDestRect = D2D1::RectF (0, 0, WinWidth, WinHeight);
		memDestRect = D2D1::RectU (0, 0, WinWidth, WinHeight);
		D2dBitmap->CopyFromMemory (&memDestRect, bkehaldisp_GetFramebuffer ( ), WinWidth * FbBytePPxl);
		D2dRenderTarget->DrawBitmap (D2dBitmap, bmpDestRect);

		hr = D2dRenderTarget->EndDraw ( );
		if (FAILED (hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources ( );
		}
		EndPaint (HWind, &ps);
	}
	return 0;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static LRESULT HandleDestroy (UINT message, WPARAM wParam, LPARAM lParam)
{
	TerminateThread (HThreadGraphic, 0);
	KillTimer (HWind, TmrIDSwTimer);
	DiscardGraphicsResources ( );
	SafeRelease (&D2dFactory);
	CoUninitialize ( );
	PostQuitMessage (0);
	return 0;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static LRESULT HandleMouse (UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_LBUTTONDOWN ||
		message == WM_MOUSEMOVE)
	{ // acquisisco le coordinate del punto di tocco
		Touch.XPos = GET_X_LPARAM (lParam);
		Touch.YPos = GET_Y_LPARAM (lParam);
	}

	switch (message)
	{
	case WM_LBUTTONDOWN:
		Touch.Touch = true;
		break;
	case WM_LBUTTONUP:
		Touch.Touch = false;
		break;
	}
	return 0;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static ATOM MyRegisterClass (HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {0};

	wcex.cbSize = sizeof (WNDCLASSEX);

	wcex.lpszClassName = WinClassName;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WinClassProc;
	wcex.hInstance = hInstance;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	return RegisterClassExW (&wcex);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
	RECT winSize = {0, 0, WinWidth, WinHeight}; // Dimensione dell'area client che voglio per la finestra
	AppHInst = hInstance; // Archivia l'handle di istanza nella variabile di classe

	/* calcolo la dimensione totale della finestra in base alla dimensione
	   dell'area client che voglio.
	*/
	if (AdjustWindowRect (&winSize, WS_OVERLAPPEDWINDOW, false) == 0)
	{
		return FALSE;
	}

	// aggiusto le coordinate della finestra ritornate da 'AdjustWindowRect'
	if (winSize.top < 0)
	{
		winSize.bottom -= winSize.top;
		winSize.top = 0;
	}
	if (winSize.left < 0)
	{
		winSize.right -= winSize.left;
		winSize.left = 0;
	}

	// creo la finestra principale del programma
	HWind = CreateWindowW (
		WinClassName,
		WinTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, /* x0, y0 position */
		winSize.right, winSize.bottom, /* x, y size */
		nullptr, nullptr, hInstance, nullptr);

	if (!HWind)
	{ 
		return FALSE;
	}

	//TODO gldtimer_InitModule ( );
	SetTimer (HWind, // handle to main window 
			  TmrIDSwTimer, // timer identifier 
			  100, // ms interval 
			  (TIMERPROC)NULL); // no timer callback

	// rendo visibile la finestra del programma 
	ShowWindow (HWind, nCmdShow);
	// aggiorna l'area client della finestra inviando in messaggio WM_PAINT
	UpdateWindow (HWind);

	return TRUE;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static HRESULT CreateGraphicsResources ( )
{
	HRESULT hr = S_OK;
	// verifico se le risorse grafiche sono state scartate. In questo caso le ricreo.
	if (D2dRenderTarget == NULL)
	{
		RECT rc;
		GetClientRect (HWind, &rc);

		D2D1_SIZE_U size = D2D1::SizeU (rc.right, rc.bottom);

		hr = D2dFactory->CreateHwndRenderTarget (
			D2D1::RenderTargetProperties ( ),
			D2D1::HwndRenderTargetProperties (HWind, size),
			&D2dRenderTarget);
		if (SUCCEEDED (hr))
		{
			D2D1_SIZE_U bmpSize = D2D1::SizeU (WinWidth, WinHeight);
			D2D1_BITMAP_PROPERTIES bmpProp;

			bmpProp.pixelFormat = D2D1::PixelFormat (DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
#pragma warning(push)
#pragma warning(disable: 4996)
			D2dFactory->GetDesktopDpi (&bmpProp.dpiX, &bmpProp.dpiY);
#pragma warning(pop)
			hr = D2dRenderTarget->CreateBitmap (bmpSize, bmpProp, &D2dBitmap);
		}
	}
	return hr;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static void DiscardGraphicsResources ( )
{
	SafeRelease (&D2dBitmap);
	SafeRelease (&D2dRenderTarget);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Mone.
 Ret: - None.
______________________________________________________________________________*/
static DWORD WINAPI ThreadGraphic (LPVOID lpParam)
{
	while (1)
	{
		app_Task ( );
	}
}
