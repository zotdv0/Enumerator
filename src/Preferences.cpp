#include "Preferences.h"
#include "PluginDefinition.h"
#include <strsafe.h>
#include <Shlwapi.h>

#define DEFAULT_START_NUMBER (0)
#define DEFAULT_INCREMENT (1)
#define DEFAULT_INSERT_MODE (InsertMode::AtSelectionStart)
#define DEFAULT_NUMBER_FORMAT TEXT("%d. ")
#define NUMBER_FORMAT_SIZE (256)
#define DEFAULT_SKIP_EMPTY (FALSE)


Preferences::Preferences()
{
	this->number_format = new char[NUMBER_FORMAT_SIZE];
	set_defaults();
}

Preferences::~Preferences()
{
	delete[] this->number_format;
}

void Preferences::set_defaults()
{
	this->start_number = DEFAULT_START_NUMBER;
	this->increment = DEFAULT_INCREMENT;
	this->insert_mode = DEFAULT_INSERT_MODE;
	wcstombs(this->number_format, DEFAULT_NUMBER_FORMAT, NUMBER_FORMAT_SIZE);
	this->skip_empty = DEFAULT_SKIP_EMPTY;
}

void Preferences::init(const NppData& nppData)
{
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)this->INI_PATH);
	if (!PathFileExists(this->INI_PATH))
	{
		CreateDirectory(this->INI_PATH, NULL);
	}
	PathAppend(this->INI_PATH, Preferences::INI_FILENAME);
	if (PathFileExists(this->INI_PATH))
	{
		this->read();
	}
	else
	{
		this->write();
	}
}

#define READ_PREF_INT(name) GetPrivateProfileInt(Preferences::INI_SECTION, (Preferences::PREF_##name), (DEFAULT_##name), this->INI_PATH)
#define READ_PREF_STR(str, name) GetPrivateProfileString(Preferences::INI_SECTION, (Preferences::PREF_##name), (DEFAULT_##name), (str), (name##_SIZE), this->INI_PATH)
void Preferences::read()
{
	this->start_number = READ_PREF_INT(START_NUMBER);
	this->increment = READ_PREF_INT(INCREMENT);
	this->insert_mode = (InsertMode)READ_PREF_INT(INSERT_MODE);
	this->skip_empty = READ_PREF_INT(SKIP_EMPTY);
	TCHAR buf[NUMBER_FORMAT_SIZE];
	READ_PREF_STR(buf, NUMBER_FORMAT);
	wcstombs(this->number_format, buf, NUMBER_FORMAT_SIZE);
}

#define FORMAT_BUF(var, format) StringCbPrintf(buf, buf_size, TEXT(format), (var))
#define WRITE_BUF(name) WritePrivateProfileString(Preferences::INI_SECTION, (Preferences::PREF_##name), buf, this->INI_PATH)
#define WRITE_PREF_INT(var, name) FORMAT_BUF((SSIZE_T)(var), "%lld"); WRITE_BUF(name)
#define WRITE_PREF_STR(var, name) FORMAT_BUF(var, "\"%hs\""); WRITE_BUF(name)
void Preferences::write()
{
	const size_t buf_size = NUMBER_FORMAT_SIZE + 2*sizeof(TCHAR);
	TCHAR buf[buf_size];
	
	WRITE_PREF_INT(this->start_number, START_NUMBER);
	WRITE_PREF_INT(this->increment, INCREMENT);
	WRITE_PREF_INT(this->insert_mode, INSERT_MODE);
	WRITE_PREF_INT(this->skip_empty, SKIP_EMPTY);
	WRITE_PREF_STR(this->number_format, NUMBER_FORMAT);
	
}
