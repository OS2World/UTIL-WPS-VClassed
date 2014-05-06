/*  <VClassed 1.6, a PM OS/2 WorkPlace Shell class manager>
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

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vclassed.h"

#define WM_LoadCList WM_USER+1

MRESULT EXPENTRY VCEdProc (HWND wnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
   static HWND LBwnd, Popup;
   static PSZ *Classes, *Modules;
   static SHORT Selcount;
   static SHORT Selection;
   static BOOL Sorted;
   POBJCLASS p,q;
   ULONG Size;
   POINTL mspos;
   char tmp[80];

   switch (msg) {
     case WM_INITDLG :
           LBwnd = WinWindowFromID(wnd,DID_List);
           // Load & Set popup menu
           Popup = WinLoadMenu(HWND_DESKTOP,NULLHANDLE,MNU_PopupList);
           WinEnableMenuItem(Popup,BTN_Dereg,FALSE);
           WinEnableMenuItem(Popup,BTN_Create,FALSE);

           Sorted = TRUE;
           WinCheckMenuItem(Popup,BTN_Sorted,Sorted);

           WinPostMsg(wnd,WM_LoadCList,0,0);
           return(0);
     case WM_BUTTON2DOWN :
           mspos.x = MOUSEMSG(&msg)->x;
           mspos.y = MOUSEMSG(&msg)->y;
           if (WinWindowFromPoint(wnd, &mspos, FALSE) == LBwnd)
           {
              // Executes popup menu
              WinPopupMenu(wnd,wnd,Popup, mspos.x, mspos.y, 1,PU_KEYBOARD | PU_MOUSEBUTTON1 | PU_HCONSTRAIN | PU_VCONSTRAIN);
              return((MRESULT) 1);
           }
           return(WinDefDlgProc(wnd,msg,mp1,mp2));
     case WM_CONTROL :
         if (SHORT1FROMMP(mp1) == DID_List)
            {
             switch (SHORT2FROMMP(mp1)) {
              case LN_SELECT : { Selcount = 0;
                                 Selection = LIT_FIRST;
                                 while ((Selection = SHORT1FROMMR(WinSendMsg(LBwnd,LM_QUERYSELECTION,MPFROMSHORT(Selection),0))) != LIT_NONE) Selcount++;
                                 if (Selcount == 1)
                                  {
                                   tmp[0] = 0;
                                   strcat(tmp,*(Classes+ SHORT1FROMMR(WinSendMsg(LBwnd,LM_QUERYSELECTION,(MPARAM) LIT_FIRST,0))) );
                                   strcat(tmp," -> ");
                                   strcat(tmp,*(Modules+ SHORT1FROMMR(WinSendMsg(LBwnd,LM_QUERYSELECTION,(MPARAM) LIT_FIRST,0))) );
                                   WinSetDlgItemText(wnd,ST_Info,tmp);
                                  }
                                   else
                                     {
                                        sprintf(tmp,"%d class(es) selected",Selcount);
                                        WinSetDlgItemText(wnd,ST_Info,tmp);
                                     }

                                  WinEnableMenuItem(Popup,BTN_Dereg,(Selcount != 0));

                                  WinEnableMenuItem(Popup,BTN_Create,(Selcount==1));
                                }

              default : return(WinDefDlgProc(wnd,msg,mp1,mp2));
              }
            }
         return(WinDefDlgProc(wnd,msg,mp1,mp2));
     case WM_COMMAND :
       switch (SHORT1FROMMP(mp1)) {
        case BTN_Sorted :
            Sorted = !Sorted;
            WinCheckMenuItem(Popup,BTN_Sorted,Sorted);
            WinPostMsg(wnd,WM_LoadCList,0,0);
            return(0);
        case BTN_About :
                WinDlgBox(HWND_DESKTOP,wnd,&WinDefDlgProc,NULLHANDLE,DLG_About,NULL);
                return(0);
        case DID_OK :
        case DID_CANCEL :
                WinPostMsg(wnd,WM_CLOSE,0,0);
                return(0);
        case BTN_Dereg :
              if (Selcount != 0)
               { char AText[255];
                 SHORT n;
                 BOOL reload = FALSE;
                 Selection = LIT_FIRST;

                 for(n=0;n<Selcount;n++)
                 {
                  Selection = SHORT1FROMMR(WinSendMsg(LBwnd,LM_QUERYSELECTION,MPFROMSHORT(Selection),0));
                  sprintf(AText,"Are you sure you want do deregister class : %s ?",*(Classes+Selection));
                  if (WinMessageBox(HWND_DESKTOP,wnd,AText,"Confirm",0,MB_YESNO | MB_MOVEABLE | MB_ICONQUESTION) == MBID_YES)
                    {
                      if (WinDeregisterObjectClass(*(Classes+Selection))) WinMessageBox(HWND_DESKTOP,wnd,"Class deregistered succesfully","Success",0,MB_OK | MB_MOVEABLE | MB_INFORMATION);
                      else WinMessageBox(HWND_DESKTOP,wnd,"Error deregistering class","Error",0,MB_OK | MB_MOVEABLE | MB_ICONHAND);
                      reload = TRUE;
                    }
                  }
                  if (reload) WinPostMsg(wnd,WM_LoadCList,0,0);
               }
             else WinMessageBox(HWND_DESKTOP,wnd,"At least one class must be selected","Error",0,MB_OK | MB_MOVEABLE | MB_ICONHAND);
             return(0);
        case BTN_Register :
              if (WinDlgBox(HWND_DESKTOP,wnd,&RegisterDlgProc,NULLHANDLE,DLG_Register,NULL) == DID_OK) WinPostMsg(wnd,WM_LoadCList,0,0);
              return(0);
        case BTN_Create :
             { CRecord Cl;

               if (Selcount == 1)
                {
                  Cl.cbSize = sizeof(CRecord);
                  Selection = SHORT1FROMMR(WinSendMsg(LBwnd,LM_QUERYSELECTION,MPFROMSHORT(LIT_FIRST),0));
                  Cl.data = strdup(*(Classes + Selection));
                  WinDlgBox(HWND_DESKTOP,wnd,&CreateDlgProc,NULLHANDLE,DLG_CreateObj,&Cl);
                }
                else WinMessageBox(HWND_DESKTOP,wnd,"One class must be selected to create an object","Error",0,MB_OK | MB_MOVEABLE | MB_ICONHAND);
              return(0);
             }
        case BTN_Refresh :
           WinPostMsg(wnd,WM_LoadCList,0,0);
           return(0);
        default :
          return(WinDefDlgProc(wnd,msg,mp1,mp2));
         }
     case (WM_LoadCList) :
         {
           WinSetDlgItemText(wnd,ST_Info,"Loading class list ...");
           if (WinQueryLboxCount(LBwnd) != 0) WinSendMsg(LBwnd,LM_DELETEALL,0,0);

           p = NULL;
           if (WinEnumObjectClasses(p,&Size))
            {
              static PSZ *C1, *M1;

              p = (POBJCLASS) malloc(Size);

              if (Classes != NULL) free(Classes);
              if (Modules != NULL) free(Modules);

              Classes = malloc(Size);
              Modules = malloc(Size);
              C1 = malloc(Size);
              M1 = malloc(Size);

              WinEnumObjectClasses(p,&Size);
              q = p;
              Selcount = 0;

              while (p != NULL)
               {
                 if (Sorted) WinInsertLboxItem(LBwnd,LIT_SORTASCENDING,p->pszClassName);
                 else WinInsertLboxItem(LBwnd,LIT_END,p->pszClassName);
                 *(C1 + Selcount) = (PSZ) p->pszClassName;
                 *(M1 + Selcount) = (PSZ) p->pszModName;
                 p = p->pNext;
                 Selcount++;
               }
              free(q);

              for (Size = 0;Size < Selcount;Size++)
               { SHORT pos;

                  if (Sorted == TRUE) pos = SHORT1FROMMR(WinSendMsg(LBwnd,LM_SEARCHSTRING,MPFROM2SHORT(LSS_CASESENSITIVE,LIT_FIRST),MPFROMP(*(C1+Size))));
                  else pos = Size;
                     *(Classes + pos) = *(C1+Size);
                     *(Modules + pos) = *(M1+Size);
               }
              free(C1);
              free(M1);
            }
            WinSetDlgItemText(wnd,ST_Info,"Class list loaded");
            sprintf(tmp,"%d",Selcount);
            WinSetDlgItemText(wnd,ST_Count,tmp);
            Selcount = 0;
           return(0);
         }
     case WM_CLOSE :
                     if (WinMessageBox(HWND_DESKTOP,wnd,"Are you sure you want to quit VClassed ?","Confirm",0,MB_YESNO | MB_ICONQUESTION | MB_MOVEABLE) == MBID_YES)
                       { return(WinDefDlgProc(wnd,msg,mp1,mp2)); }
                     else
                       { return((MPARAM) 0); }

     case WM_DESTROY :
        WinDestroyWindow(Popup);
        return(0);
     default:
       return(WinDefDlgProc(wnd,msg,mp1,mp2));
     }
}

void main()
{
  HAB mhab;
  HMQ mhmq;
  HWND dlgwnd;
  HSWITCH thetask;
  HPOINTER icon;
  RECTL mrect;
  SWCNTRL tswitch;
  PID pid;

 /* Startup */
  mhab = WinInitialize(0);
  mhmq = WinCreateMsgQueue(mhab,0);

 /* Main block */
  icon = WinLoadPointer(HWND_DESKTOP,NULLHANDLE,1); // Load pointer for Main dialog
  dlgwnd = WinLoadDlg(HWND_DESKTOP,HWND_DESKTOP,&VCEdProc,NULLHANDLE,1,NULL); // Load main dialog
  WinSendMsg(dlgwnd,WM_SETICON,(MPARAM) icon,0); // Set new dialog icon
  WinQueryWindowRect(dlgwnd,&mrect);

  WinQueryWindowProcess(dlgwnd,&pid,NULL);
  tswitch.hwnd = dlgwnd;
  tswitch.hwndIcon = icon;
  tswitch.idProcess = pid;
  tswitch.idSession = 0;
  tswitch.uchVisibility = SWL_VISIBLE;
  tswitch.fbJump = SWL_JUMPABLE;
  strcpy(tswitch.szSwtitle,"VClassed 1.5 - Class Manager");
  WinSetWindowPos(dlgwnd,0,(WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN) - (mrect.xRight-mrect.xLeft)) / 2,
                           (WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN) - (mrect.yTop-mrect.yBottom)) / 2, 0,0, SWP_MOVE | SWP_SHOW);

  thetask = WinAddSwitchEntry(&tswitch);

  WinProcessDlg(dlgwnd);

 /* Close'n'Clean */
  WinRemoveSwitchEntry(thetask);
  WinDestroyWindow(dlgwnd);
  WinDestroyMsgQueue(mhmq);
  WinTerminate(mhab);
}