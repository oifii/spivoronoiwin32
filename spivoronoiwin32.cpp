/*
 * Copyright (c) 2010-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// spivoronoiwin32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "spivoronoiwin32.h"
#include <mmsystem.h> //for timeSetEvent()
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;
#include "oifiilib.h" //note: oifiilib.lib/.dll is an MFC extension and resource DLL


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name


int global_titlebardisplay=1; //0 for off, 1 for on
int global_acceleratoractive=0; //0 for off, 1 for on
int global_menubardisplay=0; //0 for off, 1 for on

DWORD global_startstamp_ms;
string global_imagefolder="."; //the local folder of the .scr
//string global_imagefolder="c:\\temp\\spivideocaptureframetodisk\\";
//string global_imagefolder="c:\\temp\\spissmandelbrotmidi\\";
//string global_imagefolder="c:\\temp\\spicapturewebcam\\";
float global_duration_sec=180;
float global_sleeptimepervoronoi_sec=5;
int global_x=100;
int global_y=100;
int global_xwidth=300;
int global_yheight=300;
BYTE global_alpha=255;

int global_imageid=0;
HWND global_hwnd=NULL;
MMRESULT global_timer=0;
int global_imageheight=-1; //will be computed within WM_SIZE handler
int global_imagewidth=-1; //will be computed within WM_SIZE handler 
vector<string> global_txtfilenames;
COW2Doc* global_pOW2Doc=NULL;
COW2View* global_pOW2View=NULL;


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}


void CALLBACK StartGlobalProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	//WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment);

	DWORD nowstamp_ms = GetTickCount();
	while( (global_duration_sec<0.0f) || ((nowstamp_ms-global_startstamp_ms)/1000.0f)<global_duration_sec )
	{
		if(global_pOW2Doc) delete global_pOW2Doc;
		if(global_pOW2View) delete global_pOW2View;

		if(global_txtfilenames.size()>0)
		{
			//reset image id if end of file
			if(global_imageid>=global_txtfilenames.size()) global_imageid=0;

				
				global_pOW2Doc = new COW2Doc(global_hwnd);
				global_pOW2View = new COW2View(global_hwnd, (COWDocument*)global_pOW2Doc);
				/*
				// Initialize a CCreateContext to point to the active document. 
				// With this context, the new view is added to the document 
				// when the view is created in CView::OnCreate().
				CCreateContext myCreateContext;
				myCreateContext.m_pNewViewClass = NULL;
				myCreateContext.m_pNewDocTemplate = NULL;
				myCreateContext.m_pLastView = NULL;
				myCreateContext.m_pCurrentFrame = NULL;
				myCreateContext.m_pCurrentDoc = global_pOW2Doc;
				// The ID of the initial active view is AFX_IDW_PANE_FIRST. 
				// Incrementing this value by one for additional views works 
				// in the standard document/view case but the technique cannot 
				// be extended for the CSplitterWnd case.
				UINT viewID = AFX_IDW_PANE_FIRST + 1;
				CRect rect(0, 0, 0, 0); // Gets resized later. 
				// Create the new view. In this example, the view persists for 
				// the life of the application. The application automatically 
				// deletes the view when the application is closed.
				//global_pOW2View->Create(NULL, _T("AnyWindowName"), WS_CHILD, rect, m_pMainWnd, viewID, &myCreateContext);
				CWnd* pParentWnd = CWnd::FromHandle(global_hwnd);
				global_pOW2View->Create(NULL, _T("AnyWindowName"), WS_CHILD, rect, pParentWnd, viewID, &myCreateContext);
				*/
				global_pOW2View->OnCreate(NULL);
				if(global_pOW2Doc!=NULL && global_pOW2View!=NULL) 
				{
					bool bresult=global_pOW2Doc->OnOpenDocument((global_txtfilenames[global_imageid]).c_str());
					// When a document template creates a view, the WM_INITIALUPDATE 
					// message is sent automatically. However, this code must 
					// explicitly send the message, as follows.
					//global_pOW2View->SendMessage(WM_INITIALUPDATE, 0, 0);
					if(bresult) 
					{
						global_pOW2View->OnInitialUpdate();
						//global_pOW2View->SetZoomFactor(4.0); //spi 2014 note, inactive for now
						//global_pOW2View->SetZoomMode(OW_ZOOMNORMAL); 
						// crashed oifiilib.dll
						/*
						global_pOW2View->SetZoomMode(2); 
						global_pOW2View->ZoomPercent(400); //COWView
						//global_pOW2View->OnZoomZoomnormal(); 
						*/

						InvalidateRect(global_hwnd, NULL, false);
						global_pOW2Doc->OnAnalysisTextureextraction();
						//global_pOW2View->m_pViewDataSettings->bDrawPointset = TRUE;
						global_pOW2View->m_pViewDataSettings->bDrawPointset = FALSE;
						global_pOW2View->m_pViewDataSettings->bDrawVoronoiset = TRUE;
						InvalidateRect(global_hwnd, NULL, false);
					}
				}
				//trigger WM_PAINT
				//InvalidateRect(global_hwnd, NULL, false);
				global_imageid++;

		}
		Sleep((int)(global_sleeptimepervoronoi_sec*1000));
		//StatusAddText(global_line.c_str());



		nowstamp_ms = GetTickCount();
	}
	PostMessage(global_hwnd, WM_DESTROY, 0, 0);
}




PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	global_startstamp_ms = GetTickCount();

	//LPWSTR *szArgList;
	LPSTR *szArgList;
	int nArgs;

	//szArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	if(nArgs>1)
	{
		global_imagefolder = szArgList[1];
	}
	if(nArgs>2)
	{
		global_duration_sec = atof(szArgList[2]);
	}
	if(nArgs>3)
	{
		global_sleeptimepervoronoi_sec = atof(szArgList[3]);
	}
	if(nArgs>4)
	{
		global_x = atoi(szArgList[4]);
	}
	if(nArgs>5)
	{
		global_y = atoi(szArgList[5]);
	}
	if(nArgs>6)
	{
		global_xwidth = atoi(szArgList[6]);
	}
	if(nArgs>7)
	{
		global_yheight = atoi(szArgList[7]);
	}
	if(nArgs>8)
	{
		global_alpha = atoi(szArgList[8]);
	}
	if(nArgs>9)
	{
		global_titlebardisplay = atoi(szArgList[9]);
	}
	if(nArgs>10)
	{
		global_menubardisplay = atoi(szArgList[10]);
	}
	if(nArgs>11)
	{
		global_acceleratoractive = atoi(szArgList[11]);
	}
	LocalFree(szArgList);


	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SPIVORONOIWIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	if(global_acceleratoractive)
	{
		hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPIVORONOIWIN32));
	}
	else
	{
		hAccelTable = NULL;
	}
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPIVORONOIWIN32));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);

	if(global_menubardisplay)
	{
		wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SPIVORONOIWIN32);
	}
	else
	{
		wcex.lpszMenuName = NULL; //no menu
	}
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

	//global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
	//global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"Segoe Script");

	if(global_titlebardisplay)
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, //original with WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	else
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_VISIBLE, //no WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	if (!hWnd)
	{
		return FALSE;
	}
	global_hwnd = hWnd;

	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	global_timer=timeSetEvent(100,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		{
			//0) execute cmd line to get all folder's .bmp filenames
			string quote = "\"";
			string pathfilter;
			string path=global_imagefolder;
			pathfilter = path + "\\*.bmp";
			//pathfilter = path + "\\*.jpg";
			string systemcommand;
			//systemcommand = "DIR " + quote + pathfilter + quote + "/B /O:N > wsic_filenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
			systemcommand = "DIR " + quote + pathfilter + quote + "/B /S /O:N > spivoronoi_filenames.txt"; // /S for adding path into "wsic_filenames.txt"
			system(systemcommand.c_str());

			//2) load in all "spiss_filenames.txt" file
			//vector<string> global_txtfilenames;
			ifstream ifs("spivoronoi_filenames.txt");
			string temp;
			while(getline(ifs,temp))
			{
				//txtfilenames.push_back(path + "\\" + temp);
				global_txtfilenames.push_back(temp);
			}

			
		}
		break;
	case WM_SIZE:
		{
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);

			global_imagewidth = rcClient.right - 0;
			global_imageheight = rcClient.bottom - 0; 
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if(global_pOW2Doc!=NULL && global_pOW2View!=NULL)
		{
			CDC* pDC = CDC::FromHandle(hdc);
			global_pOW2View->OnDraw(pDC);
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		if (global_timer) timeKillEvent(global_timer);
		if(global_pOW2Doc) delete global_pOW2Doc;
		if(global_pOW2View) delete global_pOW2View;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
