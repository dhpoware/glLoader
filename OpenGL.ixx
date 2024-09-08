// Copyright (c) 2024 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

module;

#include <windows.h>
#include <GL/glcorearb.h>
#include <memory>

export module OpenGL;

// The OpenGLContext class is a wrapper around the WGL API in opengl32.dll.
// It provides a way to create an OpenGL rendering context for a window.
// The class contains replacements for all the WGL functions in opengl32.dll.
// These methods have the same signatures as the WGL functions as defined in wingdi.h.
// They even have the same spelling as the WGL functions.
// These methods forward the calls to the real WGL functions in opengl32.dll at runtime.
// This means that the OpenGLContext class can be used as a drop-in replacement for the WGL API.
// Since the real WGL functions are loaded dynamically at runtime, there is no need to statically link to opengl32.lib.

export class OpenGLContext
{
public:
	// Create an OpenGL rendering context for a window.	
	
	static std::shared_ptr<OpenGLContext> createForWindow(HWND hWnd, PIXELFORMATDESCRIPTOR &pfd);

	// The following methods are replacements for the WGL functions in opengl32.dll:

	BOOL wglCopyContext(HGLRC hglrcSource, HGLRC hglrcDest, UINT mask);
	HGLRC wglCreateContext(HDC hdc);
	HGLRC wglCreateLayerContext(HDC hdc, int iLayerPlane);
	BOOL wglDeleteContext(HGLRC hglrc);
	BOOL wglDescribeLayerPlane(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd);
	HGLRC wglGetCurrentContext();
	HDC wglGetCurrentDC();
	int wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF *pcr);
	PROC wglGetProcAddress(LPCSTR lpszProc);
	BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc);
	BOOL wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL bRealize);
	int wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF *pcr);
	BOOL wglShareLists(HGLRC hglrc1, HGLRC hglrc2);
	BOOL SwapBuffers(HDC hdc);
	BOOL wglSwapLayerBuffers(HDC hdc, UINT fuPlanes);
	DWORD wglSwapMultipleBuffers(UINT count, const WGLSWAP *toSwap);
	BOOL wglUseFontBitmapsA(HDC hdc, DWORD first, DWORD count, DWORD listBase);
	BOOL wglUseFontBitmapsW(HDC hdc, DWORD first, DWORD count, DWORD listBase);
	BOOL wglUseFontOutlinesA(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf);
	BOOL wglUseFontOutlinesW(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf);

private:
	using PFNWGLCOPYCONTEXTPROC = BOOL(WINAPI*)(HGLRC hglrcSource, HGLRC hglrcDest, UINT mask);
	using PFNWGLCREATECONTEXTPROC = HGLRC(WINAPI*)(HDC hdc);
	using PFNWGLCREATELAYERCONTEXTPROC = HGLRC(WINAPI*)(HDC hdc, int iLayerPlane);
	using PFNWGLDELETECONTEXTPROC = BOOL(WINAPI*)(HGLRC hglrc);
	using PFNWGLDESCRIBELAYERPLANEPROC = BOOL(WINAPI*)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd);
	using PFNWGLGETCURRENTCONTEXTPROC = HGLRC(WINAPI*)(VOID);
	using PFNWGLGETCURRENTDCPROC = HDC(WINAPI*)(VOID);
	using PFNWGLGETLAYERPALETTEENTRIESPROC = int(WINAPI*)(HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF* pcr);
	using PFNWGLMAKECURRENTPROC = BOOL(WINAPI*)(HDC hdc, HGLRC hglrc);
	using PFNWGLREALIZELAYERPALETTEPROC = BOOL(WINAPI*)(HDC hdc, int iLayerPlane, BOOL bRealize);
	using PFNWGLSETLAYERPALETTEENTRIESPROC = int(WINAPI*)(HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF* pcr);
	using PFNWGLSHARELISTSPROC = BOOL(WINAPI*)(HGLRC hglrc1, HGLRC hglrc2);
	using PFNWGLSWAPLAYERBUFFERSPROC = BOOL(WINAPI*)(HDC hdc, UINT fuPlanes);
	using PFNWGLSWAPMULTIPLEBUFFERSPROC = DWORD(WINAPI*)(UINT count, const WGLSWAP* toSwap);
	using PFNWGLUSEFONTBITMAPSPROC = BOOL(WINAPI*)(HDC hdc, DWORD first, DWORD count, DWORD listBase);
	using PFNWGLUSEFONTOUTLINESPROC = BOOL(WINAPI*)(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf);

	PFNWGLCOPYCONTEXTPROC m_pfnWglCopyContext{nullptr};
	PFNWGLCREATECONTEXTPROC m_pfnWglCreateContext{nullptr};
	PFNWGLCREATELAYERCONTEXTPROC m_pfnWglCreateLayerContext{nullptr};
	PFNWGLDELETECONTEXTPROC m_pfnWglDeleteContext{nullptr};
	PFNWGLDESCRIBELAYERPLANEPROC m_pfnWglDescribeLayerPlane{nullptr};
	PFNWGLGETCURRENTCONTEXTPROC m_pfnWglGetCurrentContext{nullptr};
	PFNWGLGETCURRENTDCPROC m_pfnWglGetCurrentDC{nullptr};
	PFNWGLGETLAYERPALETTEENTRIESPROC m_pfnWglGetLayerPaletteEntries{nullptr};
	PFNWGLMAKECURRENTPROC m_pfnWglMakeCurrent{nullptr};
	PFNWGLREALIZELAYERPALETTEPROC m_pfnWglRealizeLayerPalette{nullptr};
	PFNWGLSETLAYERPALETTEENTRIESPROC m_pfnWglSetLayerPaletteEntries{nullptr};
	PFNWGLSHARELISTSPROC m_pfnWglShareLists{nullptr};
	PFNWGLSWAPLAYERBUFFERSPROC m_pfnWglSwapLayerBuffers{nullptr};
	PFNWGLSWAPMULTIPLEBUFFERSPROC m_pfnWglSwapMultipleBuffers{nullptr};
	PFNWGLUSEFONTBITMAPSPROC m_pfnWglUseFontBitmapsA{nullptr};
	PFNWGLUSEFONTBITMAPSPROC m_pfnWglUseFontBitmapsW{nullptr};
	PFNWGLUSEFONTOUTLINESPROC m_pfnWglUseFontOutlinesA{nullptr};
	PFNWGLUSEFONTOUTLINESPROC m_pfnWglUseFontOutlinesW{nullptr};
};

extern "C"
{
	//
	// GL_VERSION_1_0
	//

	export void glBlendFunc(GLenum sfactor, GLenum dfactor);
	export void glClear(GLbitfield mask);
	export void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	export void glClearDepth(GLdouble depth);
	export void glClearStencil(GLint s);
	export void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	export void glCullFace(GLenum mode);
	export void glDepthFunc(GLenum func);
	export void glDepthMask(GLboolean flag);
	export void glDepthRange(GLdouble n, GLdouble f);
	export void glDisable(GLenum cap);
	export void glDrawBuffer(GLenum buf);
	export void glEnable(GLenum cap);
	export void glFinish(void);
	export void glFlush(void);
	export void glFrontFace(GLenum mode);
	export void glGetBooleanv(GLenum pname, GLboolean* data);
	export void glGetDoublev(GLenum pname, GLdouble* data);
	export GLenum glGetError(void);
	export void glGetFloatv(GLenum pname, GLfloat* data);
	export void glGetIntegerv(GLenum pname, GLint* data);
	export const GLubyte* glGetString(GLenum name);
	export void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels);
	export void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params);
	export void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params);
	export void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params);
	export void glGetTexParameteriv(GLenum target, GLenum pname, GLint* params);
	export void glHint(GLenum target, GLenum mode);
	export GLboolean glIsEnabled(GLenum cap);
	export void glLineWidth(GLfloat width);
	export void glLogicOp(GLenum opcode);
	export void glPixelStoref(GLenum pname, GLfloat param);
	export void glPixelStorei(GLenum pname, GLint param);
	export void glPointSize(GLfloat size);
	export void glPolygonMode(GLenum face, GLenum mode);
	export void glReadBuffer(GLenum src);
	export void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
	export void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
	export void glStencilFunc(GLenum func, GLint ref, GLuint mask);
	export void glStencilMask(GLuint mask);
	export void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
	export void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels);
	export void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
	export void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
	export void glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params);
	export void glTexParameteri(GLenum target, GLenum pname, GLint param);
	export void glTexParameteriv(GLenum target, GLenum pname, const GLint* params);
	export void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

	//
	// GL_VERSION_1_1
	//

	export void glBindTexture(GLenum target, GLuint texture);
	export void glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
	export void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	export void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	export void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	export void glDeleteTextures(GLsizei n, const GLuint* textures);
	export void glDrawArrays(GLenum mode, GLint first, GLsizei count);
	export void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
	export void glGenTextures(GLsizei n, GLuint* textures);
	export void glGetPointerv(GLenum pname, void** params);
	export GLboolean glIsTexture(GLuint texture);
	export void glPolygonOffset(GLfloat factor, GLfloat units);
	export void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
	export void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
}