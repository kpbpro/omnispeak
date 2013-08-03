/*
Omnispeak: A Commander Keen Reimplementation
Copyright (C) 2012 David Gow <david@ingeniumdigital.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ID_VL_H
#define ID_VL_H

#include <stdint.h>
#include <stdbool.h>

/* Util Functions */
typedef struct VL_EGAPaletteEntry
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} VL_EGAPaletteEntry;

extern VL_EGAPaletteEntry VL_EGAPalette[16];
void VL_SetDefaultPalette();
void VL_SetPalEntry(int id, uint8_t r, uint8_t g, uint8_t b);
void VL_UnmaskedToRGB(void *src,void *dest, int x, int y, int pitch, int w, int h);
void VL_MaskedToRGBA(void *src,void *dest, int x, int y, int pitch, int w, int h);
void VL_MaskedBlitToRGB(void *src,void *dest, int x, int y, int pitch, int w, int h);
void VL_MaskedBlitClipToRGB(void *src,void *dest, int x, int y, int pitch, int w, int h, int dw, int dh);
void VL_1bppToRGBA(void *src,void *dest, int x, int y, int pitch, int w, int h, int colour);
void VL_1bppBlitToRGB(void *src,void *dest, int x, int y, int pitch, int w, int h, int colour);

int VL_MemUsed();
int VL_NumSurfaces();

typedef enum VL_SurfaceUsage
{
	VL_SurfaceUsage_Default,
	VL_SurfaceUsage_FrameBuffer,
	VL_SurfaceUsage_FrontBuffer,
	VL_SurfaceUsage_Sprite
} VL_SurfaceUsage;

typedef struct VL_Backend
{
	void (*setVideoMode)(int w, int h);
	void* (*createSurface)(int w, int h, VL_SurfaceUsage usage);
	void (*destroySurface)(void *surface);
	long (*getSurfaceMemUse)(void *surface);
	void (*surfaceRect)(void *dst_surface, int x, int y, int w, int h, int colour);
	void (*surfaceToSurface)(void *src_surface, void *dst_surface, int x, int y, int sx, int sy, int sw, int sh);
	void (*surfaceToSelf)(void *surface, int x, int y, int sx, int sy, int sw, int sh);
	void (*unmaskedToSurface)(void *src, void *dst_surface, int x, int y, int w, int h);
	void (*maskedToSurface)(void *src, void *dst_surface, int x, int y, int w, int h);
	void (*maskedBlitToSurface)(void *src, void *dst_surface, int x, int y, int w, int h);
	void (*bitToSurface)(void *src, void *dst_surface, int x, int y, int w, int h, int colour);
	void (*bitBlitToSurface)(void *src, void *dst_surface, int x, int y, int w, int h, int colour);
	void (*present)(void *surface, int scrollXpx, int scrollYpx);
} VL_Backend;

void VL_InitScreen();
void *VL_CreateSurface(int w, int h);
void VL_SurfaceRect(void *dst, int x, int y, int w, int h, int colour);
void VL_ScreenRect(int x, int y, int w, int h, int colour);
void VL_SurfaceToSurface(void *src, void *dst, int x, int y, int sx, int sy, int sw, int sh);
void VL_SurfaceToScreen(void *src, int x, int y, int sx, int sy, int sw, int sh);
void VL_SurfaceToSelf(void *surf, int x, int y, int sx, int sy, int sw, int sh);
void VL_UnmaskedToSurface(void *src, void *dest, int x, int y, int w, int h); 
void VL_UnmaskedToScreen(void *src, int x, int y, int w, int h);
void VL_MaskedToSurface(void *src, void *dest, int x, int y, int w, int h);
void VL_MaskedBlitToSurface(void *src, void *dest, int x, int y, int w, int h);
void VL_MaskedToScreen(void *src, int x, int y, int w, int h);
void VL_MaskedBlitToScreen(void *src, int x, int y, int w, int h);
void VL_1bppToScreen(void *src, int x, int y, int w, int h, int colour);
void VL_1bppBlitToScreen(void *src, int x, int y, int w, int h, int colour);

int VL_GetTics(bool wait);
void VL_SetScrollCoords(int x, int y);
void VL_Present();
#endif //ID_VL_H
