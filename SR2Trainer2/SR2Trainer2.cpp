// CProcess class based on code by Jared Bruni http://www.lostsidedead.com/gameprog

#include <stdio.h>
#include <iostream>
#include <sstream>

#include "SR2Trainer2.h"
#include "resource.h"

// Poke every 500 milliseconds
#define TIMER_INTERRUPT 500
#define ID_TIMER 1

HINSTANCE SR2Trainer2::s_hInst;

HCURSOR* SR2Trainer2::s_hSRCursor;
WPARAM SR2Trainer2::s_hButtonFont = SR2Trainer2::ButtonFont();

HWND SR2Trainer2::s_hMainWin;
HWND SR2Trainer2::s_hMemWin;

CProcess SR2Trainer2::s_xProcess("Soul Reaver 2");

SR_Key SR2Trainer2::s_axKeys[107];

SR_Button SR2Trainer2::s_axButtons[] =
{
	{"Debug On", SR2_DebugOn},	{"Debug Off", SR2_DebugOff},
	{"Pass Through On", SR2_PassThroughOn},	{"Pass Through Off", SR2_PassThroughOff},
};

SR_Combo SR2Trainer2::s_axComboBoxes[] =
{
	{SR2_DbgOnKey, VK_F11},	{SR2_DbgOffKey, VK_F12},
	{SR2_PassThroughOnKey, VK_F9},	{SR2_PassThroughOffKey, VK_F10},
};

u_char SR2Trainer2::SR2_DbgOn = VK_F11;
u_char SR2Trainer2::SR2_DbgOff = VK_F12;
u_char SR2Trainer2::SR2_PassOn = VK_F9;
u_char SR2Trainer2::SR2_PassOff = VK_F10;

u_int SR2Trainer2::s_uStandardMenu = 0x00573810;
u_int SR2Trainer2::s_uRazielMenu = 0x005735D0;
u_int SR2Trainer2::s_uCameraMenu = 0x00573708;
u_int SR2Trainer2::s_uStatsMenu = 0x005738D0;
u_int SR2Trainer2::s_uSaveMenu = 0x00573948;
u_int SR2Trainer2::s_uCheatsMenu = 0x00573ED0;
u_int SR2Trainer2::s_uCombatCamMenu = 0x00574230;
u_int SR2Trainer2::s_uEndOfMenu = 0x005935C8;

u_int SR2Trainer2::s_uStandardMenuText = 0x005754EC;
u_int SR2Trainer2::s_uRazielMenuText = 0x00574D60;
u_int SR2Trainer2::s_uCameraMenuText = 0x00574A08;

u_int SR2Trainer2::s_uShortShortStats = 0x00702D40;

u_int SR2Trainer2::s_uTheCamera = 0x00735340;
u_int SR2Trainer2::s_uCAMERA_SetInstanceFocus = 0x0048E450;
u_int SR2Trainer2::s_uINSTANCE_Query = 0x004B47D0;
u_int SR2Trainer2::s_uINSTANCE_InPlane = 0x004B48A0;

u_int SR2Trainer2::s_uAllocated = 0x00000000;
u_int SR2Trainer2::s_uMemTracker = 0x00000000;

u_int SR2Trainer2::s_uSReavr = 0x00000000;

bool SR2Trainer2::s_bInitialised = false;

int APIENTRY WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR line,int CmdShow)
{
	SR2Trainer2::Initialise(hInst);

	MSG msg;
	while (GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

void SR2Trainer2::Initialise(HINSTANCE hInst)
{
	s_hInst = hInst;

	WNDCLASS wc;

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = CreatePatternBrush(LoadBitmap(s_hInst,MAKEINTRESOURCE(IDB_BITMAP1)));
	wc.hInstance = s_hInst;
	wc.hIcon = LoadIcon(s_hInst,MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(s_hInst,MAKEINTRESOURCE(IDC_CURSOR1));// LoadCursor(NULL,IDC_ARROW);
	s_hSRCursor = &wc.hCursor;
	wc.lpfnWndProc = (WNDPROC) SR2Trainer2::MainWindowProc;
	wc.lpszClassName = "MainWindow";
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);

	wc.lpszMenuName = NULL;
	wc.hIcon = LoadIcon(s_hInst,MAKEINTRESOURCE(IDI_ICON2));

	wc.lpszClassName = "MemWindow";
	wc.lpfnWndProc = (WNDPROC) SR2Trainer2::MemWindowProc;

	RegisterClass(&wc);

	CreateMainWindow(0,0,345,210);
}

LRESULT APIENTRY SR2Trainer2::MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	u_short hw = HIWORD(wParam);
	switch(msg)
	{
		case WM_DESTROY:
		{
			CleanUp();
			KillTimer(hWnd, ID_TIMER);
			PostQuitMessage(0);
			break;
		}
		case WM_CREATE:
		{
			SetTimer(hWnd, ID_TIMER, TIMER_INTERRUPT, NULL);
			break;
		}
		case WM_SIZE:
		{
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			LPRECT rcParent = (LPRECT)(LPARAM)&rcClient;
			MoveWindow(
				s_hMemWin,
				0,
				0,
				rcParent->right,
				rcParent->bottom,
				TRUE
			);
			break;
		}
		case WM_TIMER:
		{
			WriteMem(hWnd);
			break;
		}
		case WM_COMMAND:
		{			
			OnMenu(wParam);
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(hWnd,msg,wParam,lParam);
}

void SR2Trainer2::OnMenu(WPARAM wParam)
{
	switch(wParam)
	{
		case ID_MODE_SR1:
			//ShowWindow(s_hMemWin,SW_SHOW);
			//ShowWindow(s_hSearchWin,SW_HIDE);
			break;
		case ID_MODE_SR2:
			//ShowWindow(s_hMemWin,SW_HIDE);
			//ShowWindow(s_hSearchWin,SW_SHOW);
			break;
		case ID_EXIT_TRAINER:
			SendMessage(s_hMainWin,WM_CLOSE, 0, 0);
			break;
	}
}

LRESULT APIENTRY SR2Trainer2::MemWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	u_short hw = HIWORD(wParam);
    switch (msg)
	{
		case WM_CLOSE:
			ShowWindow(hWnd,SW_HIDE);
			break;
		case WM_CREATE:
	    	break;
		case WM_COMMAND:
		{
			switch (hw)
			{
				case BN_CLICKED:
				{
					if ((LOWORD(wParam)) == SR2_DebugOn)
					{
						DebugOn();
					}

					if ((LOWORD(wParam)) == SR2_DebugOff)
					{
						DebugOff();
					}

					if ((LOWORD(wParam)) == SR2_PassThroughOn)
					{
						PassThroughOn();
					}

					if ((LOWORD(wParam)) == SR2_PassThroughOff)
					{
						PassThroughOff();
					}

					break;
				}
				case CBN_DROPDOWN:
				{
#ifdef SOUL_REAVER_FUNCTIONS
					if ((LOWORD(wParam)) == PM_SoulReaverObjects)
					{
						if (strcmp(s_xProcess.GetWindowTitle(), "Soul Reaver 2") == 0)
						{
							u_char* paucCurrentAddress = (u_char*)0x007020e0;
							SendDlgItemMessage(hWnd, PM_SoulReaverObjects, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
							bool bFoundObject = false;

							do
							{
								u_char* paucCurrentObject = s_xProcess.Read(paucCurrentAddress, 0x28);
								bFoundObject = (strlen((char*)paucCurrentObject) > 0);
								if (bFoundObject)
								{
									SendDlgItemMessage(hWnd, PM_SoulReaverObjects, CB_ADDSTRING, (WPARAM)0, (LPARAM)paucCurrentObject);
								}
								delete[] paucCurrentObject;
								paucCurrentAddress += 0x28;
							}
							while (bFoundObject);
						}
					}
#endif
					break;
				}
				case CBN_SELCHANGE:
				{
					if ((LOWORD(wParam))==SR2_DbgOnKey)
					{
						SR2_DbgOn = s_axKeys[SendDlgItemMessage(hWnd, SR2_DbgOnKey, CB_GETCURSEL, (WPARAM)0, (LPARAM)0)].vk;
					}
					
					if ((LOWORD(wParam))==SR2_DbgOffKey)
					{
						SR2_DbgOff = s_axKeys[SendDlgItemMessage(hWnd, SR2_DbgOffKey, CB_GETCURSEL, (WPARAM)0, (LPARAM)0)].vk;
					}

#ifdef SOUL_REAVER_FUNCTIONS
					//if ((LOWORD(wParam))==PM_Processes)
					//{
					//	int iIndex = SendDlgItemMessage(hWnd,PM_Processes,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
					//	CProcessDesc* pxDesc = s_xProcessList.m_xList.FindIndex(iIndex);
					//	if (pxDesc)
					//	{
					//		s_xProcess.FindProcess(pxDesc->szName);
					//	}
					//}
#endif
					break;
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(hWnd,msg,wParam,lParam);
}

WPARAM SR2Trainer2::ButtonFont()
{
	LOGFONT lf;
	HFONT hFont = 0;
	GetObject(hFont, sizeof(LOGFONT), &lf);
	strcpy(lf.lfFaceName,"Microsoft Sans Serif");
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfItalic = 0;
	lf.lfWeight = 100;
	lf.lfHeight = 14;
	lf.lfWidth = 5;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	// lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	hFont = CreateFontIndirect(&lf);
	return (WPARAM) hFont;
}

void SR2Trainer2::CreateKeyStrings()
{
	int	kName =	0;
	char* str = new char[20];
	UINT nScanCode;
	for	(int kCode = 0;	kCode <	255; kCode++)
	{
		nScanCode =	MapVirtualKeyEx(kCode, 0, GetKeyboardLayout(0));
		if ((kCode >= 162 && kCode <= 165) || (kCode>=33 && kCode <= 46))
		{
			nScanCode |= 0x100;
		}

		switch (kCode)
		{
			case VK_CLEAR:
			case VK_RETURN:
			case VK_LWIN:
			case VK_RWIN:
			case VK_APPS:
			case VK_SEPARATOR:
			case VK_DIVIDE:
				nScanCode |= 0x100;
		}

		GetKeyNameText(nScanCode <<	16,	str, 20);

		if (kCode>=172 && kCode<=183) *str=0;
		if (kCode==VK_SNAPSHOT)	strcpy(str,	"PRINT SCREEN");
		if (kCode==VK_LSHIFT)	strcpy(str,	"LEFT SHIFT");
		if (kCode==VK_LCONTROL)	strcpy(str,	"LEFT CTRL");
		if (kCode==VK_LMENU)	strcpy(str,	"LEFT ALT");

		if (str==0 || *str==0) continue;

		s_axKeys[kName].name = str;
		s_axKeys[kName].vk = kCode;
		str = new char[20];
		kName++;
	}
}

void SR2Trainer2::CreateHKComboCol(SR_Combo c, HWND hWnd)
{
	for (int j=0; j<107; j++)
	{
		SendDlgItemMessage(hWnd,c.id,CB_ADDSTRING,(WPARAM)0,(LPARAM)s_axKeys[j].name);
		if (s_axKeys[j].vk == c.defKey)
		{
			SendDlgItemMessage(hWnd,c.id,CB_SETCURSEL,(WPARAM)j,(LPARAM)0);
		}
	}
}

void SR2Trainer2::CreateMainWindow(int x, int y, int w, int h)
{
	s_hMainWin = CreateWindow("MainWindow", "Soul Reaver 2 Trainer v2.0", WS_OVERLAPPEDWINDOW, x, y, w, h, 0, 0, s_hInst, 0);

	CreateMemWindow(s_hMainWin, 100, 100, 100, 100);

	ShowWindow(s_hMainWin, SW_SHOW);
	UpdateWindow(s_hMainWin);
}

void SR2Trainer2::CreateMemWindow(HWND hParent, int x, int y, int w, int h)
{
	s_hMemWin = CreateWindowEx(
		WS_EX_CLIENTEDGE, "MemWindow", "", WS_VISIBLE | WS_CHILD | WS_MAXIMIZE,
		x, y, w, h, hParent, (HMENU) SR2_SubWin, s_hInst, 0
	);

	CreateButtonCol(s_axButtons, s_hMemWin, 4, 50, 30, 100, 20);
	CreateKeyStrings();
	CreateComboCol(s_axComboBoxes, s_hMemWin, 4, 180, 30, 100, 200);

#ifdef SOUL_REAVER_FUNCTIONS
	CreateSRObjectList(PM_SoulReaverObjects, s_hMemWin, 432, 10, 160, 200);
#endif

	ShowWindow(s_hMemWin, SW_SHOW); // SW_MAXIMIZE);
}

void SR2Trainer2::CreateButtonCol(SR_Button* b, HWND hParent, int noOfButtons, int x, int y, int w, int h)
{
	HWND hButton;
	for (int i = 0; i < noOfButtons; i++)
	{
		hButton = CreateWindowEx(
			0, "Button", b[i].text, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
			x, y, w, h, hParent, (HMENU)b[i].id, s_hInst, 0
		);

		SendDlgItemMessage(hParent, b[i].id,WM_SETFONT,s_hButtonFont,FALSE);
		SetClassLong(hButton, GCL_HCURSOR, (long) *s_hSRCursor);
		y += 25;
	}
}

void SR2Trainer2::CreateComboCol(SR_Combo* c, HWND hParent, int noOfComboBoxes, int x, int y, int w, int h)
{
	HWND theCombo;
	for (int i = 0; i < noOfComboBoxes; i++)
	{
		theCombo = CreateWindowEx(
			0, "ComboBox","", WS_CHILD | WS_VISIBLE | 0x00010003 | CBS_UPPERCASE | WS_VSCROLL,
			x, y, w, h, hParent, (HMENU) c[i].id, s_hInst, 0
		);

		SendDlgItemMessage(hParent, c[i].id, WM_SETFONT, s_hButtonFont, FALSE);
		SetClassLong(theCombo, GCL_HCURSOR, (long) *s_hSRCursor);

		y += 25;
		CreateHKComboCol(c[i], hParent);
	}
}

#ifdef SOUL_REAVER_FUNCTIONS
void SR2Trainer2::CreateSRObjectList(ControlID id, HWND hParent, int x, int y, int w, int h)
{
	HWND hCombo = CreateWindowEx(
		0, "ComboBox","", WS_CHILD | WS_VISIBLE | 0x00010003 | WS_VSCROLL,
		x, y, w, h, hParent, (HMENU) id, s_hInst, 0
	);

	SendDlgItemMessage(hParent, id, WM_SETFONT, s_hButtonFont, FALSE);
	SetClassLong(hCombo, GCL_HCURSOR, (long) *s_hSRCursor);
}
#endif

bool SR2Trainer2::SetUpProcess()
{
	s_xProcess.FindProcess();

	if (s_xProcess.WasUpdated())
	{
		s_uAllocated = 0x00000000;
		s_uMemTracker = 0x00000000;
		s_bInitialised = false;
	}

	if (s_xProcess.IsRunning())
	{
		return true;
	}

	return false;
}

void SR2Trainer2::CleanUp()
{
}

void SR2Trainer2::FindMe()
{
	u_int *address;
	__asm
	{
		mov eax, ebp; // Should be esp for stack pointer?
		add eax, 0x04;
		mov [address],eax;
		mov eax, 00000001;
	}
	char* errInfo = new char[50];
	sprintf(errInfo,"Made it to address 0x%08X without crashing!", *address);
	MessageBox(NULL, errInfo, "ERROR", MB_ICONERROR | MB_OK);
}

void SR2Trainer2::WriteMem(HWND hWnd)
{
	// Debug On
	if (s_xProcess.IsKeyPressed(SR2_DbgOn))
	{
		DebugOn();
	}

	// Debug Off
	if (s_xProcess.IsKeyPressed(SR2_DbgOff))
	{
		DebugOff();
	}

	// Pass Through On
	if (s_xProcess.IsKeyPressed(SR2_PassOn))
	{
		PassThroughOn();
	}

	// Pass Through Off
	if (s_xProcess.IsKeyPressed(SR2_PassOff))
	{
		PassThroughOff();
	}
}

void SR2Trainer2::DebugOn()
{
	if (SetUpProcess())
	{
		if (!s_bInitialised)
		{
			s_bInitialised = true;
			s_uAllocated = s_xProcess.Alloc(2048);
			s_uMemTracker = s_uAllocated;

			SR_MenuItem xMainMenuItem =	{ 8, 0, 0, s_uStandardMenuText, s_uStandardMenu, 0x00000000 };

			SR_MenuItem* pxMoveMenu = NULL;

			u_int uNewStatsMenu = 0x00000000;
			u_int uNewSaveMenu = 0x00000000;
			u_int uNewCombatCamMenu = 0x00000000;
			
			pxMoveMenu = (SR_MenuItem*) s_xProcess.Read((BYTE*)s_uStatsMenu, sizeof(SR_MenuItem[6]));
			s_xProcess.Write(s_uMemTracker, (BYTE*) pxMoveMenu, sizeof(SR_MenuItem[6]));
			uNewStatsMenu = s_uMemTracker;
			s_uMemTracker += s_xProcess.SizeOfLast();
			delete[] pxMoveMenu;

			pxMoveMenu = (SR_MenuItem*) s_xProcess.Read((BYTE*)s_uSaveMenu, sizeof(SR_MenuItem[4]));
			s_xProcess.Write(s_uMemTracker, (BYTE*) pxMoveMenu, sizeof(SR_MenuItem[4]));
			uNewSaveMenu = s_uMemTracker;
			s_uMemTracker += s_xProcess.SizeOfLast();
			delete[] pxMoveMenu;

			// Events menu has 7 items.  May need to move that too.

			uNewCombatCamMenu = s_uCombatCamMenu - sizeof(SR_MenuItem[1]);
			s_xProcess.Write(uNewCombatCamMenu, (BYTE*) &xMainMenuItem, sizeof(SR_MenuItem[1]));
			
			u_int uShortStatsText = s_xProcess.WriteString(s_uMemTracker, "Short Stats");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uGhostCamText = s_xProcess.WriteString(s_uMemTracker, "Ghost Cam");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uStatsMenuText = s_xProcess.WriteString(s_uMemTracker, "Stats Menu...");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uSaveMenuText = s_xProcess.WriteString(s_uMemTracker, "Save Menu...");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uEventMenuText = s_xProcess.WriteString(s_uMemTracker, "Debug Events Menu...");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uFogMenuText = s_xProcess.WriteString(s_uMemTracker, "Fog Menu...");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uGoodiesMenuText = s_xProcess.WriteString(s_uMemTracker, "Goodies Menu...");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uDrawFlagsMenuText = s_xProcess.WriteString(s_uMemTracker, "Draw Flags Menu...");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uCheatsMenuText = s_xProcess.WriteString(s_uMemTracker, "Cheats Menu...");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uCombatCamMenuText = s_xProcess.WriteString(s_uMemTracker, "Combat Cam Menu...");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uAIMenuText = s_xProcess.WriteString(s_uMemTracker, "AI/Script Menu...");
			s_uMemTracker += s_xProcess.SizeOfLast();
			u_int uPossessText = s_xProcess.WriteString(s_uMemTracker, "Possess");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_uMemTracker++;

			u_int uPossessed = s_uMemTracker;
			s_xProcess.Write(s_uMemTracker, 0x00000000u);
			s_uMemTracker += s_xProcess.SizeOfLast();

			u_int uPossessFunction = s_uMemTracker;
			s_xProcess.WriteXString(s_uMemTracker, "55 53 50 8B 2D");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.Write(s_uMemTracker, uPossessed);
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "85 ED 75 06 8B 2D 90 2C 70 00 8B 6D 08 85 ED 75 06");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "8B 2D 24 2F 70 00 85 ED 74 44");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "68 01 00 00 00 55 E8");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteDiff(s_uMemTracker, s_uINSTANCE_Query);
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "5D 83 C4 04");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "25 03 05 00 00 85 C0 74 D7"); // and eax,00000503 on PC but and eax,00000103 on PS2
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "66 8B 0D 82 2C 70 00 51");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "55 E8");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteDiff(s_uMemTracker, s_uINSTANCE_InPlane);
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "83 C4 08 85 C0 74 C2");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "55 68 40 53 73 00 E8");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteDiff(s_uMemTracker, s_uCAMERA_SetInstanceFocus);
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "83 C4 04 5D");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "89 2D");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.Write(s_uMemTracker, uPossessed);
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "31 C0 58 5B 5D C3");
			s_uMemTracker += s_xProcess.SizeOfLast();

			SR_MenuItem axNewItems[] =
			{
				{0,		0,	0,	uShortStatsText,	s_uShortShortStats,	0x00000004},
				{8,		0,	0,	s_uCameraMenuText,	s_uCameraMenu,		0x00000000},
				{8,		0,	0,	uAIMenuText,		0x005742A8,			0x00000000},
				{8,		0,	0,	s_uRazielMenuText,	s_uRazielMenu,		0x00000000},
				//{0,		0,	0,	uGhostCamText,		s_uShortShortStats,	0x00010000},
				{6,		0,	0,	uPossessText,		uPossessFunction,	0x00000000}, // Temp
				{8,		0,	0,	uGoodiesMenuText,	0x00573B10,			0x00000000},
				// Sound
				// Debug CD Loads (Crashes and pointless with no CD.)
				{8,		0,	0,	uSaveMenuText,		uNewSaveMenu,		0x00000000},
				{8,		0,	0,	uEventMenuText,		0x005739A8,			0x00000000},
				{8,		0,	0,	uCheatsMenuText,	s_uCheatsMenu,		0x00000000},
				{8,		0,	0,	uFogMenuText,		0x00573A50,			0x00000000}, // Fog menu was part of the goodies menu.
				//{8,		0,	0,	uStatsMenuText,		uNewStatsMenu,		0x00000000},
				{8,		0,	0,	uDrawFlagsMenuText,	0x00573C90,			0x00000000},
				{8,		0,	0,	uCombatCamMenuText,	uNewCombatCamMenu,	0x00000000},
				{11,	0,	0,	s_uEndOfMenu,		s_uShortShortStats,	0x00000000},
			};

			s_xProcess.Write(s_uStandardMenu + sizeof(SR_MenuItem[4]), (BYTE*) axNewItems, sizeof(axNewItems));

			pxMoveMenu = (SR_MenuItem*) s_xProcess.Read((BYTE*)s_uCheatsMenu, sizeof(SR_MenuItem[3]));
			s_xProcess.Write(s_uCheatsMenu, (BYTE*) &xMainMenuItem, sizeof(SR_MenuItem[1]));
			s_xProcess.Write(s_uCheatsMenu + sizeof(SR_MenuItem[1]), (BYTE*) pxMoveMenu, sizeof(SR_MenuItem[3]));
			delete[] pxMoveMenu;

			// 004A1F1C = colour manipulation nearby

			u_int auColours[12] =
			{
				0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
				0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
			};

			s_xProcess.Write(s_uMemTracker, auColours, 12);
			s_uMemTracker += s_xProcess.SizeOfLast();

			u_int uCustomGlyphIsGlyphSet = s_uMemTracker;
			s_xProcess.WriteXString(s_uMemTracker, "83 FE 03 7C 06 B8 00 00 00 00 C3 B8 01 00 00 00 C3");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "E8");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteDiff(s_uMemTracker, 0x004B1A20);
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "C3");
			s_uMemTracker += s_xProcess.SizeOfLast();

			s_xProcess.WriteXString(0x004B2439, "E8");
			s_xProcess.WriteDiff(0x004B243A, uCustomGlyphIsGlyphSet);

			// Number to draw and spacing
			s_xProcess.WriteXString(0x004B24BF, "81 C3 AA 3E 00 00 83 FE 0C");

			// Initial position
			s_xProcess.WriteXString(0x004B1D5A, "B8 55 01 00 00");

			// Clockwise
			s_xProcess.WriteXString(0x004B1EC2, "E8");
			s_xProcess.WriteDiff(0x004B1EC3, s_uMemTracker);
			s_xProcess.WriteXString(0x004B1EC7, "90 90");
			s_xProcess.WriteXString(0x004B1EDC, "66 C7 86 90 00 00 00 0C 00");
			s_xProcess.WriteXString(0x004B1EF8, "90 90 90 90 90 90 90");

			s_xProcess.WriteXString(s_uMemTracker, "66 8B 86 90 00 00 00 BA 55 01 00 00 0F AF D0 66 FF 8E 90 00 00 00 81 EA AA 02 00 00 C3");
			s_uMemTracker += s_xProcess.SizeOfLast();

			// Anti-clockwise
			s_xProcess.WriteXString(0x004B1FE2, "E8");
			s_xProcess.WriteDiff(0x004B1FE3, s_uMemTracker);
			s_xProcess.WriteXString(0x004B1FE7, "90 90");
			s_xProcess.WriteXString(0x004B1FF5, "66 3D 0C 00");
			s_xProcess.WriteXString(0x004B201C, "90 90 90 90 90 90 90");

			s_xProcess.WriteXString(s_uMemTracker, "66 8B 86 90 00 00 00 BA 55 01 00 00 0F AF D0 66 FF 86 90 00 00 00 C3");
			s_uMemTracker += s_xProcess.SizeOfLast();

			// Rotation speed
			s_xProcess.Write(0x004E389C, 48.0f);

			

			// Water Reaver FX set
			u_int uWaterReaverEffectSet = s_uMemTracker;
			s_xProcess.WriteXString(s_uMemTracker, "20 01 00 00 1A 00 00 00 20 01 00 00 25 00 00 00 20 01 00 00 25 01 00 00 20 01 00 00 25 02 00 00 20 01 00 00 25 03 00 00 20 01 00 00 25 04 00 00 20 01 00 00 25 05 00 00 20 01 00 00 25 06 00 00 20 01 00 00 25 07 00 00 00 FF 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Earth Reaver FX set
			u_int uEarthReaverEffectSet = s_uMemTracker;
			// Insert "20 01 00 00 0E 06 00 00" at byte 8 for lightning bolt.
			s_xProcess.WriteXString(s_uMemTracker, "20 01 00 00 1A 00 00 00 20 01 00 00 25 08 00 00 20 01 00 00 25 09 00 00 20 01 00 00 25 0A 00 00 20 01 00 00 25 0B 00 00 20 01 00 00 25 0C 00 00 20 01 00 00 25 0D 00 00 00 FF 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();

			// Earth Reaver trail colour
			//s_xProcess.Write(0x05791A44, 0x800E4824u);

			// CustomGFXO
			u_int uCustomGFXO = s_uMemTracker;
			s_xProcess.WriteXString(s_uMemTracker, "79 00 24 00 01 00 F8 36 12 00 18 37 2A 00 E8 39 00 00 58 48 00 00 58 48 2A 00 58 48 00 00 00 49 00 00 00 49");
			s_uMemTracker += s_xProcess.SizeOfLast();

			//                                      00                                              10                                              20          24                                  30                                              40                                              50                                              60                                              70

			// Water Reaver FX #1
			s_xProcess.WriteXString(s_uMemTracker, "28 00 01 04 0F 00 03 81 00 00 48 42 00 00 48 42 00 00 48 42 00 00 00 00 00 00 00 00 00 00 00 00 0F 25 3F 80 2D B4 DC 80 04 00 00 00 FF FF 1A 00 F1 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 08 81 00 01 0A D7 A3 3C 00 00 00 00 00 00 80 3F 03 00 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Water Reaver FX #2
			s_xProcess.WriteXString(s_uMemTracker, "09 00 01 00 23 00 03 81 00 00 40 40 00 00 40 40 00 00 80 C0 00 00 00 00 00 00 00 00 00 00 00 00 06 11 1C 80 09 21 32 80 03 01 01 00 FF FF 23 00 01 00 00 00 00 00 00 00 00 00 00 10 00 00 E9 03 02 00 00 00 00 00 00 00 00 00 0D 00 12 00 08 00 05 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 03 00 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Water Reaver FX #3
			s_xProcess.WriteXString(s_uMemTracker, "1E 00 01 00 0F 00 08 81 00 00 00 43 00 00 00 43 00 00 00 43 00 00 00 00 00 00 00 00 00 00 00 00 03 09 0E 80 05 12 1C 80 02 01 01 00 FF FF 28 00 FE 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 02 00 00 00 00 00 00 00 00 00 00 00 09 00 00 00 00 00 00 00 08 81 00 01 8F C2 75 3C 00 00 00 00 00 00 80 3F 03 00 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Water Reaver FX #4
			s_xProcess.WriteXString(s_uMemTracker, "28 00 01 00 12 00 03 81 00 00 00 43 00 00 00 43 00 00 00 43 00 00 00 00 00 00 00 00 00 00 00 00 02 06 02 80 0F 20 26 80 02 01 01 00 FF FF 28 00 01 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 00 00 00 00 00 00 00 08 81 00 01 0A D7 A3 3C 00 00 00 00 00 00 80 3F 03 00 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Water Reaver FX #5
			s_xProcess.WriteXString(s_uMemTracker, "19 00 01 00 19 00 21 81 00 00 00 00 00 00 00 00 00 00 80 C0 00 00 00 00 00 00 00 00 00 00 00 00 06 11 1C 80 09 21 32 80 02 01 01 FF FF FF 16 00 01 00 00 00 00 00 00 00 00 00 00 10 00 01 E9 03 00 00 00 00 00 00 00 00 00 00 08 00 1E 00 0A 00 04 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 21 00 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Water Reaver FX #6
			s_xProcess.WriteXString(s_uMemTracker, "0A 00 01 00 05 00 21 81 00 00 00 00 00 00 00 00 00 00 80 C0 00 00 00 00 00 00 00 00 00 00 00 00 03 09 0E 80 05 12 1C 80 02 01 01 FF FF FF 13 00 01 00 00 00 00 00 00 00 00 00 00 10 00 01 E9 03 00 00 00 00 00 00 00 00 00 00 03 00 14 00 04 00 03 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 21 00 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Water Reaver FX #7
			s_xProcess.WriteXString(s_uMemTracker, "06 00 01 00 19 00 20 81 00 00 00 00 00 00 00 00 00 00 80 C0 00 00 00 00 00 00 00 00 00 00 00 00 06 11 1C 80 09 21 32 80 02 01 01 FF FF FF 16 00 01 00 00 00 00 00 00 00 00 00 00 10 00 01 E9 03 00 00 00 00 00 00 00 00 00 00 08 00 1E 00 0A 00 04 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 20 00 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Water Reaver FX #8
			s_xProcess.WriteXString(s_uMemTracker, "04 00 01 00 05 00 20 81 00 00 00 00 00 00 00 00 00 00 80 C0 00 00 00 00 00 00 00 00 00 00 00 00 03 09 0E 80 05 12 1C 80 02 01 01 FF FF FF 13 00 01 00 00 00 00 00 00 00 00 00 00 10 00 01 E9 03 00 00 00 00 00 00 00 00 00 00 03 00 14 00 04 00 03 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 20 00 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();

			//                                      00                                              10                                              20          24                                  30                                              40                                              50                                              60                                              70

			// Earth Reaver FX #1
			s_xProcess.WriteXString(s_uMemTracker, "22 00 01 04 00 00 03 81 00 00 00 43 00 00 00 43 00 00 00 43 00 00 00 00 00 00 00 00 00 00 00 00 24 48 0E 80 24 48 0E 80 04 00 00 00 FF FF 21 00 F6 03 00 00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 04 00 00 00 08 81 00 01 0A D7 A3 3C 00 00 00 00 00 00 80 3F 03 01 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Earth Reaver FX #2
			s_xProcess.WriteXString(s_uMemTracker, "28 00 01 00 05 00 03 81 00 00 00 00 00 00 00 00 00 00 A0 C0 00 00 00 00 00 00 00 00 00 00 00 00 24 48 0E 80 24 48 0E 80 10 01 01 00 FF FF 2D 00 FE 00 00 00 00 00 00 00 00 00 00 10 00 00 E9 03 00 00 00 00 00 00 00 00 00 00 00 00 06 00 03 00 14 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 03 01 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Earth Reaver FX #3
			s_xProcess.WriteXString(s_uMemTracker, "11 00 01 00 06 00 03 81 00 00 00 00 00 00 00 00 00 00 C0 C0 00 00 00 00 00 00 00 00 00 00 00 00 24 48 0E 80 24 48 0E 80 02 01 01 00 FF FF 2D 00 FF 00 00 00 00 00 00 00 00 00 00 10 00 00 E9 03 00 00 00 00 00 00 00 00 00 00 00 00 03 00 01 00 08 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 03 01 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Earth Reaver FX #4
			s_xProcess.WriteXString(s_uMemTracker, "14 00 01 00 14 00 20 81 00 00 00 00 00 00 00 00 00 00 80 C0 00 00 00 00 00 00 00 00 00 00 00 00 0D 20 07 80 07 0E 04 80 02 01 01 FF FF FF 14 00 01 00 00 00 00 00 00 00 00 00 00 10 00 01 E9 03 00 00 00 00 00 00 00 00 00 00 04 00 3C 00 07 00 02 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 20 01 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Earth Reaver FX #5
			s_xProcess.WriteXString(s_uMemTracker, "08 00 01 00 19 00 21 81 00 00 00 00 00 00 00 00 00 00 80 C0 00 00 00 00 00 00 00 00 00 00 00 00 24 48 0E 80 18 30 0A 80 02 01 01 FF FF FF 14 00 03 00 00 00 00 00 00 00 00 00 00 10 00 01 E9 03 00 00 00 00 00 00 00 00 00 00 04 00 1E 00 0A 00 04 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 21 01 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			// Earth Reaver FX #6
			s_xProcess.WriteXString(s_uMemTracker, "01 00 01 00 1E 00 20 81 00 00 00 00 00 00 00 00 00 00 80 C0 00 00 00 00 00 00 00 00 00 00 00 00 64 FA 0C 49 1E 96 00 7F 07 01 01 FF FF FF 13 00 FD 00 00 00 00 00 00 00 00 00 00 10 00 01 E9 03 00 00 14 00 00 00 00 00 00 00 03 00 06 00 01 00 01 00 00 00 81 81 00 01 00 00 00 00 00 00 00 00 00 00 80 3F 20 01 81 00 00 00 00 80 00 00 FF 00 00 00 00 00");
			s_uMemTracker += s_xProcess.SizeOfLast();

			// FX_StartCustomParticleGen
			u_int uFX_StartCustomParticleGen = s_uMemTracker;
			s_xProcess.WriteXString(s_uMemTracker, "8B 54 24 04 53 55 56 57 33 DB 8B EA B8");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.Write(s_uMemTracker, uCustomGFXO);
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "E9");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteDiff(s_uMemTracker, 0x004A9F8Eu);
			s_uMemTracker += s_xProcess.SizeOfLast();

			// case CustomPFX:
			u_int uCase_CustomPFX = s_uMemTracker;
			s_xProcess.WriteXString(s_uMemTracker, "8B 44 24 1C 8B 4C 24 18 50 33 D2 0F BE 46 02 8A 56 03 51 33 C9 6A 00 8A 4E 01 52 8B 54 24 20 50 51 52 E8");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteDiff(s_uMemTracker, uFX_StartCustomParticleGen);
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.WriteXString(s_uMemTracker, "83 C4 1C 5F 5E 5B C3");
			s_uMemTracker += s_xProcess.SizeOfLast();

			// switch (eEffectType)
			u_int uSwitchEffectType = s_uMemTracker;
			s_xProcess.WriteXString(s_uMemTracker, "1E B3 4A 00 62 B3 4A 00 BF B7 4A 00 B4 B3 4A 00 E2 B3 4A 00 0C B4 4A 00 37 B4 4A 00 BF B7 4A 00 3F B4 4A 00 51 B4 4A 00 63 B4 4A 00 BF B7 4A 00 DA B4 4A 00 02 B5 4A 00 28 B5 4A 00 BF B7 4A 00 44 B5 4A 00 6A B5 4A 00 85 B5 4A 00 A0 B5 4A 00 03 B6 4A 00 24 B6 4A 00 50 B6 4A 00 6E B6 4A 00 91 B6 4A 00 A4 B6 4A 00 CE B6 4A 00 EB B6 4A 00 FE B6 4A 00 12 B7 4A 00 29 B7 4A 00 40 B7 4A 00 57 B7 4A 00 85 B7 4A 00 9B B7 4A 00");
			s_uMemTracker += s_xProcess.SizeOfLast();
			s_xProcess.Write(s_uMemTracker, uCase_CustomPFX);
			s_uMemTracker += s_xProcess.SizeOfLast();

			s_xProcess.Write(0x004AB310, (u_char)0x23);
			s_xProcess.Write(0x004AB31A, uSwitchEffectType);

			u_char* paucCurrentAddress = (u_char*)0x007020e0;
			bool bFoundObject = false;

			do
			{
				u_char* paucCurrentEntry = s_xProcess.Read(paucCurrentAddress, 0x28);
				char* szObjectName = (char*)paucCurrentEntry;
				u_int* puObjectAddress = (u_int*)(paucCurrentEntry + 0x0C);
				bFoundObject = (puObjectAddress && strlen(szObjectName) > 0);
				if (bFoundObject && strcmp("sreavr", szObjectName) == 0)
				{
					s_uSReavr = *puObjectAddress;
					break;
				}
				delete[] paucCurrentEntry;
				paucCurrentAddress += 0x28;
			}
			while (bFoundObject);

			s_xProcess.Write(s_uSReavr + 0x0100, uWaterReaverEffectSet);
			s_xProcess.Write(s_uSReavr + 0x0104, uEarthReaverEffectSet);

			
		}

		s_xProcess.WriteXString(0x0049EA02, "E829010000");
		s_xProcess.WriteXString(0x0049EB32, "C705C452730000007043C3");
		s_xProcess.WriteXString(0x0049EF8B, "E9A2FBFFFF");
		s_xProcess.WriteXString(0x00702E16, "04");
		s_xProcess.WriteXString(0x007352C6, "D0");

		// Short Short Stats 0x04000000 = Short not "short-short" stats.
		// Short Short Stats 0x00000020 = Sound debug output.
	}
}

void SR2Trainer2::PassThroughOn()
{
	if (SetUpProcess())
	{
		s_xProcess.WriteXString(0x0041F380, "C39090");
	}
}

void SR2Trainer2::PassThroughOff()
{
	if (SetUpProcess())
	{
		s_xProcess.WriteXString(0x0041F380, "83EC10");
	}
}

void SwapIntegers(int& iIntA, int& iIntB)
{
	int iIntTemp = iIntA; 
	iIntB = iIntA;
	iIntB = iIntTemp;
}

void X( int* A, unsigned int L )
{
	if( A == NULL ) return ;
	for( int Index = (L - 1) ; Index >= 0 ; Index-- )
	{
		int T = A[Index] ;
		A[Index] = A[L-Index+1] ;
		A[L-Index-1] = T ;
	}
}

void SR2Trainer2::DebugOff()
{
	if (SetUpProcess())
	{
		s_xProcess.WriteXString(0x0049EA02, "E8B8270100");
		s_xProcess.WriteXString(0x004B11C6, "C705C452730000007043");
	}

	//int A = 0;
	//int B = 0;
	//SwapIntegers(A, B);
	//int TestArray[] = { 1, 2, 3, 4, 5 };
	//X(TestArray, 5);
}