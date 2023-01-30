#pragma once

// Suppress external warnings
#include "WarningGuardOn.h"

// Define target system to Windows 10
// https://docs.microsoft.com/pl-pl/cpp/porting/modifying-winver-and-win32-winnt?view=vs-2019
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

// Exclude unused WinAPI stuff
// https://stackoverflow.com/questions/1394910/how-to-tame-the-windows-headers-useful-defines

#ifndef _ZE_USE_WINDOWS_DEFINES
 // Cryptography, DDE, RPC, Shell, and Windows Sockets
#define WIN32_LEAN_AND_MEAN
// CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOGDICAPMASKS
// MF_*
#define NOMENUS
// IDI_*
#define NOICONS
// SC_*
#define NOSYSCOMMANDS
// Binary and Tertiary raster ops
#define NORASTEROPS
// OEM Resource values
#define OEMRESOURCE
// Atom Manager routines
#define NOATOM
// DrawText() and DT_*
#define NODRAWTEXT
// GMEM_*, LMEM_*, GHND, LHND, associated routine
#define NOMEMMGR
// typedef METAFILEPICT
#define NOMETAFILE
// OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOOPENFILE
// SB_* and scrolling routines
#define NOSCROLL
// All Service Controller routines, SERVICE_ equates, etc.
#define NOSERVICE
// Sound driver routines
#define NOSOUND
// typedef TEXTMETRIC and associated routines
#define NOTEXTMETRIC
// SetWindowsHook and WH_*
#define NOWH
// COMM driver routines
#define NOCOMM
// Kanji support stuff
#define NOKANJI
// Help engine interface
#define NOHELP
// Profiler interface
#define NOPROFILER
// DeferWindowPos routines
#define NODEFERWINDOWPOS
// Modem Configuration Extensions
#define NOMCX
// No Remote Procedure Calls (client-server)
#define NORPC
// No proxy
#define NOPROXYSTUB
// No windows images
#define NOIMAGE
// No tape device support
#define NOTAPE
// All KERNEL defines and routines
#define NOKERNEL
// Min/Max macros
#ifndef NOMINMAX
#define NOMINMAX
#endif
// No ANSI methods
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

// Used for display manipulation:
//
// Screen colors
// #define NOCOLOR
// SM_*
// #define NOSYSMETRICS

// Used by ImGui:
//
// Control and Dialog routines
// #define NOCTLMGR
// Clipboard routines
// #define NOCLIPBOARD
#endif

// Strict use of WinAPI types, errors on unwanted conversion between
#define STRICT 1
// Allow cmath defines
#define _USE_MATH_DEFINES

#include <Windows.h>
#include "WarningGuardOff.h"