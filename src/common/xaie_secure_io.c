/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_secure_io.c
* @{
*
* Implementation of _XAie_SecureFopen().
*
* Why this wrapper exists
* -----------------------
* fopen() creates files with mode 0666 (modulo the process umask). CodeQL's
* cpp/world-writable-file-creation rule flags such calls because its umask
* flow analysis is basic-block-local and easily defeated by control flow
* between an umask() call and the matching fopen(). Working around the rule
* by setting the process umask is also wrong on its merits: umask() is
* process-global state and racy when other threads create files concurrently.
*
* The fix is to pass the mode bits explicitly to the creation primitive
* (open / _sopen_s) and wrap the resulting fd with fdopen so callers can
* keep using FILE* / fprintf.
*
* Hardening applied unconditionally
* ---------------------------------
*   POSIX:
*     O_NOFOLLOW : refuse to open `Path` if it is a symlink. Mitigates
*                  symlink-race attacks against output paths in shared
*                  directories (CWE-59 / CWE-61).
*     O_CLOEXEC  : the fd is not inherited by fork+exec'd children.
*   Windows:
*     _O_NOINHERIT : Windows analogue of O_CLOEXEC.
*     _O_BINARY    : never let the CRT silently CRLF-translate output.
*                    Overridden by an explicit "t" in the mode string.
*
*
******************************************************************************/
/***************************** Include Files *********************************/
#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
/* Expose POSIX symbols (O_NOFOLLOW, O_CLOEXEC, fdopen) under --std=c11. */
#define _POSIX_C_SOURCE 200809L
#endif

#include "xaie_secure_io_internal.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#include <share.h>
#else
#include <unistd.h>
#endif

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* See xaie_secure_io_internal.h for documentation.
*
*******************************************************************************/
FILE *_XAie_SecureFopen(const char *Path, const char *Mode)
{
	if (Path == NULL || Mode == NULL || Mode[0] == '\0') {
		errno = EINVAL;
		return NULL;
	}

	/* Pure read opens ("r" / "rb") never create the file. */
	if (Mode[0] == 'r' && strchr(Mode, '+') == NULL) {
#ifdef _WIN32
		FILE *Rf = NULL;
		errno_t Rc = fopen_s(&Rf, Path, Mode);

		if (Rc != 0) {
			errno = (int)Rc;
			return NULL;
		}
		return Rf;
#else
		return fopen(Path, Mode);
#endif
	}

	int Update = (strchr(Mode, '+') != NULL);
	int OFlags = 0;

	switch (Mode[0]) {
	case 'w':
		OFlags = (Update ? O_RDWR : O_WRONLY) | O_CREAT | O_TRUNC;
		break;
	case 'a':
		OFlags = (Update ? O_RDWR : O_WRONLY) | O_CREAT | O_APPEND;
		break;
	case 'r': /* "r+": open existing for read/write; do not create. */
		OFlags = O_RDWR;
		break;
	default:
		errno = EINVAL;
		return NULL;
	}

	int Fd;
#ifdef _WIN32
	OFlags |= _O_NOINHERIT;
	OFlags |= (strchr(Mode, 't') != NULL) ? _O_TEXT : _O_BINARY;

	errno_t Rc = _sopen_s(&Fd, Path, OFlags,
			      _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (Rc != 0) {
		errno = Rc;
		return NULL;
	}
#else
	OFlags |= O_NOFOLLOW | O_CLOEXEC;
	Fd = open(Path, OFlags, S_IRUSR | S_IWUSR);
	if (Fd < 0) {
		return NULL;
	}
#endif

#ifdef _WIN32
	FILE *Fp = _fdopen(Fd, Mode);
#else
	FILE *Fp = fdopen(Fd, Mode);
#endif
	if (Fp == NULL) {
		int Saved = errno;
#ifdef _WIN32
		_close(Fd);
#else
		close(Fd);
#endif
		errno = Saved;
		return NULL;
	}
	return Fp;
}

/*****************************************************************************/
/**
*
* See xaie_secure_io_internal.h for documentation.
*
*******************************************************************************/
const char *_XAie_Strerror(int ErrNum, char *Buf, size_t BufLen)
{
	if (Buf == NULL || BufLen == 0U) {
		return "";
	}

#ifdef _WIN32
	if (strerror_s(Buf, BufLen, ErrNum) != 0) {
		Buf[0] = '\0';
	}
	return Buf;
#elif defined(_GNU_SOURCE)
	/* glibc's strerror_r returns a (possibly static) pointer. */
	return strerror_r(ErrNum, Buf, BufLen);
#elif (_POSIX_C_SOURCE >= 200112L) || defined(_XOPEN_SOURCE)
	/* XSI strerror_r writes into Buf and returns 0 on success. */
	if (strerror_r(ErrNum, Buf, BufLen) != 0) {
		Buf[0] = '\0';
	}
	return Buf;
#else
	{
		const char *Msg = strerror(ErrNum);

		if (Msg != NULL) {
			(void)snprintf(Buf, BufLen, "%s", Msg);
		} else {
			Buf[0] = '\0';
		}
		return Buf;
	}
#endif
}
/** @} */
