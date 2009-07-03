/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#include "SDL_x11xim.h"
#include "../../events/SDL_keyboard_c.h"

int X11_GetIMInfo(_THIS, SDL_SysIMinfo *info)
{

       if ( info->version.major <= SDL_MAJOR_VERSION ) {
               if ( SDL_VERSIONNUM(info->version.major,
                       info->version.minor,
                       info->version.patch) >=
                       SDL_VERSIONNUM(1, 2, 8) ) {
#ifdef ENABLE_INPUTMETHOD
                   info->xim = IM_Context.SDL_XIM;
                   info->xic = &IM_Context.SDL_XIC;
#else
                   info->xim = NULL;
                   info->xic = NULL;
#endif
               }
                       return(1);
       } else {
               SDL_SetError("Application not compiled with SDL %d.%d\n",
                       SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
               return(-1);
       }
}

/* vi: set ts=4 sw=4 expandtab: */
