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
#include <string.h>
#include <stdlib.h>

#include "vclassed.h"

MRESULT EXPENTRY RegisterDlgProc (HWND wnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  switch (msg) {
    case WM_INITDLG :
       WinSendMsg(WinWindowFromID(wnd,IN_Class),EM_SETTEXTLIMIT,MPFROMSHORT(40),0);
       WinSendMsg(WinWindowFromID(wnd,IN_Dll),EM_SETTEXTLIMIT,MPFROMSHORT(255),0);
       return(0);
    case WM_COMMAND :
        switch (SHORT1FROMMP(mp1)) {
         case DID_OK :
           { char aclass[41], dll[256];
             WinQueryDlgItemText(wnd,IN_Class,40,aclass);
             WinQueryDlgItemText(wnd,IN_Dll,255,dll);
             if (strlen(aclass) == 0) { WinMessageBox(HWND_DESKTOP,wnd,"You must provide a class name","Error",0,MB_OK | MB_MOVEABLE | MB_ICONHAND);
                                       return(0); }
             if (strlen(dll) == 0) { WinMessageBox(HWND_DESKTOP,wnd,"You must provide a dll file","Error",0,MB_OK | MB_MOVEABLE | MB_ICONHAND);
                                       return(0); }
             if (WinRegisterObjectClass(aclass,dll)) WinMessageBox(HWND_DESKTOP,wnd,"Class registered succesfully","Success",0, MB_OK | MB_MOVEABLE | MB_INFORMATION);
             else WinMessageBox(HWND_DESKTOP,wnd,"Error registering new class","Error",0, MB_OK | MB_MOVEABLE | MB_ICONHAND);
             WinDismissDlg(wnd,DID_OK);
             return(0);
           }
         case BTN_Browse :
          {
            FILEDLG FileDlg;
            char pszTitle[]="Select DLL";
            char pszFullFile[]="*.dll";
            HWND hwndFileDlg;

            memset(&FileDlg, 0, sizeof(FILEDLG));
            FileDlg.cbSize=sizeof(FILEDLG);
            FileDlg.fl = FDS_CENTER | FDS_OPEN_DIALOG;
            FileDlg.pszTitle=pszTitle;
            strcpy(FileDlg.szFullFile, pszFullFile);
            FileDlg.pszOKButton = malloc(10);
            strcpy(FileDlg.pszOKButton,"~Register");
            hwndFileDlg = WinFileDlg(HWND_DESKTOP, wnd, &FileDlg);
            if ( FileDlg.lReturn != DID_CANCEL & hwndFileDlg != NULLHANDLE ) WinSetDlgItemText(wnd,IN_Dll,FileDlg.szFullFile);
            free(FileDlg.pszOKButton);
            return(0);
          }
         default : return(WinDefDlgProc(wnd,msg,mp1,mp2));
        }
    default :
     return(WinDefDlgProc(wnd,msg,mp1,mp2));
    }
}

MRESULT EXPENTRY CreateDlgProc (HWND wnd,ULONG msg,MPARAM mp1,MPARAM mp2)
{
  static char aclass[55];

  switch (msg) {
   case WM_INITDLG :
    { CRecord *Cl;
      HWND Combo;
      char tmp[255];
      Cl = PVOIDFROMMP(mp2);
      tmp[0] = 0;
      strcat(tmp,"Create new object : ");
      strcat(tmp,Cl->data);
      WinSetWindowText(wnd,tmp);
      strcpy(aclass,Cl->data);

      WinSendMsg(WinWindowFromID(wnd,DLGCRE_Title),EM_SETTEXTLIMIT,MPFROMSHORT(255),0);
      Combo = WinWindowFromID(wnd,DLGCRE_Dest);
      WinSendMsg(Combo,EM_SETTEXTLIMIT,MPFROMSHORT(255),0);
      WinSendMsg(WinWindowFromID(wnd,DLGCRE_Parm),EM_SETTEXTLIMIT,MPFROMSHORT(255),0);

      strcpy(tmp,"<WP_DESKTOP>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_NOWHERE>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_OS2SYS>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_TEMPS>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_CONFIG>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_START>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_INFO>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_DRIVES>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_APPSFOLDER>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_CONNCETIONSFOLDER>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);
      strcpy(tmp,"<WP_GAMES>");
      WinSendMsg(Combo,LM_INSERTITEM,MPFROMSHORT(LIT_END),tmp);



      WinCheckButton(wnd,DLGCRE_Check,1);
      return(0);
    }
   case WM_COMMAND :
    switch (SHORT1FROMMP(mp1)) {
      case DID_OK :
       {
         char title[256], dest[256], parms[256];
         ULONG mode=0;

         WinQueryDlgItemText(wnd,DLGCRE_Title,255,title);
         if (strlen(title) == 0) { WinMessageBox(HWND_DESKTOP,wnd,"You must provide a title for the object","Error",0,MB_OK | MB_MOVEABLE | MB_ICONHAND);
                                       return(0); }
         WinQueryDlgItemText(wnd,DLGCRE_Dest,30,dest);
         if (strlen(dest) == 0) { WinMessageBox(HWND_DESKTOP,wnd,"You must provide a destination folder for the object","Error",0,MB_OK | MB_MOVEABLE | MB_ICONHAND);
                                       return(0); }
         WinQueryDlgItemText(wnd,DLGCRE_Parm,255,parms);

         switch ( SHORT1FROMMP(WinSendDlgItemMsg(wnd,DLGCRE_Check,BM_QUERYCHECKINDEX,0,0)) ) {
           case 0 : mode = CO_FAILIFEXISTS; break;
           case 1 : mode = CO_REPLACEIFEXISTS; break;
           case 2 : mode = CO_UPDATEIFEXISTS; break;
           }

         if (WinCreateObject(aclass,title,parms,dest,mode) == NULLHANDLE) WinMessageBox(HWND_DESKTOP,wnd,"Error creating new object","Error",0,MB_OK | MB_MOVEABLE | MB_ICONHAND);
         else WinMessageBox(HWND_DESKTOP,wnd,"Object created successfully","Success",0,MB_OK | MB_MOVEABLE | MB_INFORMATION);

        WinDismissDlg(wnd,DID_OK);
        return(0);
       }
      default :
       return(WinDefDlgProc(wnd,msg,mp1,mp2));
      }
   default:
    return(WinDefDlgProc(wnd,msg,mp1,mp2));
  }
}


