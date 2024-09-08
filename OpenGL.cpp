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
#include <cassert>
#include <memory>

module OpenGL;

#define LOAD_ENTRYPOINT(name, var, type) \
    if (!var) \
    { \
        var = reinterpret_cast<type>(Loader::instance().getProcAddress(name)); \
        assert(var != nullptr); \
    }

//
// Loader is a singleton class that loads the OpenGL library and retrieves function pointers to OpenGL functions.
//

class Loader
{
public:
	static Loader &instance();

	void *getProcAddress(const char *pszName) const;

private:
	Loader();
	~Loader();

	using PFNWGLGETPROCADDRESSPROC = void *(APIENTRY *)(const char *);

	HMODULE m_hLibGL;
	PFNWGLGETPROCADDRESSPROC m_pfnWglGetProcAddress;
};

Loader &Loader::instance()
{
	static Loader theInstance;
	return theInstance;
}

Loader::Loader() : m_hLibGL(nullptr), m_pfnWglGetProcAddress(nullptr)
{
	m_hLibGL = LoadLibraryA("opengl32.dll");

	if (m_hLibGL != nullptr)
	{
		m_pfnWglGetProcAddress = reinterpret_cast<PFNWGLGETPROCADDRESSPROC>(GetProcAddress(m_hLibGL, "wglGetProcAddress"));
	}
}

Loader::~Loader()
{
	if (m_hLibGL != nullptr)
	{
		FreeLibrary(m_hLibGL);
		m_hLibGL = nullptr;
	}
}

void *Loader::getProcAddress(const char* pszName) const
{
	void *pfn{nullptr};

	if (m_pfnWglGetProcAddress)
	{
		pfn = m_pfnWglGetProcAddress(pszName);
		
		if (!pfn || pfn == reinterpret_cast<void*>(1) || pfn == reinterpret_cast<void*>(2) || pfn == reinterpret_cast<void*>(3) || pfn == reinterpret_cast<void*>(-1))
		{
			pfn = GetProcAddress(m_hLibGL, pszName);
		}
	}

	return pfn;
}

//
// OpenGLContext methods
//

std::shared_ptr<OpenGLContext> OpenGLContext::createForWindow(HWND hWnd, PIXELFORMATDESCRIPTOR &pfd)
{
	std::shared_ptr<OpenGLContext> pContext{new OpenGLContext()};

	HDC hDC{GetDC(hWnd)};

	if (!hDC)
		return std::shared_ptr<OpenGLContext>{};
        
	int pf{ChoosePixelFormat(hDC, &pfd)};

    if (!SetPixelFormat(hDC, pf, &pfd))
		return std::shared_ptr<OpenGLContext>{};
		
	return pContext;
}

BOOL OpenGLContext::wglCopyContext(HGLRC hglrcSource, HGLRC hglrcDest, UINT mask)
{
	LOAD_ENTRYPOINT("wglCopyContext", m_pfnWglCopyContext, PFNWGLCOPYCONTEXTPROC);
	return m_pfnWglCopyContext(hglrcSource, hglrcDest, mask);
}

HGLRC OpenGLContext::wglCreateContext(HDC hdc)
{
	LOAD_ENTRYPOINT("wglCreateContext", m_pfnWglCreateContext, PFNWGLCREATECONTEXTPROC);
	return m_pfnWglCreateContext(hdc);
}

HGLRC OpenGLContext::wglCreateLayerContext(HDC hdc, int iLayerPlane)
{
	LOAD_ENTRYPOINT("wglCreateLayerContext", m_pfnWglCreateLayerContext, PFNWGLCREATELAYERCONTEXTPROC);
	return m_pfnWglCreateLayerContext(hdc, iLayerPlane);
}

BOOL OpenGLContext::wglDeleteContext(HGLRC hglrc)
{
	LOAD_ENTRYPOINT("wglDeleteContext", m_pfnWglDeleteContext, PFNWGLDELETECONTEXTPROC);
	return m_pfnWglDeleteContext(hglrc);
}

BOOL OpenGLContext::wglDescribeLayerPlane(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd)
{
	LOAD_ENTRYPOINT("wglDescribeLayerPlane", m_pfnWglDescribeLayerPlane, PFNWGLDESCRIBELAYERPLANEPROC);
	return m_pfnWglDescribeLayerPlane(hdc, iPixelFormat, iLayerPlane, nBytes, plpd);
}

HGLRC OpenGLContext::wglGetCurrentContext()
{
	LOAD_ENTRYPOINT("wglGetCurrentContext", m_pfnWglGetCurrentContext, PFNWGLGETCURRENTCONTEXTPROC);
	return m_pfnWglGetCurrentContext();
}

HDC OpenGLContext::wglGetCurrentDC()
{
	LOAD_ENTRYPOINT("wglGetCurrentDC", m_pfnWglGetCurrentDC, PFNWGLGETCURRENTDCPROC);
	return m_pfnWglGetCurrentDC();
}

int OpenGLContext::wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF *pcr)
{
	LOAD_ENTRYPOINT("wglGetLayerPaletteEntries", m_pfnWglGetLayerPaletteEntries, PFNWGLGETLAYERPALETTEENTRIESPROC);
	return m_pfnWglGetLayerPaletteEntries(hdc, iLayerPlane, iStart, cEntries, pcr);
}

PROC OpenGLContext::wglGetProcAddress(LPCSTR lpszProc)
{
	return reinterpret_cast<PROC>(Loader::instance().getProcAddress(lpszProc));
}

BOOL OpenGLContext::wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
	LOAD_ENTRYPOINT("wglMakeCurrent", m_pfnWglMakeCurrent, PFNWGLMAKECURRENTPROC);
	return m_pfnWglMakeCurrent(hdc, hglrc);
}

BOOL OpenGLContext::wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL bRealize)
{
	LOAD_ENTRYPOINT("wglRealizeLayerPalette", m_pfnWglRealizeLayerPalette, PFNWGLREALIZELAYERPALETTEPROC);
	return m_pfnWglRealizeLayerPalette(hdc, iLayerPlane, bRealize);
}

int OpenGLContext::wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, const COLORREF *pcr)
{
	LOAD_ENTRYPOINT("wglSetLayerPaletteEntries", m_pfnWglSetLayerPaletteEntries, PFNWGLSETLAYERPALETTEENTRIESPROC);
	return m_pfnWglSetLayerPaletteEntries(hdc, iLayerPlane, iStart, cEntries, pcr);
}

BOOL OpenGLContext::wglShareLists(HGLRC hglrc1, HGLRC hglrc2)
{
	LOAD_ENTRYPOINT("wglShareLists", m_pfnWglShareLists, PFNWGLSHARELISTSPROC);
	return m_pfnWglShareLists(hglrc1, hglrc2);
}

BOOL OpenGLContext::SwapBuffers(HDC hdc)
{
	//LOAD_ENTRYPOINT("SwapBuffers", m_pfnSwapBuffers, PFNSWAPBUFFERSPROC);
	//return m_pfnSwapBuffers(hdc);
	
	// WARNING
	// The above code doesn't work because the SwapBuffers() function is not exported by OpenGL32.dll.
	// Instead, it's exported by Gdi32.dll. Consequently, we have to use the SwapBuffers() function directly.
	// This is not a problem because the SwapBuffers() function is a standard Windows function that is always available.
	
	return ::SwapBuffers(hdc);
}

BOOL OpenGLContext::wglSwapLayerBuffers(HDC hdc, UINT fuPlanes)
{
	LOAD_ENTRYPOINT("wglSwapLayerBuffers", m_pfnWglSwapLayerBuffers, PFNWGLSWAPLAYERBUFFERSPROC);
	return m_pfnWglSwapLayerBuffers(hdc, fuPlanes);
}

DWORD OpenGLContext::wglSwapMultipleBuffers(UINT count, const WGLSWAP *toSwap)
{
	LOAD_ENTRYPOINT("wglSwapMultipleBuffers", m_pfnWglSwapMultipleBuffers, PFNWGLSWAPMULTIPLEBUFFERSPROC);
	return m_pfnWglSwapMultipleBuffers(count, toSwap);
}

BOOL OpenGLContext::wglUseFontBitmapsA(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
	LOAD_ENTRYPOINT("wglUseFontBitmapsA", m_pfnWglUseFontBitmapsA, PFNWGLUSEFONTBITMAPSPROC);
	return m_pfnWglUseFontBitmapsA(hdc, first, count, listBase);
}

BOOL OpenGLContext::wglUseFontBitmapsW(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
	LOAD_ENTRYPOINT("wglUseFontBitmapsW", m_pfnWglUseFontBitmapsW, PFNWGLUSEFONTBITMAPSPROC);
	return m_pfnWglUseFontBitmapsW(hdc, first, count, listBase);
}

BOOL OpenGLContext::wglUseFontOutlinesA(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf)
{
	LOAD_ENTRYPOINT("wglUseFontOutlinesA", m_pfnWglUseFontOutlinesA, PFNWGLUSEFONTOUTLINESPROC);
	return m_pfnWglUseFontOutlinesA(hdc, first, count, listBase, deviation, extrusion, format, lpgmf);
}

BOOL OpenGLContext::wglUseFontOutlinesW(HDC hdc, DWORD first, DWORD count, DWORD listBase, FLOAT deviation, FLOAT extrusion, int format, LPGLYPHMETRICSFLOAT lpgmf)
{
	LOAD_ENTRYPOINT("wglUseFontOutlinesW", m_pfnWglUseFontOutlinesW, PFNWGLUSEFONTOUTLINESPROC);
	return m_pfnWglUseFontOutlinesW(hdc, first, count, listBase, deviation, extrusion, format, lpgmf);
}

//
// GL_VERSION_1_0
//

void glCullFace(GLenum mode)
{
	using PFNGLCULLFACEPROC = void(APIENTRY *)(GLenum mode);
	static PFNGLCULLFACEPROC pfnCullFace{nullptr};
	LOAD_ENTRYPOINT("glCullFace", pfnCullFace, PFNGLCULLFACEPROC);
	pfnCullFace(mode);
}

void glFrontFace(GLenum mode)
{
	using PFNGLFRONTFACEPROC = void(APIENTRY *)(GLenum mode);
	static PFNGLFRONTFACEPROC pfnFrontFace{nullptr};
	LOAD_ENTRYPOINT("glFrontFace", pfnFrontFace, PFNGLFRONTFACEPROC);
	pfnFrontFace(mode);
}

void glHint(GLenum target, GLenum mode)
{
	using PFNGLHINTPROC = void(APIENTRY *)(GLenum target, GLenum mode);
	static PFNGLHINTPROC pfnHint{nullptr};
	LOAD_ENTRYPOINT("glHint", pfnHint, PFNGLHINTPROC);
	pfnHint(target, mode);
}

void glLineWidth(GLfloat width)
{
	using PFNGLLINEWIDTHPROC = void(APIENTRY *)(GLfloat width);
	static PFNGLLINEWIDTHPROC pfnLineWidth{nullptr};
	LOAD_ENTRYPOINT("glLineWidth", pfnLineWidth, PFNGLLINEWIDTHPROC);
	pfnLineWidth(width);
}

void glPointSize(GLfloat size)
{
	using PFNGLPOINTSIZEPROC = void(APIENTRY *)(GLfloat size);
	static PFNGLPOINTSIZEPROC pfnPointSize{nullptr};
	LOAD_ENTRYPOINT("glPointSize", pfnPointSize, PFNGLPOINTSIZEPROC);
	pfnPointSize(size);
}

void glPolygonMode(GLenum face, GLenum mode)
{
	using PFNGLPOLYGONMODEPROC = void(APIENTRY *)(GLenum face, GLenum mode);
	static PFNGLPOLYGONMODEPROC pfnPolygonMode{nullptr};
	LOAD_ENTRYPOINT("glPolygonMode", pfnPolygonMode, PFNGLPOLYGONMODEPROC);
	pfnPolygonMode(face, mode);
}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLSCISSORPROC = void(APIENTRY *)(GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLSCISSORPROC pfnScissor{nullptr};
	LOAD_ENTRYPOINT("glScissor", pfnScissor, PFNGLSCISSORPROC);
	pfnScissor(x, y, width, height);
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	using PFNGLTEXPARAMETERFPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLfloat param);
	static PFNGLTEXPARAMETERFPROC pfnTexParameterf{nullptr};
	LOAD_ENTRYPOINT("glTexParameterf", pfnTexParameterf, PFNGLTEXPARAMETERFPROC);
	pfnTexParameterf(target, pname, param);
}

void glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	using PFNGLTEXPARAMETERFVPROC = void(APIENTRY *)(GLenum target, GLenum pname, const GLfloat* params);
	static PFNGLTEXPARAMETERFVPROC pfnTexParameterfv{nullptr};
	LOAD_ENTRYPOINT("glTexParameterfv", pfnTexParameterfv, PFNGLTEXPARAMETERFVPROC);
	pfnTexParameterfv(target, pname, params);
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	using PFNGLTEXPARAMETERIPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint param);
	static PFNGLTEXPARAMETERIPROC pfnTexParameteri{nullptr};
	LOAD_ENTRYPOINT("glTexParameteri", pfnTexParameteri, PFNGLTEXPARAMETERIPROC);
	pfnTexParameteri(target, pname, param);
}

void glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	using PFNGLTEXPARAMETERIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, const GLint* params);
	static PFNGLTEXPARAMETERIVPROC pfnTexParameteriv{nullptr};
	LOAD_ENTRYPOINT("glTexParameteriv", pfnTexParameteriv, PFNGLTEXPARAMETERIVPROC);
	pfnTexParameteriv(target, pname, params);
}

void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXIMAGE1DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXIMAGE1DPROC pfnTexImage1D{nullptr};
	LOAD_ENTRYPOINT("glTexImage1D", pfnTexImage1D, PFNGLTEXIMAGE1DPROC);
	pfnTexImage1D(target, level, internalformat, width, border, format, type, pixels);
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXIMAGE2DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXIMAGE2DPROC pfnTexImage2D{nullptr};
	LOAD_ENTRYPOINT("glTexImage2D", pfnTexImage2D, PFNGLTEXIMAGE2DPROC);
	pfnTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

void glDrawBuffer(GLenum buf)
{
	using PFNGLDRAWBUFFERPROC = void(APIENTRY *)(GLenum buf);
	static PFNGLDRAWBUFFERPROC pfnDrawBuffer{nullptr};
	LOAD_ENTRYPOINT("glDrawBuffer", pfnDrawBuffer, PFNGLDRAWBUFFERPROC);
	pfnDrawBuffer(buf);
}

void glClear(GLbitfield mask)
{
	using PFNGLCLEARPROC = void(APIENTRY *)(GLbitfield mask);
	static PFNGLCLEARPROC pfnClear{nullptr};
	LOAD_ENTRYPOINT("glClear", pfnClear, PFNGLCLEARPROC);
	pfnClear(mask);
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	using PFNGLCLEARCOLORPROC = void(APIENTRY *)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	static PFNGLCLEARCOLORPROC pfnClearColor{nullptr};
	LOAD_ENTRYPOINT("glClearColor", pfnClearColor, PFNGLCLEARCOLORPROC);
	pfnClearColor(red, green, blue, alpha);
}

void glClearStencil(GLint s)
{
	using PFNGLCLEARSTENCILPROC = void(APIENTRY *)(GLint s);
	static PFNGLCLEARSTENCILPROC pfnClearStencil{nullptr};
	LOAD_ENTRYPOINT("glClearStencil", pfnClearStencil, PFNGLCLEARSTENCILPROC);
	pfnClearStencil(s);
}

void glClearDepth(GLdouble depth)
{
	using PFNGLCLEARDEPTHPROC = void(APIENTRY *)(GLdouble depth);
	static PFNGLCLEARDEPTHPROC pfnClearDepth{nullptr};
	LOAD_ENTRYPOINT("glClearDepth", pfnClearDepth, PFNGLCLEARDEPTHPROC);
	pfnClearDepth(depth);
}

void glStencilMask(GLuint mask)
{
	using PFNGLSTENCILMASKPROC = void(APIENTRY *)(GLuint mask);
	static PFNGLSTENCILMASKPROC pfnStencilMask{nullptr};
	LOAD_ENTRYPOINT("glStencilMask", pfnStencilMask, PFNGLSTENCILMASKPROC);
	pfnStencilMask(mask);
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	using PFNGLCOLORMASKPROC = void(APIENTRY *)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	static PFNGLCOLORMASKPROC pfnColorMask{nullptr};
	LOAD_ENTRYPOINT("glColorMask", pfnColorMask, PFNGLCOLORMASKPROC);
	pfnColorMask(red, green, blue, alpha);
}

void glDepthMask(GLboolean flag)
{
	using PFNGLDEPTHMASKPROC = void(APIENTRY *)(GLboolean flag);
	static PFNGLDEPTHMASKPROC pfnDepthMask{nullptr};
	LOAD_ENTRYPOINT("glDepthMask", pfnDepthMask, PFNGLDEPTHMASKPROC);
	pfnDepthMask(flag);
}

void glDisable(GLenum cap)
{
	using PFNGLDISABLEPROC = void(APIENTRY *)(GLenum cap);
	static PFNGLDISABLEPROC pfnDisable{nullptr};
	LOAD_ENTRYPOINT("glDisable", pfnDisable, PFNGLDISABLEPROC);
	pfnDisable(cap);
}

void glEnable(GLenum cap)
{
	using PFNGLENABLEPROC = void(APIENTRY *)(GLenum cap);
	static PFNGLENABLEPROC pfnEnable{nullptr};
	LOAD_ENTRYPOINT("glEnable", pfnEnable, PFNGLENABLEPROC);
	pfnEnable(cap);
}

void glFinish(void)
{
	using PFNGLFINISHPROC = void(APIENTRY *)(void);
	static PFNGLFINISHPROC pfnFinish{nullptr};
	LOAD_ENTRYPOINT("glFinish", pfnFinish, PFNGLFINISHPROC);
	pfnFinish();
}

void glFlush(void)
{
	using PFNGLFLUSHPROC = void(APIENTRY *)(void);
	static PFNGLFLUSHPROC pfnFlush{nullptr};
	LOAD_ENTRYPOINT("glFlush", pfnFlush, PFNGLFLUSHPROC);
	pfnFlush();
}

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	using PFNGLBLENDFUNCPROC = void(APIENTRY *)(GLenum sfactor, GLenum dfactor);
	static PFNGLBLENDFUNCPROC pfnBlendFunc{nullptr};
	LOAD_ENTRYPOINT("glBlendFunc", pfnBlendFunc, PFNGLBLENDFUNCPROC);
	pfnBlendFunc(sfactor, dfactor);
}

void glLogicOp(GLenum opcode)
{
	using PFNGLLOGICOPPROC = void(APIENTRY *)(GLenum opcode);
	static PFNGLLOGICOPPROC pfnLogicOp{nullptr};
	LOAD_ENTRYPOINT("glLogicOp", pfnLogicOp, PFNGLLOGICOPPROC);
	pfnLogicOp(opcode);
}

void glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	using PFNGLSTENCILFUNCPROC = void(APIENTRY *)(GLenum func, GLint ref, GLuint mask);
	static PFNGLSTENCILFUNCPROC pfnStencilFunc{nullptr};
	LOAD_ENTRYPOINT("glStencilFunc", pfnStencilFunc, PFNGLSTENCILFUNCPROC);
	pfnStencilFunc(func, ref, mask);
}

void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	using PFNGLSTENCILOPPROC = void(APIENTRY *)(GLenum fail, GLenum zfail, GLenum zpass);
	static PFNGLSTENCILOPPROC pfnStencilOp{nullptr};
	LOAD_ENTRYPOINT("glStencilOp", pfnStencilOp, PFNGLSTENCILOPPROC);
	pfnStencilOp(fail, zfail, zpass);
}

void glDepthFunc(GLenum func)
{
	using PFNGLDEPTHFUNCPROC = void(APIENTRY *)(GLenum func);
	static PFNGLDEPTHFUNCPROC pfnDepthFunc{nullptr};
	LOAD_ENTRYPOINT("glDepthFunc", pfnDepthFunc, PFNGLDEPTHFUNCPROC);
	pfnDepthFunc(func);
}

void glPixelStoref(GLenum pname, GLfloat param)
{
	using PFNGLPIXELSTOREFPROC = void(APIENTRY *)(GLenum pname, GLfloat param);
	static PFNGLPIXELSTOREFPROC pfnPixelStoref{nullptr};
	LOAD_ENTRYPOINT("glPixelStoref", pfnPixelStoref, PFNGLPIXELSTOREFPROC);
	pfnPixelStoref(pname, param);
}

void glPixelStorei(GLenum pname, GLint param)
{
	using PFNGLPIXELSTOREIPROC = void(APIENTRY *)(GLenum pname, GLint param);
	static PFNGLPIXELSTOREIPROC pfnPixelStorei{nullptr};
	LOAD_ENTRYPOINT("glPixelStorei", pfnPixelStorei, PFNGLPIXELSTOREIPROC);
	pfnPixelStorei(pname, param);
}

void glReadBuffer(GLenum src)
{
	using PFNGLREADBUFFERPROC = void(APIENTRY *)(GLenum src);
	static PFNGLREADBUFFERPROC pfnReadBuffer{nullptr};
	LOAD_ENTRYPOINT("glReadBuffer", pfnReadBuffer, PFNGLREADBUFFERPROC);
	pfnReadBuffer(src);
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
	using PFNGLREADPIXELSPROC = void(APIENTRY *)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
	static PFNGLREADPIXELSPROC pfnReadPixels{nullptr};
	LOAD_ENTRYPOINT("glReadPixels", pfnReadPixels, PFNGLREADPIXELSPROC);
	pfnReadPixels(x, y, width, height, format, type, pixels);
}

void glGetBooleanv(GLenum pname, GLboolean* data)
{
	using PFNGLGETBOOLEANVPROC = void(APIENTRY *)(GLenum pname, GLboolean* data);
	static PFNGLGETBOOLEANVPROC pfnGetBooleanv{nullptr};
	LOAD_ENTRYPOINT("glGetBooleanv", pfnGetBooleanv, PFNGLGETBOOLEANVPROC);
	pfnGetBooleanv(pname, data);
}

void glGetDoublev(GLenum pname, GLdouble* data)
{
	using PFNGLGETDOUBLEVPROC = void(APIENTRY *)(GLenum pname, GLdouble* data);
	static PFNGLGETDOUBLEVPROC pfnGetDoublev{nullptr};
	LOAD_ENTRYPOINT("glGetDoublev", pfnGetDoublev, PFNGLGETDOUBLEVPROC);
	pfnGetDoublev(pname, data);
}

GLenum glGetError(void)
{
	using PFNGLGETERRORPROC = GLenum(APIENTRY *)(void);
	static PFNGLGETERRORPROC pfnGetError{nullptr};
	LOAD_ENTRYPOINT("glGetError", pfnGetError, PFNGLGETERRORPROC);
	return pfnGetError();
}

void glGetFloatv(GLenum pname, GLfloat* data)
{
	using PFNGLGETFLOATVPROC = void(APIENTRY *)(GLenum pname, GLfloat* data);
	static PFNGLGETFLOATVPROC pfnGetFloatv{nullptr};
	LOAD_ENTRYPOINT("glGetFloatv", pfnGetFloatv, PFNGLGETFLOATVPROC);
	pfnGetFloatv(pname, data);
}

void glGetIntegerv(GLenum pname, GLint* data)
{
	using PFNGLGETINTEGERVPROC = void(APIENTRY *)(GLenum pname, GLint* data);
	static PFNGLGETINTEGERVPROC pfnGetIntegerv{nullptr};
	LOAD_ENTRYPOINT("glGetIntegerv", pfnGetIntegerv, PFNGLGETINTEGERVPROC);
	pfnGetIntegerv(pname, data);
}

const GLubyte* glGetString(GLenum name)
{
	using PFNGLGETSTRINGPROC = const GLubyte*(APIENTRY *)(GLenum name);
	static PFNGLGETSTRINGPROC pfnGetString{nullptr};
	LOAD_ENTRYPOINT("glGetString", pfnGetString, PFNGLGETSTRINGPROC);
	return pfnGetString(name);
}

void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
	using PFNGLGETTEXIMAGEPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum format, GLenum type, void* pixels);
	static PFNGLGETTEXIMAGEPROC pfnGetTexImage{nullptr};
	LOAD_ENTRYPOINT("glGetTexImage", pfnGetTexImage, PFNGLGETTEXIMAGEPROC);
	pfnGetTexImage(target, level, format, type, pixels);
}

void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
	using PFNGLGETTEXPARAMETERFVPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLfloat* params);
	static PFNGLGETTEXPARAMETERFVPROC pfnGetTexParameterfv{nullptr};
	LOAD_ENTRYPOINT("glGetTexParameterfv", pfnGetTexParameterfv, PFNGLGETTEXPARAMETERFVPROC);
	pfnGetTexParameterfv(target, pname, params);
}

void glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETTEXPARAMETERIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint* params);
	static PFNGLGETTEXPARAMETERIVPROC pfnGetTexParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetTexParameteriv", pfnGetTexParameteriv, PFNGLGETTEXPARAMETERIVPROC);
	pfnGetTexParameteriv(target, pname, params);
}

void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params)
{
	using PFNGLGETTEXLEVELPARAMETERFVPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum pname, GLfloat* params);
	static PFNGLGETTEXLEVELPARAMETERFVPROC pfnGetTexLevelParameterfv{nullptr};
	LOAD_ENTRYPOINT("glGetTexLevelParameterfv", pfnGetTexLevelParameterfv, PFNGLGETTEXLEVELPARAMETERFVPROC);
	pfnGetTexLevelParameterfv(target, level, pname, params);
}

void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params)
{
	using PFNGLGETTEXLEVELPARAMETERIVPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum pname, GLint* params);
	static PFNGLGETTEXLEVELPARAMETERIVPROC pfnGetTexLevelParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetTexLevelParameteriv", pfnGetTexLevelParameteriv, PFNGLGETTEXLEVELPARAMETERIVPROC);
	pfnGetTexLevelParameteriv(target, level, pname, params);
}

GLboolean glIsEnabled(GLenum cap)
{
	using PFNGLISENABLEDPROC = GLboolean(APIENTRY *)(GLenum cap);
	static PFNGLISENABLEDPROC pfnIsEnabled{nullptr};
	LOAD_ENTRYPOINT("glIsEnabled", pfnIsEnabled, PFNGLISENABLEDPROC);
	return pfnIsEnabled(cap);
}

void glDepthRange(GLdouble n, GLdouble f)
{
	using PFNGLDEPTHRANGEPROC = void(APIENTRY *)(GLdouble n, GLdouble f);
	static PFNGLDEPTHRANGEPROC pfnDepthRange{nullptr};
	LOAD_ENTRYPOINT("glDepthRange", pfnDepthRange, PFNGLDEPTHRANGEPROC);
	pfnDepthRange(n, f);
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLVIEWPORTPROC = void(APIENTRY *)(GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLVIEWPORTPROC pfnViewport{nullptr};
	LOAD_ENTRYPOINT("glViewport", pfnViewport, PFNGLVIEWPORTPROC);
	pfnViewport(x, y, width, height);
}

//
// GL_VERSION_1_1
//

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	using PFNGLDRAWARRAYSPROC = void(APIENTRY *)(GLenum mode, GLint first, GLsizei count);
	static PFNGLDRAWARRAYSPROC pfnDrawArrays{nullptr};
	LOAD_ENTRYPOINT("glDrawArrays", pfnDrawArrays, PFNGLDRAWARRAYSPROC);
	pfnDrawArrays(mode, first, count);
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
	using PFNGLDRAWELEMENTSPROC = void(APIENTRY *)(GLenum mode, GLsizei count, GLenum type, const void* indices);
	static PFNGLDRAWELEMENTSPROC pfnDrawElements{nullptr};
	LOAD_ENTRYPOINT("glDrawElements", pfnDrawElements, PFNGLDRAWELEMENTSPROC);
	pfnDrawElements(mode, count, type, indices);
}

void glGetPointerv(GLenum pname, void** params)
{
	using PFNGLGETPOINTERVPROC = void(APIENTRY *)(GLenum pname, void** params);
	static PFNGLGETPOINTERVPROC pfnGetPointerv{nullptr};
	LOAD_ENTRYPOINT("glGetPointerv", pfnGetPointerv, PFNGLGETPOINTERVPROC);
	pfnGetPointerv(pname, params);
}

void glPolygonOffset(GLfloat factor, GLfloat units)
{
	using PFNGLPOLYGONOFFSETPROC = void(APIENTRY *)(GLfloat factor, GLfloat units);
	static PFNGLPOLYGONOFFSETPROC pfnPolygonOffset{nullptr};
	LOAD_ENTRYPOINT("glPolygonOffset", pfnPolygonOffset, PFNGLPOLYGONOFFSETPROC);
	pfnPolygonOffset(factor, units);
}

void glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	using PFNGLCOPYTEXIMAGE1DPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
	static PFNGLCOPYTEXIMAGE1DPROC pfnCopyTexImage1D{nullptr};
	LOAD_ENTRYPOINT("glCopyTexImage1D", pfnCopyTexImage1D, PFNGLCOPYTEXIMAGE1DPROC);
	pfnCopyTexImage1D(target, level, internalformat, x, y, width, border);
}

void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	using PFNGLCOPYTEXIMAGE2DPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	static PFNGLCOPYTEXIMAGE2DPROC pfnCopyTexImage2D{nullptr};
	LOAD_ENTRYPOINT("glCopyTexImage2D", pfnCopyTexImage2D, PFNGLCOPYTEXIMAGE2DPROC);
	pfnCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	using PFNGLCOPYTEXSUBIMAGE1DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	static PFNGLCOPYTEXSUBIMAGE1DPROC pfnCopyTexSubImage1D{nullptr};
	LOAD_ENTRYPOINT("glCopyTexSubImage1D", pfnCopyTexSubImage1D, PFNGLCOPYTEXSUBIMAGE1DPROC);
	pfnCopyTexSubImage1D(target, level, xoffset, x, y, width);
}

void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLCOPYTEXSUBIMAGE2DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLCOPYTEXSUBIMAGE2DPROC pfnCopyTexSubImage2D{nullptr};
	LOAD_ENTRYPOINT("glCopyTexSubImage2D", pfnCopyTexSubImage2D, PFNGLCOPYTEXSUBIMAGE2DPROC);
	pfnCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXSUBIMAGE1DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXSUBIMAGE1DPROC pfnTexSubImage1D{nullptr};
	LOAD_ENTRYPOINT("glTexSubImage1D", pfnTexSubImage1D, PFNGLTEXSUBIMAGE1DPROC);
	pfnTexSubImage1D(target, level, xoffset, width, format, type, pixels);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXSUBIMAGE2DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXSUBIMAGE2DPROC pfnTexSubImage2D{nullptr};
	LOAD_ENTRYPOINT("glTexSubImage2D", pfnTexSubImage2D, PFNGLTEXSUBIMAGE2DPROC);
	pfnTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void glBindTexture(GLenum target, GLuint texture)
{
	using PFNGLBINDTEXTUREPROC = void(APIENTRY *)(GLenum target, GLuint texture);
	static PFNGLBINDTEXTUREPROC pfnBindTexture{nullptr};
	LOAD_ENTRYPOINT("glBindTexture", pfnBindTexture, PFNGLBINDTEXTUREPROC);
	pfnBindTexture(target, texture);
}

void glDeleteTextures(GLsizei n, const GLuint* textures)
{
	using PFNGLDELETETEXTURESPROC = void(APIENTRY *)(GLsizei n, const GLuint* textures);
	static PFNGLDELETETEXTURESPROC pfnDeleteTextures{nullptr};
	LOAD_ENTRYPOINT("glDeleteTextures", pfnDeleteTextures, PFNGLDELETETEXTURESPROC);
	pfnDeleteTextures(n, textures);
}

void glGenTextures(GLsizei n, GLuint* textures)
{
	using PFNGLGENTEXTURESPROC = void(APIENTRY *)(GLsizei n, GLuint* textures);
	static PFNGLGENTEXTURESPROC pfnGenTextures{nullptr};
	LOAD_ENTRYPOINT("glGenTextures", pfnGenTextures, PFNGLGENTEXTURESPROC);
	pfnGenTextures(n, textures);
}

GLboolean glIsTexture(GLuint texture)
{
	using PFNGLISTEXTUREPROC = GLboolean(APIENTRY *)(GLuint texture);
	static PFNGLISTEXTUREPROC pfnIsTexture{nullptr};
	LOAD_ENTRYPOINT("glIsTexture", pfnIsTexture, PFNGLISTEXTUREPROC);
	return pfnIsTexture(texture);
}