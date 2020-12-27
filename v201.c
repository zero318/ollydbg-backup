/*
 * Copyright (c) 2013 Toni Spets <toni.spets@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "backup.h"
#include "list.h"

#include "v201.h"
//

#define PLUGINNAME      L"Ultra Backup"
#define PLUGINDESC      L"Backup and restore current labels and comments."

typedef struct NAME_TYPE {
    const int type;
    const char type_string[256];
} NAME_TYPE;

const NAME_TYPE LABEL_TYPE = { NM_LABEL, "LABEL" };
const NAME_TYPE COMMENT_TYPE = { NM_COMMENT, "COMMENT" };
const NAME_TYPE EXPORT_TYPE = { NM_EXPORT, "EXPORT" };
const NAME_TYPE DEMANGLED_EXPORT_TYPE = { NM_DEEXP, "DEMANGLED_EXPORT" };
const NAME_TYPE IMPORT_TYPE = { NM_IMPORT, "IMPORT" };
const NAME_TYPE DEMANGLED_IMPORT_TYPE = { NM_DEIMP, "DEMANGLED_IMPORT" };
const NAME_TYPE DEBUG_TYPE = { NM_DEBUG, "DEBUG" };
const NAME_TYPE DEMANGLED_DEBUG_TYPE = { NM_DEDEBUG, "DEMANGLED_DEBUG" };
const NAME_TYPE ANALYSIS_LABEL_TYPE = { NM_ANLABEL, "ANALYSIS_LABEL" };
const NAME_TYPE ANALYSIS_COMMENT_TYPE = { NM_ANALYSE, "ANALYSIS_COMMENT" };
const NAME_TYPE PARAMETER_TYPE = { NM_MARK, "PARAMETER" };
const NAME_TYPE FUNC_CALL_TYPE = { NM_CALLED, "FUNC_CALL" };
const NAME_TYPE RETURN_TYPE_TYPE = { NM_RETTYPE, "RETURN_TYPE" };
const NAME_TYPE MODULE_TYPE = { NM_MODCOMM, "MODULE" };
const NAME_TYPE TRICKY_TYPE = { NM_TRICK, "TRICKY" };
const NAME_TYPE MANGLED_ANALYSIS_LABEL_TYPE = { NM_ANLABEL + 0x1, "MANGLED_ANALYSIS_LABEL" };

static const NAME_TYPE * UserNameTypes[] = {
    &LABEL_TYPE,
    &COMMENT_TYPE,
    NULL
};

static const NAME_TYPE * SystemNameTypes[] = {
    &EXPORT_TYPE,
    &DEMANGLED_EXPORT_TYPE,
    &IMPORT_TYPE,
    &DEMANGLED_IMPORT_TYPE,
    &ANALYSIS_LABEL_TYPE,
    NULL
};

static const NAME_TYPE * FuncCallsNameTypes[] = {
    &RETURN_TYPE_TYPE,
    &FUNC_CALL_TYPE,
    &PARAMETER_TYPE,
    NULL
};

static const NAME_TYPE * AllNameTypes[] = {
    &LABEL_TYPE,
    &COMMENT_TYPE,
    &EXPORT_TYPE,
    &DEMANGLED_EXPORT_TYPE,
    &IMPORT_TYPE,
    &DEMANGLED_IMPORT_TYPE,
    &DEBUG_TYPE,
    &DEMANGLED_DEBUG_TYPE,
    &ANALYSIS_LABEL_TYPE,
    &MANGLED_ANALYSIS_LABEL_TYPE,
    &ANALYSIS_COMMENT_TYPE,
    &PARAMETER_TYPE,
    &FUNC_CALL_TYPE,
    &RETURN_TYPE_TYPE,
    &MODULE_TYPE,
    &TRICKY_TYPE,
    NULL
};

static const int RawTypeLookup[] = {
    [NM_LABEL] = 0,
    [NM_COMMENT] = 1,
    [NM_EXPORT] = 2,
    [NM_DEEXP] = 3,
    [NM_IMPORT] = 4,
    [NM_DEIMP] = 5,
    [NM_DEBUG] = 6,
    [NM_DEDEBUG] = 7,
    [NM_ANLABEL] = 8,
    [NM_ANLABEL+1] = 9,
    [NM_ANALYSE] = 10,
    [NM_MARK] = 11,
    [NM_CALLED] = 12,
    [NM_RETTYPE] = 13,
    [NM_MODCOMM] = 14,
    [NM_TRICK] = 15
};

static void LoadFromFile(t_module *module, const wchar_t *filename);
static void SaveToFile(t_module *module, const wchar_t *filename, const NAME_TYPE** Names);

static bool initialized = false;

extc int _export cdecl ODBG2_Pluginquery(int ollydbgversion, ulong *features, wchar_t pluginname[SHORTNAME], wchar_t pluginversion[SHORTNAME])
{
    if (ollydbgversion < 201)
        return 0;

    initialized = true;

    wcscpy_s(pluginname, SHORTNAME, PLUGINNAME);
    wcscpy_s(pluginversion, SHORTNAME, REV);
    return PLUGIN_VERSION;
}

static int menucb(t_table *pt, wchar_t *name, ulong index, int mode)
{
    if (!initialized)
        return MENU_GRAYED;

    t_module *module = Findmainmodule();

    if (module == NULL)
        return MENU_GRAYED;

    if (mode == MENU_VERIFY)
        return MENU_NORMAL;

    if (mode == MENU_EXECUTE) {

        wchar_t buf[MAXPATH];
        wcscpy_s(buf, _countof(buf), module->path);

        wchar_t *last_stop = wcsrchr(buf, L'.');

        if (!last_stop) {
            return MENU_ABSENT;
        }

        *last_stop= L'\0';

        wchar_t* temp;
        const NAME_TYPE** name_types;

        switch (index) {
            if (0) {
            case 0:
                temp = L"-user.csv";
                name_types = UserNameTypes;
            } else if (0) {
            case 4:
                temp = L"-system.csv";
                name_types = SystemNameTypes;
            } else if (0) {
            case 8:
                temp = L"-func-calls.csv";
                name_types = FuncCallsNameTypes;
            } else {
            case 12:
                temp = L"-all.csv";
                name_types = AllNameTypes;
            }
                wcscat_s(buf, _countof(buf), temp);
                SaveToFile(module, buf, name_types);
                break;

            //if (0) {
            ////case :
            //    temp = L"-smart-user.csv";
            //    name_types = UserNameTypes;
            //} else if (0) {
            //case 16:
            //    temp = L"-smart-system.csv";
            //    name_types = SystemNameTypes;
            //} else if (0) {
            ////case 8:
            //    temp = L"-smart-func-calls.csv";
            //    name_types = FuncCallsNameTypes;
            //} else {
            ////case 12:
            //    temp = L"-smart-all.csv";
            //    name_types = AllNameTypes;
            //}
            //    wcscat_s(buf, _countof(buf), temp);
            //    SmartSaveToFile(module, buf, name_types);
            //    break;


            if (0) {
            case 1:
                temp = L"-user";
                name_types = UserNameTypes;
            } else if (0) {
            case 5:
                temp = L"-system";
                name_types = SystemNameTypes;
            } else if (0) {
            case 9:
                temp = L"-func-calls";
                name_types = FuncCallsNameTypes;
            } else {
            case 13:
                temp = L"-all";
                name_types = AllNameTypes;
            }
            {
                wchar_t tbuf[32];
                wcscat_s(buf, _countof(buf), temp);
                GetDateFormatW(LOCALE_USER_DEFAULT, 0, NULL, L"-yyyy''MM''dd", tbuf, _countof(tbuf));
                wcscat_s(buf, _countof(buf), tbuf);
                GetTimeFormatW(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, NULL, L"_hh''mm''ss", tbuf, sizeof tbuf);
                GetTimeFormatW(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, NULL, L"_hh''mm''ss", tbuf, _countof(tbuf));
                wcscat_s(buf, _countof(buf), tbuf);
                wcscat_s(buf, _countof(buf), L".csv");
                SaveToFile(module, buf, name_types);
                break;
            }

            if (0) {
            case 2:
                temp = L"-user.csv";
            } else if (0) {
            case 6:
                temp = L"-system.csv";
            } else if (0) {
            case 10:
                temp = L"-func-calls.csv";
            } else {
            case 14:
                temp = L"-all.csv";
            }
                wcscat_s(buf, _countof(buf), temp);
                LoadFromFile(module, buf);
                break;

            if (0) {
            case 3:
                temp = L"-user.csv";
            } else if (0) {
            case 7:
                temp = L"-system.csv";
            } else if (0) {
            case 11:
                temp = L"-func-calls.csv";
            } else {
            case 15:
                temp = L"-all.csv";
            }
                wcscat_s(buf, _countof(buf), temp);
                if (Browsefilename(L"Select a User CSV file...", buf, NULL, NULL, L".csv", NULL, 0)) {
                    LoadFromFile(module, buf);
                }
                break;
        }
    }

    return MENU_ABSENT;
}

t_menu pluginmenu[] = {
    {
        L"&Save User Labels to MODULE-user.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 0 }
    },
    {
        L"S&ave User Labels to MODULE-user-YYYYMMDD_HHMMSS.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 1 }
    },
    {
        L"&Load User Labels from MODULE-user.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 2 }
    },
    {
        L"L&oad User Labels from...",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 3 }
    },
    {
        L"Save S&ystem Labels to MODULE-system.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 4 }
    },
    {
        L"Save System Labels to MODULE-system-YYYYMMDD_HHMMSS.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 5 }
    },
    {
        L"Load System Labels from MODULE-system.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 6 }
    },
    {
        L"Load System Labels from...",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 7 }
    },
    {
        L"Save Func Calls to MODULE-func-calls.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 8 }
    },
    {
        L"Save Func Calls to MODULE-func-calls-YYYYMMDD_HHMMSS.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 9 }
    },
    {
        L"Load Func Calls from MODULE-func-calls.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 10 }
    },
    {
        L"Load Func Calls from...",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 11 }
    },
    {
        L"Save All to MODULE-all.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 12 }
    },
    {
        L"Save All to MODULE-all-YYYYMMDD_HHMMSS.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 13 }
    },
    {
        L"Load All from MODULE-all.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 14 }
    },
    {
        L"Load All from...",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 15 }
    },
    /*{
        L"Save Smart System Labels to MODULE-user.csv",
        NULL,
        K_NONE,
        menucb,
        NULL,
        { 16 }
    },*/
    {
        NULL,
        NULL,
        K_NONE,
        NULL,
        NULL,
        { 0 }
    }
};

t_menu mainmenu[] = {
    {
        L"&" PLUGINNAME,
        PLUGINDESC,
        K_NONE,
        NULL,
        pluginmenu,
        { 0 }
    },
    {
        NULL,
        NULL,
        K_NONE,
        NULL,
        NULL,
        { 0 }
    }
};

bool backup_save_2(const char *filename, rva_t *rvas, char *message)
{
    FILE *fh = fopen(filename, "wb");

    if (!fh) {
        sprintf(message, "File %s could not be opened for writing", filename);
        return false;
    }

    if (!rvas) {
        strcpy(message, "Nothing to save");
        return false;
    }

    fprintf(fh, "RVA,label_type,label\r\n");

    int labels = 0;
    int comments = 0;
    int exports = 0;
    int demangled_exports = 0;
    int imports = 0;
    int demangled_imports = 0;
    int debugs = 0;
    int demangled_debugs = 0;
    int analysis_labels = 0;
    int analysis_comments = 0;
    int parameters = 0;
    int func_calls = 0;
    int return_types = 0;
    int modules = 0;
    int trickies = 0;
    bool invalid;

    LIST_FOREACH (rvas, rva_t, rva) {
        invalid = false;
        switch (rva->raw_type) {
            case NM_LABEL:      ++labels; break;
            case NM_COMMENT:    ++comments; break;
            case NM_EXPORT:     ++exports; break;
            case NM_DEEXP:      ++demangled_exports; break;
            case NM_IMPORT:     ++imports; break;
            case NM_DEIMP:      ++demangled_imports; break;
            case NM_DEBUG:      ++debugs; break;
            case NM_DEDEBUG:    ++demangled_debugs; break;
            case NM_ANLABEL:    ++analysis_labels; break;
            case NM_ANLABEL + 1:  /*--rva->raw_type;*/ break;
            case NM_ANALYSE:    ++analysis_comments; break;
            case NM_MARK:       ++parameters; break;
            case NM_CALLED:     ++func_calls; break;
            case NM_RETTYPE:    ++return_types; break;
            case NM_MODCOMM:    ++modules; break;
            case NM_TRICK:      ++trickies; break;
            default:            invalid = true; break;
        }
        if (!invalid) {
            fprintf(fh, "%08X,", rva->address);

            fwrite(AllNameTypes[RawTypeLookup[rva->raw_type]]->type_string, strlen(AllNameTypes[RawTypeLookup[rva->raw_type]]->type_string), 1, fh);

            fwrite(",", 1, 1, fh);

            if (strchr(rva->name, ',') || strchr(rva->name, '"'))
                csv_fwrite(fh, rva->name, strlen(rva->name));
            else
                fwrite(rva->name, strlen(rva->name), 1, fh);

            fwrite("\r\n", 2, 1, fh);
        }
    }

    fclose(fh);

    sprintf(message, "Saved %d labels and %d comments to %s", labels, comments, filename);
    return true;
}

extc t_menu _export cdecl *ODBG2_Pluginmenu(wchar_t *type)
{
    if (lstrcmp(type, PWM_MAIN) == 0) {
        return mainmenu;
    }

    return NULL;
}

static void LoadFromFile(t_module *module, const wchar_t *filename)
{
    wchar_t unicode[TEXTLEN];
    char utf[TEXTLEN];

    char message[1024];

    Unicodetoutf(filename, wcslen(filename), utf, _countof(utf));

    rva_t *rvas = backup_load(utf, message);

    if (rvas == NULL) {
        Utftounicode(message, strlen(message), unicode, _countof(unicode));
        Flash(unicode);
        return;
    }

    bool invalid;
    int parse_type;

    LIST_FOREACH (rvas, rva_t, rva) {
        if (rva->type[0]) {
            invalid = false;
            switch (rva->type[0]) {
                case 'L':   parse_type = NM_LABEL; break;
                case 'E':   parse_type = NM_EXPORT; break;
                case 'I':   parse_type = NM_IMPORT; break;
                case 'P':   parse_type = NM_MARK; break;
                case 'R':   parse_type = NM_RETTYPE; break;
                case 'M':   parse_type = NM_MODCOMM; break;
                case 'T':   parse_type = NM_TRICK; break;
                case 'C':   parse_type = rva->type[1] == 'A' ? NM_CALLED : NM_COMMENT; break;
                case 'A':   parse_type = strlen(rva->type) > 9 && rva->type[9] == 'C' ? NM_ANALYSE : NM_ANLABEL; break;
                case 'D':
                    if (strlen(rva->type) > 10) {
                        switch (rva->type[10]) {
                            case 'I':   parse_type = NM_DEIMP; break;
                            case 'E':   parse_type = NM_DEEXP; break;
                            case 'D':   parse_type = NM_DEDEBUG; break;
                            case 'A':   invalid = true; break;
                            default:    invalid = true; break;
                        }
                    } else {
                        parse_type = NM_DEBUG;
                    }
                    break;
                default:    invalid = true; break;
            }
            /*if (strcmp(rva->type, "LABEL") == 0) {
                parse_type = NM_LABEL;
            } else if (strcmp(rva->type, "COMMENT") == 0) {
                parse_type = NM_COMMENT;
            } else if (strcmp(rva->type, "EXPORT") == 0) {
                parse_type = NM_EXPORT;
            } else if (strcmp(rva->type, "DEMANGLED_EXPORT") == 0) {
                parse_type = NM_DEEXP;
            } else if (strcmp(rva->type, "IMPORT") == 0) {
                parse_type = NM_IMPORT;
            } else if (strcmp(rva->type, "DEMANGLED_IMPORT") == 0) {
                parse_type = NM_DEIMP;
            } else if (strcmp(rva->type, "DEBUG") == 0) {
                parse_type = NM_DEBUG;
            } else if (strcmp(rva->type, "DEMANGLED_DEBUG") == 0) {
                parse_type = NM_DEDEBUG;
            } else if (strcmp(rva->type, "ANALYSIS_LABEL") == 0) {
                parse_type = NM_ANLABEL;
            } else if (strcmp(rva->type, "ANALYSIS_COMMENT") == 0) {
                parse_type = NM_ANALYSE;
            } else if (strcmp(rva->type, "PARAMETER") == 0) {
                parse_type = NM_MARK;
            } else if (strcmp(rva->type, "CALLS_FUNC") == 0) {
                parse_type = NM_CALLED;
            } else if (strcmp(rva->type, "RETURN_TYPE") == 0) {
                parse_type = NM_RETTYPE;
            } else if (strcmp(rva->type, "MODULE") == 0) {
                parse_type = NM_MODCOMM;
            } else if (strcmp(rva->type, "TRICKY") == 0) {
                parse_type = NM_TRICK;
            } else {
                invalid = true;
            }*/
            if (!invalid && rva->name[0]) {
                Utftounicode(rva->name, strlen(rva->name), unicode, _countof(unicode));
                QuickinsertnameW(module->base + rva->address, parse_type, unicode);
            }
        }
    }

    LIST_FREE(rvas);

    Mergequickdata();

    Utftounicode(message, strlen(message), unicode, _countof(unicode));
    Info(unicode);
}

static void SaveToFile(t_module *module, const wchar_t *filename, const NAME_TYPE** Names)
{
    if (Names) {
        unsigned int end = module->base + module->size;

        wchar_t unicode[TEXTLEN];
        wchar_t buffer[TEXTLEN];
        wchar_t buffer2[TEXTLEN];
        char utf[TEXTLEN];

        rva_t *rvas = NULL;

        for (unsigned int address = end; address > module->base; address--) {
            int i = 0;
            do {
                buffer[0] = L"\0";
                FindnameW(address, Names[i]->type == NM_ANLABEL + 1 ? NM_ANLABEL : Names[i]->type, buffer, _countof(buffer));
                if (buffer[0]) {
                    rva_t *rva = malloc(sizeof(rva_t));
                    rva->address = address - module->base;
                    rva->raw_type = Names[i]->type;
                    if (Names[i]->type == NM_ANLABEL) {
                        memcpy(&buffer2, &buffer, sizeof(buffer));
                        if (DemanglenameW(&buffer2, &buffer2, 0)) {
                            Unicodetoutf(buffer2, _countof(buffer2), rva->name, sizeof(rva->name));
                        } else {
                            Unicodetoutf(buffer, _countof(buffer), rva->name, sizeof(rva->name));
                        }
                    } else if (Names[i]->type == NM_ANLABEL + 1) {
                        memcpy(&buffer2, &buffer, sizeof(buffer));
                        if (DemanglenameW(&buffer2, &buffer2, 0)) {
                            Unicodetoutf(buffer, _countof(buffer), rva->name, sizeof(rva->name));
                        } else {
                            free(rva);
                            continue;
                        }
                    } else {
                        Unicodetoutf(buffer, _countof(buffer), rva->name, sizeof(rva->name));
                    }
                    LIST_INSERT(rvas, rva);
                }
            } while (Names[++i]);
        }

        char message[1024];
        Unicodetoutf(filename, wcslen(filename), utf, _countof(utf));

        if (backup_save_2(utf, rvas, message)) {
            Utftounicode(message, strlen(message), unicode, _countof(unicode));
            Info(unicode);
        } else {
            Utftounicode(message, strlen(message), unicode, _countof(unicode));
            Flash(unicode);
        }
    } else {
        Flash(L"Internal name type error");
    }
}

static void SmartSaveToFile(t_module *module, const wchar_t *filename, const NAME_TYPE** Names)
{
    if (Names) {
        unsigned int end = module->base + module->size;

        wchar_t unicode[TEXTLEN];
        wchar_t buffer[TEXTLEN];
        wchar_t buffer2[TEXTLEN];
        char utf[TEXTLEN];

        rva_t *rvas = NULL;

        for (unsigned int address = end; address > module->base; address--) {
            int i = 0;
            do {
                buffer[0] = L"\0";
                FindnameW(address, Names[i]->type, buffer, _countof(buffer));
                if (buffer[0]) {
                    rva_t *rva = malloc(sizeof(rva_t));
                    rva->address = address - module->base;
                    wchar_t* main_buffer;
                    rva->raw_type = Names[i]->type;
                    switch (Names[i]->type) {
                        case (NM_ANLABEL + 1):
                            main_buffer = DemanglenameW(&buffer, &buffer2, 0) ? &buffer2 : &buffer;
                            break;
                        default:
                            main_buffer = &buffer;
                            break;
                    }
                    switch (rva->raw_type) {
                        case NM_LABEL:
                        case NM_ANLABEL:
                            ;
                    }
                    if (Names[i]->type == NM_ANLABEL) {
                        memcpy(&buffer2, &buffer, sizeof(buffer));
                        if (DemanglenameW(&buffer, &buffer2, 0)) {
                            Unicodetoutf(buffer2, _countof(buffer2), rva->name, sizeof(rva->name));
                        } else {
                            Unicodetoutf(buffer, _countof(buffer), rva->name, sizeof(rva->name));
                        }
                    } else {
                        Unicodetoutf(buffer, _countof(buffer), rva->name, sizeof(rva->name));
                    }
                    LIST_INSERT(rvas, rva);
                }
            } while (Names[++i]);
        }

        char message[1024];
        Unicodetoutf(filename, wcslen(filename), utf, _countof(utf));

        if (backup_save_2(utf, rvas, message)) {
            Utftounicode(message, strlen(message), unicode, _countof(unicode));
            Info(unicode);
        } else {
            Utftounicode(message, strlen(message), unicode, _countof(unicode));
            Flash(unicode);
        }
    } else {
        Flash(L"Internal name type error");
    }
}