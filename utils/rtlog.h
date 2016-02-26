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
 *  Light-Weight Logging
 */

#include <stdint.h>

#ifdef rt_enable_logging

extern void rtlog(const char *, uint64_t);
extern void rt_dump(void);
extern void rt_init(void);
extern void rt_set_size(int);

#else /* ! rt_enable_logging */

#define rtlog(...)
#define rt_dump(...)
#define rt_init(...)
#define rt_set_size(...)

#endif

#define rt_log(f, v)  rtlog(f, (uint64_t) v)
