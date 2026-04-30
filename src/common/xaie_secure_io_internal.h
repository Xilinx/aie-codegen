/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_secure_io_internal.h
* @{
*
* Cross-platform helpers for opening stdio streams with restrictive file
* permissions. Use these instead of fopen() when the open may create the
* file, so that newly created files are owner-readable/writable only and
* the surrounding code does not need to manipulate the process umask.
*
* Internal API — declarations live here (with the `_XAie_` prefix) because
* this helper is consumed only by aie-codegen's own translation units and
* is not part of the public ABI.
*
*
******************************************************************************/
#ifndef XAIE_SECURE_IO_INTERNAL_H
#define XAIE_SECURE_IO_INTERNAL_H

/***************************** Include Files *********************************/
#include <stdio.h>

/************************** Function Prototypes ******************************/
/*****************************************************************************/
/**
*
* Open `Path` as a stdio stream, restricting the file to owner-only
* read/write (POSIX 0600) when the open creates the file.
*
* `Mode` accepts the same primary characters as fopen(3): "r", "w", "a",
* optionally followed by "+", "b", "t". Pure read opens ("r" / "rb") cannot
* create a file and are forwarded to fopen() unchanged.
*
* On Linux the file is opened with O_NOFOLLOW (refuses pre-placed symlinks
* at `Path`) and O_CLOEXEC (the fd is not inherited by exec'd children).
* On Windows the corresponding flags are _O_NOINHERIT and _O_BINARY (the
* CRT's read-only bit is set, but the actual security descriptor is
* inherited from the parent directory).
*
* @param        Path: Path of the file to open.
* @param        Mode: fopen-compatible mode string.
*
* @return       FILE pointer on success; NULL with errno set on failure.
*
* @note         None.
*
*******************************************************************************/
FILE *_XAie_SecureFopen(const char *Path, const char *Mode);

#endif /* XAIE_SECURE_IO_INTERNAL_H */
/** @} */
