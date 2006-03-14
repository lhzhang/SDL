/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

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

/* WGL implementation of SDL OpenGL support */

#if SDL_VIDEO_OPENGL
#include "SDL_opengl.h"
#endif
#include "SDL_lowvideo.h"
#include "SDL_wingl_c.h"

#if SDL_VIDEO_OPENGL
#define DEFAULT_GL_DRIVER_PATH "OPENGL32.DLL"
#endif

/* If setting the HDC fails, we may need to recreate the window (MSDN) */
static int WIN_GL_ResetWindow(_THIS)
{
	int status = 0;

#if 0 /* This doesn't work with DirectX code (see CVS comments) */
#ifndef _WIN32_WCE /* FIXME WinCE needs the UNICODE version of CreateWindow() */
	/* If we were passed a window, then we can't create a new one */
	if ( !SDL_windowid ) {
		/* Save the existing window attributes */
		LONG style;
		RECT rect = { 0, 0, 0, 0 };
		style = GetWindowLong(SDL_Window, GWL_STYLE);
		GetWindowRect(SDL_Window, &rect);
		DestroyWindow(SDL_Window);
		WIN_FlushMessageQueue();

		SDL_Window = CreateWindow(SDL_Appname, SDL_Appname,
		                          style,
		                          rect.left, rect.top,
		                          (rect.right-rect.left)+1,
		                          (rect.top-rect.bottom)+1,
		                          NULL, NULL, SDL_Instance, NULL);
		WIN_FlushMessageQueue();

		if ( SDL_Window ) {
			this->SetCaption(this, this->wm_title, this->wm_icon);
		} else {
			SDL_SetError("Couldn't create window");
			status = -1;
		}
	} else
#endif /* !_WIN32_WCE */
#endif
	{
		SDL_SetError("Unable to reset window for OpenGL context");
		status = -1;
	}
	return(status);
}

#if SDL_VIDEO_OPENGL

static int ExtensionSupported(const char *extension, const char *extensions)
{
	const char *start;
	const char *where, *terminator;

	/* Extension names should not have spaces. */
	where = SDL_strchr(extension, ' ');
	if ( where || *extension == '\0' )
	      return 0;
	
	if ( ! extensions )
		return 0;

	/* It takes a bit of care to be fool-proof about parsing the
	 *      OpenGL extensions string. Don't be fooled by sub-strings,
	 *           etc. */
	
	start = extensions;
	
	for (;;)
	{
		where = SDL_strstr(start, extension);
		if (!where) break;
		
		terminator = where + SDL_strlen(extension);
		if (where == start || *(where - 1) == ' ')
	        if (*terminator == ' ' || *terminator == '\0') return 1;

		start = terminator;
	}
	
	return 0;
}

static void Init_WGL_ARB_extensions(_THIS)
{
	HWND hwnd;
	HDC hdc;
	HGLRC hglrc;
	int pformat;
	const char * (WINAPI *wglGetExtensionsStringARB)(HDC) = 0;
	const char *extensions;
	
	hwnd = CreateWindow(SDL_Appname, SDL_Appname, WS_POPUP | WS_DISABLED,
	                    0, 0, 10, 10,
	                    NULL, NULL, SDL_Instance, NULL);
	WIN_FlushMessageQueue();

	hdc = GetDC(hwnd);

	pformat = ChoosePixelFormat(hdc, &GL_pfd);
	SetPixelFormat(hdc, pformat, &GL_pfd);

	hglrc = this->gl_data->wglCreateContext(hdc);
	if ( hglrc ) {
		this->gl_data->wglMakeCurrent(hdc, hglrc);
	}

	wglGetExtensionsStringARB = (const char * (WINAPI *)(HDC))
		this->gl_data->wglGetProcAddress("wglGetExtensionsStringARB");

	if( wglGetExtensionsStringARB ) {
		extensions = wglGetExtensionsStringARB(hdc);
	} else {
		extensions = NULL;
	}

	this->gl_data->WGL_ARB_pixel_format = 0;
	if( ExtensionSupported("WGL_ARB_pixel_format", extensions) ) {
		this->gl_data->wglChoosePixelFormatARB =
			(BOOL (WINAPI *)(HDC, const int *, const FLOAT *, UINT, int *, UINT *))
			this->gl_data->wglGetProcAddress("wglChoosePixelFormatARB");
		this->gl_data->wglGetPixelFormatAttribivARB =
			(BOOL (WINAPI *)(HDC, int, int, UINT, const int *, int *))
			this->gl_data->wglGetProcAddress("wglGetPixelFormatAttribivARB");

		if( (this->gl_data->wglChoosePixelFormatARB != NULL) &&
		    (this->gl_data->wglGetPixelFormatAttribivARB != NULL) ) {
			this->gl_data->WGL_ARB_pixel_format = 1;
		}
	}
	
	if ( hglrc ) {
		this->gl_data->wglMakeCurrent(NULL, NULL);
		this->gl_data->wglDeleteContext(hglrc);
	}
	ReleaseDC(hwnd, hdc);
	DestroyWindow(hwnd);
	WIN_FlushMessageQueue();
}

#endif /* SDL_VIDEO_OPENGL */

int WIN_GL_SetupWindow(_THIS)
{
	int retval;
#if SDL_VIDEO_OPENGL
	int i;
	unsigned int matching;
	int iAttribs[64];
	int *iAttr;
	float fAttribs[1] = { 0 };

	/* load the gl driver from a default path */
	if ( ! this->gl_config.driver_loaded ) {
		/* no driver has been loaded, use default (ourselves) */
		if ( WIN_GL_LoadLibrary(this, NULL) < 0 ) {
			return(-1);
		}
	}

	for ( i=0; ; ++i ) {
		/* Get the window device context for our OpenGL drawing */
		GL_hdc = GetDC(SDL_Window);
		if ( GL_hdc == NULL ) {
			SDL_SetError("Unable to get DC for SDL_Window");
			return(-1);
		}

		/* Set up the pixel format descriptor with our needed format */
		SDL_memset(&GL_pfd, 0, sizeof(GL_pfd));
		GL_pfd.nSize = sizeof(GL_pfd);
		GL_pfd.nVersion = 1;
		GL_pfd.dwFlags = (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL);
		if ( this->gl_config.double_buffer ) {
			GL_pfd.dwFlags |= PFD_DOUBLEBUFFER;
		}
		if ( this->gl_config.stereo ) {
			GL_pfd.dwFlags |= PFD_STEREO;
		}
		GL_pfd.iPixelType = PFD_TYPE_RGBA;
		GL_pfd.cColorBits = this->gl_config.buffer_size;
		GL_pfd.cRedBits = this->gl_config.red_size;
		GL_pfd.cGreenBits = this->gl_config.green_size;
		GL_pfd.cBlueBits = this->gl_config.blue_size;
		GL_pfd.cAlphaBits = this->gl_config.alpha_size;
		GL_pfd.cAccumRedBits = this->gl_config.accum_red_size;
		GL_pfd.cAccumGreenBits = this->gl_config.accum_green_size;
		GL_pfd.cAccumBlueBits = this->gl_config.accum_blue_size;
		GL_pfd.cAccumAlphaBits = this->gl_config.accum_alpha_size;
		GL_pfd.cAccumBits =
			(GL_pfd.cAccumRedBits + GL_pfd.cAccumGreenBits +
			 GL_pfd.cAccumBlueBits + GL_pfd.cAccumAlphaBits);
		GL_pfd.cDepthBits = this->gl_config.depth_size;
		GL_pfd.cStencilBits = this->gl_config.stencil_size;

		/* initialize WGL_ARB_pixel_format */
		Init_WGL_ARB_extensions(this);

		/* setup WGL_ARB_pixel_format attribs */
		iAttr = &iAttribs[0];

		*iAttr++ = WGL_DRAW_TO_WINDOW_ARB;
		*iAttr++ = GL_TRUE;
		*iAttr++ = WGL_ACCELERATION_ARB;
		*iAttr++ = WGL_FULL_ACCELERATION_ARB;
		*iAttr++ = WGL_RED_BITS_ARB;
		*iAttr++ = this->gl_config.red_size;
		*iAttr++ = WGL_GREEN_BITS_ARB;
		*iAttr++ = this->gl_config.green_size;
		*iAttr++ = WGL_BLUE_BITS_ARB;
		*iAttr++ = this->gl_config.blue_size;
		
		if ( this->gl_config.alpha_size ) {
			*iAttr++ = WGL_ALPHA_BITS_ARB;
			*iAttr++ = this->gl_config.alpha_size;
		}

		*iAttr++ = WGL_DOUBLE_BUFFER_ARB;
		*iAttr++ = this->gl_config.double_buffer;

		*iAttr++ = WGL_DEPTH_BITS_ARB;
		*iAttr++ = this->gl_config.depth_size;

		if ( this->gl_config.stencil_size ) {
			*iAttr++ = WGL_STENCIL_BITS_ARB;
			*iAttr++ = this->gl_config.stencil_size;
		}

		if ( this->gl_config.accum_red_size ) {
			*iAttr++ = WGL_ACCUM_RED_BITS_ARB;
			*iAttr++ = this->gl_config.accum_red_size;
		}

		if ( this->gl_config.accum_green_size ) {
			*iAttr++ = WGL_ACCUM_GREEN_BITS_ARB;
			*iAttr++ = this->gl_config.accum_green_size;
		}

		if ( this->gl_config.accum_blue_size ) {
			*iAttr++ = WGL_ACCUM_BLUE_BITS_ARB;
			*iAttr++ = this->gl_config.accum_blue_size;
		}

		if ( this->gl_config.accum_alpha_size ) {
			*iAttr++ = WGL_ACCUM_ALPHA_BITS_ARB;
			*iAttr++ = this->gl_config.accum_alpha_size;
		}

		if ( this->gl_config.stereo ) {
			*iAttr++ = WGL_STEREO_ARB;
			*iAttr++ = GL_TRUE;
		}

		if ( this->gl_config.multisamplebuffers ) {
			*iAttr++ = WGL_SAMPLE_BUFFERS_ARB;
			*iAttr++ = this->gl_config.multisamplebuffers;
		}

		if ( this->gl_config.multisamplesamples ) {
			*iAttr++ = WGL_SAMPLES_ARB;
			*iAttr++ = this->gl_config.multisamplesamples;
		}

		*iAttr = 0;

		/* Choose and set the closest available pixel format */
		if ( !this->gl_data->WGL_ARB_pixel_format ||
		     !this->gl_data->wglChoosePixelFormatARB(GL_hdc, iAttribs, fAttribs, 1, &pixel_format, &matching) ||
		     !matching ) {
			pixel_format = ChoosePixelFormat(GL_hdc, &GL_pfd);
			this->gl_data->WGL_ARB_pixel_format = 0;
		}
		if ( !pixel_format ) {
			SDL_SetError("No matching GL pixel format available");
			return(-1);
		}
		if ( !SetPixelFormat(GL_hdc, pixel_format, &GL_pfd) ) {
			if ( i == 0 ) {
				/* First time through, try resetting the window */
				if ( WIN_GL_ResetWindow(this) < 0 ) {
					return(-1);
				}
				continue;
			}
			SDL_SetError("Unable to set HDC pixel format");
			return(-1);
		}
		/* We either succeeded or failed by this point */
		break;
	}
	DescribePixelFormat(GL_hdc, pixel_format, sizeof(GL_pfd), &GL_pfd);

	GL_hrc = this->gl_data->wglCreateContext(GL_hdc);
	if ( GL_hrc == NULL ) {
		SDL_SetError("Unable to create GL context");
		return(-1);
	}
	gl_active = 1;
#else
	SDL_SetError("WIN driver not configured with OpenGL");
#endif
	if ( gl_active ) {
		retval = 0;
	} else {
		retval = -1;
	}
	return(retval);
}

void WIN_GL_ShutDown(_THIS)
{
#if SDL_VIDEO_OPENGL
	/* Clean up OpenGL */
	if ( GL_hrc ) {
		this->gl_data->wglMakeCurrent(NULL, NULL);
		this->gl_data->wglDeleteContext(GL_hrc);
		GL_hrc = NULL;
	}
	if ( GL_hdc ) {
		ReleaseDC(SDL_Window, GL_hdc);
		GL_hdc = NULL;
	}
	gl_active = 0;

	WIN_GL_UnloadLibrary(this);
#endif /* SDL_VIDEO_OPENGL */
}

#if SDL_VIDEO_OPENGL

/* Make the current context active */
int WIN_GL_MakeCurrent(_THIS)
{
	int retval;

	retval = 0;
	if ( ! this->gl_data->wglMakeCurrent(GL_hdc, GL_hrc) ) {
		SDL_SetError("Unable to make GL context current");
		retval = -1;
	}
	return(retval);
}

/* Get attribute data from glX. */
int WIN_GL_GetAttribute(_THIS, SDL_GLattr attrib, int* value)
{
	int retval;
	
	if ( this->gl_data->WGL_ARB_pixel_format ) {
		int wgl_attrib;

		switch(attrib) {
		    case SDL_GL_RED_SIZE:
			wgl_attrib = WGL_RED_BITS_ARB;
			break;
		    case SDL_GL_GREEN_SIZE:
			wgl_attrib = WGL_GREEN_BITS_ARB;
			break;
		    case SDL_GL_BLUE_SIZE:
			wgl_attrib = WGL_BLUE_BITS_ARB;
			break;
		    case SDL_GL_ALPHA_SIZE:
			wgl_attrib = WGL_ALPHA_BITS_ARB;
			break;
		    case SDL_GL_DOUBLEBUFFER:
			wgl_attrib = WGL_DOUBLE_BUFFER_ARB;
			break;
		    case SDL_GL_BUFFER_SIZE:
			wgl_attrib = WGL_COLOR_BITS_ARB;
			break;
		    case SDL_GL_DEPTH_SIZE:
			wgl_attrib = WGL_DEPTH_BITS_ARB;
			break;
		    case SDL_GL_STENCIL_SIZE:
			wgl_attrib = WGL_STENCIL_BITS_ARB;
			break;
		    case SDL_GL_ACCUM_RED_SIZE:
			wgl_attrib = WGL_ACCUM_RED_BITS_ARB;
			break;
		    case SDL_GL_ACCUM_GREEN_SIZE:
			wgl_attrib = WGL_ACCUM_GREEN_BITS_ARB;
			break;
		    case SDL_GL_ACCUM_BLUE_SIZE:
			wgl_attrib = WGL_ACCUM_BLUE_BITS_ARB;
			break;
		    case SDL_GL_ACCUM_ALPHA_SIZE:
			wgl_attrib = WGL_ACCUM_ALPHA_BITS_ARB;
			break;
		    case SDL_GL_STEREO:
			wgl_attrib = WGL_STEREO_ARB;
			break;
		    case SDL_GL_MULTISAMPLEBUFFERS:
			wgl_attrib = WGL_SAMPLE_BUFFERS_ARB;
			break;
		    case SDL_GL_MULTISAMPLESAMPLES:
			wgl_attrib = WGL_SAMPLES_ARB;
			break;
		    default:
			return(-1);
		}
		this->gl_data->wglGetPixelFormatAttribivARB(GL_hdc, pixel_format, 0, 1, &wgl_attrib, value);

		return 0;
	}

	retval = 0;
	switch ( attrib ) {
	    case SDL_GL_RED_SIZE:
		*value = GL_pfd.cRedBits;
		break;
	    case SDL_GL_GREEN_SIZE:
		*value = GL_pfd.cGreenBits;
		break;
	    case SDL_GL_BLUE_SIZE:
		*value = GL_pfd.cBlueBits;
		break;
	    case SDL_GL_ALPHA_SIZE:
		*value = GL_pfd.cAlphaBits;
		break;
	    case SDL_GL_DOUBLEBUFFER:
		if ( GL_pfd.dwFlags & PFD_DOUBLEBUFFER ) {
			*value = 1;
		} else {
			*value = 0;
		}
		break;
	    case SDL_GL_BUFFER_SIZE:
		*value = GL_pfd.cColorBits;
		break;
	    case SDL_GL_DEPTH_SIZE:
		*value = GL_pfd.cDepthBits;
		break;
	    case SDL_GL_STENCIL_SIZE:
		*value = GL_pfd.cStencilBits;
		break;
	    case SDL_GL_ACCUM_RED_SIZE:
		*value = GL_pfd.cAccumRedBits;
		break;
	    case SDL_GL_ACCUM_GREEN_SIZE:
		*value = GL_pfd.cAccumGreenBits;
		break;
	    case SDL_GL_ACCUM_BLUE_SIZE:
		*value = GL_pfd.cAccumBlueBits;
		break;
	    case SDL_GL_ACCUM_ALPHA_SIZE:
		*value = GL_pfd.cAccumAlphaBits;
		break;
	    case SDL_GL_STEREO:
		if ( GL_pfd.dwFlags & PFD_STEREO ) {
			*value = 1;
		} else {
			*value = 0;
		}
		break;
	    case SDL_GL_MULTISAMPLEBUFFERS:
		*value = 0;
		break;
	    case SDL_GL_MULTISAMPLESAMPLES:
		*value = 1;
		break;
	    default:
		retval = -1;
		break;
	}
	return retval;
}

void WIN_GL_SwapBuffers(_THIS)
{
	SwapBuffers(GL_hdc);
}

void WIN_GL_UnloadLibrary(_THIS)
{
	if ( this->gl_config.driver_loaded ) {
		FreeLibrary((HMODULE)this->gl_config.dll_handle);

		this->gl_data->wglGetProcAddress = NULL;
		this->gl_data->wglCreateContext = NULL;
		this->gl_data->wglDeleteContext = NULL;
		this->gl_data->wglMakeCurrent = NULL;
		this->gl_data->wglChoosePixelFormatARB = NULL;
		this->gl_data->wglGetPixelFormatAttribivARB = NULL;

		this->gl_config.dll_handle = NULL;
		this->gl_config.driver_loaded = 0;
	}
}

/* Passing a NULL path means load pointers from the application */
int WIN_GL_LoadLibrary(_THIS, const char* path) 
{
	HMODULE handle;

 	if ( gl_active ) {
 		SDL_SetError("OpenGL context already created");
 		return -1;
 	}

	if ( path == NULL ) {
		path = DEFAULT_GL_DRIVER_PATH;
	}
	handle = LoadLibrary(path);
	if ( handle == NULL ) {
		SDL_SetError("Could not load OpenGL library");
		return -1;
	}

	/* Unload the old driver and reset the pointers */
	WIN_GL_UnloadLibrary(this);

	/* Load new function pointers */
	SDL_memset(this->gl_data, 0, sizeof(*this->gl_data));
	this->gl_data->wglGetProcAddress = (void * (WINAPI *)(const char *))
		GetProcAddress(handle, "wglGetProcAddress");
	this->gl_data->wglCreateContext = (HGLRC (WINAPI *)(HDC))
		GetProcAddress(handle, "wglCreateContext");
	this->gl_data->wglDeleteContext = (BOOL (WINAPI *)(HGLRC))
		GetProcAddress(handle, "wglDeleteContext");
	this->gl_data->wglMakeCurrent = (BOOL (WINAPI *)(HDC, HGLRC))
		GetProcAddress(handle, "wglMakeCurrent");

	if ( (this->gl_data->wglGetProcAddress == NULL) ||
	     (this->gl_data->wglCreateContext == NULL) ||
	     (this->gl_data->wglDeleteContext == NULL) ||
	     (this->gl_data->wglMakeCurrent == NULL) ) {
		SDL_SetError("Could not retrieve OpenGL functions");
		FreeLibrary(handle);
		return -1;
	}

	this->gl_config.dll_handle = handle;
	SDL_strlcpy(this->gl_config.driver_path, path, SDL_arraysize(this->gl_config.driver_path));
	this->gl_config.driver_loaded = 1;
	return 0;
}

void *WIN_GL_GetProcAddress(_THIS, const char* proc)
{
	void *func;

	/* This is to pick up extensions */
	func = this->gl_data->wglGetProcAddress(proc);
	if ( ! func ) {
		/* This is probably a normal GL function */
		func = GetProcAddress(this->gl_config.dll_handle, proc);
	}
	return func;
}

#endif /* SDL_VIDEO_OPENGL */
