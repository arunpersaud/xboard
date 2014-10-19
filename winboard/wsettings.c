/*
 * woptions.h -- Options dialog box routines for WinBoard
 *
 * Copyright 2003, 2009, 2010, 2011, 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

/*
 * Engine-settings dialog. The complexity come from an attempt to present the engine-defined options
 * in a nicey formatted layout. To this end we first run a back-end pre-formatter, which will distribute
 * the controls over two columns (the minimum required, as some are double width). It also takes care of
 * grouping options that start with the same word (mainly for "Polyglot ..." options). It assigns relative
 * suitability to break points between lines, and in the end decides if and where to break up the list
 * for display in multiple (2*N) columns.
 * The thus obtained list representing the topology of the layout is then passed to a front-end routine
 * that generates the actual dialog box from it.
 */

#include "config.h"

#include <windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "winboard.h"
#include "backendz.h"

#define _(s) T_(s)
#define N_(s) s

int layoutList[2*MAX_OPTIONS];
int checkList[2*MAX_OPTIONS];
int comboList[2*MAX_OPTIONS];
int buttonList[2*MAX_OPTIONS];
int boxList[2*MAX_OPTIONS];
int groupNameList[2*MAX_OPTIONS];
int breaks[MAX_OPTIONS];
int checks, combos, buttons, layout, groups;
char title[MSG_SIZ];
char *engineName, *engineDir, *engineChoice, *engineLine, *nickName, *params;
Boolean isUCI, hasBook, storeVariant, v1, addToList, useNick, isUCCI;
extern Option installOptions[], matchOptions[];
char *engineList[MAXENGINES] = {""}, *engineMnemonic[MAXENGINES] = {""};
void (*okFunc)();
ChessProgramState *activeCps;
Option *activeList;
int InstallOK P((void));
typedef int ButtonCallback(HWND h);
ButtonCallback *comboCallback;

void
PrintOpt(int i, int right, Option *optionList)
{
    if(i<0) {
	if(!right) fprintf(debugFP, "%30s", "");
    } else {
	Option opt = optionList[i];
	switch(opt.type) {
	    case Slider:
	    case Spin:
		fprintf(debugFP, "%20.20s [    +/-]", opt.name);
		break;
	    case TextBox:
	    case FileName:
	    case PathName:
		fprintf(debugFP, "%20.20s [______________________________________]", opt.name);
		break;
	    case Label:
		fprintf(debugFP, "%41.41s", opt.name);
		break;
	    case CheckBox:
		fprintf(debugFP, "[x] %-26.25s", opt.name);
		break;
	    case ComboBox:
		fprintf(debugFP, "%20.20s [ COMBO ]", opt.name);
		break;
	    case Button:
	    case SaveButton:
	    case ResetButton:
		fprintf(debugFP, "[ %26.26s ]", opt.name);
	    case Message:
	    default:
		break;
	}
    }
    fprintf(debugFP, right ? "\n" : " ");
}

void
CreateOptionDialogTest(int *list, int nr, Option *optionList)
{
    int line;

    for(line = 0; line < nr; line+=2) {
	PrintOpt(list[line+1], 0, optionList);
	PrintOpt(list[line], 1, optionList);
    }
}

void
LayoutOptions(int firstOption, int endOption, char *groupName, Option *optionList)
{
    int i, b = strlen(groupName), stop, prefix, right, nextOption, firstButton = buttons;
    Control lastType, nextType=Label;

    nextOption = firstOption;
    while(nextOption < endOption) {
	checks = combos = 0; stop = 0;
	lastType = Button; // kludge to make sure leading Spin will not be prefixed
	// first separate out buttons for later treatment, and collect consecutive checks and combos
	while(nextOption < endOption && !stop) {
	    switch(nextType = optionList[nextOption].type) {
		case CheckBox: checkList[checks++] = nextOption; lastType = CheckBox; break;
		case ComboBox: comboList[combos++] = nextOption; lastType = ComboBox; break;
		case ResetButton:
		case SaveButton:
		case Button:  buttonList[buttons++] = nextOption; lastType = Button; break;
		case TextBox:
		case ListBox:
		case FileName:
		case PathName:
		case Slider:
		case Label:
		case Spin: stop++;
		default:
		case Message: ; // cannot happen
	    }
	    nextOption++;
	}
	// We now must be at the end, or looking at a spin or textbox (in nextType)
	if(!stop)
	    nextType = Button; // kudge to flush remaining checks and combos undistorted
	// Take a new line if a spin follows combos or checks, or when we encounter a textbox
	if((combos+checks || nextType == TextBox || nextType == ListBox || nextType == FileName || nextType == PathName || nextType == Label) && layout&1) {
	    layoutList[layout++] = -1;
	}
	// The last check or combo before a spin will be put on the same line as that spin (prefix)
	// and will thus not be grouped with other checks and combos
	prefix = -1;
	if(nextType == Spin && lastType != Button) {
	    if(lastType == CheckBox) prefix = checkList[--checks]; else
	    if(lastType == ComboBox) prefix = comboList[--combos];
	}
	// if a combo is followed by a textbox, it must stay at the end of the combo/checks list to appear
	// immediately above the textbox, so treat it as check. (A check would automatically be and remain there.)
	if((nextType == TextBox || nextType == ListBox || nextType == FileName || nextType == PathName) && lastType == ComboBox)
	    checkList[checks++] = comboList[--combos];
	// Now append the checks behind the (remaining) combos to treat them as one group
	for(i=0; i< checks; i++)
	    comboList[combos++] = checkList[i];
	// emit the consecutive checks and combos in two columns
	right = combos/2; // rounded down if odd!
	for(i=0; i<right; i++) {
	    breaks[layout/2] = 2;
	    layoutList[layout++] = comboList[i];
	    layoutList[layout++] = comboList[i + right];
	}
	// An odd check or combo (which could belong to following textBox) will be put in the left column
	// If there was an even number of checks and combos the last one will automatically be in that position
	if(combos&1) {
	    layoutList[layout++] = -1;
	    layoutList[layout++] = comboList[2*right];
	}
	if(nextType == ListBox) {
	    // A listBox will be left-adjusted, and cause rearrangement of the elements before it to the right column
	    breaks[layout/2] = lastType == Button ? 0 : 100;
	    layoutList[layout++] = -1;
	    layoutList[layout++] = nextOption - 1;
	    for(i=optionList[nextOption-1].min; i>0; i--) { // extra high text edit
		breaks[layout/2] = -1;
		layoutList[layout++] = -1;
		layoutList[layout++] = -1;
	    }
	} else 
	if(nextType == TextBox || nextType == FileName || nextType == PathName || nextType == Label) {
	    // A textBox is double width, so must be left-adjusted, and the right column remains empty
	    breaks[layout/2] = lastType == Button ? 0 : 100;
	    layoutList[layout++] = -1;
	    layoutList[layout++] = nextOption - 1;
	    if(optionList[nextOption-1].min) { // extra high text edit: goes right of existing listbox
		layout -= 2; // remove
		layoutList[layout-2*optionList[nextOption-1].min-2] = nextOption - 1;
	    }
	} else if(nextType == Spin) {
	    // A spin will go in the next available position (right to left!). If it had to be prefixed with
	    // a check or combo, this next position must be to the right, and the prefix goes left to it.
	    layoutList[layout++] = nextOption - 1;
	    if(prefix >= 0) layoutList[layout++] = prefix;
	}
    }
    // take a new line if needed
    if(layout&1) layoutList[layout++] = -1;
    // emit the buttons belonging in this group; loose buttons are saved for last, to appear at bottom of dialog
    if(b) {
	while(buttons > firstButton)
	    layoutList[layout++] = buttonList[--buttons];
	if(layout&1) layoutList[layout++] = -1;
    }
}

char *
EndMatch(char *s1, char *s2)
{
	char *p, *q;
	p = s1; while(*p) p++;
	q = s2; while(*q) q++;
	while(p > s1 && q > s2 && *p == *q) { p--; q--; }
	if(p[1] == 0) return NULL;
	return p+1;
}

void
DesignOptionDialog(int nrOpt, Option *optionList)
{
    int k=0, n=0;
    char buf[MSG_SIZ];

    layout = 0;
    buttons = groups = 0;
    while(k < nrOpt) { // k steps through 'solitary' options
	// look if we hit a group of options having names that start with the same word
	int groupSize = 1, groupNameLength = 50;
	sscanf(optionList[k].name, "%s", buf); // get first word of option name
	while(k + groupSize < nrOpt &&
	      strstr(optionList[k+groupSize].name, buf) == optionList[k+groupSize].name) {
		int j;
		for(j=0; j<groupNameLength; j++) // determine common initial part of option names
		    if( optionList[k].name[j] != optionList[k+groupSize].name[j]) break;
		groupNameLength = j;
		groupSize++;

	}
	if(groupSize > 3) {
		// We found a group to terminates the current section
		LayoutOptions(n, k, "", optionList); // flush all solitary options appearing before the group
		groupNameList[groups] = groupNameLength;
		boxList[groups++] = layout; // group start in even entries
		LayoutOptions(k, k+groupSize, buf, optionList); // flush the group
		boxList[groups++] = layout; // group end in odd entries
		k = n = k + groupSize;
	} else k += groupSize; // small groups are grouped with the solitary options
    }
    if(n != k) LayoutOptions(n, k, "", optionList); // flush remaining solitary options
    // decide if and where we break into two column pairs

    // Emit buttons and add OK and cancel
//    for(k=0; k<buttons; k++) layoutList[layout++] = buttonList[k];
 
   // Create the dialog window
    if(appData.debugMode) CreateOptionDialogTest(layoutList, layout, optionList);
//    CreateOptionDialog(layoutList, layout, optionList);
    if(!activeCps) okFunc = optionList[nrOpt].target;
}

#include <windows.h>

extern HINSTANCE hInst;

typedef struct {
    DLGITEMTEMPLATE item;
    WORD code;
    WORD controlType;
    wchar_t d1, data;
    WORD creationData;
} Item;

struct {
    DLGTEMPLATE header;
    WORD menu;
    WORD winClass;
    wchar_t title[20];
    WORD pointSize;
    wchar_t fontName[14];
    Item control[MAX_OPTIONS];
} template = {
    { DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_SETFONT, 0, 0, 0, 0, 295, 300 },
    0x0000, 0x0000, L"Engine #1 Settings ", 8, L"MS Sans Serif"
};

char *
AddCR(char *s)
{
    char *p=s, *q;
    int n=0;
    while(p = strchr(p, '\n')) p++, n++; // count linefeeds
    p = q = malloc(strlen(s) + n + 1);
    while(*p++ = *s++) if(p[-1] == '\n') p[-1] = '\r', *p++ = '\n';
    return q;
}

void
SetOptionValues(HWND hDlg, ChessProgramState *cps, Option *optionList)
// Put all current option values in controls, and write option names next to them
{
    HANDLE hwndCombo;
    int i, k;
    char **choices, *name;

    for(i=0; i<layout+buttons; i++) {
	int j=layoutList[i];
	if(j == -2) SetDlgItemText( hDlg, 2000+2*i, ". . ." );
	if(j<0) continue;
	name = cps ? optionList[j].name : _(optionList[j].name);
	if(strstr(name, "Polyglot ") == name) name += 9;
	SetDlgItemText( hDlg, 2000+2*i, name );
//if(appData.debugMode) fprintf(debugFP, "# %s = %d\n",optionList[j].name, optionList[j].value );
	switch(optionList[j].type) {
	    case Spin:
		SetDlgItemInt( hDlg, 2001+2*i, cps ? optionList[j].value : *(int*)optionList[j].target, TRUE );
		break;
	    case TextBox:
	    case FileName:
	    case PathName:
		name = AddCR(cps ? optionList[j].textValue : *(char**)optionList[j].target); // stupid CR...
		SetDlgItemText( hDlg, 2001+2*i, name);
		free(name);
		break;
	    case CheckBox:
		CheckDlgButton( hDlg, 2000+2*i, (cps ? optionList[j].value : *(Boolean*)optionList[j].target) != 0);
		break;
	    case ComboBox:
		choices = (char**) optionList[j].textValue;
		hwndCombo = GetDlgItem(hDlg, 2001+2*i);
		SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);
		for(k=0; k<optionList[j].max; k++) {
		    SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM) choices[k]);
		}
		SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) choices[optionList[j].value]);
		break;
	    case ListBox:
		choices = (char**) optionList[j].choice;
		hwndCombo = GetDlgItem(hDlg, 2001+2*i);
		SendMessage(hwndCombo, LB_RESETCONTENT, 0, 0);
		for(k=0; k<optionList[j].max; k++) {
		    SendMessage(hwndCombo, LB_ADDSTRING, 0, (LPARAM) choices[k]);
		}
		break;
	    case Button:
	    case SaveButton:
	    default:
		break;
	}
    }
    SetDlgItemText( hDlg, IDOK, _("OK") );
    SetDlgItemText( hDlg, IDCANCEL, _("Cancel") );
    title[0] &= ~32; // capitalize
    SetWindowText( hDlg, title);
    for(i=0; i<groups; i+=2) {
	int id, p; char buf[MSG_SIZ];
	id = k = boxList[i];
	if(layoutList[k] < 0) k++;
	if(layoutList[k] < 0) continue;
	for(p=0; p<groupNameList[i]; p++) buf[p] = optionList[layoutList[k]].name[p];
	buf[p] = 0;
	SetDlgItemText( hDlg, 2000+2*(id+MAX_OPTIONS), buf );
    }
}


int
GetOptionValues(HWND hDlg, ChessProgramState *cps, Option *optionList)
// read out all controls, and if value is altered, remember it and send it to the engine
{
    int i, k, new=0, changed=0, len;
    char **choices, newText[MSG_SIZ], buf[MSG_SIZ], *text;
    BOOL success;

    for(i=0; i<layout; i++) {
	int j=layoutList[i];
	if(j<0) continue;
	switch(optionList[j].type) {
	    case Spin:
		new = GetDlgItemInt( hDlg, 2001+2*i, &success, TRUE );
		if(!success) break;
		if(new < optionList[j].min) new = optionList[j].min;
		if(new > optionList[j].max) new = optionList[j].max;
		if(!cps) { *(int*)optionList[j].target = new; break; }
		changed = 2*(optionList[j].value != new);
		optionList[j].value = new;
		break;
	    case TextBox:
	    case FileName:
	    case PathName:
		if(cps) len = MSG_SIZ - strlen(optionList[j].name) - 9, text = newText;
		else    len = GetWindowTextLength(GetDlgItem(hDlg, 2001+2*i)) + 1, text = (char*) malloc(len);
		success = GetDlgItemText( hDlg, 2001+2*i, text, len );
		if(!success) text[0] = NULLCHAR; // empty string can be valid input
		if(!cps) {
		    char *p;
		    p = (optionList[j].type != FileName ? strdup(text) : InterpretFileName(text, homeDir)); // all files relative to homeDir!
		    FREE(*(char**)optionList[j].target); *(char**)optionList[j].target = p;
		    free(text); text = p;
		    while(*p++ = *text++) if(p[-1] == '\r') p--; // crush CR
		    break;
		}
		changed = strcmp(optionList[j].textValue, newText) != 0;
		safeStrCpy(optionList[j].textValue, newText, MSG_SIZ - (optionList[j].textValue - optionList[j].name) );
		break;
	    case CheckBox:
		new = IsDlgButtonChecked( hDlg, 2000+2*i );
		if(!cps) { *(Boolean*)optionList[j].target = new; break; }
		changed = 2*(optionList[j].value != new);
		optionList[j].value = new;
		break;
	    case ComboBox:
		choices = (char**) optionList[j].textValue;
		success = GetDlgItemText( hDlg, 2001+2*i, newText, MSG_SIZ );
		if(!success) break;
		new = -1;
		for(k=0; k<optionList[j].max; k++) {
		    if(choices[k] && !strcmp(choices[k], newText)) new = k;
		}
		if(!cps && new > 0) {
		    if(*(char**)optionList[j].target) free(*(char**)optionList[j].target);
		    *(char**)optionList[j].target = strdup(optionList[j].choice[new]);
		    break;
		}
		changed = new >= 0 && (optionList[j].value != new);
		if(changed) optionList[j].value = new;
		break;
	    case ListBox:
		if(optionList[j].textValue)
		    *(int*) optionList[j].textValue = SendDlgItemMessage(hDlg, 2001+2*i, LB_GETCURSEL, 0, 0);
	    case Button:
	    default:
		break; // are treated instantly, so they have been sent already
	}
	if(changed == 2)
	  snprintf(buf, MSG_SIZ, "option %s=%d\n", optionList[j].name, new); else
	if(changed == 1)
	  snprintf(buf, MSG_SIZ, "option %s=%s\n", optionList[j].name, newText);
	if(changed) SendToProgram(buf, cps);
    }
    if(!cps && okFunc) return ((ButtonCallback*) okFunc)(0);
    return 1;
}

char *defaultExt[] = { NULL, "pgn", "fen", "exe", "trn", "bin", "log", "ini" };

LRESULT CALLBACK SettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    char buf[MSG_SIZ];
    int i, j, ext;

    switch( message )
    {
    case WM_INITDIALOG:

//        CenterWindow(hDlg, GetWindow(hDlg, GW_OWNER));
	SetOptionValues(hDlg, activeCps, activeList);

        SetFocus(GetDlgItem(hDlg, IDCANCEL));

        break;

    case WM_COMMAND:
        switch( LOWORD(wParam) ) {
        case IDOK:
	    if(!GetOptionValues(hDlg, activeCps, activeList)) return FALSE;
            EndDialog( hDlg, 0 );
	    comboCallback = NULL; activeCps = NULL;
            return TRUE;

        case IDCANCEL:
            EndDialog( hDlg, 1 );
	    comboCallback = NULL; activeCps = NULL;
            return TRUE;

	default:
	    // program-defined push buttons
	    i = LOWORD(wParam);
	    if( i>=2000 &&  i < 2000+2*(layout+buttons)) {
		j = layoutList[(i - 2000)/2];
		if(j == -2) {
		          char filter[] =
				"All files\0*.*\0Game files\0*.pgn;*.gam\0Position files\0*.fen;*.epd;*.pos\0"
				"EXE files\0*.exe;*.jar\0Tournament files (*.trn)\0*.trn\0"
				"BIN Files\0*.bin\0LOG Files\0*.log\0INI Files\0*.ini\0"
				"Image files\0*.bmp\0\0";
		          OPENFILENAME ofn;

			  GetDlgItemText( hDlg, i+3, buf, MSG_SIZ );

		          ZeroMemory( &ofn, sizeof(ofn) );

		          ofn.lStructSize = sizeof(ofn);
		          ofn.hwndOwner = hDlg;
		          ofn.hInstance = hInst;
		          ofn.lpstrFilter = filter;
			  ofn.nFilterIndex      = 1L + (ext = activeCps ? 0 : activeList[layoutList[(i-2000)/2+1]].max & 31);
			  ofn.lpstrDefExt       = defaultExt[ext];
		          ofn.lpstrFile = buf;
		          ofn.nMaxFile = sizeof(buf);
		          ofn.lpstrTitle = _("Choose File");
		          ofn.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_HIDEREADONLY;

		          if( activeList[layoutList[(i-2000)/2+1]].max & 32 ?
		                                       GetOpenFileName( &ofn ) :
		                                       GetSaveFileName( &ofn ) ) {
		              SetDlgItemText( hDlg, i+3, buf );
		          }
		} else
		if(j == -3) {
		    GetDlgItemText( hDlg, i+3, buf, MSG_SIZ );
		    if( BrowseForFolder( _("Choose Folder:"), buf ) ) {
			SetDlgItemText( hDlg, i+3, buf );
		    }
		}
		if(j < 0) break;
		if(comboCallback && activeList[j].type == ComboBox && HIWORD(wParam) == CBN_SELCHANGE) {
		    if(j > 5) break; // Yegh! Must solve problem with more than one combobox in dialog
		    (*comboCallback)(hDlg);
		    break;
		} else
		if(activeList[j].type == ListBox && HIWORD(wParam) == /*LBN_SELCHANGE*/ LBN_DBLCLK) {
		    ((ButtonCallback *) activeList[j].target)(hDlg);
		    break;
		} else
		if( activeList[j].type  == SaveButton)
		     GetOptionValues(hDlg, activeCps, activeList);
		else if( activeList[j].type  != Button) break;
		else if( !activeCps ) { (*(ButtonCallback*) activeList[j].target)(hDlg); break; }
		snprintf(buf, MSG_SIZ, "option %s\n", activeList[j].name);
		SendToProgram(buf, activeCps);
	    }
	    break;
        }

        break;
    }

    return FALSE;
}

void AddControl(int x, int y, int w, int h, int type, int style, int n)
{
    int i;

    i = template.header.cdit++;
    template.control[i].item.style = style;
    template.control[i].item.dwExtendedStyle = 0;
    template.control[i].item.x = x;
    template.control[i].item.y = y;
    template.control[i].item.cx = w;
    template.control[i].item.cy = h;
    template.control[i].item.id = 2000 + n;
    template.control[i].code = 0xFFFF;
    template.control[i].controlType = type;
    template.control[i].d1 = ' ';
    template.control[i].data = 0;
    template.control[i].creationData = 0;
}

void AddOption(int x, int y, Control type, int i)
{
    int extra, num = ES_NUMBER;

    switch(type) {
	case Spin+100:
	    num = 0; // needs text control for accepting negative numbers
	case Slider:
	case Spin:
	    AddControl(x, y+1, 95, 9, 0x0082, SS_ENDELLIPSIS | WS_VISIBLE | WS_CHILD, i);
	    AddControl(x+95, y, 50, 11, 0x0081, ES_AUTOHSCROLL | num | WS_BORDER | WS_VISIBLE | WS_CHILD | WS_TABSTOP, i+1);
	    break;
	case TextBox:
	    extra = 13*activeList[layoutList[i/2]].min; // when extra high, left-align and put description text above it
	    AddControl(x+(extra?50:0), y+1, 95, 9, 0x0082, SS_ENDELLIPSIS | WS_VISIBLE | WS_CHILD, i);
	    AddControl(x+(extra?50:95), y+(extra?13:0), extra?105:200, 11+(extra?extra-13:0), 0x0081, ES_AUTOHSCROLL | WS_BORDER | WS_VISIBLE |
					WS_CHILD | WS_TABSTOP | (extra ? ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL :0), i+1);
	    break;
	case ListBox:
	    AddControl(x, y+1, 95, 9, 0x0082, SS_ENDELLIPSIS | WS_VISIBLE | WS_CHILD, i);
	    extra = 13*activeList[layoutList[i/2]].min;
	    AddControl(x, y+13, 105, 11+extra-13, 0x0083, LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_HSCROLL | WS_BORDER | LBS_NOTIFY |
								 WS_VISIBLE | WS_CHILD | WS_TABSTOP, i+1);
	    break;
	case Label:
	    extra = activeList[layoutList[i/2]].value;
	    AddControl(x+extra, y+1, 290-extra, 9, 0x0082, SS_ENDELLIPSIS | WS_VISIBLE | WS_CHILD | WS_TABSTOP, i);
	    break;
	case FileName:
	case PathName:
	    AddControl(x, y+1, 95, 9, 0x0082, SS_ENDELLIPSIS | WS_VISIBLE | WS_CHILD, i);
	    AddControl(x+95, y, 180, 11, 0x0081, ES_AUTOHSCROLL | WS_BORDER | WS_VISIBLE | WS_CHILD | WS_TABSTOP, i+1);
	    AddControl(x+275, y, 20, 12, 0x0080, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, i-2);
	    layoutList[i/2-1] = -2 - (type == PathName);
	    break;
	case CheckBox:
	    AddControl(x, y, 145, 11, 0x0080, BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD | WS_TABSTOP, i);
	    break;
	case ComboBox:
	    AddControl(x, y+1, 95, 9, 0x0082, SS_ENDELLIPSIS | WS_VISIBLE | WS_CHILD, i);
	    AddControl(x+95, y-1, !activeCps && x<10 ? 120 : 50, 500, 0x0085,
			CBS_AUTOHSCROLL | CBS_DROPDOWN | WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL, i+1);
	    break;
	case Button:
	case ResetButton:
	case SaveButton:
	    AddControl(x-2, y, 65, 13, 0x0080, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, i);
	case Message:
	default:
	    break;
    }

}

void
CreateDialogTemplate(int *layoutList, int nr, Option *optionList)
{
    int i, ii, j, x=1, y=0, maxY=0, buttonRows, breakPoint = 1000, k=0;

    template.header.cdit = 0;
    template.header.cx = 307;
    buttonRows = (buttons + 1 + 3)/4; // 4 per row, rounded up
    if(nr > 50) {
	breakPoint = (nr+2*buttonRows+1)/2 & ~1;
	template.header.cx = 625;
    }

    for(ii=0; ii<nr; ii++) {
	i = ii^1; if(i == nr) i = ii; // if two on one line, swap order of treatment, to get good (left to right) tabbing order.
	if(k < groups && ii == boxList[k]) {
	    y += 10;
	    AddControl(x+2, y+13*(i>>1)-2, 301, 13*(boxList[k+1]-boxList[k]>>1)+8,
						0x0082, WS_VISIBLE | WS_CHILD | SS_BLACKFRAME, 2400);
	    AddControl(x+60, y+13*(i>>1)-6, 10*groupNameList[k]/3, 10,
						0x0082, SS_ENDELLIPSIS | WS_VISIBLE | WS_CHILD, 2*(ii+MAX_OPTIONS));
	}
	j = layoutList[i];
	if(j >= 0) {
	    int neg = (optionList[j].type == Spin && optionList[j].min < 0 ? 100 : 0); // flags spin with negative range
	    AddOption(x+155-150*(i&1), y+13*(i>>1)+5, optionList[j].type + neg, 2*i);
	    // listboxes have the special power to adjust the width of the column they are in
	    if(optionList[j].type == ListBox) x -= optionList[j].value, template.header.cx -= optionList[j].value;
	}
	if(k < groups && ii+1 == boxList[k+1]) {
	    k += 2; y += 4;
	}
	if(ii+1 >= breakPoint && breaks[ii+1>>1] >= 0) { x += 318; maxY = y+13*(ii+1>>1)+5; y = -13*(ii+1>>1); breakPoint = 1000; }
    }
    // add butons at the bottom of dialog window
    y += 13*(nr>>1)+5;

    for(i=0; i<buttons; i++) {
	AddControl(x+70*(i%4)+5, y+18*(i/4), 65, 15, 0x0080, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, 2*(nr+i));
	layoutList[nr+i] = buttonList[i];
    }
    AddControl(x+225, y+18*(buttonRows-1), 30, 15, 0x0080, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, IDOK-2000);
    AddControl(x+260, y+18*(buttonRows-1), 40, 15, 0x0080, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP, IDCANCEL-2000);
    y += 18*buttonRows; if(y < maxY) y = maxY;
    template.title[8] = optionList == first.option ? '1' :  '2';
    template.header.cy = y+2;
    template.header.style &= ~WS_VSCROLL;
}

void
EngineOptionsPopup(HWND hwnd, ChessProgramState *cps)
{
    FARPROC lpProc = MakeProcInstance( (FARPROC) SettingsProc, hInst );

    activeCps = cps; activeList = cps->option;
    snprintf(title, MSG_SIZ, _("%s Engine Settings (%s)"), T_(cps->which), cps->tidy);
    DesignOptionDialog(cps->nrOptions, cps->option);
    CreateDialogTemplate(layoutList, layout, cps->option);


    DialogBoxIndirect( hInst, &template.header, hwnd, (DLGPROC)lpProc );

    FreeProcInstance(lpProc);

    return;
}

int EnterGroup P((HWND hDlg));

static int engineNr, selected;

int InstallOK()
{
    if(selected >= 0) { ASSIGN(engineLine, engineList[selected]); }
    if(engineLine[0] == '#') { DisplayError(_("Select single engine from the group"), 0); return 0; }
    if(isUCCI) isUCI = 2;
    if(!engineNr) Load(&first, 0); else Load(&second, 1);
    return 1;
}

Option installOptions[] = {
//  {   0,  0,    0, NULL, (void*) &engineLine, (char*) engineMnemonic, engineList, ComboBox, N_("Select engine from list:") },
  { 195, 14,    0, NULL, (void*) &EnterGroup, (char*) &selected, engineMnemonic, ListBox, N_("Select engine from list:") },
  {   0,  0,    0, NULL, NULL, NULL, NULL, Label, N_("or specify one below:") },
  {   0,  0,    0, NULL, (void*) &nickName, NULL, NULL, TextBox, N_("Nickname (optional):") },
  {   0,  0,    0, NULL, (void*) &useNick, NULL, NULL, CheckBox, N_("Use nickname in PGN tag") },
  {   0,  0, 32+3, NULL, (void*) &engineName, NULL, NULL, FileName, N_("Engine (.exe or .jar):") },
  {   0,  0,    0, NULL, (void*) &params, NULL, NULL, TextBox, N_("command-line parameters:") },
  {   0,  0,    0, NULL, (void*) &wbOptions, NULL, NULL, TextBox, N_("Special WinBoard options:") },
  {   0,  0,    0, NULL, (void*) &engineDir, NULL, NULL, PathName, N_("directory:") },
  {  95,  0,    0, NULL, NULL, NULL, NULL, Label, N_("(Directory will be derived from engine path when left empty)") },
  {   0,  0,    0, NULL, (void*) &addToList, NULL, NULL, CheckBox, N_("Add this engine to the list") },
  {   0,  0,    0, NULL, (void*) &hasBook, NULL, NULL, CheckBox, N_("Must not use GUI book") },
  {   0,  0,    0, NULL, (void*) &storeVariant, NULL, NULL, CheckBox, N_("Force current variant with this engine") },
  {   0,  0,    0, NULL, (void*) &isUCI, NULL, NULL, CheckBox, N_("UCI") },
  {   0,  0,    0, NULL, (void*) &v1, NULL, NULL, CheckBox, N_("WB protocol v1 (skip waiting for features)") },
  {   0,  0,    0, NULL, (void*) &isUCCI, NULL, NULL, CheckBox, N_("UCCI / USI (uses specified /uxiAdapter)") },
  {   0,  1,    0, NULL, (void*) &InstallOK, "", NULL, EndMark , "" }
};

void
GenericPopup(HWND hwnd, Option *optionList)
{
    FARPROC lpProc = MakeProcInstance( (FARPROC) SettingsProc, hInst );
    int n=0;

    while(optionList[n].type != EndMark) n++;
    activeCps = NULL; activeList = optionList;
    DesignOptionDialog(n, optionList);
    CreateDialogTemplate(layoutList, layout, optionList);

    DialogBoxIndirect( hInst, &template.header, hwnd, (DLGPROC)lpProc );

    FreeProcInstance(lpProc);

    return;
}

int
EnterGroup(HWND hDlg)
{
    char buf[MSG_SIZ];
    HANDLE hwndCombo = GetDlgItem(hDlg, 2001+2*1);
    int i = SendDlgItemMessage(hDlg, 2001+2*1, LB_GETCURSEL, 0, 0);
    if(i == 0) buf[0] = NULLCHAR; // back to top level
    else if(engineList[i][0] == '#') safeStrCpy(buf, engineList[i], MSG_SIZ); // group header, open group
    else {
	ASSIGN(engineLine, engineList[i]);
	if(isUCCI) isUCI = 2;
	if(!engineNr) Load(&first, 0); else Load(&second, 1);
	EndDialog( hDlg, 0 );
	return 0; // normal line, select engine
    }
    installOptions[0].max = NamesToList(firstChessProgramNames, engineList, engineMnemonic, buf); // replace list by only the group contents
    SendMessage(hwndCombo, LB_RESETCONTENT, 0, 0);
    SendMessage(hwndCombo, LB_ADDSTRING, 0, (LPARAM) buf);
    for(i=1; i<installOptions[0].max; i++) {
	    SendMessage(hwndCombo, LB_ADDSTRING, 0, (LPARAM) engineMnemonic[i]);
    }
//    SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM) 0, (LPARAM) buf);
    return 0;
}

void LoadEnginePopUp(HWND hwnd, int nr)
{
    isUCI = isUCCI = storeVariant = v1 = useNick = FALSE; addToList = hasBook = TRUE; // defaults
    engineNr = nr;
    if(engineDir)    free(engineDir);    engineDir = strdup("");
    if(params)       free(params);       params = strdup("");
    if(nickName)     free(nickName);     nickName = strdup("");
    if(engineLine)   free(engineLine);   engineLine = strdup("");
    if(engineName)   free(engineName);   engineName = strdup("");
    ASSIGN(wbOptions, "");
    installOptions[0].max = NamesToList(firstChessProgramNames, engineList, engineMnemonic, ""); // only top level
    snprintf(title, MSG_SIZ, _("Load %s Engine"), nr ? _("second") : _("first"));

    GenericPopup(hwnd, installOptions);
}

int PickTheme P((HWND hDlg));
void DeleteTheme P((HWND hDlg));

int ThemeOK()
{
    if(selected >= 0) { ASSIGN(engineLine, engineList[selected]); }
    if(engineLine[0] == '#') { DisplayError(_("Select single theme from the group"), 0); return 0; }
    LoadTheme();
    return 1;
}

Option themeOptions[] = {
  { 195, 14,    0, NULL, (void*) &PickTheme, (char*) &selected, engineMnemonic, ListBox, N_("Select theme from list:") },
  {   0,  0,    0, NULL, NULL, NULL, NULL, Label, N_("or specify new theme below:") },
  {   0,  0,    0, NULL, (void*) &nickName, NULL, NULL, TextBox, N_("Theme name:") },
  {   0,  0,    0, NULL, (void*) &appData.useBitmaps, NULL, NULL, CheckBox, N_("Use board textures") },
  {   0,  0, 32+0, NULL, (void*) &appData.liteBackTextureFile, NULL, NULL, FileName, N_("Light-square texture:") },
  {   0,  0, 32+0, NULL, (void*) &appData.darkBackTextureFile, NULL, NULL, FileName, N_("Dark-square texture:") },
  {   0,  0,    3, NULL, (void*) &appData.darkBackTextureMode, "", NULL, Spin, N_("Dark reorientation mode:") },
  {   0,  0,    3, NULL, (void*) &appData.liteBackTextureMode, "", NULL, Spin, N_("Light reorientation mode:") },
  {   0,  0,    0, NULL, (void*) &appData.useBorder, NULL, NULL, CheckBox, N_("Draw border around board") },
  {   0,  0, 32+0, NULL, (void*) &appData.border, NULL, NULL, FileName, N_("Optional border bitmap:") },
  {   0,  0,    0, NULL, NULL, NULL, NULL, Label, N_("        Beware: a specified piece font will prevail over piece bitmaps") },
  {   0,  0,    0, NULL, (void*) &appData.pieceDirectory, NULL, NULL, PathName, N_("Directory with piece bitmaps:") },
  {   0,  0,    0, NULL, (void*) &appData.useFont, NULL, NULL, CheckBox, N_("Use piece font") },
  {   0, 50,  150, NULL, (void*) &appData.fontPieceSize, "", NULL, Spin, N_("Font size (%):") },
  {   0,  0,    0, NULL, (void*) &appData.renderPiecesWithFont, NULL, NULL, TextBox, N_("Font name:") },
  {   0,  0,    0, NULL, (void*) &appData.fontToPieceTable, NULL, NULL, TextBox, N_("Font piece to char:") },
//  {   0,  0,    0, NULL, (void*) &DeleteTheme, NULL, NULL, Button, N_("Up") },
//  {   0,  0,    0, NULL, (void*) &DeleteTheme, NULL, NULL, Button, N_("Down") },
  {   0,  0,    0, NULL, (void*) &DeleteTheme, NULL, NULL, Button, N_("Delete Theme") },
  {   0,  1,    0, NULL, (void*) &ThemeOK, "", NULL, EndMark , "" }
};

void
DeleteTheme (HWND hDlg)
{
    char *p, *q;
    int i, selected = SendDlgItemMessage(hDlg, 2001+2*1, LB_GETCURSEL, 0, 0);
    HANDLE hwndCombo = GetDlgItem(hDlg, 2001+2*1);
    if(selected < 0) return;
    if(p = strstr(appData.themeNames, engineList[selected])) {
	if(q = strchr(p, '\n')) strcpy(p, q+1);
    }
    themeOptions[0].max = NamesToList(appData.themeNames, engineList, engineMnemonic, ""); // replace list by only the group contents
    SendMessage(hwndCombo, LB_RESETCONTENT, 0, 0);
    SendMessage(hwndCombo, LB_ADDSTRING, 0, (LPARAM) "");
    for(i=1; i<themeOptions[0].max; i++) {
	    SendMessage(hwndCombo, LB_ADDSTRING, 0, (LPARAM) engineMnemonic[i]);
    }
}

int
PickTheme (HWND hDlg)
{
    char buf[MSG_SIZ];
    HANDLE hwndCombo = GetDlgItem(hDlg, 2001+2*1);
    int i = SendDlgItemMessage(hDlg, 2001+2*1, LB_GETCURSEL, 0, 0);
    if(i == 0) buf[0] = NULLCHAR; // back to top level
    else if(engineList[i][0] == '#') safeStrCpy(buf, engineList[i], MSG_SIZ); // group header, open group
    else {
	ASSIGN(engineLine, engineList[i]);
	LoadTheme();
	EndDialog( hDlg, 0 );
	return 0; // normal line, select engine
    }
    themeOptions[0].max = NamesToList(appData.themeNames, engineList, engineMnemonic, buf); // replace list by only the group contents
    SendMessage(hwndCombo, LB_RESETCONTENT, 0, 0);
    SendMessage(hwndCombo, LB_ADDSTRING, 0, (LPARAM) buf);
    for(i=1; i<themeOptions[0].max; i++) {
	    SendMessage(hwndCombo, LB_ADDSTRING, 0, (LPARAM) engineMnemonic[i]);
    }
    return 0;
}

void ThemeOptionsPopup(HWND hwnd)
{
    addToList = TRUE; // defaults
    if(nickName)     free(nickName);     nickName = strdup("");
    if(engineLine)   free(engineLine);   engineLine = strdup("");
    themeOptions[0].max = NamesToList(appData.themeNames, engineList, engineMnemonic, ""); // only top level
    snprintf(title, MSG_SIZ, _("Board themes"));

    GenericPopup(hwnd, themeOptions);
}

Boolean autoinc, twice, swiss;
char *tfName;

int MatchOK()
{
    if(autoinc) appData.loadGameIndex = appData.loadPositionIndex = -(twice + 1); else
    if(!appData.loadGameFile[0]) appData.loadGameIndex = -2*twice; // kludge to pass value of "twice" for use in GUI book
    if(swiss) { appData.defaultMatchGames = 1; appData.tourneyType = -1; }
    if(CreateTourney(tfName) && !matchMode) { // CreateTourney reloads original settings if file already existed
	MatchEvent(2);
	return 1; // close dialog
    }
    return matchMode || !appData.participants[0]; // if we failed to create and are not in playing, forbid popdown if there are participants
}

void PseudoOK(HWND hDlg)
{
    okFunc = 0;
    GetOptionValues(hDlg, activeCps, activeList);
    EndDialog( hDlg, 0 );
    comboCallback = NULL; activeCps = NULL;

    if(autoinc) appData.loadGameIndex = appData.loadPositionIndex = -(twice + 1); else
    if(!appData.loadGameFile[0]) appData.loadGameIndex = -2*twice; // kludge to pass value of "twice" for use in GUI book
    if(swiss) { appData.defaultMatchGames = 1; appData.tourneyType = -1; }
}

char *GetParticipants(HWND hDlg)
{
    int len = GetWindowTextLength(GetDlgItem(hDlg, 2001+2*0)) + 1;
    char *participants,*p, *q;
    if(len < 4) return NULL; // box is empty (enough)
    participants = (char*) malloc(len);
    GetDlgItemText(hDlg, 2001+2*0, participants, len );
    p = q = participants;
    while(*p++ = *q++) if(p[-1] == '\r') p--;
    return participants;
}

void ReplaceParticipant(HWND hDlg)
{
    char *participants = GetParticipants(hDlg);
    Substitute(participants, TRUE);
}
	
void UpgradeParticipant(HWND hDlg)
{
    char *participants = GetParticipants(hDlg);
    Substitute(participants, FALSE);
}

void Inspect(HWND hDlg)
{
    FILE *f;
    char name[MSG_SIZ];
    GetDlgItemText(hDlg, 2001+2*33, name, MSG_SIZ );
    if(name[0] && (f = fopen(name, "r")) ) {
	char *saveSaveFile;
	saveSaveFile = appData.saveGameFile; appData.saveGameFile = NULL; // this is a persistent option, protect from change
	ParseArgsFromFile(f);
	autoinc = ((appData.loadPositionFile[0] ? appData.loadGameIndex : appData.loadPositionIndex) < 0);
	twice = ((appData.loadPositionFile[0] ? appData.loadGameIndex : appData.loadPositionIndex) == -2);
	swiss = appData.tourneyType < 0;
	SetOptionValues(hDlg, NULL, activeList);
	FREE(appData.saveGameFile); appData.saveGameFile = saveSaveFile;
    } else DisplayError(_("First you must specify an existing tourney file to clone"), 0);
}

void TimeControlOptionsPopup P((HWND hDlg));
void UciOptionsPopup P((HWND hDlg));
int AddToTourney P((HWND hDlg));

Option tourneyOptions[] = {
  { 80,  15,        0, NULL, (void*) &AddToTourney, NULL, engineMnemonic, ListBox, N_("Select Engine:") },
  { 0xD, 15,        0, NULL, (void*) &appData.participants, "", NULL, TextBox, N_("Tourney participants:") },
  { 0,  0,          4, NULL, (void*) &tfName, "", NULL, FileName, N_("Tournament file:") },
  { 30, 0,          0, NULL, NULL, NULL, NULL, Label, N_("If you specify an existing file, the rest of this dialog will be ignored.") },
  { 30, 0,          0, NULL, NULL, NULL, NULL, Label, N_("Otherwise, the file will be created, with the settings you specify below:") },
  { 0,  0,          0, NULL, (void*) &swiss, "", NULL, CheckBox, N_("Use Swiss pairing engine (cycles = rounds)") },
  { 0,  0,         10, NULL, (void*) &appData.tourneyType, "", NULL, Spin, N_("Tourney type (0=RR, 1=gauntlet):") },
  { 0,  0,          0, NULL, (void*) &appData.cycleSync, "", NULL, CheckBox, N_("Sync after cycle") },
  { 0,  1, 1000000000, NULL, (void*) &appData.tourneyCycles, "", NULL, Spin, N_("Number of tourney cycles:") },
  { 0,  0,          0, NULL, (void*) &appData.roundSync, "", NULL, CheckBox, N_("Sync after round") },
  { 0,  1, 1000000000, NULL, (void*) &appData.defaultMatchGames, "", NULL, Spin, N_("Games per Match / Pairing:") },
  { 0,  0,          1, NULL, (void*) &appData.saveGameFile, "", NULL, FileName, N_("File for saving tourney games:") },
  { 0,  0,       32+1, NULL, (void*) &appData.loadGameFile, "", NULL, FileName, N_("Game File with Opening Lines:") },
  { 0, -2, 1000000000, NULL, (void*) &appData.loadGameIndex, "", NULL, Spin, N_("Game Number:") },
  { 0,  0,       32+2, NULL, (void*) &appData.loadPositionFile, "", NULL, FileName, N_("File with Start Positions:") },
  { 0, -2, 1000000000, NULL, (void*) &appData.loadPositionIndex, "", NULL, Spin, N_("Position Number:") },
  { 0,  0,          0, NULL, (void*) &autoinc, "", NULL, CheckBox, N_("Step through lines/positions in file") },
  { 0,  0, 1000000000, NULL, (void*) &appData.rewindIndex, "", NULL, Spin, N_("Rewind after (0 = never):") },
  { 0,  0,          0, NULL, (void*) &twice, "", NULL, CheckBox, N_("Use each line/position twice") },
  { 0,  0,          0, NULL, (void*) &appData.defNoBook, "", NULL, CheckBox, N_("Make all use GUI book by default") },
  { 0,  0, 1000000000, NULL, (void*) &appData.matchPause, "", NULL, Spin, N_("Pause between Games (ms):") },
  { 0,  0,          0, NULL, (void*) &ReplaceParticipant, "", NULL, Button, N_("Replace Engine") },
  { 0,  0,          0, NULL, (void*) &UpgradeParticipant, "", NULL, Button, N_("Upgrade Engine") },
  { 0,  0,          0, NULL, (void*) &TimeControlOptionsPopup, "", NULL, Button, N_("Time Control...") },
  { 0,  0,          0, NULL, (void*) &UciOptionsPopup, "", NULL, Button, N_("Common Engine...") },
  { 0,  0,          0, NULL, (void*) &Inspect, "", NULL, Button, N_("Clone Tourney") },
  { 0,  0,          0, NULL, (void*) &PseudoOK, "", NULL, Button, N_("Continue Later") },
  { 0, 0, 0, NULL, (void*) &MatchOK, "", NULL, EndMark , "" }
};

int AddToTourney(HWND hDlg)
{
  char buf[MSG_SIZ];
  HANDLE hwndCombo = GetDlgItem(hDlg, 2001+2*1);
  int i = SendDlgItemMessage(hDlg, 2001+2*1, LB_GETCURSEL, 0, 0);
  if(i<0) return 0;
  if(i == 0) buf[0] = NULLCHAR; // back to top level
  else if(engineList[i][0] == '#') safeStrCpy(buf, engineList[i], MSG_SIZ); // group header, open group
  else { // normal line, select engine
    snprintf(buf, MSG_SIZ, "%s\r\n", engineMnemonic[i]);
    SendMessage( GetDlgItem(hDlg, 2001+2*0), EM_SETSEL, 99999, 99999 );
    SendMessage( GetDlgItem(hDlg, 2001+2*0), EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) buf );
    return 0;
  }
  tourneyOptions[0].max = NamesToList(firstChessProgramNames, engineList, engineMnemonic, buf); // replace list by only the group contents
  SendMessage(hwndCombo, LB_RESETCONTENT, 0, 0);
  SendMessage(hwndCombo, LB_ADDSTRING, 0, (LPARAM) buf);
  for(i=1; i<tourneyOptions[0].max; i++) {
    SendMessage(hwndCombo, LB_ADDSTRING, 0, (LPARAM) engineMnemonic[i]);
  }
//  SendMessage(hwndCombo, CB_SELECTSTRING, (WPARAM) 0, (LPARAM) buf);
  return 0;
}

void TourneyPopup(HWND hwnd)
{
    int n = NamesToList(firstChessProgramNames, engineList, engineMnemonic, "");
    autoinc = appData.loadGameIndex < 0 || appData.loadPositionIndex < 0;
    twice = appData.loadGameIndex == -2 || appData.loadPositionIndex == -2; swiss = appData.tourneyType < 0;
    tourneyOptions[0].max = n;
    snprintf(title, MSG_SIZ, _("Tournament and Match Options"));
    ASSIGN(tfName, appData.tourneyFile[0] ? appData.tourneyFile : MakeName(appData.defName));

    GenericPopup(hwnd, tourneyOptions);
}
