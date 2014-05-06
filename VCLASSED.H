#define MNU_PopupList   1

#define DLG_About       2
#define DLG_Register    3
#define DLG_CreateObj   4


#define DLGCRE_Dest     201
#define DLGCRE_Parm     203
#define DLGCRE_Title    200
#define DLGCRE_Check    110

#define BTN_Browse      130

#define BTN_About       120
#define BTN_Register    121
#define BTN_Dereg       122
#define BTN_Create      123
#define BTN_Refresh     124
#define BTN_Sorted      126

#define IN_Class        101
#define IN_Dll          102


#define ST_Info         130
#define ST_Count        131

#define DID_List        104

typedef struct _CRecord { ULONG cbSize; PSZ data; } CRecord;

MRESULT EXPENTRY RegisterDlgProc (HWND wnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY CreateDlgProc (HWND wnd,ULONG msg,MPARAM mp1,MPARAM mp2);
MRESULT EXPENTRY TemplateDlgProc (HWND wnd,ULONG msg,MPARAM mp1,MPARAM mp2);

