//----------------------------------------------------------------------------
// ZetaScale
// Copyright (c) 2016, SanDisk Corp. and/or all its affiliates.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published by the Free
// Software Foundation;
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License v2.1 for more details.
//
// A copy of the GNU Lesser General Public License v2.1 is provided with this package and
// can also be found at: http://opensource.org/licenses/LGPL-2.1
// You should have received a copy of the GNU Lesser General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307 USA.
//----------------------------------------------------------------------------

/*
 * File:   XResume.h
 * Author: Jim
 *
 * Created on February 29, 2008
 *
 * (c) Copyright 2008, Schooner Information Technology, Inc.
 * http:                                     //www.schoonerinfotech.com/
 *
 * $Id: XMbox.h 396 2008-02-29 22:55:43Z jim $
 */

#ifndef _SDFAPPCOMMON_X_RESUME
#define _SDFAPPCOMMON_X_RESUME

/**
 * @brief Cross-thread mailbox.  This is the common portion (both threads)
 */

#include "fth/fth.h"
#include "fth/fthThread.h"

#include "XList.h"


struct ptofThreadPtrs;

// Use a non-shmem version for now since shmem-based fthThread pointers do not exist
typedef struct ptofThreadPtrs {
    struct fthThread *head;
    struct fthThread *tail;
} ptofThreadPtrs_t;

XLIST_H(XResume, fthThread, resumeNext);

void XResume(struct fthThread *thread, uint64_t arg);
struct fthThread *XSpawn(void (*startRoutine)(uint64_t), long minStackSize);

#endif
