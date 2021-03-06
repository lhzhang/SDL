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

#ifndef _SDL_x11video_h
#define _SDL_x11video_h

#include "../SDL_sysvideo.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#if SDL_VIDEO_DRIVER_X11_XINERAMA
#include "../Xext/extensions/Xinerama.h"
#endif
#if SDL_VIDEO_DRIVER_X11_XRANDR
#include <X11/extensions/Xrandr.h>
#endif
#if SDL_VIDEO_DRIVER_X11_VIDMODE
#include "../Xext/extensions/xf86vmode.h"
#endif
#if SDL_VIDEO_DRIVER_X11_XINPUT
#include <X11/extensions/XInput.h>
#endif
#if SDL_VIDEO_DRIVER_X11_SCRNSAVER
#include <X11/extensions/scrnsaver.h>
#endif

#include "SDL_x11dyn.h"

#include "SDL_x11events.h"
#include "SDL_x11gamma.h"
#include "SDL_x11keyboard.h"
#include "SDL_x11modes.h"
#include "SDL_x11mouse.h"
#include "SDL_x11opengl.h"
#include "SDL_x11window.h"

/* Private display data */

typedef struct SDL_VideoData
{
    Display *display;
    char *classname;
    XIM im;
    Uint32 screensaver_activity;
    int numwindows;
    SDL_WindowData **windowlist;
    int windowlistlength;
    int keyboard;
    Atom WM_DELETE_WINDOW;
    SDL_scancode key_layout[256];
} SDL_VideoData;

/* IM context */
struct {
    XIM SDL_XIM;
    XIC SDL_XIC;
    union {
        char *im_multi_byte_buffer;
        wchar_t *im_wide_char_buffer;
    } string; 
    int im_buffer_len;
    int im_compose_len;

    /* Switch of XIM InputContext */
    char ic_focus;
    /* Decide if OverTheSpot, OnTheSpot, and Root input style is enabled. */
    char bEnable_OverTheSpot;
    char bEnable_OnTheSpot;
    char bEnable_Root;

    XVaNestedList preedit_attr_orig;
    XVaNestedList status_attr_orig;
    XIMStyle im_style_orig;

    XVaNestedList preedit_attr_now;
    XVaNestedList status_attr_now;
    XIMStyle im_style_now;

    XFontSet fontset;
} IM_Context;

extern SDL_bool X11_UseDirectColorVisuals();

#endif /* _SDL_x11video_h */

/* vi: set ts=4 sw=4 expandtab: */
