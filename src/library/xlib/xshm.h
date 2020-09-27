/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_XSHM_H_INCL
#define LIBTAS_XSHM_H_INCL

#include "config.h"
#ifdef LIBTAS_HAS_XRANDR

#include "../global.h"
#include <X11/extensions/XShm.h>

namespace libtas {

OVERRIDE Bool XShmPutImage(
    Display*        /* dpy */,
    Drawable        /* d */,
    GC          /* gc */,
    XImage*     /* image */,
    int         /* src_x */,
    int         /* src_y */,
    int         /* dst_x */,
    int         /* dst_y */,
    unsigned int    /* src_width */,
    unsigned int    /* src_height */,
    Bool        /* send_event */
);

}

#endif
#endif