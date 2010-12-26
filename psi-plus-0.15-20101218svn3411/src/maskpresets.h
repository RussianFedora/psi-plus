#ifndef MASKPRESETS_H
#define MASKPRESETS_H

#define CLIENTS_PRESET 11
#define OS_PRESET      15

#define OS_USERDEF_PRESET 0
#define OS_DEFAULT_PRESET 1

#define CLIENT_USERDEF_PRESET 0
#define CLIENT_DEFAULT_PRESET 1

typedef struct
{
    QString name, version, capsnode, capsver;
} Clientpreset;

QString ospresetname[OS_PRESET] = {
	 "user defined"
	,"default"
	,"Windows 98"
	,"Windows ME"
	,"Windows 2000"
	,"Windows XP"
	,"Windows Server 2003"
	,"Windows Vista"
	,"Windows 7"
	,"Linux"
	,"Plan9"
	,"FreeBSD"
	,"NetBSD"
	,"OpenBSD"
	,"Solaris"
};

Clientpreset clientpreset[CLIENTS_PRESET] = {
	 {"user defined", "", "", ""}
	,{"default",    "", "", ""}
	,{"Bombus",     "0.7.1429M-Zlib", "http://bombus-im.org/java",          "0.7.1429M-Zlib"}
	,{"Gajim",      "0.12.5",         "http://gajim.org",                   "0.12.5"        }
	,{"Mcabber",    "0.9.10",         "http://mcabber.com/caps",            "0.9.10"        }
	,{"Miranda",    "0.9.0.1",        "http://miranda-im.org/caps",         "0.9.0.1"       }
	,{"Pidgin",     "2.5.9",          "http://pidgin.im",                   "2.5.9"         }
	,{"Psi",        "0.13-dev",       "http://psi-im.org/caps",             "0.13-dev-rev2" }
	,{"QIP Infium", "9032",           "http://qip.ru/caps",                 "9032"          }
	,{"Talisman",   "0.1.1.15",       "http://jabrvista.net.ru",            "0.1.1.15"      }
	,{"Tkabber",    "0.11.1",         "http://tkabber.jabber.ru",           "0.11.1"        }
};

#endif

