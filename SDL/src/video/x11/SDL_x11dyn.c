/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2004 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#if 1
#define DEBUG_DYNAMIC_X11 1
#endif

#include "SDL_x11dyn.h"

#ifdef DEBUG_DYNAMIC_X11
#include <stdio.h>
#endif

#ifdef SDL_VIDEO_DRIVER_X11_DYNAMIC
#include <dlfcn.h>
#include "SDL_name.h"
#include "SDL_loadso.h"
static const char *x11_library = SDL_VIDEO_DRIVER_X11_DYNAMIC;
static void *x11_handle = NULL;
static const char *x11ext_library = SDL_VIDEO_DRIVER_X11_DYNAMIC_XEXT;
static void *x11ext_handle = NULL;

typedef struct
{
    void *lib;
    const char *libname;
} x11libitem;

static void *X11_GetSym(const char *fnname, int *rc)
{
	int i;
	void *fn = NULL;
	const x11libitem libs[] =
	{
		{ x11_handle, "libX11" },
		{ x11ext_handle, "libX11ext" },
	};

	for (i = 0; i < (sizeof (libs) / sizeof (libs[0])); i++)
	{
		if (libs[i].lib != NULL)
		{
			fn = SDL_LoadFunction(libs[i].lib, fnname);
			if (fn != NULL)
				break;
		}
	}

	#if DEBUG_DYNAMIC_X11
	if (fn != NULL)
	    printf("X11: Found '%s' in %s (%p)\n", fnname, libs[i].libname, fn);
	else
		printf("X11: Symbol '%s' NOT FOUND!\n", fnname);
	#endif

	if (fn == NULL)
		*rc = 0;  /* kill this module. */

	return fn;
}


/* Define all the function pointers and wrappers... */
#define SDL_X11_MODULE(modname)
#define SDL_X11_SYM(rc,fn,params,args,ret) \
	static rc (*p##fn) params = NULL; \
	rc fn params { ret p##fn args ; }
#include "SDL_x11sym.h"
#undef SDL_X11_MODULE
#undef SDL_X11_SYM
#endif  /* SDL_VIDEO_DRIVER_X11_DYNAMIC */

/* Annoying varargs entry point... */
#ifdef X_HAVE_UTF8_STRING
XIC (*pXCreateIC)(XIM,...) = NULL;
#endif

/* These SDL_X11_HAVE_* flags are here whether you have dynamic X11 or not. */
#define SDL_X11_MODULE(modname) int SDL_X11_HAVE_##modname = 1;
#define SDL_X11_SYM(rc,fn,params,args,ret)
#include "SDL_x11sym.h"
#undef SDL_X11_MODULE
#undef SDL_X11_SYM


static int x11_load_refcount = 0;

void SDL_X11_UnloadSymbols(void)
{
	#ifdef SDL_VIDEO_DRIVER_X11_DYNAMIC
	/* Don't actually unload if more than one module is using the libs... */
	if (x11_load_refcount > 0) {
		if (--x11_load_refcount == 0) {
			/* set all the function pointers to NULL. */
			#define SDL_X11_MODULE(modname) SDL_X11_HAVE_##modname = 1;
			#define SDL_X11_SYM(rc,fn,params,args,ret) p##fn = NULL;
			#include "SDL_x11sym.h"
			#undef SDL_X11_MODULE
			#undef SDL_X11_SYM

            #ifdef X_HAVE_UTF8_STRING
            pXCreateIC = NULL;
            #endif

			if (x11_handle != NULL) {
				SDL_UnloadObject(x11_handle);
				x11_handle = NULL;
			}
			if (x11ext_handle != NULL) {
				SDL_UnloadObject(x11ext_handle);
				x11ext_handle = NULL;
			}
		}
	}
	#endif
}

/* returns non-zero if all needed symbols were loaded. */
int SDL_X11_LoadSymbols(void)
{
	int rc = 1;  /* always succeed if not using Dynamic X11 stuff. */

	#ifdef SDL_VIDEO_DRIVER_X11_DYNAMIC
	/* deal with multiple modules (dga, x11, etc) needing these symbols... */
	if (x11_load_refcount++ == 0) {
		int *thismod = NULL;
		x11_handle = SDL_LoadObject(x11_library);
		x11ext_handle = SDL_LoadObject(x11ext_library);
		#define SDL_X11_MODULE(modname) thismod = &SDL_X11_HAVE_##modname;
		#define SDL_X11_SYM(a,fn,x,y,z) p##fn = X11_GetSym(#fn,thismod);
		#include "SDL_x11sym.h"
		#undef SDL_X11_MODULE
		#undef SDL_X11_SYM

		#ifdef X_HAVE_UTF8_STRING
		pXCreateIC = X11_GetSym("XCreateIC",&SDL_X11_HAVE_UTF8);
		#endif

		if (!SDL_X11_HAVE_BASEXLIB) {  /* some required symbol didn't load. */
			SDL_X11_UnloadSymbols();  /* in case something got loaded... */
		}
	}
	#else
		#ifdef X_HAVE_UTF8_STRING
		pXCreateIC = XCreateIC;
		#endif
	#endif

	return rc;
}

/* end of SDL_x11dyn.c ... */

