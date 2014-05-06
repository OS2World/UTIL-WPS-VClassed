/*  <Install, part of the VClassed package, a PM OS/2 WorkPlace Shell class manager>
    Copyright (C) 1996, 1997, 1998  Daniele Vistalli

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


    You can contact me :
	Daniele Vistalli :
            dvistalli@tin.it
	    virusface@usa.net

*/
#define INCL_WIN
#define INCL_PM
#define INCL_DOSPROCESS
#define INCL_DOSSESMGR
#define INCL_DOSERRORS

#define HF_STDOUT 1

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "install.h"

static BOOL installed = FALSE, installing = FALSE; // Flags to determine program state
static HFILE UnpackPipe; // The pipe from where to read the unpack output
static char programpath[255]; // The execution path

void setButtons(HWND wnd)
{
   if (installing)
    { WinEnableWindow(WinWindowFromID(wnd,BTN_EXIT),FALSE);
      WinEnableWindow(WinWindowFromID(wnd,BTN_INSTALL),FALSE); }
   else
    { WinEnableWindow(WinWindowFromID(wnd,BTN_EXIT),TRUE);
      WinEnableWindow(WinWindowFromID(wnd,BTN_INSTALL),TRUE);
    }

   if (installed)
    { WinEnableWindow(WinWindowFromID(wnd,BTN_EXIT),TRUE);
      WinEnableWindow(WinWindowFromID(wnd,BTN_INSTALL),FALSE); }
   else
    { WinEnableWindow(WinWindowFromID(wnd,BTN_EXIT),TRUE);
      WinEnableWindow(WinWindowFromID(wnd,BTN_INSTALL),TRUE);
    }
}

int PathCreate(const char aPath[])
{ char tmp[240];
  char *c;

  strcpy(tmp,aPath);

  if (DosSetCurrentDir(tmp) != NO_ERROR)
   { c = strrchr(tmp,'\\');
     if (*(c-1) != ':') *c = 0;
     else *(c+1) = 0;
     if (!PathCreate(tmp)) return 0;
     if (DosCreateDir(aPath,NULL) != NO_ERROR) return 0;
   }

  return 1;
}

VOID _System UnpackReader(ULONG parm)
{
   HAB rhab;
   HMQ mhmq;
   CHAR str[100], c, dbg[10];
   int cnt = 0;
   ULONG ulRead;
   int endcnt = 0;

   rhab = WinInitialize(0);
   mhmq = WinCreateMsgQueue(rhab,0);

   str[0] = 0;
   c = 0;

   do { DosRead(UnpackPipe,
                 &c,
                 1,
                 &ulRead);
        if (c != 13) str[cnt++] = c;
        else
          { if (str[0] == 32) endcnt++;
            WinInsertLboxItem(WinWindowFromID((HWND)parm,LB_MSGLIST),LIT_END,str);
            cnt = 0;
            memset(str,0,100);
            DosRead(UnpackPipe,&c,1,&ulRead);
          }
     } while (endcnt != 2);

  DosClose(UnpackPipe);

  WinDestroyMsgQueue(mhmq);
  WinTerminate(rhab);
  DosExit(0,0);
}

VOID _System InstallThread(ULONG parm) // Parm is the dialog handle
{
  HPIPE hReadPipe;
  HFILE hW, hW1;
  CHAR szFailName[CCHMAXPATH], szArg[255], destdir[200];
  ULONG ulSessID    = 0;          /* Session ID returned          */
  PID pid = 0;
  TID tid;
  STARTDATA SData       = {0};

  HFILE hfSave = -1, hfNew = HF_STDOUT;

  HAB rhab;
  HMQ mhmq;

  rhab = WinInitialize(0);
  mhmq = WinCreateMsgQueue(rhab,0);

  WinQueryDlgItemText((HWND)parm,IF_DEST,200,destdir); // Extract the destination path

  if (strlen(destdir) == 0)
   { WinMessageBox(HWND_DESKTOP,(HWND)parm,"You must provide a destination directory","Attention",0,MB_OK | MB_ICONQUESTION | MB_MOVEABLE);
     installing = FALSE;
     installed = FALSE;
     setButtons((HWND)parm);
     WinDestroyMsgQueue(mhmq);
     WinTerminate(rhab);
     DosExit(0,0);
   }

  if (destdir[strlen(destdir)-1] == '\\') destdir[strlen(destdir)-1] = 0;

  if (DosSetCurrentDir(destdir) != NO_ERROR)
    {
       if (WinMessageBox(HWND_DESKTOP,(HWND)parm,"Destination directory doesn't exist, should I create it ?","Attention",0,MB_YESNO | MB_ICONQUESTION | MB_MOVEABLE) != MBID_YES)
        {
          installing = FALSE;
          installed = FALSE;
          setButtons((HWND)parm);
          WinDestroyMsgQueue(mhmq);
          WinTerminate(rhab);
          DosExit(0,0);
        }
       strcpy(szArg,destdir);

       if (!PathCreate(szArg))
        {
          WinMessageBox(HWND_DESKTOP,(HWND)parm,"I've got an error while creating the destination directory","Error",0,MB_OK | MB_ICONASTERISK | MB_MOVEABLE);
          installing = FALSE;
          installed = FALSE;
          setButtons((HWND)parm);
          WinDestroyMsgQueue(mhmq);
          WinTerminate(rhab);
          DosExit(0,0);
        }

       strcpy(szArg,"Created destination directory ...");
       WinInsertLboxItem(WinWindowFromID((HWND)parm,LB_MSGLIST),LIT_END,szArg);
    }

  DosDupHandle(HF_STDOUT,&hfSave);
  DosCreatePipe( &hReadPipe, &hW1, 0 ) ;
  hW = 1;
  DosDupHandle( hW1, &hW ) ;

  memset(szArg,0,sizeof(szArg)); // Create the parameters string
  strcat(szArg,programpath);
  strcat(szArg,"VCLASSED.PAK \"");
  strcat(szArg,destdir);
  strcat(szArg,"\"");

  SData.Length  = sizeof(STARTDATA);
  SData.Related = SSF_RELATED_CHILD; /* start an independent session */
  SData.FgBg    = SSF_FGBG_BACK;           /* start session in foreground  */
  SData.TraceOpt = SSF_TRACEOPT_NONE;      /* No trace                     */
             /* Start an OS/2 session using "CMD.EXE /K" */
  SData.PgmTitle = "";
  SData.PgmName = "UNPACK.EXE";
  SData.PgmInputs = szArg;                     /* Keep session up           */

  SData.TermQ = 0;                            /* No termination queue      */
  SData.Environment = 0;                      /* No environment string     */
  SData.InheritOpt = SSF_INHERTOPT_PARENT;     /* Inherit shell's environ.  */
  SData.SessionType = SSF_TYPE_DEFAULT; /* Windowed VIO session      */
  SData.IconFile = 0;                         /* No icon association       */
  SData.PgmHandle = 0;
            /* Open the session VISIBLE and MAXIMIZED */
  SData.PgmControl = SSF_CONTROL_INVISIBLE;
  SData.Reserved = 0;
  SData.ObjectBuffer  = szFailName; /* Contains info if DosExecPgm fails */
  SData.ObjectBuffLen = CCHMAXPATH;

  UnpackPipe = hReadPipe;

  DosCreateThread(&tid,UnpackReader,parm,CREATE_READY,9000L);

  DosStartSession(&SData, &ulSessID, &pid);  /* Start the session */

  DosDupHandle(hfSave,&hfNew);

  DosWaitThread(&tid,DCWW_WAIT);

  if (SHORT1FROMMR(WinSendMsg(WinWindowFromID((HWND)parm,CB_CREATEOBJECTS),BM_QUERYCHECK,0,0)) == 1)
   {
     memset(szArg,0,sizeof(szArg));
     strcat(szArg,"ICONFILE=");
     strcat(szArg,destdir);
     strcat(szArg,"\\VCLASSED.ICO;");
     strcat(szArg,"OBJECTID=<VCLASSED>");
     WinCreateObject("WPFolder","VClassed 1.6",szArg,"<WP_DESKTOP>",CO_REPLACEIFEXISTS);
     WinInsertLboxItem(WinWindowFromID((HWND)parm,LB_MSGLIST),LIT_END,"Created : VClassed's Folder");

     memset(szArg,0,sizeof(szArg));
     strcat(szArg,"EXENAME=");
     strcat(szArg,destdir);
     strcat(szArg,"\\VCLASSED.EXE;PROGTYPE=PM");
     WinCreateObject("WPProgram","VClassed",szArg,"<VCLASSED>",CO_REPLACEIFEXISTS);
     WinInsertLboxItem(WinWindowFromID((HWND)parm,LB_MSGLIST),LIT_END,"Created : VClassed's Program Object");

     memset(szArg,0,sizeof(szArg));
     strcat(szArg,"SHADOWID=");
     strcat(szArg,destdir);
     strcat(szArg,"\\README.TXT");
     WinCreateObject("WPShadow","VClassed's Readme",szArg,"<VCLASSED>",CO_REPLACEIFEXISTS);
     WinInsertLboxItem(WinWindowFromID((HWND)parm,LB_MSGLIST),LIT_END,"Created : VClassed's Readme");
   }

  installing = FALSE;
  installed = TRUE;

  setButtons((HWND)parm);

  WinDestroyMsgQueue(mhmq);
  WinTerminate(rhab);
  DosExit(0,0);
}

MRESULT EXPENTRY InstProc (HWND wnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  TID tid;
   switch (msg) {
     case WM_INITDLG :
           return(0);
     case WM_COMMAND :
       switch (SHORT1FROMMP(mp1)) {
        case BTN_INSTALL :
               setButtons(wnd);
               installing = TRUE;
               DosCreateThread(&tid,InstallThread,(ULONG)wnd,CREATE_READY,20000L);
               return(0);
        case DID_OK :
        case DID_CANCEL :
        case BTN_EXIT :
                WinPostMsg(wnd,WM_CLOSE,0,0);
                return(0);
       }
     case WM_CLOSE : if (installing) return((MPARAM) 0); // Don't close if installing

                     if (installed) return(WinDefDlgProc(wnd,msg,mp1,mp2)); // If installed don't ask for exit confirmation

                     if (WinMessageBox(HWND_DESKTOP,wnd,"Are you sure you want to quit VClassed's Install?","Confirm",0,MB_YESNO | MB_ICONQUESTION | MB_MOVEABLE) == MBID_YES)
                       { return(WinDefDlgProc(wnd,msg,mp1,mp2)); }
                     else
                       { return((MPARAM) 0); }
     case WM_DESTROY :
        return(0);
     default:
       return(WinDefDlgProc(wnd,msg,mp1,mp2));
     }
}

void main(int argc, char *argv[])
{
  HAB mhab;
  HMQ mhmq;
  HPOINTER icon;
  RECTL mrect;
  HWND dlgwnd;
  char *c;

 /* Startup */
  mhab = WinInitialize(0);
  mhmq = WinCreateMsgQueue(mhab,0);

  strcpy(programpath,argv[0]);
  if ((c = strrchr(programpath,'\\')) != NULL)
   { c++;
     *c = 0;
   }

 /* Main block */
  icon = WinLoadPointer(HWND_DESKTOP,NULLHANDLE,1); // Load pointer for Main dialog
  dlgwnd = WinLoadDlg(HWND_DESKTOP,HWND_DESKTOP,&InstProc,NULLHANDLE, DLG_INSTALLVCLASSED,NULL); // Load main dialog
  WinSendMsg(dlgwnd,WM_SETICON,(MPARAM) icon,0); // Set new dialog icon
  WinQueryWindowRect(dlgwnd,&mrect);
  WinSetWindowPos(dlgwnd,0,(WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN) - (mrect.xRight-mrect.xLeft)) / 2,
                           (WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN) - (mrect.yTop-mrect.yBottom)) / 2, 0,0, SWP_MOVE | SWP_SHOW);



  WinProcessDlg(dlgwnd);

 /* Close'n'Clean */
  WinDestroyWindow(dlgwnd);
  WinDestroyMsgQueue(mhmq);
  WinTerminate(mhab);
}