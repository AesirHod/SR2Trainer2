#ifndef INC_PRIMEMOVER_H
#define INC_PRIMEMOVER_H

#include <windows.h>
#include <commctrl.h>
#include "Types.h"
#include "process.h"

//#define SOUL_REAVER_FUNCTIONS (1)

enum ControlID
{
	SR2_DebugOn = 1, SR2_DbgOnKey, SR2_DebugOff, SR2_DbgOffKey,
	SR2_SubWin,
#ifdef SOUL_REAVER_FUNCTIONS
	PM_SoulReaverObjects,
#endif
};

struct SR_Button
{
	char* text;
	int id;
};

struct SR_Combo
{
	int id;
	u_char defKey;
};

struct SR_Key
{
	int vk;
	char* name;
};

struct SR_MenuItem
{
	u_int m_uType;
	u_int m_uArg1;
	u_int m_uArg2;
	u_int m_uText;
	u_int m_uAddress1;
	u_int m_uAddress2;
};

class SR2Trainer2
{
public:

	static void Initialise(HINSTANCE hInst);

	static LRESULT APIENTRY MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT APIENTRY MemWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static WPARAM ButtonFont();

	static void OnMenu(WPARAM);
	static void CreateHKComboCol(SR_Combo c, HWND hWnd);
	static void CreateKeyStrings();
	static void CreateMainWindow(int x, int y, int w, int h);
	static void CreateMemWindow(HWND hParent, int x, int y, int w, int h);
	static void CreateButtonCol(SR_Button* b, HWND hParent, int noOfButtons, int x, int y, int w, int h);
	static void CreateComboCol(SR_Combo* c, HWND hParent, int noOfButtons, int x, int y, int w, int h);

#ifdef SOUL_REAVER_FUNCTIONS
	static void CreateSRObjectList(ControlID id, HWND hParent, int x, int y, int w, int h);
#endif

	static bool SetUpProcess();
	static void CleanUp();
	static void FindMe();

	static void WriteMem(HWND hWnd);
	static void DebugOn();
	static void DebugOff();

	static HINSTANCE s_hInst;

	static HCURSOR* s_hSRCursor;
	static WPARAM s_hButtonFont;

	static HWND s_hMainWin;
	static HWND s_hMemWin;

	static CProcess s_xProcess;

	static SR_Key s_axKeys[];
	static SR_Button s_axButtons[];
	static SR_Combo s_axComboBoxes[];
	static u_char SR2_DbgOn;
	static u_char SR2_DbgOff;

	static u_int s_uStandardMenu;
	static u_int s_uRazielMenu;
	static u_int s_uCameraMenu;
	static u_int s_uStatsMenu;
	static u_int s_uSaveMenu;
	static u_int s_uCheatsMenu;
	static u_int s_uCombatCamMenu;
	static u_int s_uEndOfMenu;

	static u_int s_uStandardMenuText;
	static u_int s_uRazielMenuText;
	static u_int s_uCameraMenuText;

	static u_int s_uShortShortStats;

	static u_int s_uTheCamera;
	static u_int s_uCAMERA_SetInstanceFocus;
	static u_int s_uINSTANCE_Query;
	static u_int s_uINSTANCE_InPlane;

	static u_int s_uAllocated;
	static u_int s_uMemTracker;

	static u_int s_uSReavr;

	static bool s_bInitialised;
};

#endif