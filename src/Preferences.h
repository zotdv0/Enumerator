#pragma once
#include "PluginDefinition.h"

enum InsertMode { AtSelectionStart, AtSelectionEnd, ReplaceSelection };
enum SortBy { IndexAsc, IndexDesc, PositionAsc, PositionDesc };

class Preferences {
public:
	SSIZE_T start_number;
	SSIZE_T increment;
	LPSTR number_format;
	InsertMode insert_mode;

	Preferences();
	~Preferences();

	void set_defaults();
	void init(const NppData &);
	void read();
	void write();

	TCHAR INI_PATH[MAX_PATH];
protected:

	LPCTSTR INI_FILENAME = NPP_PLUGIN_NAME_DEF ".ini";
	LPCTSTR INI_SECTION = TEXT("Preferences");
	LPCTSTR PREF_START_NUMBER = TEXT("StartNumber");
	LPCTSTR PREF_INCREMENT = TEXT("Increment");
	LPCTSTR PREF_NUMBER_FORMAT = TEXT("NumberFormat");
	LPCTSTR PREF_INSERT_MODE = TEXT("InsertMode");
};