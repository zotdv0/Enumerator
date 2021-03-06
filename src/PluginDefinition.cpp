//this file is part of notepad++
//Copyright (C)2021 Denis Zotov <zot.dv0@gmail.com>
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <strsafe.h>
#include <cstdlib>
#include "Preferences.h"

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;
Preferences prefs;
HINSTANCE gModule;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
    gModule = (HINSTANCE)hModule;
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
    prefs.init(nppData);

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("Ascending by index"), enumerateSelectionsByIndexAsc);
    setCommand(1, TEXT("Ascending by position"), enumerateSelectionsByPositionAsc);
    setCommand(2, TEXT("Descending by index"), enumerateSelectionsByIndexDesc);
    setCommand(3, TEXT("Descending by position"), enumerateSelectionsByPositionDesc);

}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
    // Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR* cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey* sk, bool check0nInit)
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//

inline LRESULT callNPP(HWND hwnd, UINT message, LPARAM arg0 = NULL, LPARAM arg1 = NULL)
{
    return ::SendMessage(hwnd, message, arg0, arg1);
}

HWND getScintilla()
{
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, NULL, (LPARAM)&which);
    switch (which)
    {
    case 0: return nppData._scintillaMainHandle;
    case 1: return nppData._scintillaSecondHandle;
    default: return NULL;
    }
}

typedef long long pos_t;

struct Selection { size_t index; pos_t position; bool isEmpty; };

// Compare selections by position
int comparePositions(LPCVOID a, LPCVOID b)
{
    return (int)(((Selection*)a)->position - ((Selection*)b)->position);
}

// Generic enumeration function
void enumerateGeneric(bool byPosition = false, bool desc = false)
{
    const HWND curScintilla = getScintilla();

    // Start section for undo
    callNPP(curScintilla, SCI_BEGINUNDOACTION);

    // Initialize selections
    const size_t selectionsCount = callNPP(curScintilla, SCI_GETSELECTIONS);
    Selection* selections = new Selection[selectionsCount];
    for (size_t index = 0; index < selectionsCount; ++index)
    {
        selections[index].index = index;
        selections[index].position = callNPP(curScintilla, SCI_GETSELECTIONNSTART, index);
        pos_t selectionEnd = callNPP(curScintilla, SCI_GETSELECTIONNEND, index);
        selections[index].isEmpty = (selections[index].position == selectionEnd);
    }

    // Sort by position
    if (byPosition)
    {
        qsort(selections, selectionsCount, sizeof(Selection), comparePositions);
    }

    long long number = prefs.start_number;
    const size_t offset = desc ? selectionsCount - 1 : 0;
    pos_t selStart = 0, selEnd = 0;
    char* buffer = new char[128 * MB_LEN_MAX];
    for (size_t index = 0; index < selectionsCount; ++index)
    {
        Selection& curSelection = selections[desc ? selectionsCount - 1 - index : index];

        // Skip empty selection
        if (prefs.skip_empty && curSelection.isEmpty)
        {
            continue;
        }

        // Locate current selection
        switch (prefs.insert_mode)
        {
        case InsertMode::AtSelectionStart:
            selStart = callNPP(curScintilla, SCI_GETSELECTIONNSTART, curSelection.index);
            selEnd = selStart;
            break;
        case InsertMode::AtSelectionEnd:
            selStart = callNPP(curScintilla, SCI_GETSELECTIONNEND, curSelection.index);
            selEnd = selStart;
            break;
        case InsertMode::ReplaceSelection:
            selStart = callNPP(curScintilla, SCI_GETSELECTIONNSTART, curSelection.index);
            selEnd = callNPP(curScintilla, SCI_GETSELECTIONNEND, curSelection.index);
            break;
        }
        callNPP(curScintilla, SCI_SETTARGETRANGE, selStart, selEnd);

        // Write number
        StringCbPrintfA(buffer, 128 * MB_LEN_MAX, prefs.number_format, number);
        callNPP(curScintilla, SCI_REPLACETARGET, -1, (LPARAM)buffer);

        // Set next number
        number += prefs.increment;
    }

    // End section for undo
    callNPP(curScintilla, SCI_ENDUNDOACTION);

    // Clean up
    delete[] selections;
}

// Enumerating funcions realization
void enumerateSelectionsByIndexAsc() { enumerateGeneric(false, false); }
void enumerateSelectionsByIndexDesc() { enumerateGeneric(false, true); }
void enumerateSelectionsByPositionAsc() { enumerateGeneric(true, false); }
void enumerateSelectionsByPositionDesc() { enumerateGeneric(true, true); }

