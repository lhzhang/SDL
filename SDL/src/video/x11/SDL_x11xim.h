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

#ifndef _SDL_x11xim_h
#define _SDL_x11xim_h

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

/* Private input method data */
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

#define IM_Context            (this->hidden->IM_Context)
/*
#define SDL_XIM               (this->hidden->IM_Context.SDL_XIM)
#define SDL_XIC               (this->hidden->IM_Context.SDL_XIC)
#define im_multi_byte_buffer  (this->hidden->IM_Context.string.im_multi_byte_buffer)
#define im_wide_char_buffer   (this->hidden->IM_Context.string.im_wide_char_buffer)
#define im_buffer_len         (this->hidden->IM_Context.im_buffer_len)
#define im_compose_len        (this->hidden->IM_Context.im_compose_len)
#define ic_focus              (this->hidden->IM_Context.ic_focus)
#define bEnable_OverTheSpot   (this->hidden->IM_Context.bEnable_OverTheSpot)
#define bEnable_OnTheSpot     (this->hidden->IM_Context.bEnable_OnTheSpot)
#define bEnable_Root          (this->hidden->IM_Context.bEnable_Root)
#define preedit_attr_orig     (this->hidden->IM_Context.preedit_attr_orig)
#define im_style_orig         (this->hidden->IM_Context.im_style_orig)   
#define preedit_attr_now      (this->hidden->IM_Context.preedit_attr_now)
#define im_style_now          (this->hidden->IM_Context.im_style_now)   
#define fontset               (this->hidden->IM_Context.fontset)   
*/

/* Functions to IM */
extern int X11_GetIMInfo(_THIS, SDL_SysIMinfo *info);

#endif /* _SDL_x11xim_h */

/* vi: set ts=4 sw=4 expandtab: */
