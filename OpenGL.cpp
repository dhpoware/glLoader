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

import <string>;
import <utility>;

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

OpenGLContext::OpenGLContext()
{
}

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

bool OpenGLContext::extensionSupported(const std::wstring extensionName)
{
	if (m_extensions.empty())
	{
		if (createDummyGLWindow())
		{
			// The dummy window will only be at most an OpenGL 2.1 rendering
            // context so we can still use the deprecated GL_EXTENSIONS flag
            // to glGetString().

			if (const char *pszExtensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)))
				m_extensions = toWideString(pszExtensions);

			// Load the WGL extension strings.
			
			m_extensions += L' ';
			m_extensions += toWideString(wglGetExtensionsStringARB(wglGetCurrentDC()));

			destroyDummyGLWindow();
		}
	}

	return m_extensions.find(extensionName) != std::wstring::npos;
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

HGLRC OpenGLContext::wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int* attribList)
{
	// An OpenGL 3.1 rendering context is created using the new
    // wglCreateContextAttribsARB() function. This new function was introduced
    // in OpenGL 3.0 to maintain backwards compatibility with existing OpenGL
    // 2.1 and older applications. To create an OpenGL 3.1 rendering context
    // first create an OpenGL 2.1 or older rendering context using the
    // wglCreateContext() function. Activate the context and then call the new
    // wglCreateContextAttribsARB() function to create an OpenGL 3.1 rendering
    // context. Once the context is created activate it to enable OpenGL 3.1
    // functionality.
    //
    // For further details see:
    // http://www.opengl.org/registry/specs/ARB/wgl_create_context.txt

	HGLRC hContext{nullptr};
    HGLRC hCurrentContext{wglGetCurrentContext()};

	if (!hCurrentContext)
    {
        if (!(hCurrentContext = wglCreateContext(hDC)))
            return nullptr;

        if (!wglMakeCurrent(hDC, hCurrentContext))
        {
            wglDeleteContext(hCurrentContext);
            return nullptr;
        }

        LOAD_ENTRYPOINT("wglCreateContextAttribsARB", m_pfnWglCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC);
        
        if (m_pfnWglCreateContextAttribsARB)
            hContext = m_pfnWglCreateContextAttribsARB(hDC, hShareContext, attribList);

        wglMakeCurrent(hDC, nullptr);
        wglDeleteContext(hCurrentContext);
    }
	else
    {
        if (!wglMakeCurrent(hDC, hCurrentContext))
            return nullptr;

        LOAD_ENTRYPOINT("wglCreateContextAttribsARB", m_pfnWglCreateContextAttribsARB, PFNWGLCREATECONTEXTATTRIBSARBPROC);
        
        if (m_pfnWglCreateContextAttribsARB)
            hContext = m_pfnWglCreateContextAttribsARB(hDC, hShareContext, attribList);
    }

    return hContext;
}

const char *OpenGLContext::wglGetExtensionsStringARB(HDC hDC)
{
	if (!m_pfnWglGetExtensionsStringARB)
		m_pfnWglGetExtensionsStringARB = reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGARBPROC>(wglGetProcAddress("wglGetExtensionsStringARB"));
	
	return m_pfnWglGetExtensionsStringARB(hDC);
}

BOOL OpenGLContext::wglSwapIntervalEXT(int interval)
{
    LOAD_ENTRYPOINT("wglSwapIntervalEXT", m_pfnWglSwapIntervalEXT, PFNWGLSWAPINTERVALEXTPROC);
    return m_pfnWglSwapIntervalEXT(interval);
}

int OpenGLContext::wglGetSwapIntervalEXT(void)
{
	LOAD_ENTRYPOINT("wglGetSwapIntervalEXT", m_pfnWglGetSwapIntervalEXT, PFNWGLGETSWAPINTERVALEXTPROC);
    return m_pfnWglGetSwapIntervalEXT();
}

LRESULT CALLBACK OpenGLContext::dummyGLWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OpenGLContext *pContext{nullptr};

	if (msg == WM_NCCREATE)
	{
		pContext = reinterpret_cast<OpenGLContext *>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pContext));
	}
	else
	{
		pContext = reinterpret_cast<OpenGLContext *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (!pContext)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	pContext->m_hWnd = hWnd;
	return pContext->dummyGLWndProcImpl(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK OpenGLContext::dummyGLWndProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		if (!(m_hDC = GetDC(hWnd)))
			return -1;
		break;

	case WM_DESTROY:
		if (m_hDC)
		{
			if (m_hRC)
			{
				wglMakeCurrent(m_hDC, nullptr);
				wglDeleteContext(m_hRC);
				m_hRC = nullptr;
			}

			ReleaseDC(hWnd, m_hDC);
			m_hDC = nullptr;
		}

		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool OpenGLContext::createDummyGLWindow()
{
	m_wcl.cbSize = sizeof(m_wcl);
	m_wcl.style = CS_OWNDC;
	m_wcl.lpfnWndProc = dummyGLWndProc;
	m_wcl.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
	m_wcl.lpszClassName = L"dummyGLWindowClass";

	if (!RegisterClassEx(&m_wcl))
		return false;

	m_hWnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, m_wcl.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, m_wcl.hInstance, this);

	if (!m_hWnd)
	{
		UnregisterClassW(m_wcl.lpszClassName, m_wcl.hInstance);
		return false;
	}
	else
	{
		PIXELFORMATDESCRIPTOR pfd
		{
			.nSize = sizeof(pfd),
			.nVersion = 1,
			.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
			.iPixelType = PFD_TYPE_RGBA,
			.cColorBits = 24,
			.cDepthBits = 16,
			.iLayerType = PFD_MAIN_PLANE,
		};

		int pf{ChoosePixelFormat(m_hDC, &pfd)};

		if (!SetPixelFormat(m_hDC, pf, &pfd))
			return false;

		if (!(m_hRC = wglCreateContext(m_hDC)))
			return false;

		if (!wglMakeCurrent(m_hDC, m_hRC))
			return false;

		return true;
	}
}

void OpenGLContext::destroyDummyGLWindow() const
{
	if (m_hWnd)
	{
		PostMessage(m_hWnd, WM_CLOSE, 0, 0);

        BOOL bRet{FALSE};
        MSG msg{};

		while ((bRet = GetMessage(&msg, 0, 0, 0)) != 0)
		{ 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}        

	UnregisterClassW(m_wcl.lpszClassName, m_wcl.hInstance);
}

std::wstring OpenGLContext::toWideString(const char *pszStr) const
{
	// This conversion is Windows platform specific and is taken from here:
	// https://learn.microsoft.com/en-us/cpp/text/how-to-convert-between-various-string-types?view=msvc-170

    size_t newsize{std::strlen(pszStr) + 1};
	wchar_t *pszWideString = new wchar_t[newsize];
    size_t convertedChars{};
	mbstowcs_s(&convertedChars, pszWideString, newsize, pszStr, _TRUNCATE);
	std::wstring result{pszWideString};
	delete[] pszWideString;
	return result;
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

//
// GL_VERSION_1_2
//

void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices)
{
	using PFNGLDRAWRANGEELEMENTSPROC = void(APIENTRY *)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices);
	static PFNGLDRAWRANGEELEMENTSPROC pfnDrawRangeElements{nullptr};
	LOAD_ENTRYPOINT("glDrawRangeElements", pfnDrawRangeElements, PFNGLDRAWRANGEELEMENTSPROC);
	pfnDrawRangeElements(mode, start, end, count, type, indices);
}

void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXIMAGE3DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXIMAGE3DPROC pfnTexImage3D{nullptr};
	LOAD_ENTRYPOINT("glTexImage3D", pfnTexImage3D, PFNGLTEXIMAGE3DPROC);
	pfnTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXSUBIMAGE3DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXSUBIMAGE3DPROC pfnTexSubImage3D{nullptr};
	LOAD_ENTRYPOINT("glTexSubImage3D", pfnTexSubImage3D, PFNGLTEXSUBIMAGE3DPROC);
	pfnTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLCOPYTEXSUBIMAGE3DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLCOPYTEXSUBIMAGE3DPROC pfnCopyTexSubImage3D{nullptr};
	LOAD_ENTRYPOINT("glCopyTexSubImage3D", pfnCopyTexSubImage3D, PFNGLCOPYTEXSUBIMAGE3DPROC);
	pfnCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

//
// GL_VERSION_1_3
//

void glActiveTexture(GLenum texture)
{
	using PFNGLACTIVETEXTUREPROC = void(APIENTRY *)(GLenum texture);
	static PFNGLACTIVETEXTUREPROC pfnActiveTexture{nullptr};
	LOAD_ENTRYPOINT("glActiveTexture", pfnActiveTexture, PFNGLACTIVETEXTUREPROC);
	pfnActiveTexture(texture);
}

void glSampleCoverage(GLfloat value, GLboolean invert)
{
	using PFNGLSAMPLECOVERAGEPROC = void(APIENTRY *)(GLfloat value, GLboolean invert);
	static PFNGLSAMPLECOVERAGEPROC pfnSampleCoverage{nullptr};
	LOAD_ENTRYPOINT("glSampleCoverage", pfnSampleCoverage, PFNGLSAMPLECOVERAGEPROC);
	pfnSampleCoverage(value, invert);
}

void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)
{
	using PFNGLCOMPRESSEDTEXIMAGE3DPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data);
	static PFNGLCOMPRESSEDTEXIMAGE3DPROC pfnCompressedTexImage3D{nullptr};
	LOAD_ENTRYPOINT("glCompressedTexImage3D", pfnCompressedTexImage3D, PFNGLCOMPRESSEDTEXIMAGE3DPROC);
	pfnCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
	using PFNGLCOMPRESSEDTEXIMAGE2DPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);
	static PFNGLCOMPRESSEDTEXIMAGE2DPROC pfnCompressedTexImage2D{nullptr};
	LOAD_ENTRYPOINT("glCompressedTexImage2D", pfnCompressedTexImage2D, PFNGLCOMPRESSEDTEXIMAGE2DPROC);
	pfnCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data)
{
	using PFNGLCOMPRESSEDTEXIMAGE1DPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data);
	static PFNGLCOMPRESSEDTEXIMAGE1DPROC pfnCompressedTexImage1D{nullptr};
	LOAD_ENTRYPOINT("glCompressedTexImage1D", pfnCompressedTexImage1D, PFNGLCOMPRESSEDTEXIMAGE1DPROC);
	pfnCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
}

void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
	using PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);
	static PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC pfnCompressedTexSubImage3D{nullptr};
	LOAD_ENTRYPOINT("glCompressedTexSubImage3D", pfnCompressedTexSubImage3D, PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC);
	pfnCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
	using PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
	static PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC pfnCompressedTexSubImage2D{nullptr};
	LOAD_ENTRYPOINT("glCompressedTexSubImage2D", pfnCompressedTexSubImage2D, PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC);
	pfnCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
	using PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data);
	static PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC pfnCompressedTexSubImage1D{nullptr};
	LOAD_ENTRYPOINT("glCompressedTexSubImage1D", pfnCompressedTexSubImage1D, PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC);
	pfnCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
}

void glGetCompressedTexImage(GLenum target, GLint level, void* img)
{
	using PFNGLGETCOMPRESSEDTEXIMAGEPROC = void(APIENTRY *)(GLenum target, GLint level, void* img);
	static PFNGLGETCOMPRESSEDTEXIMAGEPROC pfnGetCompressedTexImage{nullptr};
	LOAD_ENTRYPOINT("glGetCompressedTexImage", pfnGetCompressedTexImage, PFNGLGETCOMPRESSEDTEXIMAGEPROC);
	pfnGetCompressedTexImage(target, level, img);
}

//
// GL_VERSION_1_4
//

void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
	using PFNGLBLENDFUNCSEPARATEPROC = void(APIENTRY *)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
	static PFNGLBLENDFUNCSEPARATEPROC pfnBlendFuncSeparate{nullptr};
	LOAD_ENTRYPOINT("glBlendFuncSeparate", pfnBlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC);
	pfnBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

void glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount)
{
	using PFNGLMULTIDRAWARRAYSPROC = void(APIENTRY *)(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount);
	static PFNGLMULTIDRAWARRAYSPROC pfnMultiDrawArrays{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawArrays", pfnMultiDrawArrays, PFNGLMULTIDRAWARRAYSPROC);
	pfnMultiDrawArrays(mode, first, count, drawcount);
}

void glMultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount)
{
	using PFNGLMULTIDRAWELEMENTSPROC = void(APIENTRY *)(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount);
	static PFNGLMULTIDRAWELEMENTSPROC pfnMultiDrawElements{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawElements", pfnMultiDrawElements, PFNGLMULTIDRAWELEMENTSPROC);
	pfnMultiDrawElements(mode, count, type, indices, drawcount);
}

void glPointParameterf(GLenum pname, GLfloat param)
{
	using PFNGLPOINTPARAMETERFPROC = void(APIENTRY *)(GLenum pname, GLfloat param);
	static PFNGLPOINTPARAMETERFPROC pfnPointParameterf{nullptr};
	LOAD_ENTRYPOINT("glPointParameterf", pfnPointParameterf, PFNGLPOINTPARAMETERFPROC);
	pfnPointParameterf(pname, param);
}

void glPointParameterfv(GLenum pname, const GLfloat* params)
{
	using PFNGLPOINTPARAMETERFVPROC = void(APIENTRY *)(GLenum pname, const GLfloat* params);
	static PFNGLPOINTPARAMETERFVPROC pfnPointParameterfv{nullptr};
	LOAD_ENTRYPOINT("glPointParameterfv", pfnPointParameterfv, PFNGLPOINTPARAMETERFVPROC);
	pfnPointParameterfv(pname, params);
}

void glPointParameteri(GLenum pname, GLint param)
{
	using PFNGLPOINTPARAMETERIPROC = void(APIENTRY *)(GLenum pname, GLint param);
	static PFNGLPOINTPARAMETERIPROC pfnPointParameteri{nullptr};
	LOAD_ENTRYPOINT("glPointParameteri", pfnPointParameteri, PFNGLPOINTPARAMETERIPROC);
	pfnPointParameteri(pname, param);
}

void glPointParameteriv(GLenum pname, const GLint* params)
{
	using PFNGLPOINTPARAMETERIVPROC = void(APIENTRY *)(GLenum pname, const GLint* params);
	static PFNGLPOINTPARAMETERIVPROC pfnPointParameteriv{nullptr};
	LOAD_ENTRYPOINT("glPointParameteriv", pfnPointParameteriv, PFNGLPOINTPARAMETERIVPROC);
	pfnPointParameteriv(pname, params);
}

void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	using PFNGLBLENDCOLORPROC = void(APIENTRY *)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	static PFNGLBLENDCOLORPROC pfnBlendColor{nullptr};
	LOAD_ENTRYPOINT("glBlendColor", pfnBlendColor, PFNGLBLENDCOLORPROC);
	pfnBlendColor(red, green, blue, alpha);
}

void glBlendEquation(GLenum mode)
{
	using PFNGLBLENDEQUATIONPROC = void(APIENTRY *)(GLenum mode);
	static PFNGLBLENDEQUATIONPROC pfnBlendEquation{nullptr};
	LOAD_ENTRYPOINT("glBlendEquation", pfnBlendEquation, PFNGLBLENDEQUATIONPROC);
	pfnBlendEquation(mode);
}

//
// GL_VERSION_1_5
//

void glGenQueries(GLsizei n, GLuint* ids)
{
	using PFNGLGENQUERIESPROC = void(APIENTRY *)(GLsizei n, GLuint* ids);
	static PFNGLGENQUERIESPROC pfnGenQueries{nullptr};
	LOAD_ENTRYPOINT("glGenQueries", pfnGenQueries, PFNGLGENQUERIESPROC);
	pfnGenQueries(n, ids);
}

void glDeleteQueries(GLsizei n, const GLuint* ids)
{
	using PFNGLDELETEQUERIESPROC = void(APIENTRY *)(GLsizei n, const GLuint* ids);
	static PFNGLDELETEQUERIESPROC pfnDeleteQueries{nullptr};
	LOAD_ENTRYPOINT("glDeleteQueries", pfnDeleteQueries, PFNGLDELETEQUERIESPROC);
	pfnDeleteQueries(n, ids);
}

GLboolean glIsQuery(GLuint id)
{
	using PFNGLISQUERYPROC = GLboolean(APIENTRY *)(GLuint id);
	static PFNGLISQUERYPROC pfnIsQuery{nullptr};
	LOAD_ENTRYPOINT("glIsQuery", pfnIsQuery, PFNGLISQUERYPROC);
	return pfnIsQuery(id);
}

void glBeginQuery(GLenum target, GLuint id)
{
	using PFNGLBEGINQUERYPROC = void(APIENTRY *)(GLenum target, GLuint id);
	static PFNGLBEGINQUERYPROC pfnBeginQuery{nullptr};
	LOAD_ENTRYPOINT("glBeginQuery", pfnBeginQuery, PFNGLBEGINQUERYPROC);
	pfnBeginQuery(target, id);
}

void glEndQuery(GLenum target)
{
	using PFNGLENDQUERYPROC = void(APIENTRY *)(GLenum target);
	static PFNGLENDQUERYPROC pfnEndQuery{nullptr};
	LOAD_ENTRYPOINT("glEndQuery", pfnEndQuery, PFNGLENDQUERYPROC);
	pfnEndQuery(target);
}

void glGetQueryiv(GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETQUERYIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint* params);
	static PFNGLGETQUERYIVPROC pfnGetQueryiv{nullptr};
	LOAD_ENTRYPOINT("glGetQueryiv", pfnGetQueryiv, PFNGLGETQUERYIVPROC);
	pfnGetQueryiv(target, pname, params);
}

void glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params)
{
	using PFNGLGETQUERYOBJECTIVPROC = void(APIENTRY *)(GLuint id, GLenum pname, GLint* params);
	static PFNGLGETQUERYOBJECTIVPROC pfnGetQueryObjectiv{nullptr};
	LOAD_ENTRYPOINT("glGetQueryObjectiv", pfnGetQueryObjectiv, PFNGLGETQUERYOBJECTIVPROC);
	pfnGetQueryObjectiv(id, pname, params);
}

void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
{
	using PFNGLGETQUERYOBJECTUIVPROC = void(APIENTRY *)(GLuint id, GLenum pname, GLuint* params);
	static PFNGLGETQUERYOBJECTUIVPROC pfnGetQueryObjectuiv{nullptr};
	LOAD_ENTRYPOINT("glGetQueryObjectuiv", pfnGetQueryObjectuiv, PFNGLGETQUERYOBJECTUIVPROC);
	pfnGetQueryObjectuiv(id, pname, params);
}

void glBindBuffer(GLenum target, GLuint buffer)
{
	using PFNGLBINDBUFFERPROC = void(APIENTRY *)(GLenum target, GLuint buffer);
	static PFNGLBINDBUFFERPROC pfnBindBuffer{nullptr};
	LOAD_ENTRYPOINT("glBindBuffer", pfnBindBuffer, PFNGLBINDBUFFERPROC);
	pfnBindBuffer(target, buffer);
}

void glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
	using PFNGLDELETEBUFFERSPROC = void(APIENTRY *)(GLsizei n, const GLuint* buffers);
	static PFNGLDELETEBUFFERSPROC pfnDeleteBuffers{nullptr};
	LOAD_ENTRYPOINT("glDeleteBuffers", pfnDeleteBuffers, PFNGLDELETEBUFFERSPROC);
	pfnDeleteBuffers(n, buffers);
}

void glGenBuffers(GLsizei n, GLuint* buffers)
{
	using PFNGLGENBUFFERSPROC = void(APIENTRY *)(GLsizei n, GLuint* buffers);
	static PFNGLGENBUFFERSPROC pfnGenBuffers{nullptr};
	LOAD_ENTRYPOINT("glGenBuffers", pfnGenBuffers, PFNGLGENBUFFERSPROC);
	pfnGenBuffers(n, buffers);
}

GLboolean glIsBuffer(GLuint buffer)
{
	using PFNGLISBUFFERPROC = GLboolean(APIENTRY *)(GLuint buffer);
	static PFNGLISBUFFERPROC pfnIsBuffer{nullptr};
	LOAD_ENTRYPOINT("glIsBuffer", pfnIsBuffer, PFNGLISBUFFERPROC);
	return pfnIsBuffer(buffer);
}

void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	using PFNGLBUFFERDATAPROC = void(APIENTRY *)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
	static PFNGLBUFFERDATAPROC pfnBufferData{nullptr};
	LOAD_ENTRYPOINT("glBufferData", pfnBufferData, PFNGLBUFFERDATAPROC);
	pfnBufferData(target, size, data, usage);
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
	using PFNGLBUFFERSUBDATAPROC = void(APIENTRY *)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
	static PFNGLBUFFERSUBDATAPROC pfnBufferSubData{nullptr};
	LOAD_ENTRYPOINT("glBufferSubData", pfnBufferSubData, PFNGLBUFFERSUBDATAPROC);
	pfnBufferSubData(target, offset, size, data);
}

void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data)
{
	using PFNGLGETBUFFERSUBDATAPROC = void(APIENTRY *)(GLenum target, GLintptr offset, GLsizeiptr size, void* data);
	static PFNGLGETBUFFERSUBDATAPROC pfnGetBufferSubData{nullptr};
	LOAD_ENTRYPOINT("glGetBufferSubData", pfnGetBufferSubData, PFNGLGETBUFFERSUBDATAPROC);
	pfnGetBufferSubData(target, offset, size, data);
}

void* glMapBuffer(GLenum target, GLenum access)
{
	using PFNGLMAPBUFFERPROC = void*(APIENTRY *)(GLenum target, GLenum access);
	static PFNGLMAPBUFFERPROC pfnMapBuffer{nullptr};
	LOAD_ENTRYPOINT("glMapBuffer", pfnMapBuffer, PFNGLMAPBUFFERPROC);
	return pfnMapBuffer(target, access);
}

GLboolean glUnmapBuffer(GLenum target)
{
	using PFNGLUNMAPBUFFERPROC = GLboolean(APIENTRY *)(GLenum target);
	static PFNGLUNMAPBUFFERPROC pfnUnmapBuffer{nullptr};
	LOAD_ENTRYPOINT("glUnmapBuffer", pfnUnmapBuffer, PFNGLUNMAPBUFFERPROC);
	return pfnUnmapBuffer(target);
}

void glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETBUFFERPARAMETERIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint* params);
	static PFNGLGETBUFFERPARAMETERIVPROC pfnGetBufferParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetBufferParameteriv", pfnGetBufferParameteriv, PFNGLGETBUFFERPARAMETERIVPROC);
	pfnGetBufferParameteriv(target, pname, params);
}

void glGetBufferPointerv(GLenum target, GLenum pname, void** params)
{
	using PFNGLGETBUFFERPOINTERVPROC = void(APIENTRY *)(GLenum target, GLenum pname, void** params);
	static PFNGLGETBUFFERPOINTERVPROC pfnGetBufferPointerv{nullptr};
	LOAD_ENTRYPOINT("glGetBufferPointerv", pfnGetBufferPointerv, PFNGLGETBUFFERPOINTERVPROC);
	pfnGetBufferPointerv(target, pname, params);
}

//
// GL_VERSION_2_0
//

void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
	using PFNGLBLENDEQUATIONSEPARATEPROC = void(APIENTRY *)(GLenum modeRGB, GLenum modeAlpha);
	static PFNGLBLENDEQUATIONSEPARATEPROC pfnBlendEquationSeparate{nullptr};
	LOAD_ENTRYPOINT("glBlendEquationSeparate", pfnBlendEquationSeparate, PFNGLBLENDEQUATIONSEPARATEPROC);
	pfnBlendEquationSeparate(modeRGB, modeAlpha);
}

void glDrawBuffers(GLsizei n, const GLenum* bufs)
{
	using PFNGLDRAWBUFFERSPROC = void(APIENTRY *)(GLsizei n, const GLenum* bufs);
	static PFNGLDRAWBUFFERSPROC pfnDrawBuffers{nullptr};
	LOAD_ENTRYPOINT("glDrawBuffers", pfnDrawBuffers, PFNGLDRAWBUFFERSPROC);
	pfnDrawBuffers(n, bufs);
}

void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
	using PFNGLSTENCILOPSEPARATEPROC = void(APIENTRY *)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
	static PFNGLSTENCILOPSEPARATEPROC pfnStencilOpSeparate{nullptr};
	LOAD_ENTRYPOINT("glStencilOpSeparate", pfnStencilOpSeparate, PFNGLSTENCILOPSEPARATEPROC);
	pfnStencilOpSeparate(face, sfail, dpfail, dppass);
}

void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
	using PFNGLSTENCILFUNCSEPARATEPROC = void(APIENTRY *)(GLenum face, GLenum func, GLint ref, GLuint mask);
	static PFNGLSTENCILFUNCSEPARATEPROC pfnStencilFuncSeparate{nullptr};
	LOAD_ENTRYPOINT("glStencilFuncSeparate", pfnStencilFuncSeparate, PFNGLSTENCILFUNCSEPARATEPROC);
	pfnStencilFuncSeparate(face, func, ref, mask);
}

void glStencilMaskSeparate(GLenum face, GLuint mask)
{
	using PFNGLSTENCILMASKSEPARATEPROC = void(APIENTRY *)(GLenum face, GLuint mask);
	static PFNGLSTENCILMASKSEPARATEPROC pfnStencilMaskSeparate{nullptr};
	LOAD_ENTRYPOINT("glStencilMaskSeparate", pfnStencilMaskSeparate, PFNGLSTENCILMASKSEPARATEPROC);
	pfnStencilMaskSeparate(face, mask);
}

void glAttachShader(GLuint program, GLuint shader)
{
	using PFNGLATTACHSHADERPROC = void(APIENTRY *)(GLuint program, GLuint shader);
	static PFNGLATTACHSHADERPROC pfnAttachShader{nullptr};
	LOAD_ENTRYPOINT("glAttachShader", pfnAttachShader, PFNGLATTACHSHADERPROC);
	pfnAttachShader(program, shader);
}

void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
	using PFNGLBINDATTRIBLOCATIONPROC = void(APIENTRY *)(GLuint program, GLuint index, const GLchar* name);
	static PFNGLBINDATTRIBLOCATIONPROC pfnBindAttribLocation{nullptr};
	LOAD_ENTRYPOINT("glBindAttribLocation", pfnBindAttribLocation, PFNGLBINDATTRIBLOCATIONPROC);
	pfnBindAttribLocation(program, index, name);
}

void glCompileShader(GLuint shader)
{
	using PFNGLCOMPILESHADERPROC = void(APIENTRY *)(GLuint shader);
	static PFNGLCOMPILESHADERPROC pfnCompileShader{nullptr};
	LOAD_ENTRYPOINT("glCompileShader", pfnCompileShader, PFNGLCOMPILESHADERPROC);
	pfnCompileShader(shader);
}

GLuint glCreateProgram(void)
{
	using PFNGLCREATEPROGRAMPROC = GLuint(APIENTRY *)(void);
	static PFNGLCREATEPROGRAMPROC pfnCreateProgram{nullptr};
	LOAD_ENTRYPOINT("glCreateProgram", pfnCreateProgram, PFNGLCREATEPROGRAMPROC);
	return pfnCreateProgram();
}

GLuint glCreateShader(GLenum type)
{
	using PFNGLCREATESHADERPROC = GLuint(APIENTRY *)(GLenum type);
	static PFNGLCREATESHADERPROC pfnCreateShader{nullptr};
	LOAD_ENTRYPOINT("glCreateShader", pfnCreateShader, PFNGLCREATESHADERPROC);
	return pfnCreateShader(type);
}

void glDeleteProgram(GLuint program)
{
	using PFNGLDELETEPROGRAMPROC = void(APIENTRY *)(GLuint program);
	static PFNGLDELETEPROGRAMPROC pfnDeleteProgram{nullptr};
	LOAD_ENTRYPOINT("glDeleteProgram", pfnDeleteProgram, PFNGLDELETEPROGRAMPROC);
	pfnDeleteProgram(program);
}

void glDeleteShader(GLuint shader)
{
	using PFNGLDELETESHADERPROC = void(APIENTRY *)(GLuint shader);
	static PFNGLDELETESHADERPROC pfnDeleteShader{nullptr};
	LOAD_ENTRYPOINT("glDeleteShader", pfnDeleteShader, PFNGLDELETESHADERPROC);
	pfnDeleteShader(shader);
}

void glDetachShader(GLuint program, GLuint shader)
{
	using PFNGLDETACHSHADERPROC = void(APIENTRY *)(GLuint program, GLuint shader);
	static PFNGLDETACHSHADERPROC pfnDetachShader{nullptr};
	LOAD_ENTRYPOINT("glDetachShader", pfnDetachShader, PFNGLDETACHSHADERPROC);
	pfnDetachShader(program, shader);
}

void glDisableVertexAttribArray(GLuint index)
{
	using PFNGLDISABLEVERTEXATTRIBARRAYPROC = void(APIENTRY *)(GLuint index);
	static PFNGLDISABLEVERTEXATTRIBARRAYPROC pfnDisableVertexAttribArray{nullptr};
	LOAD_ENTRYPOINT("glDisableVertexAttribArray", pfnDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC);
	pfnDisableVertexAttribArray(index);
}

void glEnableVertexAttribArray(GLuint index)
{
	using PFNGLENABLEVERTEXATTRIBARRAYPROC = void(APIENTRY *)(GLuint index);
	static PFNGLENABLEVERTEXATTRIBARRAYPROC pfnEnableVertexAttribArray{nullptr};
	LOAD_ENTRYPOINT("glEnableVertexAttribArray", pfnEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
	pfnEnableVertexAttribArray(index);
}

void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	using PFNGLGETACTIVEATTRIBPROC = void(APIENTRY *)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
	static PFNGLGETACTIVEATTRIBPROC pfnGetActiveAttrib{nullptr};
	LOAD_ENTRYPOINT("glGetActiveAttrib", pfnGetActiveAttrib, PFNGLGETACTIVEATTRIBPROC);
	pfnGetActiveAttrib(program, index, bufSize, length, size, type, name);
}

void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
	using PFNGLGETACTIVEUNIFORMPROC = void(APIENTRY *)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
	static PFNGLGETACTIVEUNIFORMPROC pfnGetActiveUniform{nullptr};
	LOAD_ENTRYPOINT("glGetActiveUniform", pfnGetActiveUniform, PFNGLGETACTIVEUNIFORMPROC);
	pfnGetActiveUniform(program, index, bufSize, length, size, type, name);
}

void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)
{
	using PFNGLGETATTACHEDSHADERSPROC = void(APIENTRY *)(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders);
	static PFNGLGETATTACHEDSHADERSPROC pfnGetAttachedShaders{nullptr};
	LOAD_ENTRYPOINT("glGetAttachedShaders", pfnGetAttachedShaders, PFNGLGETATTACHEDSHADERSPROC);
	pfnGetAttachedShaders(program, maxCount, count, shaders);
}

GLint glGetAttribLocation(GLuint program, const GLchar* name)
{
	using PFNGLGETATTRIBLOCATIONPROC = GLint(APIENTRY *)(GLuint program, const GLchar* name);
	static PFNGLGETATTRIBLOCATIONPROC pfnGetAttribLocation{nullptr};
	LOAD_ENTRYPOINT("glGetAttribLocation", pfnGetAttribLocation, PFNGLGETATTRIBLOCATIONPROC);
	return pfnGetAttribLocation(program, name);
}

void glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
	using PFNGLGETPROGRAMIVPROC = void(APIENTRY *)(GLuint program, GLenum pname, GLint* params);
	static PFNGLGETPROGRAMIVPROC pfnGetProgramiv{nullptr};
	LOAD_ENTRYPOINT("glGetProgramiv", pfnGetProgramiv, PFNGLGETPROGRAMIVPROC);
	pfnGetProgramiv(program, pname, params);
}

void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	using PFNGLGETPROGRAMINFOLOGPROC = void(APIENTRY *)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
	static PFNGLGETPROGRAMINFOLOGPROC pfnGetProgramInfoLog{nullptr};
	LOAD_ENTRYPOINT("glGetProgramInfoLog", pfnGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC);
	pfnGetProgramInfoLog(program, bufSize, length, infoLog);
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	using PFNGLGETSHADERIVPROC = void(APIENTRY *)(GLuint shader, GLenum pname, GLint* params);
	static PFNGLGETSHADERIVPROC pfnGetShaderiv{nullptr};
	LOAD_ENTRYPOINT("glGetShaderiv", pfnGetShaderiv, PFNGLGETSHADERIVPROC);
	pfnGetShaderiv(shader, pname, params);
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	using PFNGLGETSHADERINFOLOGPROC = void(APIENTRY *)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
	static PFNGLGETSHADERINFOLOGPROC pfnGetShaderInfoLog{nullptr};
	LOAD_ENTRYPOINT("glGetShaderInfoLog", pfnGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC);
	pfnGetShaderInfoLog(shader, bufSize, length, infoLog);
}

void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source)
{
	using PFNGLGETSHADERSOURCEPROC = void(APIENTRY *)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source);
	static PFNGLGETSHADERSOURCEPROC pfnGetShaderSource{nullptr};
	LOAD_ENTRYPOINT("glGetShaderSource", pfnGetShaderSource, PFNGLGETSHADERSOURCEPROC);
	pfnGetShaderSource(shader, bufSize, length, source);
}

GLint glGetUniformLocation(GLuint program, const GLchar* name)
{
	using PFNGLGETUNIFORMLOCATIONPROC = GLint(APIENTRY *)(GLuint program, const GLchar* name);
	static PFNGLGETUNIFORMLOCATIONPROC pfnGetUniformLocation{nullptr};
	LOAD_ENTRYPOINT("glGetUniformLocation", pfnGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
	return pfnGetUniformLocation(program, name);
}

void glGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
	using PFNGLGETUNIFORMFVPROC = void(APIENTRY *)(GLuint program, GLint location, GLfloat* params);
	static PFNGLGETUNIFORMFVPROC pfnGetUniformfv{nullptr};
	LOAD_ENTRYPOINT("glGetUniformfv", pfnGetUniformfv, PFNGLGETUNIFORMFVPROC);
	pfnGetUniformfv(program, location, params);
}

void glGetUniformiv(GLuint program, GLint location, GLint* params)
{
	using PFNGLGETUNIFORMIVPROC = void(APIENTRY *)(GLuint program, GLint location, GLint* params);
	static PFNGLGETUNIFORMIVPROC pfnGetUniformiv{nullptr};
	LOAD_ENTRYPOINT("glGetUniformiv", pfnGetUniformiv, PFNGLGETUNIFORMIVPROC);
	pfnGetUniformiv(program, location, params);
}

void glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble* params)
{
	using PFNGLGETVERTEXATTRIBDVPROC = void(APIENTRY *)(GLuint index, GLenum pname, GLdouble* params);
	static PFNGLGETVERTEXATTRIBDVPROC pfnGetVertexAttribdv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribdv", pfnGetVertexAttribdv, PFNGLGETVERTEXATTRIBDVPROC);
	pfnGetVertexAttribdv(index, pname, params);
}

void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
	using PFNGLGETVERTEXATTRIBFVPROC = void(APIENTRY *)(GLuint index, GLenum pname, GLfloat* params);
	static PFNGLGETVERTEXATTRIBFVPROC pfnGetVertexAttribfv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribfv", pfnGetVertexAttribfv, PFNGLGETVERTEXATTRIBFVPROC);
	pfnGetVertexAttribfv(index, pname, params);
}

void glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
	using PFNGLGETVERTEXATTRIBIVPROC = void(APIENTRY *)(GLuint index, GLenum pname, GLint* params);
	static PFNGLGETVERTEXATTRIBIVPROC pfnGetVertexAttribiv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribiv", pfnGetVertexAttribiv, PFNGLGETVERTEXATTRIBIVPROC);
	pfnGetVertexAttribiv(index, pname, params);
}

void glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
	using PFNGLGETVERTEXATTRIBPOINTERVPROC = void(APIENTRY *)(GLuint index, GLenum pname, void** pointer);
	static PFNGLGETVERTEXATTRIBPOINTERVPROC pfnGetVertexAttribPointerv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribPointerv", pfnGetVertexAttribPointerv, PFNGLGETVERTEXATTRIBPOINTERVPROC);
	pfnGetVertexAttribPointerv(index, pname, pointer);
}

GLboolean glIsProgram(GLuint program)
{
	using PFNGLISPROGRAMPROC = GLboolean(APIENTRY *)(GLuint program);
	static PFNGLISPROGRAMPROC pfnIsProgram{nullptr};
	LOAD_ENTRYPOINT("glIsProgram", pfnIsProgram, PFNGLISPROGRAMPROC);
	return pfnIsProgram(program);
}

GLboolean glIsShader(GLuint shader)
{
	using PFNGLISSHADERPROC = GLboolean(APIENTRY *)(GLuint shader);
	static PFNGLISSHADERPROC pfnIsShader{nullptr};
	LOAD_ENTRYPOINT("glIsShader", pfnIsShader, PFNGLISSHADERPROC);
	return pfnIsShader(shader);
}

void glLinkProgram(GLuint program)
{
	using PFNGLLINKPROGRAMPROC = void(APIENTRY *)(GLuint program);
	static PFNGLLINKPROGRAMPROC pfnLinkProgram{nullptr};
	LOAD_ENTRYPOINT("glLinkProgram", pfnLinkProgram, PFNGLLINKPROGRAMPROC);
	pfnLinkProgram(program);
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
	using PFNGLSHADERSOURCEPROC = void(APIENTRY *)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
	static PFNGLSHADERSOURCEPROC pfnShaderSource{nullptr};
	LOAD_ENTRYPOINT("glShaderSource", pfnShaderSource, PFNGLSHADERSOURCEPROC);
	pfnShaderSource(shader, count, string, length);
}

void glUseProgram(GLuint program)
{
	using PFNGLUSEPROGRAMPROC = void(APIENTRY *)(GLuint program);
	static PFNGLUSEPROGRAMPROC pfnUseProgram{nullptr};
	LOAD_ENTRYPOINT("glUseProgram", pfnUseProgram, PFNGLUSEPROGRAMPROC);
	pfnUseProgram(program);
}

void glUniform1f(GLint location, GLfloat v0)
{
	using PFNGLUNIFORM1FPROC = void(APIENTRY *)(GLint location, GLfloat v0);
	static PFNGLUNIFORM1FPROC pfnUniform1f{nullptr};
	LOAD_ENTRYPOINT("glUniform1f", pfnUniform1f, PFNGLUNIFORM1FPROC);
	pfnUniform1f(location, v0);
}

void glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
	using PFNGLUNIFORM2FPROC = void(APIENTRY *)(GLint location, GLfloat v0, GLfloat v1);
	static PFNGLUNIFORM2FPROC pfnUniform2f{nullptr};
	LOAD_ENTRYPOINT("glUniform2f", pfnUniform2f, PFNGLUNIFORM2FPROC);
	pfnUniform2f(location, v0, v1);
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	using PFNGLUNIFORM3FPROC = void(APIENTRY *)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	static PFNGLUNIFORM3FPROC pfnUniform3f{nullptr};
	LOAD_ENTRYPOINT("glUniform3f", pfnUniform3f, PFNGLUNIFORM3FPROC);
	pfnUniform3f(location, v0, v1, v2);
}

void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	using PFNGLUNIFORM4FPROC = void(APIENTRY *)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	static PFNGLUNIFORM4FPROC pfnUniform4f{nullptr};
	LOAD_ENTRYPOINT("glUniform4f", pfnUniform4f, PFNGLUNIFORM4FPROC);
	pfnUniform4f(location, v0, v1, v2, v3);
}

void glUniform1i(GLint location, GLint v0)
{
	using PFNGLUNIFORM1IPROC = void(APIENTRY *)(GLint location, GLint v0);
	static PFNGLUNIFORM1IPROC pfnUniform1i{nullptr};
	LOAD_ENTRYPOINT("glUniform1i", pfnUniform1i, PFNGLUNIFORM1IPROC);
	pfnUniform1i(location, v0);
}

void glUniform2i(GLint location, GLint v0, GLint v1)
{
	using PFNGLUNIFORM2IPROC = void(APIENTRY *)(GLint location, GLint v0, GLint v1);
	static PFNGLUNIFORM2IPROC pfnUniform2i{nullptr};
	LOAD_ENTRYPOINT("glUniform2i", pfnUniform2i, PFNGLUNIFORM2IPROC);
	pfnUniform2i(location, v0, v1);
}

void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
	using PFNGLUNIFORM3IPROC = void(APIENTRY *)(GLint location, GLint v0, GLint v1, GLint v2);
	static PFNGLUNIFORM3IPROC pfnUniform3i{nullptr};
	LOAD_ENTRYPOINT("glUniform3i", pfnUniform3i, PFNGLUNIFORM3IPROC);
	pfnUniform3i(location, v0, v1, v2);
}

void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	using PFNGLUNIFORM4IPROC = void(APIENTRY *)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	static PFNGLUNIFORM4IPROC pfnUniform4i{nullptr};
	LOAD_ENTRYPOINT("glUniform4i", pfnUniform4i, PFNGLUNIFORM4IPROC);
	pfnUniform4i(location, v0, v1, v2, v3);
}

void glUniform1fv(GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLUNIFORM1FVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLfloat* value);
	static PFNGLUNIFORM1FVPROC pfnUniform1fv{nullptr};
	LOAD_ENTRYPOINT("glUniform1fv", pfnUniform1fv, PFNGLUNIFORM1FVPROC);
	pfnUniform1fv(location, count, value);
}

void glUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLUNIFORM2FVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLfloat* value);
	static PFNGLUNIFORM2FVPROC pfnUniform2fv{nullptr};
	LOAD_ENTRYPOINT("glUniform2fv", pfnUniform2fv, PFNGLUNIFORM2FVPROC);
	pfnUniform2fv(location, count, value);
}

void glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLUNIFORM3FVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLfloat* value);
	static PFNGLUNIFORM3FVPROC pfnUniform3fv{nullptr};
	LOAD_ENTRYPOINT("glUniform3fv", pfnUniform3fv, PFNGLUNIFORM3FVPROC);
	pfnUniform3fv(location, count, value);
}

void glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLUNIFORM4FVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLfloat* value);
	static PFNGLUNIFORM4FVPROC pfnUniform4fv{nullptr};
	LOAD_ENTRYPOINT("glUniform4fv", pfnUniform4fv, PFNGLUNIFORM4FVPROC);
	pfnUniform4fv(location, count, value);
}

void glUniform1iv(GLint location, GLsizei count, const GLint* value)
{
	using PFNGLUNIFORM1IVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint* value);
	static PFNGLUNIFORM1IVPROC pfnUniform1iv{nullptr};
	LOAD_ENTRYPOINT("glUniform1iv", pfnUniform1iv, PFNGLUNIFORM1IVPROC);
	pfnUniform1iv(location, count, value);
}

void glUniform2iv(GLint location, GLsizei count, const GLint* value)
{
	using PFNGLUNIFORM2IVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint* value);
	static PFNGLUNIFORM2IVPROC pfnUniform2iv{nullptr};
	LOAD_ENTRYPOINT("glUniform2iv", pfnUniform2iv, PFNGLUNIFORM2IVPROC);
	pfnUniform2iv(location, count, value);
}

void glUniform3iv(GLint location, GLsizei count, const GLint* value)
{
	using PFNGLUNIFORM3IVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint* value);
	static PFNGLUNIFORM3IVPROC pfnUniform3iv{nullptr};
	LOAD_ENTRYPOINT("glUniform3iv", pfnUniform3iv, PFNGLUNIFORM3IVPROC);
	pfnUniform3iv(location, count, value);
}

void glUniform4iv(GLint location, GLsizei count, const GLint* value)
{
	using PFNGLUNIFORM4IVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint* value);
	static PFNGLUNIFORM4IVPROC pfnUniform4iv{nullptr};
	LOAD_ENTRYPOINT("glUniform4iv", pfnUniform4iv, PFNGLUNIFORM4IVPROC);
	pfnUniform4iv(location, count, value);
}

void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLUNIFORMMATRIX2FVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLUNIFORMMATRIX2FVPROC pfnUniformMatrix2fv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix2fv", pfnUniformMatrix2fv, PFNGLUNIFORMMATRIX2FVPROC);
	pfnUniformMatrix2fv(location, count, transpose, value);
}

void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLUNIFORMMATRIX3FVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLUNIFORMMATRIX3FVPROC pfnUniformMatrix3fv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix3fv", pfnUniformMatrix3fv, PFNGLUNIFORMMATRIX3FVPROC);
	pfnUniformMatrix3fv(location, count, transpose, value);
}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLUNIFORMMATRIX4FVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLUNIFORMMATRIX4FVPROC pfnUniformMatrix4fv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix4fv", pfnUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC);
	pfnUniformMatrix4fv(location, count, transpose, value);
}

void glValidateProgram(GLuint program)
{
	using PFNGLVALIDATEPROGRAMPROC = void(APIENTRY *)(GLuint program);
	static PFNGLVALIDATEPROGRAMPROC pfnValidateProgram{nullptr};
	LOAD_ENTRYPOINT("glValidateProgram", pfnValidateProgram, PFNGLVALIDATEPROGRAMPROC);
	pfnValidateProgram(program);
}

void glVertexAttrib1d(GLuint index, GLdouble x)
{
	using PFNGLVERTEXATTRIB1DPROC = void(APIENTRY *)(GLuint index, GLdouble x);
	static PFNGLVERTEXATTRIB1DPROC pfnVertexAttrib1d{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib1d", pfnVertexAttrib1d, PFNGLVERTEXATTRIB1DPROC);
	pfnVertexAttrib1d(index, x);
}

void glVertexAttrib1dv(GLuint index, const GLdouble* v)
{
	using PFNGLVERTEXATTRIB1DVPROC = void(APIENTRY *)(GLuint index, const GLdouble* v);
	static PFNGLVERTEXATTRIB1DVPROC pfnVertexAttrib1dv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib1dv", pfnVertexAttrib1dv, PFNGLVERTEXATTRIB1DVPROC);
	pfnVertexAttrib1dv(index, v);
}

void glVertexAttrib1f(GLuint index, GLfloat x)
{
	using PFNGLVERTEXATTRIB1FPROC = void(APIENTRY *)(GLuint index, GLfloat x);
	static PFNGLVERTEXATTRIB1FPROC pfnVertexAttrib1f{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib1f", pfnVertexAttrib1f, PFNGLVERTEXATTRIB1FPROC);
	pfnVertexAttrib1f(index, x);
}

void glVertexAttrib1fv(GLuint index, const GLfloat* v)
{
	using PFNGLVERTEXATTRIB1FVPROC = void(APIENTRY *)(GLuint index, const GLfloat* v);
	static PFNGLVERTEXATTRIB1FVPROC pfnVertexAttrib1fv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib1fv", pfnVertexAttrib1fv, PFNGLVERTEXATTRIB1FVPROC);
	pfnVertexAttrib1fv(index, v);
}

void glVertexAttrib1s(GLuint index, GLshort x)
{
	using PFNGLVERTEXATTRIB1SPROC = void(APIENTRY *)(GLuint index, GLshort x);
	static PFNGLVERTEXATTRIB1SPROC pfnVertexAttrib1s{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib1s", pfnVertexAttrib1s, PFNGLVERTEXATTRIB1SPROC);
	pfnVertexAttrib1s(index, x);
}

void glVertexAttrib1sv(GLuint index, const GLshort* v)
{
	using PFNGLVERTEXATTRIB1SVPROC = void(APIENTRY *)(GLuint index, const GLshort* v);
	static PFNGLVERTEXATTRIB1SVPROC pfnVertexAttrib1sv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib1sv", pfnVertexAttrib1sv, PFNGLVERTEXATTRIB1SVPROC);
	pfnVertexAttrib1sv(index, v);
}

void glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
	using PFNGLVERTEXATTRIB2DPROC = void(APIENTRY *)(GLuint index, GLdouble x, GLdouble y);
	static PFNGLVERTEXATTRIB2DPROC pfnVertexAttrib2d{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib2d", pfnVertexAttrib2d, PFNGLVERTEXATTRIB2DPROC);
	pfnVertexAttrib2d(index, x, y);
}

void glVertexAttrib2dv(GLuint index, const GLdouble* v)
{
	using PFNGLVERTEXATTRIB2DVPROC = void(APIENTRY *)(GLuint index, const GLdouble* v);
	static PFNGLVERTEXATTRIB2DVPROC pfnVertexAttrib2dv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib2dv", pfnVertexAttrib2dv, PFNGLVERTEXATTRIB2DVPROC);
	pfnVertexAttrib2dv(index, v);
}

void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
	using PFNGLVERTEXATTRIB2FPROC = void(APIENTRY *)(GLuint index, GLfloat x, GLfloat y);
	static PFNGLVERTEXATTRIB2FPROC pfnVertexAttrib2f{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib2f", pfnVertexAttrib2f, PFNGLVERTEXATTRIB2FPROC);
	pfnVertexAttrib2f(index, x, y);
}

void glVertexAttrib2fv(GLuint index, const GLfloat* v)
{
	using PFNGLVERTEXATTRIB2FVPROC = void(APIENTRY *)(GLuint index, const GLfloat* v);
	static PFNGLVERTEXATTRIB2FVPROC pfnVertexAttrib2fv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib2fv", pfnVertexAttrib2fv, PFNGLVERTEXATTRIB2FVPROC);
	pfnVertexAttrib2fv(index, v);
}

void glVertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
	using PFNGLVERTEXATTRIB2SPROC = void(APIENTRY *)(GLuint index, GLshort x, GLshort y);
	static PFNGLVERTEXATTRIB2SPROC pfnVertexAttrib2s{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib2s", pfnVertexAttrib2s, PFNGLVERTEXATTRIB2SPROC);
	pfnVertexAttrib2s(index, x, y);
}

void glVertexAttrib2sv(GLuint index, const GLshort* v)
{
	using PFNGLVERTEXATTRIB2SVPROC = void(APIENTRY *)(GLuint index, const GLshort* v);
	static PFNGLVERTEXATTRIB2SVPROC pfnVertexAttrib2sv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib2sv", pfnVertexAttrib2sv, PFNGLVERTEXATTRIB2SVPROC);
	pfnVertexAttrib2sv(index, v);
}

void glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	using PFNGLVERTEXATTRIB3DPROC = void(APIENTRY *)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLVERTEXATTRIB3DPROC pfnVertexAttrib3d{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib3d", pfnVertexAttrib3d, PFNGLVERTEXATTRIB3DPROC);
	pfnVertexAttrib3d(index, x, y, z);
}

void glVertexAttrib3dv(GLuint index, const GLdouble* v)
{
	using PFNGLVERTEXATTRIB3DVPROC = void(APIENTRY *)(GLuint index, const GLdouble* v);
	static PFNGLVERTEXATTRIB3DVPROC pfnVertexAttrib3dv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib3dv", pfnVertexAttrib3dv, PFNGLVERTEXATTRIB3DVPROC);
	pfnVertexAttrib3dv(index, v);
}

void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
	using PFNGLVERTEXATTRIB3FPROC = void(APIENTRY *)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
	static PFNGLVERTEXATTRIB3FPROC pfnVertexAttrib3f{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib3f", pfnVertexAttrib3f, PFNGLVERTEXATTRIB3FPROC);
	pfnVertexAttrib3f(index, x, y, z);
}

void glVertexAttrib3fv(GLuint index, const GLfloat* v)
{
	using PFNGLVERTEXATTRIB3FVPROC = void(APIENTRY *)(GLuint index, const GLfloat* v);
	static PFNGLVERTEXATTRIB3FVPROC pfnVertexAttrib3fv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib3fv", pfnVertexAttrib3fv, PFNGLVERTEXATTRIB3FVPROC);
	pfnVertexAttrib3fv(index, v);
}

void glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
	using PFNGLVERTEXATTRIB3SPROC = void(APIENTRY *)(GLuint index, GLshort x, GLshort y, GLshort z);
	static PFNGLVERTEXATTRIB3SPROC pfnVertexAttrib3s{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib3s", pfnVertexAttrib3s, PFNGLVERTEXATTRIB3SPROC);
	pfnVertexAttrib3s(index, x, y, z);
}

void glVertexAttrib3sv(GLuint index, const GLshort* v)
{
	using PFNGLVERTEXATTRIB3SVPROC = void(APIENTRY *)(GLuint index, const GLshort* v);
	static PFNGLVERTEXATTRIB3SVPROC pfnVertexAttrib3sv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib3sv", pfnVertexAttrib3sv, PFNGLVERTEXATTRIB3SVPROC);
	pfnVertexAttrib3sv(index, v);
}

void glVertexAttrib4Nbv(GLuint index, const GLbyte* v)
{
	using PFNGLVERTEXATTRIB4NBVPROC = void(APIENTRY *)(GLuint index, const GLbyte* v);
	static PFNGLVERTEXATTRIB4NBVPROC pfnVertexAttrib4Nbv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4Nbv", pfnVertexAttrib4Nbv, PFNGLVERTEXATTRIB4NBVPROC);
	pfnVertexAttrib4Nbv(index, v);
}

void glVertexAttrib4Niv(GLuint index, const GLint* v)
{
	using PFNGLVERTEXATTRIB4NIVPROC = void(APIENTRY *)(GLuint index, const GLint* v);
	static PFNGLVERTEXATTRIB4NIVPROC pfnVertexAttrib4Niv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4Niv", pfnVertexAttrib4Niv, PFNGLVERTEXATTRIB4NIVPROC);
	pfnVertexAttrib4Niv(index, v);
}

void glVertexAttrib4Nsv(GLuint index, const GLshort* v)
{
	using PFNGLVERTEXATTRIB4NSVPROC = void(APIENTRY *)(GLuint index, const GLshort* v);
	static PFNGLVERTEXATTRIB4NSVPROC pfnVertexAttrib4Nsv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4Nsv", pfnVertexAttrib4Nsv, PFNGLVERTEXATTRIB4NSVPROC);
	pfnVertexAttrib4Nsv(index, v);
}

void glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
	using PFNGLVERTEXATTRIB4NUBPROC = void(APIENTRY *)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
	static PFNGLVERTEXATTRIB4NUBPROC pfnVertexAttrib4Nub{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4Nub", pfnVertexAttrib4Nub, PFNGLVERTEXATTRIB4NUBPROC);
	pfnVertexAttrib4Nub(index, x, y, z, w);
}

void glVertexAttrib4Nubv(GLuint index, const GLubyte* v)
{
	using PFNGLVERTEXATTRIB4NUBVPROC = void(APIENTRY *)(GLuint index, const GLubyte* v);
	static PFNGLVERTEXATTRIB4NUBVPROC pfnVertexAttrib4Nubv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4Nubv", pfnVertexAttrib4Nubv, PFNGLVERTEXATTRIB4NUBVPROC);
	pfnVertexAttrib4Nubv(index, v);
}

void glVertexAttrib4Nuiv(GLuint index, const GLuint* v)
{
	using PFNGLVERTEXATTRIB4NUIVPROC = void(APIENTRY *)(GLuint index, const GLuint* v);
	static PFNGLVERTEXATTRIB4NUIVPROC pfnVertexAttrib4Nuiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4Nuiv", pfnVertexAttrib4Nuiv, PFNGLVERTEXATTRIB4NUIVPROC);
	pfnVertexAttrib4Nuiv(index, v);
}

void glVertexAttrib4Nusv(GLuint index, const GLushort* v)
{
	using PFNGLVERTEXATTRIB4NUSVPROC = void(APIENTRY *)(GLuint index, const GLushort* v);
	static PFNGLVERTEXATTRIB4NUSVPROC pfnVertexAttrib4Nusv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4Nusv", pfnVertexAttrib4Nusv, PFNGLVERTEXATTRIB4NUSVPROC);
	pfnVertexAttrib4Nusv(index, v);
}

void glVertexAttrib4bv(GLuint index, const GLbyte* v)
{
	using PFNGLVERTEXATTRIB4BVPROC = void(APIENTRY *)(GLuint index, const GLbyte* v);
	static PFNGLVERTEXATTRIB4BVPROC pfnVertexAttrib4bv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4bv", pfnVertexAttrib4bv, PFNGLVERTEXATTRIB4BVPROC);
	pfnVertexAttrib4bv(index, v);
}

void glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	using PFNGLVERTEXATTRIB4DPROC = void(APIENTRY *)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static PFNGLVERTEXATTRIB4DPROC pfnVertexAttrib4d{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4d", pfnVertexAttrib4d, PFNGLVERTEXATTRIB4DPROC);
	pfnVertexAttrib4d(index, x, y, z, w);
}

void glVertexAttrib4dv(GLuint index, const GLdouble* v)
{
	using PFNGLVERTEXATTRIB4DVPROC = void(APIENTRY *)(GLuint index, const GLdouble* v);
	static PFNGLVERTEXATTRIB4DVPROC pfnVertexAttrib4dv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4dv", pfnVertexAttrib4dv, PFNGLVERTEXATTRIB4DVPROC);
	pfnVertexAttrib4dv(index, v);
}

void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	using PFNGLVERTEXATTRIB4FPROC = void(APIENTRY *)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	static PFNGLVERTEXATTRIB4FPROC pfnVertexAttrib4f{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4f", pfnVertexAttrib4f, PFNGLVERTEXATTRIB4FPROC);
	pfnVertexAttrib4f(index, x, y, z, w);
}

void glVertexAttrib4fv(GLuint index, const GLfloat* v)
{
	using PFNGLVERTEXATTRIB4FVPROC = void(APIENTRY *)(GLuint index, const GLfloat* v);
	static PFNGLVERTEXATTRIB4FVPROC pfnVertexAttrib4fv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4fv", pfnVertexAttrib4fv, PFNGLVERTEXATTRIB4FVPROC);
	pfnVertexAttrib4fv(index, v);
}

void glVertexAttrib4iv(GLuint index, const GLint* v)
{
	using PFNGLVERTEXATTRIB4IVPROC = void(APIENTRY *)(GLuint index, const GLint* v);
	static PFNGLVERTEXATTRIB4IVPROC pfnVertexAttrib4iv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4iv", pfnVertexAttrib4iv, PFNGLVERTEXATTRIB4IVPROC);
	pfnVertexAttrib4iv(index, v);
}

void glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
	using PFNGLVERTEXATTRIB4SPROC = void(APIENTRY *)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
	static PFNGLVERTEXATTRIB4SPROC pfnVertexAttrib4s{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4s", pfnVertexAttrib4s, PFNGLVERTEXATTRIB4SPROC);
	pfnVertexAttrib4s(index, x, y, z, w);
}

void glVertexAttrib4sv(GLuint index, const GLshort* v)
{
	using PFNGLVERTEXATTRIB4SVPROC = void(APIENTRY *)(GLuint index, const GLshort* v);
	static PFNGLVERTEXATTRIB4SVPROC pfnVertexAttrib4sv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4sv", pfnVertexAttrib4sv, PFNGLVERTEXATTRIB4SVPROC);
	pfnVertexAttrib4sv(index, v);
}

void glVertexAttrib4ubv(GLuint index, const GLubyte* v)
{
	using PFNGLVERTEXATTRIB4UBVPROC = void(APIENTRY *)(GLuint index, const GLubyte* v);
	static PFNGLVERTEXATTRIB4UBVPROC pfnVertexAttrib4ubv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4ubv", pfnVertexAttrib4ubv, PFNGLVERTEXATTRIB4UBVPROC);
	pfnVertexAttrib4ubv(index, v);
}

void glVertexAttrib4uiv(GLuint index, const GLuint* v)
{
	using PFNGLVERTEXATTRIB4UIVPROC = void(APIENTRY *)(GLuint index, const GLuint* v);
	static PFNGLVERTEXATTRIB4UIVPROC pfnVertexAttrib4uiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4uiv", pfnVertexAttrib4uiv, PFNGLVERTEXATTRIB4UIVPROC);
	pfnVertexAttrib4uiv(index, v);
}

void glVertexAttrib4usv(GLuint index, const GLushort* v)
{
	using PFNGLVERTEXATTRIB4USVPROC = void(APIENTRY *)(GLuint index, const GLushort* v);
	static PFNGLVERTEXATTRIB4USVPROC pfnVertexAttrib4usv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttrib4usv", pfnVertexAttrib4usv, PFNGLVERTEXATTRIB4USVPROC);
	pfnVertexAttrib4usv(index, v);
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
	using PFNGLVERTEXATTRIBPOINTERPROC = void(APIENTRY *)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
	static PFNGLVERTEXATTRIBPOINTERPROC pfnVertexAttribPointer{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribPointer", pfnVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
	pfnVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

//
// GL_VERSION_2_1
//

void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLUNIFORMMATRIX2X3FVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLUNIFORMMATRIX2X3FVPROC pfnUniformMatrix2x3fv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix2x3fv", pfnUniformMatrix2x3fv, PFNGLUNIFORMMATRIX2X3FVPROC);
	pfnUniformMatrix2x3fv(location, count, transpose, value);
}

void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLUNIFORMMATRIX3X2FVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLUNIFORMMATRIX3X2FVPROC pfnUniformMatrix3x2fv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix3x2fv", pfnUniformMatrix3x2fv, PFNGLUNIFORMMATRIX3X2FVPROC);
	pfnUniformMatrix3x2fv(location, count, transpose, value);
}

void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLUNIFORMMATRIX2X4FVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLUNIFORMMATRIX2X4FVPROC pfnUniformMatrix2x4fv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix2x4fv", pfnUniformMatrix2x4fv, PFNGLUNIFORMMATRIX2X4FVPROC);
	pfnUniformMatrix2x4fv(location, count, transpose, value);
}

void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLUNIFORMMATRIX4X2FVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLUNIFORMMATRIX4X2FVPROC pfnUniformMatrix4x2fv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix4x2fv", pfnUniformMatrix4x2fv, PFNGLUNIFORMMATRIX4X2FVPROC);
	pfnUniformMatrix4x2fv(location, count, transpose, value);
}

void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLUNIFORMMATRIX3X4FVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLUNIFORMMATRIX3X4FVPROC pfnUniformMatrix3x4fv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix3x4fv", pfnUniformMatrix3x4fv, PFNGLUNIFORMMATRIX3X4FVPROC);
	pfnUniformMatrix3x4fv(location, count, transpose, value);
}

void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLUNIFORMMATRIX4X3FVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLUNIFORMMATRIX4X3FVPROC pfnUniformMatrix4x3fv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix4x3fv", pfnUniformMatrix4x3fv, PFNGLUNIFORMMATRIX4X3FVPROC);
	pfnUniformMatrix4x3fv(location, count, transpose, value);
}

//
// GL_VERSION_3_0
//

void glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
	using PFNGLCOLORMASKIPROC = void(APIENTRY *)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
	static PFNGLCOLORMASKIPROC pfnColorMaski{nullptr};
	LOAD_ENTRYPOINT("glColorMaski", pfnColorMaski, PFNGLCOLORMASKIPROC);
	pfnColorMaski(index, r, g, b, a);
}

void glGetBooleani_v(GLenum target, GLuint index, GLboolean* data)
{
	using PFNGLGETBOOLEANI_VPROC = void(APIENTRY *)(GLenum target, GLuint index, GLboolean* data);
	static PFNGLGETBOOLEANI_VPROC pfnGetBooleani_v{nullptr};
	LOAD_ENTRYPOINT("glGetBooleani_v", pfnGetBooleani_v, PFNGLGETBOOLEANI_VPROC);
	pfnGetBooleani_v(target, index, data);
}

void glGetIntegeri_v(GLenum target, GLuint index, GLint* data)
{
	using PFNGLGETINTEGERI_VPROC = void(APIENTRY *)(GLenum target, GLuint index, GLint* data);
	static PFNGLGETINTEGERI_VPROC pfnGetIntegeri_v{nullptr};
	LOAD_ENTRYPOINT("glGetIntegeri_v", pfnGetIntegeri_v, PFNGLGETINTEGERI_VPROC);
	pfnGetIntegeri_v(target, index, data);
}

void glEnablei(GLenum target, GLuint index)
{
	using PFNGLENABLEIPROC = void(APIENTRY *)(GLenum target, GLuint index);
	static PFNGLENABLEIPROC pfnEnablei{nullptr};
	LOAD_ENTRYPOINT("glEnablei", pfnEnablei, PFNGLENABLEIPROC);
	pfnEnablei(target, index);
}

void glDisablei(GLenum target, GLuint index)
{
	using PFNGLDISABLEIPROC = void(APIENTRY *)(GLenum target, GLuint index);
	static PFNGLDISABLEIPROC pfnDisablei{nullptr};
	LOAD_ENTRYPOINT("glDisablei", pfnDisablei, PFNGLDISABLEIPROC);
	pfnDisablei(target, index);
}

GLboolean glIsEnabledi(GLenum target, GLuint index)
{
	using PFNGLISENABLEDIPROC = GLboolean(APIENTRY *)(GLenum target, GLuint index);
	static PFNGLISENABLEDIPROC pfnIsEnabledi{nullptr};
	LOAD_ENTRYPOINT("glIsEnabledi", pfnIsEnabledi, PFNGLISENABLEDIPROC);
	return pfnIsEnabledi(target, index);
}

void glBeginTransformFeedback(GLenum primitiveMode)
{
	using PFNGLBEGINTRANSFORMFEEDBACKPROC = void(APIENTRY *)(GLenum primitiveMode);
	static PFNGLBEGINTRANSFORMFEEDBACKPROC pfnBeginTransformFeedback{nullptr};
	LOAD_ENTRYPOINT("glBeginTransformFeedback", pfnBeginTransformFeedback, PFNGLBEGINTRANSFORMFEEDBACKPROC);
	pfnBeginTransformFeedback(primitiveMode);
}

void glEndTransformFeedback(void)
{
	using PFNGLENDTRANSFORMFEEDBACKPROC = void(APIENTRY *)(void);
	static PFNGLENDTRANSFORMFEEDBACKPROC pfnEndTransformFeedback{nullptr};
	LOAD_ENTRYPOINT("glEndTransformFeedback", pfnEndTransformFeedback, PFNGLENDTRANSFORMFEEDBACKPROC);
	pfnEndTransformFeedback();
}

void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	using PFNGLBINDBUFFERRANGEPROC = void(APIENTRY *)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static PFNGLBINDBUFFERRANGEPROC pfnBindBufferRange{nullptr};
	LOAD_ENTRYPOINT("glBindBufferRange", pfnBindBufferRange, PFNGLBINDBUFFERRANGEPROC);
	pfnBindBufferRange(target, index, buffer, offset, size);
}

void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	using PFNGLBINDBUFFERBASEPROC = void(APIENTRY *)(GLenum target, GLuint index, GLuint buffer);
	static PFNGLBINDBUFFERBASEPROC pfnBindBufferBase{nullptr};
	LOAD_ENTRYPOINT("glBindBufferBase", pfnBindBufferBase, PFNGLBINDBUFFERBASEPROC);
	pfnBindBufferBase(target, index, buffer);
}

void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
	using PFNGLTRANSFORMFEEDBACKVARYINGSPROC = void(APIENTRY *)(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
	static PFNGLTRANSFORMFEEDBACKVARYINGSPROC pfnTransformFeedbackVaryings{nullptr};
	LOAD_ENTRYPOINT("glTransformFeedbackVaryings", pfnTransformFeedbackVaryings, PFNGLTRANSFORMFEEDBACKVARYINGSPROC);
	pfnTransformFeedbackVaryings(program, count, varyings, bufferMode);
}

void glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
	using PFNGLGETTRANSFORMFEEDBACKVARYINGPROC = void(APIENTRY *)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
	static PFNGLGETTRANSFORMFEEDBACKVARYINGPROC pfnGetTransformFeedbackVarying{nullptr};
	LOAD_ENTRYPOINT("glGetTransformFeedbackVarying", pfnGetTransformFeedbackVarying, PFNGLGETTRANSFORMFEEDBACKVARYINGPROC);
	pfnGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

void glClampColor(GLenum target, GLenum clamp)
{
	using PFNGLCLAMPCOLORPROC = void(APIENTRY *)(GLenum target, GLenum clamp);
	static PFNGLCLAMPCOLORPROC pfnClampColor{nullptr};
	LOAD_ENTRYPOINT("glClampColor", pfnClampColor, PFNGLCLAMPCOLORPROC);
	pfnClampColor(target, clamp);
}

void glBeginConditionalRender(GLuint id, GLenum mode)
{
	using PFNGLBEGINCONDITIONALRENDERPROC = void(APIENTRY *)(GLuint id, GLenum mode);
	static PFNGLBEGINCONDITIONALRENDERPROC pfnBeginConditionalRender{nullptr};
	LOAD_ENTRYPOINT("glBeginConditionalRender", pfnBeginConditionalRender, PFNGLBEGINCONDITIONALRENDERPROC);
	pfnBeginConditionalRender(id, mode);
}

void glEndConditionalRender(void)
{
	using PFNGLENDCONDITIONALRENDERPROC = void(APIENTRY *)(void);
	static PFNGLENDCONDITIONALRENDERPROC pfnEndConditionalRender{nullptr};
	LOAD_ENTRYPOINT("glEndConditionalRender", pfnEndConditionalRender, PFNGLENDCONDITIONALRENDERPROC);
	pfnEndConditionalRender();
}

void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	using PFNGLVERTEXATTRIBIPOINTERPROC = void(APIENTRY *)(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
	static PFNGLVERTEXATTRIBIPOINTERPROC pfnVertexAttribIPointer{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribIPointer", pfnVertexAttribIPointer, PFNGLVERTEXATTRIBIPOINTERPROC);
	pfnVertexAttribIPointer(index, size, type, stride, pointer);
}

void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
	using PFNGLGETVERTEXATTRIBIIVPROC = void(APIENTRY *)(GLuint index, GLenum pname, GLint* params);
	static PFNGLGETVERTEXATTRIBIIVPROC pfnGetVertexAttribIiv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribIiv", pfnGetVertexAttribIiv, PFNGLGETVERTEXATTRIBIIVPROC);
	pfnGetVertexAttribIiv(index, pname, params);
}

void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
	using PFNGLGETVERTEXATTRIBIUIVPROC = void(APIENTRY *)(GLuint index, GLenum pname, GLuint* params);
	static PFNGLGETVERTEXATTRIBIUIVPROC pfnGetVertexAttribIuiv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribIuiv", pfnGetVertexAttribIuiv, PFNGLGETVERTEXATTRIBIUIVPROC);
	pfnGetVertexAttribIuiv(index, pname, params);
}

void glVertexAttribI1i(GLuint index, GLint x)
{
	using PFNGLVERTEXATTRIBI1IPROC = void(APIENTRY *)(GLuint index, GLint x);
	static PFNGLVERTEXATTRIBI1IPROC pfnVertexAttribI1i{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI1i", pfnVertexAttribI1i, PFNGLVERTEXATTRIBI1IPROC);
	pfnVertexAttribI1i(index, x);
}

void glVertexAttribI2i(GLuint index, GLint x, GLint y)
{
	using PFNGLVERTEXATTRIBI2IPROC = void(APIENTRY *)(GLuint index, GLint x, GLint y);
	static PFNGLVERTEXATTRIBI2IPROC pfnVertexAttribI2i{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI2i", pfnVertexAttribI2i, PFNGLVERTEXATTRIBI2IPROC);
	pfnVertexAttribI2i(index, x, y);
}

void glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z)
{
	using PFNGLVERTEXATTRIBI3IPROC = void(APIENTRY *)(GLuint index, GLint x, GLint y, GLint z);
	static PFNGLVERTEXATTRIBI3IPROC pfnVertexAttribI3i{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI3i", pfnVertexAttribI3i, PFNGLVERTEXATTRIBI3IPROC);
	pfnVertexAttribI3i(index, x, y, z);
}

void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	using PFNGLVERTEXATTRIBI4IPROC = void(APIENTRY *)(GLuint index, GLint x, GLint y, GLint z, GLint w);
	static PFNGLVERTEXATTRIBI4IPROC pfnVertexAttribI4i{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI4i", pfnVertexAttribI4i, PFNGLVERTEXATTRIBI4IPROC);
	pfnVertexAttribI4i(index, x, y, z, w);
}

void glVertexAttribI1ui(GLuint index, GLuint x)
{
	using PFNGLVERTEXATTRIBI1UIPROC = void(APIENTRY *)(GLuint index, GLuint x);
	static PFNGLVERTEXATTRIBI1UIPROC pfnVertexAttribI1ui{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI1ui", pfnVertexAttribI1ui, PFNGLVERTEXATTRIBI1UIPROC);
	pfnVertexAttribI1ui(index, x);
}

void glVertexAttribI2ui(GLuint index, GLuint x, GLuint y)
{
	using PFNGLVERTEXATTRIBI2UIPROC = void(APIENTRY *)(GLuint index, GLuint x, GLuint y);
	static PFNGLVERTEXATTRIBI2UIPROC pfnVertexAttribI2ui{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI2ui", pfnVertexAttribI2ui, PFNGLVERTEXATTRIBI2UIPROC);
	pfnVertexAttribI2ui(index, x, y);
}

void glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z)
{
	using PFNGLVERTEXATTRIBI3UIPROC = void(APIENTRY *)(GLuint index, GLuint x, GLuint y, GLuint z);
	static PFNGLVERTEXATTRIBI3UIPROC pfnVertexAttribI3ui{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI3ui", pfnVertexAttribI3ui, PFNGLVERTEXATTRIBI3UIPROC);
	pfnVertexAttribI3ui(index, x, y, z);
}

void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	using PFNGLVERTEXATTRIBI4UIPROC = void(APIENTRY *)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
	static PFNGLVERTEXATTRIBI4UIPROC pfnVertexAttribI4ui{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI4ui", pfnVertexAttribI4ui, PFNGLVERTEXATTRIBI4UIPROC);
	pfnVertexAttribI4ui(index, x, y, z, w);
}

void glVertexAttribI1iv(GLuint index, const GLint* v)
{
	using PFNGLVERTEXATTRIBI1IVPROC = void(APIENTRY *)(GLuint index, const GLint* v);
	static PFNGLVERTEXATTRIBI1IVPROC pfnVertexAttribI1iv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI1iv", pfnVertexAttribI1iv, PFNGLVERTEXATTRIBI1IVPROC);
	pfnVertexAttribI1iv(index, v);
}

void glVertexAttribI2iv(GLuint index, const GLint* v)
{
	using PFNGLVERTEXATTRIBI2IVPROC = void(APIENTRY *)(GLuint index, const GLint* v);
	static PFNGLVERTEXATTRIBI2IVPROC pfnVertexAttribI2iv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI2iv", pfnVertexAttribI2iv, PFNGLVERTEXATTRIBI2IVPROC);
	pfnVertexAttribI2iv(index, v);
}

void glVertexAttribI3iv(GLuint index, const GLint* v)
{
	using PFNGLVERTEXATTRIBI3IVPROC = void(APIENTRY *)(GLuint index, const GLint* v);
	static PFNGLVERTEXATTRIBI3IVPROC pfnVertexAttribI3iv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI3iv", pfnVertexAttribI3iv, PFNGLVERTEXATTRIBI3IVPROC);
	pfnVertexAttribI3iv(index, v);
}

void glVertexAttribI4iv(GLuint index, const GLint* v)
{
	using PFNGLVERTEXATTRIBI4IVPROC = void(APIENTRY *)(GLuint index, const GLint* v);
	static PFNGLVERTEXATTRIBI4IVPROC pfnVertexAttribI4iv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI4iv", pfnVertexAttribI4iv, PFNGLVERTEXATTRIBI4IVPROC);
	pfnVertexAttribI4iv(index, v);
}

void glVertexAttribI1uiv(GLuint index, const GLuint* v)
{
	using PFNGLVERTEXATTRIBI1UIVPROC = void(APIENTRY *)(GLuint index, const GLuint* v);
	static PFNGLVERTEXATTRIBI1UIVPROC pfnVertexAttribI1uiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI1uiv", pfnVertexAttribI1uiv, PFNGLVERTEXATTRIBI1UIVPROC);
	pfnVertexAttribI1uiv(index, v);
}

void glVertexAttribI2uiv(GLuint index, const GLuint* v)
{
	using PFNGLVERTEXATTRIBI2UIVPROC = void(APIENTRY *)(GLuint index, const GLuint* v);
	static PFNGLVERTEXATTRIBI2UIVPROC pfnVertexAttribI2uiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI2uiv", pfnVertexAttribI2uiv, PFNGLVERTEXATTRIBI2UIVPROC);
	pfnVertexAttribI2uiv(index, v);
}

void glVertexAttribI3uiv(GLuint index, const GLuint* v)
{
	using PFNGLVERTEXATTRIBI3UIVPROC = void(APIENTRY *)(GLuint index, const GLuint* v);
	static PFNGLVERTEXATTRIBI3UIVPROC pfnVertexAttribI3uiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI3uiv", pfnVertexAttribI3uiv, PFNGLVERTEXATTRIBI3UIVPROC);
	pfnVertexAttribI3uiv(index, v);
}

void glVertexAttribI4uiv(GLuint index, const GLuint* v)
{
	using PFNGLVERTEXATTRIBI4UIVPROC = void(APIENTRY *)(GLuint index, const GLuint* v);
	static PFNGLVERTEXATTRIBI4UIVPROC pfnVertexAttribI4uiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI4uiv", pfnVertexAttribI4uiv, PFNGLVERTEXATTRIBI4UIVPROC);
	pfnVertexAttribI4uiv(index, v);
}

void glVertexAttribI4bv(GLuint index, const GLbyte* v)
{
	using PFNGLVERTEXATTRIBI4BVPROC = void(APIENTRY *)(GLuint index, const GLbyte* v);
	static PFNGLVERTEXATTRIBI4BVPROC pfnVertexAttribI4bv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI4bv", pfnVertexAttribI4bv, PFNGLVERTEXATTRIBI4BVPROC);
	pfnVertexAttribI4bv(index, v);
}

void glVertexAttribI4sv(GLuint index, const GLshort* v)
{
	using PFNGLVERTEXATTRIBI4SVPROC = void(APIENTRY *)(GLuint index, const GLshort* v);
	static PFNGLVERTEXATTRIBI4SVPROC pfnVertexAttribI4sv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI4sv", pfnVertexAttribI4sv, PFNGLVERTEXATTRIBI4SVPROC);
	pfnVertexAttribI4sv(index, v);
}

void glVertexAttribI4ubv(GLuint index, const GLubyte* v)
{
	using PFNGLVERTEXATTRIBI4UBVPROC = void(APIENTRY *)(GLuint index, const GLubyte* v);
	static PFNGLVERTEXATTRIBI4UBVPROC pfnVertexAttribI4ubv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI4ubv", pfnVertexAttribI4ubv, PFNGLVERTEXATTRIBI4UBVPROC);
	pfnVertexAttribI4ubv(index, v);
}

void glVertexAttribI4usv(GLuint index, const GLushort* v)
{
	using PFNGLVERTEXATTRIBI4USVPROC = void(APIENTRY *)(GLuint index, const GLushort* v);
	static PFNGLVERTEXATTRIBI4USVPROC pfnVertexAttribI4usv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribI4usv", pfnVertexAttribI4usv, PFNGLVERTEXATTRIBI4USVPROC);
	pfnVertexAttribI4usv(index, v);
}

void glGetUniformuiv(GLuint program, GLint location, GLuint* params)
{
	using PFNGLGETUNIFORMUIVPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint* params);
	static PFNGLGETUNIFORMUIVPROC pfnGetUniformuiv{nullptr};
	LOAD_ENTRYPOINT("glGetUniformuiv", pfnGetUniformuiv, PFNGLGETUNIFORMUIVPROC);
	pfnGetUniformuiv(program, location, params);
}

void glBindFragDataLocation(GLuint program, GLuint color, const GLchar* name)
{
	using PFNGLBINDFRAGDATALOCATIONPROC = void(APIENTRY *)(GLuint program, GLuint color, const GLchar* name);
	static PFNGLBINDFRAGDATALOCATIONPROC pfnBindFragDataLocation{nullptr};
	LOAD_ENTRYPOINT("glBindFragDataLocation", pfnBindFragDataLocation, PFNGLBINDFRAGDATALOCATIONPROC);
	pfnBindFragDataLocation(program, color, name);
}

GLint glGetFragDataLocation(GLuint program, const GLchar* name)
{
	using PFNGLGETFRAGDATALOCATIONPROC = GLint(APIENTRY *)(GLuint program, const GLchar* name);
	static PFNGLGETFRAGDATALOCATIONPROC pfnGetFragDataLocation{nullptr};
	LOAD_ENTRYPOINT("glGetFragDataLocation", pfnGetFragDataLocation, PFNGLGETFRAGDATALOCATIONPROC);
	return pfnGetFragDataLocation(program, name);
}

void glUniform1ui(GLint location, GLuint v0)
{
	using PFNGLUNIFORM1UIPROC = void(APIENTRY *)(GLint location, GLuint v0);
	static PFNGLUNIFORM1UIPROC pfnUniform1ui{nullptr};
	LOAD_ENTRYPOINT("glUniform1ui", pfnUniform1ui, PFNGLUNIFORM1UIPROC);
	pfnUniform1ui(location, v0);
}

void glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
	using PFNGLUNIFORM2UIPROC = void(APIENTRY *)(GLint location, GLuint v0, GLuint v1);
	static PFNGLUNIFORM2UIPROC pfnUniform2ui{nullptr};
	LOAD_ENTRYPOINT("glUniform2ui", pfnUniform2ui, PFNGLUNIFORM2UIPROC);
	pfnUniform2ui(location, v0, v1);
}

void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	using PFNGLUNIFORM3UIPROC = void(APIENTRY *)(GLint location, GLuint v0, GLuint v1, GLuint v2);
	static PFNGLUNIFORM3UIPROC pfnUniform3ui{nullptr};
	LOAD_ENTRYPOINT("glUniform3ui", pfnUniform3ui, PFNGLUNIFORM3UIPROC);
	pfnUniform3ui(location, v0, v1, v2);
}

void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	using PFNGLUNIFORM4UIPROC = void(APIENTRY *)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static PFNGLUNIFORM4UIPROC pfnUniform4ui{nullptr};
	LOAD_ENTRYPOINT("glUniform4ui", pfnUniform4ui, PFNGLUNIFORM4UIPROC);
	pfnUniform4ui(location, v0, v1, v2, v3);
}

void glUniform1uiv(GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLUNIFORM1UIVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint* value);
	static PFNGLUNIFORM1UIVPROC pfnUniform1uiv{nullptr};
	LOAD_ENTRYPOINT("glUniform1uiv", pfnUniform1uiv, PFNGLUNIFORM1UIVPROC);
	pfnUniform1uiv(location, count, value);
}

void glUniform2uiv(GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLUNIFORM2UIVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint* value);
	static PFNGLUNIFORM2UIVPROC pfnUniform2uiv{nullptr};
	LOAD_ENTRYPOINT("glUniform2uiv", pfnUniform2uiv, PFNGLUNIFORM2UIVPROC);
	pfnUniform2uiv(location, count, value);
}

void glUniform3uiv(GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLUNIFORM3UIVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint* value);
	static PFNGLUNIFORM3UIVPROC pfnUniform3uiv{nullptr};
	LOAD_ENTRYPOINT("glUniform3uiv", pfnUniform3uiv, PFNGLUNIFORM3UIVPROC);
	pfnUniform3uiv(location, count, value);
}

void glUniform4uiv(GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLUNIFORM4UIVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint* value);
	static PFNGLUNIFORM4UIVPROC pfnUniform4uiv{nullptr};
	LOAD_ENTRYPOINT("glUniform4uiv", pfnUniform4uiv, PFNGLUNIFORM4UIVPROC);
	pfnUniform4uiv(location, count, value);
}

void glTexParameterIiv(GLenum target, GLenum pname, const GLint* params)
{
	using PFNGLTEXPARAMETERIIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, const GLint* params);
	static PFNGLTEXPARAMETERIIVPROC pfnTexParameterIiv{nullptr};
	LOAD_ENTRYPOINT("glTexParameterIiv", pfnTexParameterIiv, PFNGLTEXPARAMETERIIVPROC);
	pfnTexParameterIiv(target, pname, params);
}

void glTexParameterIuiv(GLenum target, GLenum pname, const GLuint* params)
{
	using PFNGLTEXPARAMETERIUIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, const GLuint* params);
	static PFNGLTEXPARAMETERIUIVPROC pfnTexParameterIuiv{nullptr};
	LOAD_ENTRYPOINT("glTexParameterIuiv", pfnTexParameterIuiv, PFNGLTEXPARAMETERIUIVPROC);
	pfnTexParameterIuiv(target, pname, params);
}

void glGetTexParameterIiv(GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETTEXPARAMETERIIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint* params);
	static PFNGLGETTEXPARAMETERIIVPROC pfnGetTexParameterIiv{nullptr};
	LOAD_ENTRYPOINT("glGetTexParameterIiv", pfnGetTexParameterIiv, PFNGLGETTEXPARAMETERIIVPROC);
	pfnGetTexParameterIiv(target, pname, params);
}

void glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint* params)
{
	using PFNGLGETTEXPARAMETERIUIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLuint* params);
	static PFNGLGETTEXPARAMETERIUIVPROC pfnGetTexParameterIuiv{nullptr};
	LOAD_ENTRYPOINT("glGetTexParameterIuiv", pfnGetTexParameterIuiv, PFNGLGETTEXPARAMETERIUIVPROC);
	pfnGetTexParameterIuiv(target, pname, params);
}

void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value)
{
	using PFNGLCLEARBUFFERIVPROC = void(APIENTRY *)(GLenum buffer, GLint drawbuffer, const GLint* value);
	static PFNGLCLEARBUFFERIVPROC pfnClearBufferiv{nullptr};
	LOAD_ENTRYPOINT("glClearBufferiv", pfnClearBufferiv, PFNGLCLEARBUFFERIVPROC);
	pfnClearBufferiv(buffer, drawbuffer, value);
}

void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
	using PFNGLCLEARBUFFERUIVPROC = void(APIENTRY *)(GLenum buffer, GLint drawbuffer, const GLuint* value);
	static PFNGLCLEARBUFFERUIVPROC pfnClearBufferuiv{nullptr};
	LOAD_ENTRYPOINT("glClearBufferuiv", pfnClearBufferuiv, PFNGLCLEARBUFFERUIVPROC);
	pfnClearBufferuiv(buffer, drawbuffer, value);
}

void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
	using PFNGLCLEARBUFFERFVPROC = void(APIENTRY *)(GLenum buffer, GLint drawbuffer, const GLfloat* value);
	static PFNGLCLEARBUFFERFVPROC pfnClearBufferfv{nullptr};
	LOAD_ENTRYPOINT("glClearBufferfv", pfnClearBufferfv, PFNGLCLEARBUFFERFVPROC);
	pfnClearBufferfv(buffer, drawbuffer, value);
}

void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	using PFNGLCLEARBUFFERFIPROC = void(APIENTRY *)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	static PFNGLCLEARBUFFERFIPROC pfnClearBufferfi{nullptr};
	LOAD_ENTRYPOINT("glClearBufferfi", pfnClearBufferfi, PFNGLCLEARBUFFERFIPROC);
	pfnClearBufferfi(buffer, drawbuffer, depth, stencil);
}

const GLubyte* glGetStringi(GLenum name, GLuint index)
{
	using PFNGLGETSTRINGIPROC = const GLubyte*(APIENTRY *)(GLenum name, GLuint index);
	static PFNGLGETSTRINGIPROC pfnGetStringi{nullptr};
	LOAD_ENTRYPOINT("glGetStringi", pfnGetStringi, PFNGLGETSTRINGIPROC);
	return pfnGetStringi(name, index);
}

GLboolean glIsRenderbuffer(GLuint renderbuffer)
{
	using PFNGLISRENDERBUFFERPROC = GLboolean(APIENTRY *)(GLuint renderbuffer);
	static PFNGLISRENDERBUFFERPROC pfnIsRenderbuffer{nullptr};
	LOAD_ENTRYPOINT("glIsRenderbuffer", pfnIsRenderbuffer, PFNGLISRENDERBUFFERPROC);
	return pfnIsRenderbuffer(renderbuffer);
}

void glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	using PFNGLBINDRENDERBUFFERPROC = void(APIENTRY *)(GLenum target, GLuint renderbuffer);
	static PFNGLBINDRENDERBUFFERPROC pfnBindRenderbuffer{nullptr};
	LOAD_ENTRYPOINT("glBindRenderbuffer", pfnBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC);
	pfnBindRenderbuffer(target, renderbuffer);
}

void glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
	using PFNGLDELETERENDERBUFFERSPROC = void(APIENTRY *)(GLsizei n, const GLuint* renderbuffers);
	static PFNGLDELETERENDERBUFFERSPROC pfnDeleteRenderbuffers{nullptr};
	LOAD_ENTRYPOINT("glDeleteRenderbuffers", pfnDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC);
	pfnDeleteRenderbuffers(n, renderbuffers);
}

void glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
	using PFNGLGENRENDERBUFFERSPROC = void(APIENTRY *)(GLsizei n, GLuint* renderbuffers);
	static PFNGLGENRENDERBUFFERSPROC pfnGenRenderbuffers{nullptr};
	LOAD_ENTRYPOINT("glGenRenderbuffers", pfnGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC);
	pfnGenRenderbuffers(n, renderbuffers);
}

void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLRENDERBUFFERSTORAGEPROC = void(APIENTRY *)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLRENDERBUFFERSTORAGEPROC pfnRenderbufferStorage{nullptr};
	LOAD_ENTRYPOINT("glRenderbufferStorage", pfnRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC);
	pfnRenderbufferStorage(target, internalformat, width, height);
}

void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETRENDERBUFFERPARAMETERIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint* params);
	static PFNGLGETRENDERBUFFERPARAMETERIVPROC pfnGetRenderbufferParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetRenderbufferParameteriv", pfnGetRenderbufferParameteriv, PFNGLGETRENDERBUFFERPARAMETERIVPROC);
	pfnGetRenderbufferParameteriv(target, pname, params);
}

GLboolean glIsFramebuffer(GLuint framebuffer)
{
	using PFNGLISFRAMEBUFFERPROC = GLboolean(APIENTRY *)(GLuint framebuffer);
	static PFNGLISFRAMEBUFFERPROC pfnIsFramebuffer{nullptr};
	LOAD_ENTRYPOINT("glIsFramebuffer", pfnIsFramebuffer, PFNGLISFRAMEBUFFERPROC);
	return pfnIsFramebuffer(framebuffer);
}

void glBindFramebuffer(GLenum target, GLuint framebuffer)
{
	using PFNGLBINDFRAMEBUFFERPROC = void(APIENTRY *)(GLenum target, GLuint framebuffer);
	static PFNGLBINDFRAMEBUFFERPROC pfnBindFramebuffer{nullptr};
	LOAD_ENTRYPOINT("glBindFramebuffer", pfnBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC);
	pfnBindFramebuffer(target, framebuffer);
}

void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
	using PFNGLDELETEFRAMEBUFFERSPROC = void(APIENTRY *)(GLsizei n, const GLuint* framebuffers);
	static PFNGLDELETEFRAMEBUFFERSPROC pfnDeleteFramebuffers{nullptr};
	LOAD_ENTRYPOINT("glDeleteFramebuffers", pfnDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC);
	pfnDeleteFramebuffers(n, framebuffers);
}

void glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
	using PFNGLGENFRAMEBUFFERSPROC = void(APIENTRY *)(GLsizei n, GLuint* framebuffers);
	static PFNGLGENFRAMEBUFFERSPROC pfnGenFramebuffers{nullptr};
	LOAD_ENTRYPOINT("glGenFramebuffers", pfnGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC);
	pfnGenFramebuffers(n, framebuffers);
}

GLenum glCheckFramebufferStatus(GLenum target)
{
	using PFNGLCHECKFRAMEBUFFERSTATUSPROC = GLenum(APIENTRY *)(GLenum target);
	static PFNGLCHECKFRAMEBUFFERSTATUSPROC pfnCheckFramebufferStatus{nullptr};
	LOAD_ENTRYPOINT("glCheckFramebufferStatus", pfnCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC);
	return pfnCheckFramebufferStatus(target);
}

void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	using PFNGLFRAMEBUFFERTEXTURE1DPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTURE1DPROC pfnFramebufferTexture1D{nullptr};
	LOAD_ENTRYPOINT("glFramebufferTexture1D", pfnFramebufferTexture1D, PFNGLFRAMEBUFFERTEXTURE1DPROC);
	pfnFramebufferTexture1D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	using PFNGLFRAMEBUFFERTEXTURE2DPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTURE2DPROC pfnFramebufferTexture2D{nullptr};
	LOAD_ENTRYPOINT("glFramebufferTexture2D", pfnFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC);
	pfnFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	using PFNGLFRAMEBUFFERTEXTURE3DPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
	static PFNGLFRAMEBUFFERTEXTURE3DPROC pfnFramebufferTexture3D{nullptr};
	LOAD_ENTRYPOINT("glFramebufferTexture3D", pfnFramebufferTexture3D, PFNGLFRAMEBUFFERTEXTURE3DPROC);
	pfnFramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
}

void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	using PFNGLFRAMEBUFFERRENDERBUFFERPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static PFNGLFRAMEBUFFERRENDERBUFFERPROC pfnFramebufferRenderbuffer{nullptr};
	LOAD_ENTRYPOINT("glFramebufferRenderbuffer", pfnFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC);
	pfnFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
	using PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLenum pname, GLint* params);
	static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC pfnGetFramebufferAttachmentParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetFramebufferAttachmentParameteriv", pfnGetFramebufferAttachmentParameteriv, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC);
	pfnGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void glGenerateMipmap(GLenum target)
{
	using PFNGLGENERATEMIPMAPPROC = void(APIENTRY *)(GLenum target);
	static PFNGLGENERATEMIPMAPPROC pfnGenerateMipmap{nullptr};
	LOAD_ENTRYPOINT("glGenerateMipmap", pfnGenerateMipmap, PFNGLGENERATEMIPMAPPROC);
	pfnGenerateMipmap(target);
}

void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	using PFNGLBLITFRAMEBUFFERPROC = void(APIENTRY *)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	static PFNGLBLITFRAMEBUFFERPROC pfnBlitFramebuffer{nullptr};
	LOAD_ENTRYPOINT("glBlitFramebuffer", pfnBlitFramebuffer, PFNGLBLITFRAMEBUFFERPROC);
	pfnBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC = void(APIENTRY *)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC pfnRenderbufferStorageMultisample{nullptr};
	LOAD_ENTRYPOINT("glRenderbufferStorageMultisample", pfnRenderbufferStorageMultisample, PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC);
	pfnRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	using PFNGLFRAMEBUFFERTEXTURELAYERPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static PFNGLFRAMEBUFFERTEXTURELAYERPROC pfnFramebufferTextureLayer{nullptr};
	LOAD_ENTRYPOINT("glFramebufferTextureLayer", pfnFramebufferTextureLayer, PFNGLFRAMEBUFFERTEXTURELAYERPROC);
	pfnFramebufferTextureLayer(target, attachment, texture, level, layer);
}

void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	using PFNGLMAPBUFFERRANGEPROC = void*(APIENTRY *)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	static PFNGLMAPBUFFERRANGEPROC pfnMapBufferRange{nullptr};
	LOAD_ENTRYPOINT("glMapBufferRange", pfnMapBufferRange, PFNGLMAPBUFFERRANGEPROC);
	return pfnMapBufferRange(target, offset, length, access);
}

void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	using PFNGLFLUSHMAPPEDBUFFERRANGEPROC = void(APIENTRY *)(GLenum target, GLintptr offset, GLsizeiptr length);
	static PFNGLFLUSHMAPPEDBUFFERRANGEPROC pfnFlushMappedBufferRange{nullptr};
	LOAD_ENTRYPOINT("glFlushMappedBufferRange", pfnFlushMappedBufferRange, PFNGLFLUSHMAPPEDBUFFERRANGEPROC);
	pfnFlushMappedBufferRange(target, offset, length);
}

void glBindVertexArray(GLuint array)
{
	using PFNGLBINDVERTEXARRAYPROC = void(APIENTRY *)(GLuint array);
	static PFNGLBINDVERTEXARRAYPROC pfnBindVertexArray{nullptr};
	LOAD_ENTRYPOINT("glBindVertexArray", pfnBindVertexArray, PFNGLBINDVERTEXARRAYPROC);
	pfnBindVertexArray(array);
}

void glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
	using PFNGLDELETEVERTEXARRAYSPROC = void(APIENTRY *)(GLsizei n, const GLuint* arrays);
	static PFNGLDELETEVERTEXARRAYSPROC pfnDeleteVertexArrays{nullptr};
	LOAD_ENTRYPOINT("glDeleteVertexArrays", pfnDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC);
	pfnDeleteVertexArrays(n, arrays);
}

void glGenVertexArrays(GLsizei n, GLuint* arrays)
{
	using PFNGLGENVERTEXARRAYSPROC = void(APIENTRY *)(GLsizei n, GLuint* arrays);
	static PFNGLGENVERTEXARRAYSPROC pfnGenVertexArrays{nullptr};
	LOAD_ENTRYPOINT("glGenVertexArrays", pfnGenVertexArrays, PFNGLGENVERTEXARRAYSPROC);
	pfnGenVertexArrays(n, arrays);
}

GLboolean glIsVertexArray(GLuint array)
{
	using PFNGLISVERTEXARRAYPROC = GLboolean(APIENTRY *)(GLuint array);
	static PFNGLISVERTEXARRAYPROC pfnIsVertexArray{nullptr};
	LOAD_ENTRYPOINT("glIsVertexArray", pfnIsVertexArray, PFNGLISVERTEXARRAYPROC);
	return pfnIsVertexArray(array);
}

//
// GL_VERSION_3_1
//

void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
	using PFNGLDRAWARRAYSINSTANCEDPROC = void(APIENTRY *)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
	static PFNGLDRAWARRAYSINSTANCEDPROC pfnDrawArraysInstanced{nullptr};
	LOAD_ENTRYPOINT("glDrawArraysInstanced", pfnDrawArraysInstanced, PFNGLDRAWARRAYSINSTANCEDPROC);
	pfnDrawArraysInstanced(mode, first, count, instancecount);
}

void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
	using PFNGLDRAWELEMENTSINSTANCEDPROC = void(APIENTRY *)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
	static PFNGLDRAWELEMENTSINSTANCEDPROC pfnDrawElementsInstanced{nullptr};
	LOAD_ENTRYPOINT("glDrawElementsInstanced", pfnDrawElementsInstanced, PFNGLDRAWELEMENTSINSTANCEDPROC);
	pfnDrawElementsInstanced(mode, count, type, indices, instancecount);
}

void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
	using PFNGLTEXBUFFERPROC = void(APIENTRY *)(GLenum target, GLenum internalformat, GLuint buffer);
	static PFNGLTEXBUFFERPROC pfnTexBuffer{nullptr};
	LOAD_ENTRYPOINT("glTexBuffer", pfnTexBuffer, PFNGLTEXBUFFERPROC);
	pfnTexBuffer(target, internalformat, buffer);
}

void glPrimitiveRestartIndex(GLuint index)
{
	using PFNGLPRIMITIVERESTARTINDEXPROC = void(APIENTRY *)(GLuint index);
	static PFNGLPRIMITIVERESTARTINDEXPROC pfnPrimitiveRestartIndex{nullptr};
	LOAD_ENTRYPOINT("glPrimitiveRestartIndex", pfnPrimitiveRestartIndex, PFNGLPRIMITIVERESTARTINDEXPROC);
	pfnPrimitiveRestartIndex(index);
}

void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	using PFNGLCOPYBUFFERSUBDATAPROC = void(APIENTRY *)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	static PFNGLCOPYBUFFERSUBDATAPROC pfnCopyBufferSubData{nullptr};
	LOAD_ENTRYPOINT("glCopyBufferSubData", pfnCopyBufferSubData, PFNGLCOPYBUFFERSUBDATAPROC);
	pfnCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

void glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
	using PFNGLGETUNIFORMINDICESPROC = void(APIENTRY *)(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
	static PFNGLGETUNIFORMINDICESPROC pfnGetUniformIndices{nullptr};
	LOAD_ENTRYPOINT("glGetUniformIndices", pfnGetUniformIndices, PFNGLGETUNIFORMINDICESPROC);
	pfnGetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
}

void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
	using PFNGLGETACTIVEUNIFORMSIVPROC = void(APIENTRY *)(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
	static PFNGLGETACTIVEUNIFORMSIVPROC pfnGetActiveUniformsiv{nullptr};
	LOAD_ENTRYPOINT("glGetActiveUniformsiv", pfnGetActiveUniformsiv, PFNGLGETACTIVEUNIFORMSIVPROC);
	pfnGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

void glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformName)
{
	using PFNGLGETACTIVEUNIFORMNAMEPROC = void(APIENTRY *)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformName);
	static PFNGLGETACTIVEUNIFORMNAMEPROC pfnGetActiveUniformName{nullptr};
	LOAD_ENTRYPOINT("glGetActiveUniformName", pfnGetActiveUniformName, PFNGLGETACTIVEUNIFORMNAMEPROC);
	pfnGetActiveUniformName(program, uniformIndex, bufSize, length, uniformName);
}

GLuint glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)
{
	using PFNGLGETUNIFORMBLOCKINDEXPROC = GLuint(APIENTRY *)(GLuint program, const GLchar* uniformBlockName);
	static PFNGLGETUNIFORMBLOCKINDEXPROC pfnGetUniformBlockIndex{nullptr};
	LOAD_ENTRYPOINT("glGetUniformBlockIndex", pfnGetUniformBlockIndex, PFNGLGETUNIFORMBLOCKINDEXPROC);
	return pfnGetUniformBlockIndex(program, uniformBlockName);
}

void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
	using PFNGLGETACTIVEUNIFORMBLOCKIVPROC = void(APIENTRY *)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
	static PFNGLGETACTIVEUNIFORMBLOCKIVPROC pfnGetActiveUniformBlockiv{nullptr};
	LOAD_ENTRYPOINT("glGetActiveUniformBlockiv", pfnGetActiveUniformBlockiv, PFNGLGETACTIVEUNIFORMBLOCKIVPROC);
	pfnGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
	using PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC = void(APIENTRY *)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
	static PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC pfnGetActiveUniformBlockName{nullptr};
	LOAD_ENTRYPOINT("glGetActiveUniformBlockName", pfnGetActiveUniformBlockName, PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC);
	pfnGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
	using PFNGLUNIFORMBLOCKBINDINGPROC = void(APIENTRY *)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
	static PFNGLUNIFORMBLOCKBINDINGPROC pfnUniformBlockBinding{nullptr};
	LOAD_ENTRYPOINT("glUniformBlockBinding", pfnUniformBlockBinding, PFNGLUNIFORMBLOCKBINDINGPROC);
	pfnUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

//
// GL_VERSION_3_2
//

void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
	using PFNGLDRAWELEMENTSBASEVERTEXPROC = void(APIENTRY *)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex);
	static PFNGLDRAWELEMENTSBASEVERTEXPROC pfnDrawElementsBaseVertex{nullptr};
	LOAD_ENTRYPOINT("glDrawElementsBaseVertex", pfnDrawElementsBaseVertex, PFNGLDRAWELEMENTSBASEVERTEXPROC);
	pfnDrawElementsBaseVertex(mode, count, type, indices, basevertex);
}

void glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex)
{
	using PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC = void(APIENTRY *)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex);
	static PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC pfnDrawRangeElementsBaseVertex{nullptr};
	LOAD_ENTRYPOINT("glDrawRangeElementsBaseVertex", pfnDrawRangeElementsBaseVertex, PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC);
	pfnDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
}

void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex)
{
	using PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC = void(APIENTRY *)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex);
	static PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC pfnDrawElementsInstancedBaseVertex{nullptr};
	LOAD_ENTRYPOINT("glDrawElementsInstancedBaseVertex", pfnDrawElementsInstancedBaseVertex, PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC);
	pfnDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
}

void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex)
{
	using PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC = void(APIENTRY *)(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex);
	static PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC pfnMultiDrawElementsBaseVertex{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawElementsBaseVertex", pfnMultiDrawElementsBaseVertex, PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC);
	pfnMultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
}

void glProvokingVertex(GLenum mode)
{
	using PFNGLPROVOKINGVERTEXPROC = void(APIENTRY *)(GLenum mode);
	static PFNGLPROVOKINGVERTEXPROC pfnProvokingVertex{nullptr};
	LOAD_ENTRYPOINT("glProvokingVertex", pfnProvokingVertex, PFNGLPROVOKINGVERTEXPROC);
	pfnProvokingVertex(mode);
}

GLsync glFenceSync(GLenum condition, GLbitfield flags)
{
	using PFNGLFENCESYNCPROC = GLsync(APIENTRY *)(GLenum condition, GLbitfield flags);
	static PFNGLFENCESYNCPROC pfnFenceSync{nullptr};
	LOAD_ENTRYPOINT("glFenceSync", pfnFenceSync, PFNGLFENCESYNCPROC);
	return pfnFenceSync(condition, flags);
}

GLboolean glIsSync(GLsync sync)
{
	using PFNGLISSYNCPROC = GLboolean(APIENTRY *)(GLsync sync);
	static PFNGLISSYNCPROC pfnIsSync{nullptr};
	LOAD_ENTRYPOINT("glIsSync", pfnIsSync, PFNGLISSYNCPROC);
	return pfnIsSync(sync);
}

void glDeleteSync(GLsync sync)
{
	using PFNGLDELETESYNCPROC = void(APIENTRY *)(GLsync sync);
	static PFNGLDELETESYNCPROC pfnDeleteSync{nullptr};
	LOAD_ENTRYPOINT("glDeleteSync", pfnDeleteSync, PFNGLDELETESYNCPROC);
	pfnDeleteSync(sync);
}

GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	using PFNGLCLIENTWAITSYNCPROC = GLenum(APIENTRY *)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static PFNGLCLIENTWAITSYNCPROC pfnClientWaitSync{nullptr};
	LOAD_ENTRYPOINT("glClientWaitSync", pfnClientWaitSync, PFNGLCLIENTWAITSYNCPROC);
	return pfnClientWaitSync(sync, flags, timeout);
}

void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	using PFNGLWAITSYNCPROC = void(APIENTRY *)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static PFNGLWAITSYNCPROC pfnWaitSync{nullptr};
	LOAD_ENTRYPOINT("glWaitSync", pfnWaitSync, PFNGLWAITSYNCPROC);
	pfnWaitSync(sync, flags, timeout);
}

void glGetInteger64v(GLenum pname, GLint64* data)
{
	using PFNGLGETINTEGER64VPROC = void(APIENTRY *)(GLenum pname, GLint64* data);
	static PFNGLGETINTEGER64VPROC pfnGetInteger64v{nullptr};
	LOAD_ENTRYPOINT("glGetInteger64v", pfnGetInteger64v, PFNGLGETINTEGER64VPROC);
	pfnGetInteger64v(pname, data);
}

void glGetSynciv(GLsync sync, GLenum pname, GLsizei count, GLsizei* length, GLint* values)
{
	using PFNGLGETSYNCIVPROC = void(APIENTRY *)(GLsync sync, GLenum pname, GLsizei count, GLsizei* length, GLint* values);
	static PFNGLGETSYNCIVPROC pfnGetSynciv{nullptr};
	LOAD_ENTRYPOINT("glGetSynciv", pfnGetSynciv, PFNGLGETSYNCIVPROC);
	pfnGetSynciv(sync, pname, count, length, values);
}

void glGetInteger64i_v(GLenum target, GLuint index, GLint64* data)
{
	using PFNGLGETINTEGER64I_VPROC = void(APIENTRY *)(GLenum target, GLuint index, GLint64* data);
	static PFNGLGETINTEGER64I_VPROC pfnGetInteger64i_v{nullptr};
	LOAD_ENTRYPOINT("glGetInteger64i_v", pfnGetInteger64i_v, PFNGLGETINTEGER64I_VPROC);
	pfnGetInteger64i_v(target, index, data);
}

void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params)
{
	using PFNGLGETBUFFERPARAMETERI64VPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint64* params);
	static PFNGLGETBUFFERPARAMETERI64VPROC pfnGetBufferParameteri64v{nullptr};
	LOAD_ENTRYPOINT("glGetBufferParameteri64v", pfnGetBufferParameteri64v, PFNGLGETBUFFERPARAMETERI64VPROC);
	pfnGetBufferParameteri64v(target, pname, params);
}

void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	using PFNGLFRAMEBUFFERTEXTUREPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTUREPROC pfnFramebufferTexture{nullptr};
	LOAD_ENTRYPOINT("glFramebufferTexture", pfnFramebufferTexture, PFNGLFRAMEBUFFERTEXTUREPROC);
	pfnFramebufferTexture(target, attachment, texture, level);
}

void glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	using PFNGLTEXIMAGE2DMULTISAMPLEPROC = void(APIENTRY *)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static PFNGLTEXIMAGE2DMULTISAMPLEPROC pfnTexImage2DMultisample{nullptr};
	LOAD_ENTRYPOINT("glTexImage2DMultisample", pfnTexImage2DMultisample, PFNGLTEXIMAGE2DMULTISAMPLEPROC);
	pfnTexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

void glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	using PFNGLTEXIMAGE3DMULTISAMPLEPROC = void(APIENTRY *)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static PFNGLTEXIMAGE3DMULTISAMPLEPROC pfnTexImage3DMultisample{nullptr};
	LOAD_ENTRYPOINT("glTexImage3DMultisample", pfnTexImage3DMultisample, PFNGLTEXIMAGE3DMULTISAMPLEPROC);
	pfnTexImage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

void glGetMultisamplefv(GLenum pname, GLuint index, GLfloat* val)
{
	using PFNGLGETMULTISAMPLEFVPROC = void(APIENTRY *)(GLenum pname, GLuint index, GLfloat* val);
	static PFNGLGETMULTISAMPLEFVPROC pfnGetMultisamplefv{nullptr};
	LOAD_ENTRYPOINT("glGetMultisamplefv", pfnGetMultisamplefv, PFNGLGETMULTISAMPLEFVPROC);
	pfnGetMultisamplefv(pname, index, val);
}

void glSampleMaski(GLuint maskNumber, GLbitfield mask)
{
	using PFNGLSAMPLEMASKIPROC = void(APIENTRY *)(GLuint maskNumber, GLbitfield mask);
	static PFNGLSAMPLEMASKIPROC pfnSampleMaski{nullptr};
	LOAD_ENTRYPOINT("glSampleMaski", pfnSampleMaski, PFNGLSAMPLEMASKIPROC);
	pfnSampleMaski(maskNumber, mask);
}

//
// GL_VERSION_3_3
//

void glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name)
{
	using PFNGLBINDFRAGDATALOCATIONINDEXEDPROC = void(APIENTRY *)(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name);
	static PFNGLBINDFRAGDATALOCATIONINDEXEDPROC pfnBindFragDataLocationIndexed{nullptr};
	LOAD_ENTRYPOINT("glBindFragDataLocationIndexed", pfnBindFragDataLocationIndexed, PFNGLBINDFRAGDATALOCATIONINDEXEDPROC);
	pfnBindFragDataLocationIndexed(program, colorNumber, index, name);
}

GLint glGetFragDataIndex(GLuint program, const GLchar* name)
{
	using PFNGLGETFRAGDATAINDEXPROC = GLint(APIENTRY *)(GLuint program, const GLchar* name);
	static PFNGLGETFRAGDATAINDEXPROC pfnGetFragDataIndex{nullptr};
	LOAD_ENTRYPOINT("glGetFragDataIndex", pfnGetFragDataIndex, PFNGLGETFRAGDATAINDEXPROC);
	return pfnGetFragDataIndex(program, name);
}

void glGenSamplers(GLsizei count, GLuint* samplers)
{
	using PFNGLGENSAMPLERSPROC = void(APIENTRY *)(GLsizei count, GLuint* samplers);
	static PFNGLGENSAMPLERSPROC pfnGenSamplers{nullptr};
	LOAD_ENTRYPOINT("glGenSamplers", pfnGenSamplers, PFNGLGENSAMPLERSPROC);
	pfnGenSamplers(count, samplers);
}

void glDeleteSamplers(GLsizei count, const GLuint* samplers)
{
	using PFNGLDELETESAMPLERSPROC = void(APIENTRY *)(GLsizei count, const GLuint* samplers);
	static PFNGLDELETESAMPLERSPROC pfnDeleteSamplers{nullptr};
	LOAD_ENTRYPOINT("glDeleteSamplers", pfnDeleteSamplers, PFNGLDELETESAMPLERSPROC);
	pfnDeleteSamplers(count, samplers);
}

GLboolean glIsSampler(GLuint sampler)
{
	using PFNGLISSAMPLERPROC = GLboolean(APIENTRY *)(GLuint sampler);
	static PFNGLISSAMPLERPROC pfnIsSampler{nullptr};
	LOAD_ENTRYPOINT("glIsSampler", pfnIsSampler, PFNGLISSAMPLERPROC);
	return pfnIsSampler(sampler);
}

void glBindSampler(GLuint unit, GLuint sampler)
{
	using PFNGLBINDSAMPLERPROC = void(APIENTRY *)(GLuint unit, GLuint sampler);
	static PFNGLBINDSAMPLERPROC pfnBindSampler{nullptr};
	LOAD_ENTRYPOINT("glBindSampler", pfnBindSampler, PFNGLBINDSAMPLERPROC);
	pfnBindSampler(unit, sampler);
}

void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	using PFNGLSAMPLERPARAMETERIPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, GLint param);
	static PFNGLSAMPLERPARAMETERIPROC pfnSamplerParameteri{nullptr};
	LOAD_ENTRYPOINT("glSamplerParameteri", pfnSamplerParameteri, PFNGLSAMPLERPARAMETERIPROC);
	pfnSamplerParameteri(sampler, pname, param);
}

void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param)
{
	using PFNGLSAMPLERPARAMETERIVPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, const GLint* param);
	static PFNGLSAMPLERPARAMETERIVPROC pfnSamplerParameteriv{nullptr};
	LOAD_ENTRYPOINT("glSamplerParameteriv", pfnSamplerParameteriv, PFNGLSAMPLERPARAMETERIVPROC);
	pfnSamplerParameteriv(sampler, pname, param);
}

void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
	using PFNGLSAMPLERPARAMETERFPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, GLfloat param);
	static PFNGLSAMPLERPARAMETERFPROC pfnSamplerParameterf{nullptr};
	LOAD_ENTRYPOINT("glSamplerParameterf", pfnSamplerParameterf, PFNGLSAMPLERPARAMETERFPROC);
	pfnSamplerParameterf(sampler, pname, param);
}

void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param)
{
	using PFNGLSAMPLERPARAMETERFVPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, const GLfloat* param);
	static PFNGLSAMPLERPARAMETERFVPROC pfnSamplerParameterfv{nullptr};
	LOAD_ENTRYPOINT("glSamplerParameterfv", pfnSamplerParameterfv, PFNGLSAMPLERPARAMETERFVPROC);
	pfnSamplerParameterfv(sampler, pname, param);
}

void glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint* param)
{
	using PFNGLSAMPLERPARAMETERIIVPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, const GLint* param);
	static PFNGLSAMPLERPARAMETERIIVPROC pfnSamplerParameterIiv{nullptr};
	LOAD_ENTRYPOINT("glSamplerParameterIiv", pfnSamplerParameterIiv, PFNGLSAMPLERPARAMETERIIVPROC);
	pfnSamplerParameterIiv(sampler, pname, param);
}

void glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint* param)
{
	using PFNGLSAMPLERPARAMETERIUIVPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, const GLuint* param);
	static PFNGLSAMPLERPARAMETERIUIVPROC pfnSamplerParameterIuiv{nullptr};
	LOAD_ENTRYPOINT("glSamplerParameterIuiv", pfnSamplerParameterIuiv, PFNGLSAMPLERPARAMETERIUIVPROC);
	pfnSamplerParameterIuiv(sampler, pname, param);
}

void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params)
{
	using PFNGLGETSAMPLERPARAMETERIVPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, GLint* params);
	static PFNGLGETSAMPLERPARAMETERIVPROC pfnGetSamplerParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetSamplerParameteriv", pfnGetSamplerParameteriv, PFNGLGETSAMPLERPARAMETERIVPROC);
	pfnGetSamplerParameteriv(sampler, pname, params);
}

void glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint* params)
{
	using PFNGLGETSAMPLERPARAMETERIIVPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, GLint* params);
	static PFNGLGETSAMPLERPARAMETERIIVPROC pfnGetSamplerParameterIiv{nullptr};
	LOAD_ENTRYPOINT("glGetSamplerParameterIiv", pfnGetSamplerParameterIiv, PFNGLGETSAMPLERPARAMETERIIVPROC);
	pfnGetSamplerParameterIiv(sampler, pname, params);
}

void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params)
{
	using PFNGLGETSAMPLERPARAMETERFVPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, GLfloat* params);
	static PFNGLGETSAMPLERPARAMETERFVPROC pfnGetSamplerParameterfv{nullptr};
	LOAD_ENTRYPOINT("glGetSamplerParameterfv", pfnGetSamplerParameterfv, PFNGLGETSAMPLERPARAMETERFVPROC);
	pfnGetSamplerParameterfv(sampler, pname, params);
}

void glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint* params)
{
	using PFNGLGETSAMPLERPARAMETERIUIVPROC = void(APIENTRY *)(GLuint sampler, GLenum pname, GLuint* params);
	static PFNGLGETSAMPLERPARAMETERIUIVPROC pfnGetSamplerParameterIuiv{nullptr};
	LOAD_ENTRYPOINT("glGetSamplerParameterIuiv", pfnGetSamplerParameterIuiv, PFNGLGETSAMPLERPARAMETERIUIVPROC);
	pfnGetSamplerParameterIuiv(sampler, pname, params);
}

void glQueryCounter(GLuint id, GLenum target)
{
	using PFNGLQUERYCOUNTERPROC = void(APIENTRY *)(GLuint id, GLenum target);
	static PFNGLQUERYCOUNTERPROC pfnQueryCounter{nullptr};
	LOAD_ENTRYPOINT("glQueryCounter", pfnQueryCounter, PFNGLQUERYCOUNTERPROC);
	pfnQueryCounter(id, target);
}

void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params)
{
	using PFNGLGETQUERYOBJECTI64VPROC = void(APIENTRY *)(GLuint id, GLenum pname, GLint64* params);
	static PFNGLGETQUERYOBJECTI64VPROC pfnGetQueryObjecti64v{nullptr};
	LOAD_ENTRYPOINT("glGetQueryObjecti64v", pfnGetQueryObjecti64v, PFNGLGETQUERYOBJECTI64VPROC);
	pfnGetQueryObjecti64v(id, pname, params);
}

void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params)
{
	using PFNGLGETQUERYOBJECTUI64VPROC = void(APIENTRY *)(GLuint id, GLenum pname, GLuint64* params);
	static PFNGLGETQUERYOBJECTUI64VPROC pfnGetQueryObjectui64v{nullptr};
	LOAD_ENTRYPOINT("glGetQueryObjectui64v", pfnGetQueryObjectui64v, PFNGLGETQUERYOBJECTUI64VPROC);
	pfnGetQueryObjectui64v(id, pname, params);
}

void glVertexAttribDivisor(GLuint index, GLuint divisor)
{
	using PFNGLVERTEXATTRIBDIVISORPROC = void(APIENTRY *)(GLuint index, GLuint divisor);
	static PFNGLVERTEXATTRIBDIVISORPROC pfnVertexAttribDivisor{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribDivisor", pfnVertexAttribDivisor, PFNGLVERTEXATTRIBDIVISORPROC);
	pfnVertexAttribDivisor(index, divisor);
}

void glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	using PFNGLVERTEXATTRIBP1UIPROC = void(APIENTRY *)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static PFNGLVERTEXATTRIBP1UIPROC pfnVertexAttribP1ui{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribP1ui", pfnVertexAttribP1ui, PFNGLVERTEXATTRIBP1UIPROC);
	pfnVertexAttribP1ui(index, type, normalized, value);
}

void glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
	using PFNGLVERTEXATTRIBP1UIVPROC = void(APIENTRY *)(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
	static PFNGLVERTEXATTRIBP1UIVPROC pfnVertexAttribP1uiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribP1uiv", pfnVertexAttribP1uiv, PFNGLVERTEXATTRIBP1UIVPROC);
	pfnVertexAttribP1uiv(index, type, normalized, value);
}

void glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	using PFNGLVERTEXATTRIBP2UIPROC = void(APIENTRY *)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static PFNGLVERTEXATTRIBP2UIPROC pfnVertexAttribP2ui{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribP2ui", pfnVertexAttribP2ui, PFNGLVERTEXATTRIBP2UIPROC);
	pfnVertexAttribP2ui(index, type, normalized, value);
}

void glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
	using PFNGLVERTEXATTRIBP2UIVPROC = void(APIENTRY *)(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
	static PFNGLVERTEXATTRIBP2UIVPROC pfnVertexAttribP2uiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribP2uiv", pfnVertexAttribP2uiv, PFNGLVERTEXATTRIBP2UIVPROC);
	pfnVertexAttribP2uiv(index, type, normalized, value);
}

void glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	using PFNGLVERTEXATTRIBP3UIPROC = void(APIENTRY *)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static PFNGLVERTEXATTRIBP3UIPROC pfnVertexAttribP3ui{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribP3ui", pfnVertexAttribP3ui, PFNGLVERTEXATTRIBP3UIPROC);
	pfnVertexAttribP3ui(index, type, normalized, value);
}

void glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
	using PFNGLVERTEXATTRIBP3UIVPROC = void(APIENTRY *)(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
	static PFNGLVERTEXATTRIBP3UIVPROC pfnVertexAttribP3uiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribP3uiv", pfnVertexAttribP3uiv, PFNGLVERTEXATTRIBP3UIVPROC);
	pfnVertexAttribP3uiv(index, type, normalized, value);
}

void glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	using PFNGLVERTEXATTRIBP4UIPROC = void(APIENTRY *)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static PFNGLVERTEXATTRIBP4UIPROC pfnVertexAttribP4ui{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribP4ui", pfnVertexAttribP4ui, PFNGLVERTEXATTRIBP4UIPROC);
	pfnVertexAttribP4ui(index, type, normalized, value);
}

void glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value)
{
	using PFNGLVERTEXATTRIBP4UIVPROC = void(APIENTRY *)(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
	static PFNGLVERTEXATTRIBP4UIVPROC pfnVertexAttribP4uiv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribP4uiv", pfnVertexAttribP4uiv, PFNGLVERTEXATTRIBP4UIVPROC);
	pfnVertexAttribP4uiv(index, type, normalized, value);
}

//
// GL_VERSION_4_0
//

void glMinSampleShading(GLfloat value)
{
	using PFNGLMINSAMPLESHADINGPROC = void(APIENTRY *)(GLfloat value);
	static PFNGLMINSAMPLESHADINGPROC pfnMinSampleShading{nullptr};
	LOAD_ENTRYPOINT("glMinSampleShading", pfnMinSampleShading, PFNGLMINSAMPLESHADINGPROC);
	pfnMinSampleShading(value);
}

void glBlendEquationi(GLuint buf, GLenum mode)
{
	using PFNGLBLENDEQUATIONIPROC = void(APIENTRY *)(GLuint buf, GLenum mode);
	static PFNGLBLENDEQUATIONIPROC pfnBlendEquationi{nullptr};
	LOAD_ENTRYPOINT("glBlendEquationi", pfnBlendEquationi, PFNGLBLENDEQUATIONIPROC);
	pfnBlendEquationi(buf, mode);
}

void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
	using PFNGLBLENDEQUATIONSEPARATEIPROC = void(APIENTRY *)(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
	static PFNGLBLENDEQUATIONSEPARATEIPROC pfnBlendEquationSeparatei{nullptr};
	LOAD_ENTRYPOINT("glBlendEquationSeparatei", pfnBlendEquationSeparatei, PFNGLBLENDEQUATIONSEPARATEIPROC);
	pfnBlendEquationSeparatei(buf, modeRGB, modeAlpha);
}

void glBlendFunci(GLuint buf, GLenum src, GLenum dst)
{
	using PFNGLBLENDFUNCIPROC = void(APIENTRY *)(GLuint buf, GLenum src, GLenum dst);
	static PFNGLBLENDFUNCIPROC pfnBlendFunci{nullptr};
	LOAD_ENTRYPOINT("glBlendFunci", pfnBlendFunci, PFNGLBLENDFUNCIPROC);
	pfnBlendFunci(buf, src, dst);
}

void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	using PFNGLBLENDFUNCSEPARATEIPROC = void(APIENTRY *)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
	static PFNGLBLENDFUNCSEPARATEIPROC pfnBlendFuncSeparatei{nullptr};
	LOAD_ENTRYPOINT("glBlendFuncSeparatei", pfnBlendFuncSeparatei, PFNGLBLENDFUNCSEPARATEIPROC);
	pfnBlendFuncSeparatei(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void glDrawArraysIndirect(GLenum mode, const void* indirect)
{
	using PFNGLDRAWARRAYSINDIRECTPROC = void(APIENTRY *)(GLenum mode, const void* indirect);
	static PFNGLDRAWARRAYSINDIRECTPROC pfnDrawArraysIndirect{nullptr};
	LOAD_ENTRYPOINT("glDrawArraysIndirect", pfnDrawArraysIndirect, PFNGLDRAWARRAYSINDIRECTPROC);
	pfnDrawArraysIndirect(mode, indirect);
}

void glDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect)
{
	using PFNGLDRAWELEMENTSINDIRECTPROC = void(APIENTRY *)(GLenum mode, GLenum type, const void* indirect);
	static PFNGLDRAWELEMENTSINDIRECTPROC pfnDrawElementsIndirect{nullptr};
	LOAD_ENTRYPOINT("glDrawElementsIndirect", pfnDrawElementsIndirect, PFNGLDRAWELEMENTSINDIRECTPROC);
	pfnDrawElementsIndirect(mode, type, indirect);
}

void glUniform1d(GLint location, GLdouble x)
{
	using PFNGLUNIFORM1DPROC = void(APIENTRY *)(GLint location, GLdouble x);
	static PFNGLUNIFORM1DPROC pfnUniform1d{nullptr};
	LOAD_ENTRYPOINT("glUniform1d", pfnUniform1d, PFNGLUNIFORM1DPROC);
	pfnUniform1d(location, x);
}

void glUniform2d(GLint location, GLdouble x, GLdouble y)
{
	using PFNGLUNIFORM2DPROC = void(APIENTRY *)(GLint location, GLdouble x, GLdouble y);
	static PFNGLUNIFORM2DPROC pfnUniform2d{nullptr};
	LOAD_ENTRYPOINT("glUniform2d", pfnUniform2d, PFNGLUNIFORM2DPROC);
	pfnUniform2d(location, x, y);
}

void glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	using PFNGLUNIFORM3DPROC = void(APIENTRY *)(GLint location, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLUNIFORM3DPROC pfnUniform3d{nullptr};
	LOAD_ENTRYPOINT("glUniform3d", pfnUniform3d, PFNGLUNIFORM3DPROC);
	pfnUniform3d(location, x, y, z);
}

void glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	using PFNGLUNIFORM4DPROC = void(APIENTRY *)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static PFNGLUNIFORM4DPROC pfnUniform4d{nullptr};
	LOAD_ENTRYPOINT("glUniform4d", pfnUniform4d, PFNGLUNIFORM4DPROC);
	pfnUniform4d(location, x, y, z, w);
}

void glUniform1dv(GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLUNIFORM1DVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLdouble* value);
	static PFNGLUNIFORM1DVPROC pfnUniform1dv{nullptr};
	LOAD_ENTRYPOINT("glUniform1dv", pfnUniform1dv, PFNGLUNIFORM1DVPROC);
	pfnUniform1dv(location, count, value);
}

void glUniform2dv(GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLUNIFORM2DVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLdouble* value);
	static PFNGLUNIFORM2DVPROC pfnUniform2dv{nullptr};
	LOAD_ENTRYPOINT("glUniform2dv", pfnUniform2dv, PFNGLUNIFORM2DVPROC);
	pfnUniform2dv(location, count, value);
}

void glUniform3dv(GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLUNIFORM3DVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLdouble* value);
	static PFNGLUNIFORM3DVPROC pfnUniform3dv{nullptr};
	LOAD_ENTRYPOINT("glUniform3dv", pfnUniform3dv, PFNGLUNIFORM3DVPROC);
	pfnUniform3dv(location, count, value);
}

void glUniform4dv(GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLUNIFORM4DVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLdouble* value);
	static PFNGLUNIFORM4DVPROC pfnUniform4dv{nullptr};
	LOAD_ENTRYPOINT("glUniform4dv", pfnUniform4dv, PFNGLUNIFORM4DVPROC);
	pfnUniform4dv(location, count, value);
}

void glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLUNIFORMMATRIX2DVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLUNIFORMMATRIX2DVPROC pfnUniformMatrix2dv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix2dv", pfnUniformMatrix2dv, PFNGLUNIFORMMATRIX2DVPROC);
	pfnUniformMatrix2dv(location, count, transpose, value);
}

void glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLUNIFORMMATRIX3DVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLUNIFORMMATRIX3DVPROC pfnUniformMatrix3dv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix3dv", pfnUniformMatrix3dv, PFNGLUNIFORMMATRIX3DVPROC);
	pfnUniformMatrix3dv(location, count, transpose, value);
}

void glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLUNIFORMMATRIX4DVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLUNIFORMMATRIX4DVPROC pfnUniformMatrix4dv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix4dv", pfnUniformMatrix4dv, PFNGLUNIFORMMATRIX4DVPROC);
	pfnUniformMatrix4dv(location, count, transpose, value);
}

void glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLUNIFORMMATRIX2X3DVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLUNIFORMMATRIX2X3DVPROC pfnUniformMatrix2x3dv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix2x3dv", pfnUniformMatrix2x3dv, PFNGLUNIFORMMATRIX2X3DVPROC);
	pfnUniformMatrix2x3dv(location, count, transpose, value);
}

void glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLUNIFORMMATRIX2X4DVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLUNIFORMMATRIX2X4DVPROC pfnUniformMatrix2x4dv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix2x4dv", pfnUniformMatrix2x4dv, PFNGLUNIFORMMATRIX2X4DVPROC);
	pfnUniformMatrix2x4dv(location, count, transpose, value);
}

void glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLUNIFORMMATRIX3X2DVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLUNIFORMMATRIX3X2DVPROC pfnUniformMatrix3x2dv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix3x2dv", pfnUniformMatrix3x2dv, PFNGLUNIFORMMATRIX3X2DVPROC);
	pfnUniformMatrix3x2dv(location, count, transpose, value);
}

void glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLUNIFORMMATRIX3X4DVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLUNIFORMMATRIX3X4DVPROC pfnUniformMatrix3x4dv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix3x4dv", pfnUniformMatrix3x4dv, PFNGLUNIFORMMATRIX3X4DVPROC);
	pfnUniformMatrix3x4dv(location, count, transpose, value);
}

void glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLUNIFORMMATRIX4X2DVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLUNIFORMMATRIX4X2DVPROC pfnUniformMatrix4x2dv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix4x2dv", pfnUniformMatrix4x2dv, PFNGLUNIFORMMATRIX4X2DVPROC);
	pfnUniformMatrix4x2dv(location, count, transpose, value);
}

void glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLUNIFORMMATRIX4X3DVPROC = void(APIENTRY *)(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLUNIFORMMATRIX4X3DVPROC pfnUniformMatrix4x3dv{nullptr};
	LOAD_ENTRYPOINT("glUniformMatrix4x3dv", pfnUniformMatrix4x3dv, PFNGLUNIFORMMATRIX4X3DVPROC);
	pfnUniformMatrix4x3dv(location, count, transpose, value);
}

void glGetUniformdv(GLuint program, GLint location, GLdouble* params)
{
	using PFNGLGETUNIFORMDVPROC = void(APIENTRY *)(GLuint program, GLint location, GLdouble* params);
	static PFNGLGETUNIFORMDVPROC pfnGetUniformdv{nullptr};
	LOAD_ENTRYPOINT("glGetUniformdv", pfnGetUniformdv, PFNGLGETUNIFORMDVPROC);
	pfnGetUniformdv(program, location, params);
}

GLint glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar* name)
{
	using PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC = GLint(APIENTRY *)(GLuint program, GLenum shadertype, const GLchar* name);
	static PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC pfnGetSubroutineUniformLocation{nullptr};
	LOAD_ENTRYPOINT("glGetSubroutineUniformLocation", pfnGetSubroutineUniformLocation, PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC);
	return pfnGetSubroutineUniformLocation(program, shadertype, name);
}

GLuint glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar* name)
{
	using PFNGLGETSUBROUTINEINDEXPROC = GLuint(APIENTRY *)(GLuint program, GLenum shadertype, const GLchar* name);
	static PFNGLGETSUBROUTINEINDEXPROC pfnGetSubroutineIndex{nullptr};
	LOAD_ENTRYPOINT("glGetSubroutineIndex", pfnGetSubroutineIndex, PFNGLGETSUBROUTINEINDEXPROC);
	return pfnGetSubroutineIndex(program, shadertype, name);
}

void glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values)
{
	using PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC = void(APIENTRY *)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values);
	static PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC pfnGetActiveSubroutineUniformiv{nullptr};
	LOAD_ENTRYPOINT("glGetActiveSubroutineUniformiv", pfnGetActiveSubroutineUniformiv, PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC);
	pfnGetActiveSubroutineUniformiv(program, shadertype, index, pname, values);
}

void glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)
{
	using PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC = void(APIENTRY *)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name);
	static PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC pfnGetActiveSubroutineUniformName{nullptr};
	LOAD_ENTRYPOINT("glGetActiveSubroutineUniformName", pfnGetActiveSubroutineUniformName, PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC);
	pfnGetActiveSubroutineUniformName(program, shadertype, index, bufSize, length, name);
}

void glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)
{
	using PFNGLGETACTIVESUBROUTINENAMEPROC = void(APIENTRY *)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name);
	static PFNGLGETACTIVESUBROUTINENAMEPROC pfnGetActiveSubroutineName{nullptr};
	LOAD_ENTRYPOINT("glGetActiveSubroutineName", pfnGetActiveSubroutineName, PFNGLGETACTIVESUBROUTINENAMEPROC);
	pfnGetActiveSubroutineName(program, shadertype, index, bufSize, length, name);
}

void glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint* indices)
{
	using PFNGLUNIFORMSUBROUTINESUIVPROC = void(APIENTRY *)(GLenum shadertype, GLsizei count, const GLuint* indices);
	static PFNGLUNIFORMSUBROUTINESUIVPROC pfnUniformSubroutinesuiv{nullptr};
	LOAD_ENTRYPOINT("glUniformSubroutinesuiv", pfnUniformSubroutinesuiv, PFNGLUNIFORMSUBROUTINESUIVPROC);
	pfnUniformSubroutinesuiv(shadertype, count, indices);
}

void glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint* params)
{
	using PFNGLGETUNIFORMSUBROUTINEUIVPROC = void(APIENTRY *)(GLenum shadertype, GLint location, GLuint* params);
	static PFNGLGETUNIFORMSUBROUTINEUIVPROC pfnGetUniformSubroutineuiv{nullptr};
	LOAD_ENTRYPOINT("glGetUniformSubroutineuiv", pfnGetUniformSubroutineuiv, PFNGLGETUNIFORMSUBROUTINEUIVPROC);
	pfnGetUniformSubroutineuiv(shadertype, location, params);
}

void glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint* values)
{
	using PFNGLGETPROGRAMSTAGEIVPROC = void(APIENTRY *)(GLuint program, GLenum shadertype, GLenum pname, GLint* values);
	static PFNGLGETPROGRAMSTAGEIVPROC pfnGetProgramStageiv{nullptr};
	LOAD_ENTRYPOINT("glGetProgramStageiv", pfnGetProgramStageiv, PFNGLGETPROGRAMSTAGEIVPROC);
	pfnGetProgramStageiv(program, shadertype, pname, values);
}

void glPatchParameteri(GLenum pname, GLint value)
{
	using PFNGLPATCHPARAMETERIPROC = void(APIENTRY *)(GLenum pname, GLint value);
	static PFNGLPATCHPARAMETERIPROC pfnPatchParameteri{nullptr};
	LOAD_ENTRYPOINT("glPatchParameteri", pfnPatchParameteri, PFNGLPATCHPARAMETERIPROC);
	pfnPatchParameteri(pname, value);
}

void glPatchParameterfv(GLenum pname, const GLfloat* values)
{
	using PFNGLPATCHPARAMETERFVPROC = void(APIENTRY *)(GLenum pname, const GLfloat* values);
	static PFNGLPATCHPARAMETERFVPROC pfnPatchParameterfv{nullptr};
	LOAD_ENTRYPOINT("glPatchParameterfv", pfnPatchParameterfv, PFNGLPATCHPARAMETERFVPROC);
	pfnPatchParameterfv(pname, values);
}

void glBindTransformFeedback(GLenum target, GLuint id)
{
	using PFNGLBINDTRANSFORMFEEDBACKPROC = void(APIENTRY *)(GLenum target, GLuint id);
	static PFNGLBINDTRANSFORMFEEDBACKPROC pfnBindTransformFeedback{nullptr};
	LOAD_ENTRYPOINT("glBindTransformFeedback", pfnBindTransformFeedback, PFNGLBINDTRANSFORMFEEDBACKPROC);
	pfnBindTransformFeedback(target, id);
}

void glDeleteTransformFeedbacks(GLsizei n, const GLuint* ids)
{
	using PFNGLDELETETRANSFORMFEEDBACKSPROC = void(APIENTRY *)(GLsizei n, const GLuint* ids);
	static PFNGLDELETETRANSFORMFEEDBACKSPROC pfnDeleteTransformFeedbacks{nullptr};
	LOAD_ENTRYPOINT("glDeleteTransformFeedbacks", pfnDeleteTransformFeedbacks, PFNGLDELETETRANSFORMFEEDBACKSPROC);
	pfnDeleteTransformFeedbacks(n, ids);
}

void glGenTransformFeedbacks(GLsizei n, GLuint* ids)
{
	using PFNGLGENTRANSFORMFEEDBACKSPROC = void(APIENTRY *)(GLsizei n, GLuint* ids);
	static PFNGLGENTRANSFORMFEEDBACKSPROC pfnGenTransformFeedbacks{nullptr};
	LOAD_ENTRYPOINT("glGenTransformFeedbacks", pfnGenTransformFeedbacks, PFNGLGENTRANSFORMFEEDBACKSPROC);
	pfnGenTransformFeedbacks(n, ids);
}

GLboolean glIsTransformFeedback(GLuint id)
{
	using PFNGLISTRANSFORMFEEDBACKPROC = GLboolean(APIENTRY *)(GLuint id);
	static PFNGLISTRANSFORMFEEDBACKPROC pfnIsTransformFeedback{nullptr};
	LOAD_ENTRYPOINT("glIsTransformFeedback", pfnIsTransformFeedback, PFNGLISTRANSFORMFEEDBACKPROC);
	return pfnIsTransformFeedback(id);
}

void glPauseTransformFeedback(void)
{
	using PFNGLPAUSETRANSFORMFEEDBACKPROC = void(APIENTRY *)(void);
	static PFNGLPAUSETRANSFORMFEEDBACKPROC pfnPauseTransformFeedback{nullptr};
	LOAD_ENTRYPOINT("glPauseTransformFeedback", pfnPauseTransformFeedback, PFNGLPAUSETRANSFORMFEEDBACKPROC);
	pfnPauseTransformFeedback();
}

void glResumeTransformFeedback(void)
{
	using PFNGLRESUMETRANSFORMFEEDBACKPROC = void(APIENTRY *)(void);
	static PFNGLRESUMETRANSFORMFEEDBACKPROC pfnResumeTransformFeedback{nullptr};
	LOAD_ENTRYPOINT("glResumeTransformFeedback", pfnResumeTransformFeedback, PFNGLRESUMETRANSFORMFEEDBACKPROC);
	pfnResumeTransformFeedback();
}

void glDrawTransformFeedback(GLenum mode, GLuint id)
{
	using PFNGLDRAWTRANSFORMFEEDBACKPROC = void(APIENTRY *)(GLenum mode, GLuint id);
	static PFNGLDRAWTRANSFORMFEEDBACKPROC pfnDrawTransformFeedback{nullptr};
	LOAD_ENTRYPOINT("glDrawTransformFeedback", pfnDrawTransformFeedback, PFNGLDRAWTRANSFORMFEEDBACKPROC);
	pfnDrawTransformFeedback(mode, id);
}

void glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream)
{
	using PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC = void(APIENTRY *)(GLenum mode, GLuint id, GLuint stream);
	static PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC pfnDrawTransformFeedbackStream{nullptr};
	LOAD_ENTRYPOINT("glDrawTransformFeedbackStream", pfnDrawTransformFeedbackStream, PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC);
	pfnDrawTransformFeedbackStream(mode, id, stream);
}

void glBeginQueryIndexed(GLenum target, GLuint index, GLuint id)
{
	using PFNGLBEGINQUERYINDEXEDPROC = void(APIENTRY *)(GLenum target, GLuint index, GLuint id);
	static PFNGLBEGINQUERYINDEXEDPROC pfnBeginQueryIndexed{nullptr};
	LOAD_ENTRYPOINT("glBeginQueryIndexed", pfnBeginQueryIndexed, PFNGLBEGINQUERYINDEXEDPROC);
	pfnBeginQueryIndexed(target, index, id);
}

void glEndQueryIndexed(GLenum target, GLuint index)
{
	using PFNGLENDQUERYINDEXEDPROC = void(APIENTRY *)(GLenum target, GLuint index);
	static PFNGLENDQUERYINDEXEDPROC pfnEndQueryIndexed{nullptr};
	LOAD_ENTRYPOINT("glEndQueryIndexed", pfnEndQueryIndexed, PFNGLENDQUERYINDEXEDPROC);
	pfnEndQueryIndexed(target, index);
}

void glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint* params)
{
	using PFNGLGETQUERYINDEXEDIVPROC = void(APIENTRY *)(GLenum target, GLuint index, GLenum pname, GLint* params);
	static PFNGLGETQUERYINDEXEDIVPROC pfnGetQueryIndexediv{nullptr};
	LOAD_ENTRYPOINT("glGetQueryIndexediv", pfnGetQueryIndexediv, PFNGLGETQUERYINDEXEDIVPROC);
	pfnGetQueryIndexediv(target, index, pname, params);
}

//
// GL_VERSION_4_1
//

void glReleaseShaderCompiler(void)
{
	using PFNGLRELEASESHADERCOMPILERPROC = void(APIENTRY *)(void);
	static PFNGLRELEASESHADERCOMPILERPROC pfnReleaseShaderCompiler{nullptr};
	LOAD_ENTRYPOINT("glReleaseShaderCompiler", pfnReleaseShaderCompiler, PFNGLRELEASESHADERCOMPILERPROC);
	pfnReleaseShaderCompiler();
}

void glShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryFormat, const void* binary, GLsizei length)
{
	using PFNGLSHADERBINARYPROC = void(APIENTRY *)(GLsizei count, const GLuint* shaders, GLenum binaryFormat, const void* binary, GLsizei length);
	static PFNGLSHADERBINARYPROC pfnShaderBinary{nullptr};
	LOAD_ENTRYPOINT("glShaderBinary", pfnShaderBinary, PFNGLSHADERBINARYPROC);
	pfnShaderBinary(count, shaders, binaryFormat, binary, length);
}

void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
	using PFNGLGETSHADERPRECISIONFORMATPROC = void(APIENTRY *)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
	static PFNGLGETSHADERPRECISIONFORMATPROC pfnGetShaderPrecisionFormat{nullptr};
	LOAD_ENTRYPOINT("glGetShaderPrecisionFormat", pfnGetShaderPrecisionFormat, PFNGLGETSHADERPRECISIONFORMATPROC);
	pfnGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

void glDepthRangef(GLfloat n, GLfloat f)
{
	using PFNGLDEPTHRANGEFPROC = void(APIENTRY *)(GLfloat n, GLfloat f);
	static PFNGLDEPTHRANGEFPROC pfnDepthRangef{nullptr};
	LOAD_ENTRYPOINT("glDepthRangef", pfnDepthRangef, PFNGLDEPTHRANGEFPROC);
	pfnDepthRangef(n, f);
}

void glClearDepthf(GLfloat d)
{
	using PFNGLCLEARDEPTHFPROC = void(APIENTRY *)(GLfloat d);
	static PFNGLCLEARDEPTHFPROC pfnClearDepthf{nullptr};
	LOAD_ENTRYPOINT("glClearDepthf", pfnClearDepthf, PFNGLCLEARDEPTHFPROC);
	pfnClearDepthf(d);
}

void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary)
{
	using PFNGLGETPROGRAMBINARYPROC = void(APIENTRY *)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary);
	static PFNGLGETPROGRAMBINARYPROC pfnGetProgramBinary{nullptr};
	LOAD_ENTRYPOINT("glGetProgramBinary", pfnGetProgramBinary, PFNGLGETPROGRAMBINARYPROC);
	pfnGetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

void glProgramBinary(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length)
{
	using PFNGLPROGRAMBINARYPROC = void(APIENTRY *)(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length);
	static PFNGLPROGRAMBINARYPROC pfnProgramBinary{nullptr};
	LOAD_ENTRYPOINT("glProgramBinary", pfnProgramBinary, PFNGLPROGRAMBINARYPROC);
	pfnProgramBinary(program, binaryFormat, binary, length);
}

void glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
	using PFNGLPROGRAMPARAMETERIPROC = void(APIENTRY *)(GLuint program, GLenum pname, GLint value);
	static PFNGLPROGRAMPARAMETERIPROC pfnProgramParameteri{nullptr};
	LOAD_ENTRYPOINT("glProgramParameteri", pfnProgramParameteri, PFNGLPROGRAMPARAMETERIPROC);
	pfnProgramParameteri(program, pname, value);
}

void glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
	using PFNGLUSEPROGRAMSTAGESPROC = void(APIENTRY *)(GLuint pipeline, GLbitfield stages, GLuint program);
	static PFNGLUSEPROGRAMSTAGESPROC pfnUseProgramStages{nullptr};
	LOAD_ENTRYPOINT("glUseProgramStages", pfnUseProgramStages, PFNGLUSEPROGRAMSTAGESPROC);
	pfnUseProgramStages(pipeline, stages, program);
}

void glActiveShaderProgram(GLuint pipeline, GLuint program)
{
	using PFNGLACTIVESHADERPROGRAMPROC = void(APIENTRY *)(GLuint pipeline, GLuint program);
	static PFNGLACTIVESHADERPROGRAMPROC pfnActiveShaderProgram{nullptr};
	LOAD_ENTRYPOINT("glActiveShaderProgram", pfnActiveShaderProgram, PFNGLACTIVESHADERPROGRAMPROC);
	pfnActiveShaderProgram(pipeline, program);
}

GLuint glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar* const* strings)
{
	using PFNGLCREATESHADERPROGRAMVPROC = GLuint(APIENTRY *)(GLenum type, GLsizei count, const GLchar* const* strings);
	static PFNGLCREATESHADERPROGRAMVPROC pfnCreateShaderProgramv{nullptr};
	LOAD_ENTRYPOINT("glCreateShaderProgramv", pfnCreateShaderProgramv, PFNGLCREATESHADERPROGRAMVPROC);
	return pfnCreateShaderProgramv(type, count, strings);
}

void glBindProgramPipeline(GLuint pipeline)
{
	using PFNGLBINDPROGRAMPIPELINEPROC = void(APIENTRY *)(GLuint pipeline);
	static PFNGLBINDPROGRAMPIPELINEPROC pfnBindProgramPipeline{nullptr};
	LOAD_ENTRYPOINT("glBindProgramPipeline", pfnBindProgramPipeline, PFNGLBINDPROGRAMPIPELINEPROC);
	pfnBindProgramPipeline(pipeline);
}

void glDeleteProgramPipelines(GLsizei n, const GLuint* pipelines)
{
	using PFNGLDELETEPROGRAMPIPELINESPROC = void(APIENTRY *)(GLsizei n, const GLuint* pipelines);
	static PFNGLDELETEPROGRAMPIPELINESPROC pfnDeleteProgramPipelines{nullptr};
	LOAD_ENTRYPOINT("glDeleteProgramPipelines", pfnDeleteProgramPipelines, PFNGLDELETEPROGRAMPIPELINESPROC);
	pfnDeleteProgramPipelines(n, pipelines);
}

void glGenProgramPipelines(GLsizei n, GLuint* pipelines)
{
	using PFNGLGENPROGRAMPIPELINESPROC = void(APIENTRY *)(GLsizei n, GLuint* pipelines);
	static PFNGLGENPROGRAMPIPELINESPROC pfnGenProgramPipelines{nullptr};
	LOAD_ENTRYPOINT("glGenProgramPipelines", pfnGenProgramPipelines, PFNGLGENPROGRAMPIPELINESPROC);
	pfnGenProgramPipelines(n, pipelines);
}

GLboolean glIsProgramPipeline(GLuint pipeline)
{
	using PFNGLISPROGRAMPIPELINEPROC = GLboolean(APIENTRY *)(GLuint pipeline);
	static PFNGLISPROGRAMPIPELINEPROC pfnIsProgramPipeline{nullptr};
	LOAD_ENTRYPOINT("glIsProgramPipeline", pfnIsProgramPipeline, PFNGLISPROGRAMPIPELINEPROC);
	return pfnIsProgramPipeline(pipeline);
}

void glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint* params)
{
	using PFNGLGETPROGRAMPIPELINEIVPROC = void(APIENTRY *)(GLuint pipeline, GLenum pname, GLint* params);
	static PFNGLGETPROGRAMPIPELINEIVPROC pfnGetProgramPipelineiv{nullptr};
	LOAD_ENTRYPOINT("glGetProgramPipelineiv", pfnGetProgramPipelineiv, PFNGLGETPROGRAMPIPELINEIVPROC);
	pfnGetProgramPipelineiv(pipeline, pname, params);
}

void glProgramUniform1i(GLuint program, GLint location, GLint v0)
{
	using PFNGLPROGRAMUNIFORM1IPROC = void(APIENTRY *)(GLuint program, GLint location, GLint v0);
	static PFNGLPROGRAMUNIFORM1IPROC pfnProgramUniform1i{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1i", pfnProgramUniform1i, PFNGLPROGRAMUNIFORM1IPROC);
	pfnProgramUniform1i(program, location, v0);
}

void glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	using PFNGLPROGRAMUNIFORM1IVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint* value);
	static PFNGLPROGRAMUNIFORM1IVPROC pfnProgramUniform1iv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1iv", pfnProgramUniform1iv, PFNGLPROGRAMUNIFORM1IVPROC);
	pfnProgramUniform1iv(program, location, count, value);
}

void glProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
	using PFNGLPROGRAMUNIFORM1FPROC = void(APIENTRY *)(GLuint program, GLint location, GLfloat v0);
	static PFNGLPROGRAMUNIFORM1FPROC pfnProgramUniform1f{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1f", pfnProgramUniform1f, PFNGLPROGRAMUNIFORM1FPROC);
	pfnProgramUniform1f(program, location, v0);
}

void glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORM1FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static PFNGLPROGRAMUNIFORM1FVPROC pfnProgramUniform1fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1fv", pfnProgramUniform1fv, PFNGLPROGRAMUNIFORM1FVPROC);
	pfnProgramUniform1fv(program, location, count, value);
}

void glProgramUniform1d(GLuint program, GLint location, GLdouble v0)
{
	using PFNGLPROGRAMUNIFORM1DPROC = void(APIENTRY *)(GLuint program, GLint location, GLdouble v0);
	static PFNGLPROGRAMUNIFORM1DPROC pfnProgramUniform1d{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1d", pfnProgramUniform1d, PFNGLPROGRAMUNIFORM1DPROC);
	pfnProgramUniform1d(program, location, v0);
}

void glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORM1DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
	static PFNGLPROGRAMUNIFORM1DVPROC pfnProgramUniform1dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1dv", pfnProgramUniform1dv, PFNGLPROGRAMUNIFORM1DVPROC);
	pfnProgramUniform1dv(program, location, count, value);
}

void glProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
	using PFNGLPROGRAMUNIFORM1UIPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint v0);
	static PFNGLPROGRAMUNIFORM1UIPROC pfnProgramUniform1ui{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1ui", pfnProgramUniform1ui, PFNGLPROGRAMUNIFORM1UIPROC);
	pfnProgramUniform1ui(program, location, v0);
}

void glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLPROGRAMUNIFORM1UIVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint* value);
	static PFNGLPROGRAMUNIFORM1UIVPROC pfnProgramUniform1uiv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1uiv", pfnProgramUniform1uiv, PFNGLPROGRAMUNIFORM1UIVPROC);
	pfnProgramUniform1uiv(program, location, count, value);
}

void glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
	using PFNGLPROGRAMUNIFORM2IPROC = void(APIENTRY *)(GLuint program, GLint location, GLint v0, GLint v1);
	static PFNGLPROGRAMUNIFORM2IPROC pfnProgramUniform2i{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2i", pfnProgramUniform2i, PFNGLPROGRAMUNIFORM2IPROC);
	pfnProgramUniform2i(program, location, v0, v1);
}

void glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	using PFNGLPROGRAMUNIFORM2IVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint* value);
	static PFNGLPROGRAMUNIFORM2IVPROC pfnProgramUniform2iv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2iv", pfnProgramUniform2iv, PFNGLPROGRAMUNIFORM2IVPROC);
	pfnProgramUniform2iv(program, location, count, value);
}

void glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
	using PFNGLPROGRAMUNIFORM2FPROC = void(APIENTRY *)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
	static PFNGLPROGRAMUNIFORM2FPROC pfnProgramUniform2f{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2f", pfnProgramUniform2f, PFNGLPROGRAMUNIFORM2FPROC);
	pfnProgramUniform2f(program, location, v0, v1);
}

void glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORM2FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static PFNGLPROGRAMUNIFORM2FVPROC pfnProgramUniform2fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2fv", pfnProgramUniform2fv, PFNGLPROGRAMUNIFORM2FVPROC);
	pfnProgramUniform2fv(program, location, count, value);
}

void glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
	using PFNGLPROGRAMUNIFORM2DPROC = void(APIENTRY *)(GLuint program, GLint location, GLdouble v0, GLdouble v1);
	static PFNGLPROGRAMUNIFORM2DPROC pfnProgramUniform2d{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2d", pfnProgramUniform2d, PFNGLPROGRAMUNIFORM2DPROC);
	pfnProgramUniform2d(program, location, v0, v1);
}

void glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORM2DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
	static PFNGLPROGRAMUNIFORM2DVPROC pfnProgramUniform2dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2dv", pfnProgramUniform2dv, PFNGLPROGRAMUNIFORM2DVPROC);
	pfnProgramUniform2dv(program, location, count, value);
}

void glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
	using PFNGLPROGRAMUNIFORM2UIPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint v0, GLuint v1);
	static PFNGLPROGRAMUNIFORM2UIPROC pfnProgramUniform2ui{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2ui", pfnProgramUniform2ui, PFNGLPROGRAMUNIFORM2UIPROC);
	pfnProgramUniform2ui(program, location, v0, v1);
}

void glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLPROGRAMUNIFORM2UIVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint* value);
	static PFNGLPROGRAMUNIFORM2UIVPROC pfnProgramUniform2uiv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2uiv", pfnProgramUniform2uiv, PFNGLPROGRAMUNIFORM2UIVPROC);
	pfnProgramUniform2uiv(program, location, count, value);
}

void glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
	using PFNGLPROGRAMUNIFORM3IPROC = void(APIENTRY *)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
	static PFNGLPROGRAMUNIFORM3IPROC pfnProgramUniform3i{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3i", pfnProgramUniform3i, PFNGLPROGRAMUNIFORM3IPROC);
	pfnProgramUniform3i(program, location, v0, v1, v2);
}

void glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	using PFNGLPROGRAMUNIFORM3IVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint* value);
	static PFNGLPROGRAMUNIFORM3IVPROC pfnProgramUniform3iv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3iv", pfnProgramUniform3iv, PFNGLPROGRAMUNIFORM3IVPROC);
	pfnProgramUniform3iv(program, location, count, value);
}

void glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	using PFNGLPROGRAMUNIFORM3FPROC = void(APIENTRY *)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	static PFNGLPROGRAMUNIFORM3FPROC pfnProgramUniform3f{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3f", pfnProgramUniform3f, PFNGLPROGRAMUNIFORM3FPROC);
	pfnProgramUniform3f(program, location, v0, v1, v2);
}

void glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORM3FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static PFNGLPROGRAMUNIFORM3FVPROC pfnProgramUniform3fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3fv", pfnProgramUniform3fv, PFNGLPROGRAMUNIFORM3FVPROC);
	pfnProgramUniform3fv(program, location, count, value);
}

void glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
	using PFNGLPROGRAMUNIFORM3DPROC = void(APIENTRY *)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
	static PFNGLPROGRAMUNIFORM3DPROC pfnProgramUniform3d{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3d", pfnProgramUniform3d, PFNGLPROGRAMUNIFORM3DPROC);
	pfnProgramUniform3d(program, location, v0, v1, v2);
}

void glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORM3DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
	static PFNGLPROGRAMUNIFORM3DVPROC pfnProgramUniform3dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3dv", pfnProgramUniform3dv, PFNGLPROGRAMUNIFORM3DVPROC);
	pfnProgramUniform3dv(program, location, count, value);
}

void glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	using PFNGLPROGRAMUNIFORM3UIPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
	static PFNGLPROGRAMUNIFORM3UIPROC pfnProgramUniform3ui{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3ui", pfnProgramUniform3ui, PFNGLPROGRAMUNIFORM3UIPROC);
	pfnProgramUniform3ui(program, location, v0, v1, v2);
}

void glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLPROGRAMUNIFORM3UIVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint* value);
	static PFNGLPROGRAMUNIFORM3UIVPROC pfnProgramUniform3uiv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3uiv", pfnProgramUniform3uiv, PFNGLPROGRAMUNIFORM3UIVPROC);
	pfnProgramUniform3uiv(program, location, count, value);
}

void glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	using PFNGLPROGRAMUNIFORM4IPROC = void(APIENTRY *)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	static PFNGLPROGRAMUNIFORM4IPROC pfnProgramUniform4i{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4i", pfnProgramUniform4i, PFNGLPROGRAMUNIFORM4IPROC);
	pfnProgramUniform4i(program, location, v0, v1, v2, v3);
}

void glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	using PFNGLPROGRAMUNIFORM4IVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint* value);
	static PFNGLPROGRAMUNIFORM4IVPROC pfnProgramUniform4iv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4iv", pfnProgramUniform4iv, PFNGLPROGRAMUNIFORM4IVPROC);
	pfnProgramUniform4iv(program, location, count, value);
}

void glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	using PFNGLPROGRAMUNIFORM4FPROC = void(APIENTRY *)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	static PFNGLPROGRAMUNIFORM4FPROC pfnProgramUniform4f{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4f", pfnProgramUniform4f, PFNGLPROGRAMUNIFORM4FPROC);
	pfnProgramUniform4f(program, location, v0, v1, v2, v3);
}

void glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORM4FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static PFNGLPROGRAMUNIFORM4FVPROC pfnProgramUniform4fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4fv", pfnProgramUniform4fv, PFNGLPROGRAMUNIFORM4FVPROC);
	pfnProgramUniform4fv(program, location, count, value);
}

void glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
	using PFNGLPROGRAMUNIFORM4DPROC = void(APIENTRY *)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
	static PFNGLPROGRAMUNIFORM4DPROC pfnProgramUniform4d{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4d", pfnProgramUniform4d, PFNGLPROGRAMUNIFORM4DPROC);
	pfnProgramUniform4d(program, location, v0, v1, v2, v3);
}

void glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORM4DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
	static PFNGLPROGRAMUNIFORM4DVPROC pfnProgramUniform4dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4dv", pfnProgramUniform4dv, PFNGLPROGRAMUNIFORM4DVPROC);
	pfnProgramUniform4dv(program, location, count, value);
}

void glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	using PFNGLPROGRAMUNIFORM4UIPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static PFNGLPROGRAMUNIFORM4UIPROC pfnProgramUniform4ui{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4ui", pfnProgramUniform4ui, PFNGLPROGRAMUNIFORM4UIPROC);
	pfnProgramUniform4ui(program, location, v0, v1, v2, v3);
}

void glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLPROGRAMUNIFORM4UIVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint* value);
	static PFNGLPROGRAMUNIFORM4UIVPROC pfnProgramUniform4uiv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4uiv", pfnProgramUniform4uiv, PFNGLPROGRAMUNIFORM4UIVPROC);
	pfnProgramUniform4uiv(program, location, count, value);
}

void glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX2FVPROC pfnProgramUniformMatrix2fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2fv", pfnProgramUniformMatrix2fv, PFNGLPROGRAMUNIFORMMATRIX2FVPROC);
	pfnProgramUniformMatrix2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX3FVPROC pfnProgramUniformMatrix3fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3fv", pfnProgramUniformMatrix3fv, PFNGLPROGRAMUNIFORMMATRIX3FVPROC);
	pfnProgramUniformMatrix3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX4FVPROC pfnProgramUniformMatrix4fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4fv", pfnProgramUniformMatrix4fv, PFNGLPROGRAMUNIFORMMATRIX4FVPROC);
	pfnProgramUniformMatrix4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX2DVPROC pfnProgramUniformMatrix2dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2dv", pfnProgramUniformMatrix2dv, PFNGLPROGRAMUNIFORMMATRIX2DVPROC);
	pfnProgramUniformMatrix2dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX3DVPROC pfnProgramUniformMatrix3dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3dv", pfnProgramUniformMatrix3dv, PFNGLPROGRAMUNIFORMMATRIX3DVPROC);
	pfnProgramUniformMatrix3dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX4DVPROC pfnProgramUniformMatrix4dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4dv", pfnProgramUniformMatrix4dv, PFNGLPROGRAMUNIFORMMATRIX4DVPROC);
	pfnProgramUniformMatrix4dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC pfnProgramUniformMatrix2x3fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x3fv", pfnProgramUniformMatrix2x3fv, PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC);
	pfnProgramUniformMatrix2x3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC pfnProgramUniformMatrix3x2fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x2fv", pfnProgramUniformMatrix3x2fv, PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC);
	pfnProgramUniformMatrix3x2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC pfnProgramUniformMatrix2x4fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x4fv", pfnProgramUniformMatrix2x4fv, PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC);
	pfnProgramUniformMatrix2x4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC pfnProgramUniformMatrix4x2fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x2fv", pfnProgramUniformMatrix4x2fv, PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC);
	pfnProgramUniformMatrix4x2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC pfnProgramUniformMatrix3x4fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x4fv", pfnProgramUniformMatrix3x4fv, PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC);
	pfnProgramUniformMatrix3x4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC pfnProgramUniformMatrix4x3fv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x3fv", pfnProgramUniformMatrix4x3fv, PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC);
	pfnProgramUniformMatrix4x3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC pfnProgramUniformMatrix2x3dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x3dv", pfnProgramUniformMatrix2x3dv, PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC);
	pfnProgramUniformMatrix2x3dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC pfnProgramUniformMatrix3x2dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x2dv", pfnProgramUniformMatrix3x2dv, PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC);
	pfnProgramUniformMatrix3x2dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC pfnProgramUniformMatrix2x4dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x4dv", pfnProgramUniformMatrix2x4dv, PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC);
	pfnProgramUniformMatrix2x4dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC pfnProgramUniformMatrix4x2dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x2dv", pfnProgramUniformMatrix4x2dv, PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC);
	pfnProgramUniformMatrix4x2dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC pfnProgramUniformMatrix3x4dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x4dv", pfnProgramUniformMatrix3x4dv, PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC);
	pfnProgramUniformMatrix3x4dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC pfnProgramUniformMatrix4x3dv{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x3dv", pfnProgramUniformMatrix4x3dv, PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC);
	pfnProgramUniformMatrix4x3dv(program, location, count, transpose, value);
}

void glValidateProgramPipeline(GLuint pipeline)
{
	using PFNGLVALIDATEPROGRAMPIPELINEPROC = void(APIENTRY *)(GLuint pipeline);
	static PFNGLVALIDATEPROGRAMPIPELINEPROC pfnValidateProgramPipeline{nullptr};
	LOAD_ENTRYPOINT("glValidateProgramPipeline", pfnValidateProgramPipeline, PFNGLVALIDATEPROGRAMPIPELINEPROC);
	pfnValidateProgramPipeline(pipeline);
}

void glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
	using PFNGLGETPROGRAMPIPELINEINFOLOGPROC = void(APIENTRY *)(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
	static PFNGLGETPROGRAMPIPELINEINFOLOGPROC pfnGetProgramPipelineInfoLog{nullptr};
	LOAD_ENTRYPOINT("glGetProgramPipelineInfoLog", pfnGetProgramPipelineInfoLog, PFNGLGETPROGRAMPIPELINEINFOLOGPROC);
	pfnGetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
}

void glVertexAttribL1d(GLuint index, GLdouble x)
{
	using PFNGLVERTEXATTRIBL1DPROC = void(APIENTRY *)(GLuint index, GLdouble x);
	static PFNGLVERTEXATTRIBL1DPROC pfnVertexAttribL1d{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL1d", pfnVertexAttribL1d, PFNGLVERTEXATTRIBL1DPROC);
	pfnVertexAttribL1d(index, x);
}

void glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y)
{
	using PFNGLVERTEXATTRIBL2DPROC = void(APIENTRY *)(GLuint index, GLdouble x, GLdouble y);
	static PFNGLVERTEXATTRIBL2DPROC pfnVertexAttribL2d{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL2d", pfnVertexAttribL2d, PFNGLVERTEXATTRIBL2DPROC);
	pfnVertexAttribL2d(index, x, y);
}

void glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	using PFNGLVERTEXATTRIBL3DPROC = void(APIENTRY *)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLVERTEXATTRIBL3DPROC pfnVertexAttribL3d{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL3d", pfnVertexAttribL3d, PFNGLVERTEXATTRIBL3DPROC);
	pfnVertexAttribL3d(index, x, y, z);
}

void glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	using PFNGLVERTEXATTRIBL4DPROC = void(APIENTRY *)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static PFNGLVERTEXATTRIBL4DPROC pfnVertexAttribL4d{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL4d", pfnVertexAttribL4d, PFNGLVERTEXATTRIBL4DPROC);
	pfnVertexAttribL4d(index, x, y, z, w);
}

void glVertexAttribL1dv(GLuint index, const GLdouble* v)
{
	using PFNGLVERTEXATTRIBL1DVPROC = void(APIENTRY *)(GLuint index, const GLdouble* v);
	static PFNGLVERTEXATTRIBL1DVPROC pfnVertexAttribL1dv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL1dv", pfnVertexAttribL1dv, PFNGLVERTEXATTRIBL1DVPROC);
	pfnVertexAttribL1dv(index, v);
}

void glVertexAttribL2dv(GLuint index, const GLdouble* v)
{
	using PFNGLVERTEXATTRIBL2DVPROC = void(APIENTRY *)(GLuint index, const GLdouble* v);
	static PFNGLVERTEXATTRIBL2DVPROC pfnVertexAttribL2dv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL2dv", pfnVertexAttribL2dv, PFNGLVERTEXATTRIBL2DVPROC);
	pfnVertexAttribL2dv(index, v);
}

void glVertexAttribL3dv(GLuint index, const GLdouble* v)
{
	using PFNGLVERTEXATTRIBL3DVPROC = void(APIENTRY *)(GLuint index, const GLdouble* v);
	static PFNGLVERTEXATTRIBL3DVPROC pfnVertexAttribL3dv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL3dv", pfnVertexAttribL3dv, PFNGLVERTEXATTRIBL3DVPROC);
	pfnVertexAttribL3dv(index, v);
}

void glVertexAttribL4dv(GLuint index, const GLdouble* v)
{
	using PFNGLVERTEXATTRIBL4DVPROC = void(APIENTRY *)(GLuint index, const GLdouble* v);
	static PFNGLVERTEXATTRIBL4DVPROC pfnVertexAttribL4dv{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL4dv", pfnVertexAttribL4dv, PFNGLVERTEXATTRIBL4DVPROC);
	pfnVertexAttribL4dv(index, v);
}

void glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	using PFNGLVERTEXATTRIBLPOINTERPROC = void(APIENTRY *)(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
	static PFNGLVERTEXATTRIBLPOINTERPROC pfnVertexAttribLPointer{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribLPointer", pfnVertexAttribLPointer, PFNGLVERTEXATTRIBLPOINTERPROC);
	pfnVertexAttribLPointer(index, size, type, stride, pointer);
}

void glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble* params)
{
	using PFNGLGETVERTEXATTRIBLDVPROC = void(APIENTRY *)(GLuint index, GLenum pname, GLdouble* params);
	static PFNGLGETVERTEXATTRIBLDVPROC pfnGetVertexAttribLdv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribLdv", pfnGetVertexAttribLdv, PFNGLGETVERTEXATTRIBLDVPROC);
	pfnGetVertexAttribLdv(index, pname, params);
}

void glViewportArrayv(GLuint first, GLsizei count, const GLfloat* v)
{
	using PFNGLVIEWPORTARRAYVPROC = void(APIENTRY *)(GLuint first, GLsizei count, const GLfloat* v);
	static PFNGLVIEWPORTARRAYVPROC pfnViewportArrayv{nullptr};
	LOAD_ENTRYPOINT("glViewportArrayv", pfnViewportArrayv, PFNGLVIEWPORTARRAYVPROC);
	pfnViewportArrayv(first, count, v);
}

void glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	using PFNGLVIEWPORTINDEXEDFPROC = void(APIENTRY *)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
	static PFNGLVIEWPORTINDEXEDFPROC pfnViewportIndexedf{nullptr};
	LOAD_ENTRYPOINT("glViewportIndexedf", pfnViewportIndexedf, PFNGLVIEWPORTINDEXEDFPROC);
	pfnViewportIndexedf(index, x, y, w, h);
}

void glViewportIndexedfv(GLuint index, const GLfloat* v)
{
	using PFNGLVIEWPORTINDEXEDFVPROC = void(APIENTRY *)(GLuint index, const GLfloat* v);
	static PFNGLVIEWPORTINDEXEDFVPROC pfnViewportIndexedfv{nullptr};
	LOAD_ENTRYPOINT("glViewportIndexedfv", pfnViewportIndexedfv, PFNGLVIEWPORTINDEXEDFVPROC);
	pfnViewportIndexedfv(index, v);
}

void glScissorArrayv(GLuint first, GLsizei count, const GLint* v)
{
	using PFNGLSCISSORARRAYVPROC = void(APIENTRY *)(GLuint first, GLsizei count, const GLint* v);
	static PFNGLSCISSORARRAYVPROC pfnScissorArrayv{nullptr};
	LOAD_ENTRYPOINT("glScissorArrayv", pfnScissorArrayv, PFNGLSCISSORARRAYVPROC);
	pfnScissorArrayv(first, count, v);
}

void glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
	using PFNGLSCISSORINDEXEDPROC = void(APIENTRY *)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
	static PFNGLSCISSORINDEXEDPROC pfnScissorIndexed{nullptr};
	LOAD_ENTRYPOINT("glScissorIndexed", pfnScissorIndexed, PFNGLSCISSORINDEXEDPROC);
	pfnScissorIndexed(index, left, bottom, width, height);
}

void glScissorIndexedv(GLuint index, const GLint* v)
{
	using PFNGLSCISSORINDEXEDVPROC = void(APIENTRY *)(GLuint index, const GLint* v);
	static PFNGLSCISSORINDEXEDVPROC pfnScissorIndexedv{nullptr};
	LOAD_ENTRYPOINT("glScissorIndexedv", pfnScissorIndexedv, PFNGLSCISSORINDEXEDVPROC);
	pfnScissorIndexedv(index, v);
}

void glDepthRangeArrayv(GLuint first, GLsizei count, const GLdouble* v)
{
	using PFNGLDEPTHRANGEARRAYVPROC = void(APIENTRY *)(GLuint first, GLsizei count, const GLdouble* v);
	static PFNGLDEPTHRANGEARRAYVPROC pfnDepthRangeArrayv{nullptr};
	LOAD_ENTRYPOINT("glDepthRangeArrayv", pfnDepthRangeArrayv, PFNGLDEPTHRANGEARRAYVPROC);
	pfnDepthRangeArrayv(first, count, v);
}

void glDepthRangeIndexed(GLuint index, GLdouble n, GLdouble f)
{
	using PFNGLDEPTHRANGEINDEXEDPROC = void(APIENTRY *)(GLuint index, GLdouble n, GLdouble f);
	static PFNGLDEPTHRANGEINDEXEDPROC pfnDepthRangeIndexed{nullptr};
	LOAD_ENTRYPOINT("glDepthRangeIndexed", pfnDepthRangeIndexed, PFNGLDEPTHRANGEINDEXEDPROC);
	pfnDepthRangeIndexed(index, n, f);
}

void glGetFloati_v(GLenum target, GLuint index, GLfloat* data)
{
	using PFNGLGETFLOATI_VPROC = void(APIENTRY *)(GLenum target, GLuint index, GLfloat* data);
	static PFNGLGETFLOATI_VPROC pfnGetFloati_v{nullptr};
	LOAD_ENTRYPOINT("glGetFloati_v", pfnGetFloati_v, PFNGLGETFLOATI_VPROC);
	pfnGetFloati_v(target, index, data);
}

void glGetDoublei_v(GLenum target, GLuint index, GLdouble* data)
{
	using PFNGLGETDOUBLEI_VPROC = void(APIENTRY *)(GLenum target, GLuint index, GLdouble* data);
	static PFNGLGETDOUBLEI_VPROC pfnGetDoublei_v{nullptr};
	LOAD_ENTRYPOINT("glGetDoublei_v", pfnGetDoublei_v, PFNGLGETDOUBLEI_VPROC);
	pfnGetDoublei_v(target, index, data);
}

//
// GL_VERSION_4_2
//

void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)
{
	using PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC = void(APIENTRY *)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
	static PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC pfnDrawArraysInstancedBaseInstance{nullptr};
	LOAD_ENTRYPOINT("glDrawArraysInstancedBaseInstance", pfnDrawArraysInstancedBaseInstance, PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC);
	pfnDrawArraysInstancedBaseInstance(mode, first, count, instancecount, baseinstance);
}

void glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance)
{
	using PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC = void(APIENTRY *)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance);
	static PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC pfnDrawElementsInstancedBaseInstance{nullptr};
	LOAD_ENTRYPOINT("glDrawElementsInstancedBaseInstance", pfnDrawElementsInstancedBaseInstance, PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC);
	pfnDrawElementsInstancedBaseInstance(mode, count, type, indices, instancecount, baseinstance);
}

void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)
{
	using PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC = void(APIENTRY *)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
	static PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC pfnDrawElementsInstancedBaseVertexBaseInstance{nullptr};
	LOAD_ENTRYPOINT("glDrawElementsInstancedBaseVertexBaseInstance", pfnDrawElementsInstancedBaseVertexBaseInstance, PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC);
	pfnDrawElementsInstancedBaseVertexBaseInstance(mode, count, type, indices, instancecount, basevertex, baseinstance);
}

void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint* params)
{
	using PFNGLGETINTERNALFORMATIVPROC = void(APIENTRY *)(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint* params);
	static PFNGLGETINTERNALFORMATIVPROC pfnGetInternalformativ{nullptr};
	LOAD_ENTRYPOINT("glGetInternalformativ", pfnGetInternalformativ, PFNGLGETINTERNALFORMATIVPROC);
	pfnGetInternalformativ(target, internalformat, pname, count, params);
}

void glGetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex, GLenum pname, GLint* params)
{
	using PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC = void(APIENTRY *)(GLuint program, GLuint bufferIndex, GLenum pname, GLint* params);
	static PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC pfnGetActiveAtomicCounterBufferiv{nullptr};
	LOAD_ENTRYPOINT("glGetActiveAtomicCounterBufferiv", pfnGetActiveAtomicCounterBufferiv, PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC);
	pfnGetActiveAtomicCounterBufferiv(program, bufferIndex, pname, params);
}

void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
	using PFNGLBINDIMAGETEXTUREPROC = void(APIENTRY *)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
	static PFNGLBINDIMAGETEXTUREPROC pfnBindImageTexture{nullptr};
	LOAD_ENTRYPOINT("glBindImageTexture", pfnBindImageTexture, PFNGLBINDIMAGETEXTUREPROC);
	pfnBindImageTexture(unit, texture, level, layered, layer, access, format);
}

void glMemoryBarrier(GLbitfield barriers)
{
	using PFNGLMEMORYBARRIERPROC = void(APIENTRY *)(GLbitfield barriers);
	static PFNGLMEMORYBARRIERPROC pfnMemoryBarrier{nullptr};
	LOAD_ENTRYPOINT("glMemoryBarrier", pfnMemoryBarrier, PFNGLMEMORYBARRIERPROC);
	pfnMemoryBarrier(barriers);
}

void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
	using PFNGLTEXSTORAGE1DPROC = void(APIENTRY *)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
	static PFNGLTEXSTORAGE1DPROC pfnTexStorage1D{nullptr};
	LOAD_ENTRYPOINT("glTexStorage1D", pfnTexStorage1D, PFNGLTEXSTORAGE1DPROC);
	pfnTexStorage1D(target, levels, internalformat, width);
}

void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLTEXSTORAGE2DPROC = void(APIENTRY *)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLTEXSTORAGE2DPROC pfnTexStorage2D{nullptr};
	LOAD_ENTRYPOINT("glTexStorage2D", pfnTexStorage2D, PFNGLTEXSTORAGE2DPROC);
	pfnTexStorage2D(target, levels, internalformat, width, height);
}

void glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	using PFNGLTEXSTORAGE3DPROC = void(APIENTRY *)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	static PFNGLTEXSTORAGE3DPROC pfnTexStorage3D{nullptr};
	LOAD_ENTRYPOINT("glTexStorage3D", pfnTexStorage3D, PFNGLTEXSTORAGE3DPROC);
	pfnTexStorage3D(target, levels, internalformat, width, height, depth);
}

void glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei instancecount)
{
	using PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC = void(APIENTRY *)(GLenum mode, GLuint id, GLsizei instancecount);
	static PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC pfnDrawTransformFeedbackInstanced{nullptr};
	LOAD_ENTRYPOINT("glDrawTransformFeedbackInstanced", pfnDrawTransformFeedbackInstanced, PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC);
	pfnDrawTransformFeedbackInstanced(mode, id, instancecount);
}

void glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount)
{
	using PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC = void(APIENTRY *)(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
	static PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC pfnDrawTransformFeedbackStreamInstanced{nullptr};
	LOAD_ENTRYPOINT("glDrawTransformFeedbackStreamInstanced", pfnDrawTransformFeedbackStreamInstanced, PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC);
	pfnDrawTransformFeedbackStreamInstanced(mode, id, stream, instancecount);
}

//
// GL_VERSION_4_3
//

void glClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void* data)
{
	using PFNGLCLEARBUFFERDATAPROC = void(APIENTRY *)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void* data);
	static PFNGLCLEARBUFFERDATAPROC pfnClearBufferData{nullptr};
	LOAD_ENTRYPOINT("glClearBufferData", pfnClearBufferData, PFNGLCLEARBUFFERDATAPROC);
	pfnClearBufferData(target, internalformat, format, type, data);
}

void glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data)
{
	using PFNGLCLEARBUFFERSUBDATAPROC = void(APIENTRY *)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);
	static PFNGLCLEARBUFFERSUBDATAPROC pfnClearBufferSubData{nullptr};
	LOAD_ENTRYPOINT("glClearBufferSubData", pfnClearBufferSubData, PFNGLCLEARBUFFERSUBDATAPROC);
	pfnClearBufferSubData(target, internalformat, offset, size, format, type, data);
}

void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
	using PFNGLDISPATCHCOMPUTEPROC = void(APIENTRY *)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
	static PFNGLDISPATCHCOMPUTEPROC pfnDispatchCompute{nullptr};
	LOAD_ENTRYPOINT("glDispatchCompute", pfnDispatchCompute, PFNGLDISPATCHCOMPUTEPROC);
	pfnDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}

void glDispatchComputeIndirect(GLintptr indirect)
{
	using PFNGLDISPATCHCOMPUTEINDIRECTPROC = void(APIENTRY *)(GLintptr indirect);
	static PFNGLDISPATCHCOMPUTEINDIRECTPROC pfnDispatchComputeIndirect{nullptr};
	LOAD_ENTRYPOINT("glDispatchComputeIndirect", pfnDispatchComputeIndirect, PFNGLDISPATCHCOMPUTEINDIRECTPROC);
	pfnDispatchComputeIndirect(indirect);
}

void glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
	using PFNGLCOPYIMAGESUBDATAPROC = void(APIENTRY *)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
	static PFNGLCOPYIMAGESUBDATAPROC pfnCopyImageSubData{nullptr};
	LOAD_ENTRYPOINT("glCopyImageSubData", pfnCopyImageSubData, PFNGLCOPYIMAGESUBDATAPROC);
	pfnCopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
}

void glFramebufferParameteri(GLenum target, GLenum pname, GLint param)
{
	using PFNGLFRAMEBUFFERPARAMETERIPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint param);
	static PFNGLFRAMEBUFFERPARAMETERIPROC pfnFramebufferParameteri{nullptr};
	LOAD_ENTRYPOINT("glFramebufferParameteri", pfnFramebufferParameteri, PFNGLFRAMEBUFFERPARAMETERIPROC);
	pfnFramebufferParameteri(target, pname, param);
}

void glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETFRAMEBUFFERPARAMETERIVPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint* params);
	static PFNGLGETFRAMEBUFFERPARAMETERIVPROC pfnGetFramebufferParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetFramebufferParameteriv", pfnGetFramebufferParameteriv, PFNGLGETFRAMEBUFFERPARAMETERIVPROC);
	pfnGetFramebufferParameteriv(target, pname, params);
}

void glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64* params)
{
	using PFNGLGETINTERNALFORMATI64VPROC = void(APIENTRY *)(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64* params);
	static PFNGLGETINTERNALFORMATI64VPROC pfnGetInternalformati64v{nullptr};
	LOAD_ENTRYPOINT("glGetInternalformati64v", pfnGetInternalformati64v, PFNGLGETINTERNALFORMATI64VPROC);
	pfnGetInternalformati64v(target, internalformat, pname, count, params);
}

void glInvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)
{
	using PFNGLINVALIDATETEXSUBIMAGEPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
	static PFNGLINVALIDATETEXSUBIMAGEPROC pfnInvalidateTexSubImage{nullptr};
	LOAD_ENTRYPOINT("glInvalidateTexSubImage", pfnInvalidateTexSubImage, PFNGLINVALIDATETEXSUBIMAGEPROC);
	pfnInvalidateTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth);
}

void glInvalidateTexImage(GLuint texture, GLint level)
{
	using PFNGLINVALIDATETEXIMAGEPROC = void(APIENTRY *)(GLuint texture, GLint level);
	static PFNGLINVALIDATETEXIMAGEPROC pfnInvalidateTexImage{nullptr};
	LOAD_ENTRYPOINT("glInvalidateTexImage", pfnInvalidateTexImage, PFNGLINVALIDATETEXIMAGEPROC);
	pfnInvalidateTexImage(texture, level);
}

void glInvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
	using PFNGLINVALIDATEBUFFERSUBDATAPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr length);
	static PFNGLINVALIDATEBUFFERSUBDATAPROC pfnInvalidateBufferSubData{nullptr};
	LOAD_ENTRYPOINT("glInvalidateBufferSubData", pfnInvalidateBufferSubData, PFNGLINVALIDATEBUFFERSUBDATAPROC);
	pfnInvalidateBufferSubData(buffer, offset, length);
}

void glInvalidateBufferData(GLuint buffer)
{
	using PFNGLINVALIDATEBUFFERDATAPROC = void(APIENTRY *)(GLuint buffer);
	static PFNGLINVALIDATEBUFFERDATAPROC pfnInvalidateBufferData{nullptr};
	LOAD_ENTRYPOINT("glInvalidateBufferData", pfnInvalidateBufferData, PFNGLINVALIDATEBUFFERDATAPROC);
	pfnInvalidateBufferData(buffer);
}

void glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
	using PFNGLINVALIDATEFRAMEBUFFERPROC = void(APIENTRY *)(GLenum target, GLsizei numAttachments, const GLenum* attachments);
	static PFNGLINVALIDATEFRAMEBUFFERPROC pfnInvalidateFramebuffer{nullptr};
	LOAD_ENTRYPOINT("glInvalidateFramebuffer", pfnInvalidateFramebuffer, PFNGLINVALIDATEFRAMEBUFFERPROC);
	pfnInvalidateFramebuffer(target, numAttachments, attachments);
}

void glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLINVALIDATESUBFRAMEBUFFERPROC = void(APIENTRY *)(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLINVALIDATESUBFRAMEBUFFERPROC pfnInvalidateSubFramebuffer{nullptr};
	LOAD_ENTRYPOINT("glInvalidateSubFramebuffer", pfnInvalidateSubFramebuffer, PFNGLINVALIDATESUBFRAMEBUFFERPROC);
	pfnInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
}

void glMultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride)
{
	using PFNGLMULTIDRAWARRAYSINDIRECTPROC = void(APIENTRY *)(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride);
	static PFNGLMULTIDRAWARRAYSINDIRECTPROC pfnMultiDrawArraysIndirect{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawArraysIndirect", pfnMultiDrawArraysIndirect, PFNGLMULTIDRAWARRAYSINDIRECTPROC);
	pfnMultiDrawArraysIndirect(mode, indirect, drawcount, stride);
}

void glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride)
{
	using PFNGLMULTIDRAWELEMENTSINDIRECTPROC = void(APIENTRY *)(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride);
	static PFNGLMULTIDRAWELEMENTSINDIRECTPROC pfnMultiDrawElementsIndirect{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawElementsIndirect", pfnMultiDrawElementsIndirect, PFNGLMULTIDRAWELEMENTSINDIRECTPROC);
	pfnMultiDrawElementsIndirect(mode, type, indirect, drawcount, stride);
}

void glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint* params)
{
	using PFNGLGETPROGRAMINTERFACEIVPROC = void(APIENTRY *)(GLuint program, GLenum programInterface, GLenum pname, GLint* params);
	static PFNGLGETPROGRAMINTERFACEIVPROC pfnGetProgramInterfaceiv{nullptr};
	LOAD_ENTRYPOINT("glGetProgramInterfaceiv", pfnGetProgramInterfaceiv, PFNGLGETPROGRAMINTERFACEIVPROC);
	pfnGetProgramInterfaceiv(program, programInterface, pname, params);
}

GLuint glGetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar* name)
{
	using PFNGLGETPROGRAMRESOURCEINDEXPROC = GLuint(APIENTRY *)(GLuint program, GLenum programInterface, const GLchar* name);
	static PFNGLGETPROGRAMRESOURCEINDEXPROC pfnGetProgramResourceIndex{nullptr};
	LOAD_ENTRYPOINT("glGetProgramResourceIndex", pfnGetProgramResourceIndex, PFNGLGETPROGRAMRESOURCEINDEXPROC);
	return pfnGetProgramResourceIndex(program, programInterface, name);
}

void glGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)
{
	using PFNGLGETPROGRAMRESOURCENAMEPROC = void(APIENTRY *)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name);
	static PFNGLGETPROGRAMRESOURCENAMEPROC pfnGetProgramResourceName{nullptr};
	LOAD_ENTRYPOINT("glGetProgramResourceName", pfnGetProgramResourceName, PFNGLGETPROGRAMRESOURCENAMEPROC);
	pfnGetProgramResourceName(program, programInterface, index, bufSize, length, name);
}

void glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei count, GLsizei* length, GLint* params)
{
	using PFNGLGETPROGRAMRESOURCEIVPROC = void(APIENTRY *)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei count, GLsizei* length, GLint* params);
	static PFNGLGETPROGRAMRESOURCEIVPROC pfnGetProgramResourceiv{nullptr};
	LOAD_ENTRYPOINT("glGetProgramResourceiv", pfnGetProgramResourceiv, PFNGLGETPROGRAMRESOURCEIVPROC);
	pfnGetProgramResourceiv(program, programInterface, index, propCount, props, count, length, params);
}

GLint glGetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar* name)
{
	using PFNGLGETPROGRAMRESOURCELOCATIONPROC = GLint(APIENTRY *)(GLuint program, GLenum programInterface, const GLchar* name);
	static PFNGLGETPROGRAMRESOURCELOCATIONPROC pfnGetProgramResourceLocation{nullptr};
	LOAD_ENTRYPOINT("glGetProgramResourceLocation", pfnGetProgramResourceLocation, PFNGLGETPROGRAMRESOURCELOCATIONPROC);
	return pfnGetProgramResourceLocation(program, programInterface, name);
}

GLint glGetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar* name)
{
	using PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC = GLint(APIENTRY *)(GLuint program, GLenum programInterface, const GLchar* name);
	static PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC pfnGetProgramResourceLocationIndex{nullptr};
	LOAD_ENTRYPOINT("glGetProgramResourceLocationIndex", pfnGetProgramResourceLocationIndex, PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC);
	return pfnGetProgramResourceLocationIndex(program, programInterface, name);
}

void glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding)
{
	using PFNGLSHADERSTORAGEBLOCKBINDINGPROC = void(APIENTRY *)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
	static PFNGLSHADERSTORAGEBLOCKBINDINGPROC pfnShaderStorageBlockBinding{nullptr};
	LOAD_ENTRYPOINT("glShaderStorageBlockBinding", pfnShaderStorageBlockBinding, PFNGLSHADERSTORAGEBLOCKBINDINGPROC);
	pfnShaderStorageBlockBinding(program, storageBlockIndex, storageBlockBinding);
}

void glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	using PFNGLTEXBUFFERRANGEPROC = void(APIENTRY *)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static PFNGLTEXBUFFERRANGEPROC pfnTexBufferRange{nullptr};
	LOAD_ENTRYPOINT("glTexBufferRange", pfnTexBufferRange, PFNGLTEXBUFFERRANGEPROC);
	pfnTexBufferRange(target, internalformat, buffer, offset, size);
}

void glTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	using PFNGLTEXSTORAGE2DMULTISAMPLEPROC = void(APIENTRY *)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static PFNGLTEXSTORAGE2DMULTISAMPLEPROC pfnTexStorage2DMultisample{nullptr};
	LOAD_ENTRYPOINT("glTexStorage2DMultisample", pfnTexStorage2DMultisample, PFNGLTEXSTORAGE2DMULTISAMPLEPROC);
	pfnTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

void glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	using PFNGLTEXSTORAGE3DMULTISAMPLEPROC = void(APIENTRY *)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static PFNGLTEXSTORAGE3DMULTISAMPLEPROC pfnTexStorage3DMultisample{nullptr};
	LOAD_ENTRYPOINT("glTexStorage3DMultisample", pfnTexStorage3DMultisample, PFNGLTEXSTORAGE3DMULTISAMPLEPROC);
	pfnTexStorage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

void glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)
{
	using PFNGLTEXTUREVIEWPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
	static PFNGLTEXTUREVIEWPROC pfnTextureView{nullptr};
	LOAD_ENTRYPOINT("glTextureView", pfnTextureView, PFNGLTEXTUREVIEWPROC);
	pfnTextureView(texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers);
}

void glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
	using PFNGLBINDVERTEXBUFFERPROC = void(APIENTRY *)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
	static PFNGLBINDVERTEXBUFFERPROC pfnBindVertexBuffer{nullptr};
	LOAD_ENTRYPOINT("glBindVertexBuffer", pfnBindVertexBuffer, PFNGLBINDVERTEXBUFFERPROC);
	pfnBindVertexBuffer(bindingindex, buffer, offset, stride);
}

void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
	using PFNGLVERTEXATTRIBFORMATPROC = void(APIENTRY *)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
	static PFNGLVERTEXATTRIBFORMATPROC pfnVertexAttribFormat{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribFormat", pfnVertexAttribFormat, PFNGLVERTEXATTRIBFORMATPROC);
	pfnVertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
}

void glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
	using PFNGLVERTEXATTRIBIFORMATPROC = void(APIENTRY *)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static PFNGLVERTEXATTRIBIFORMATPROC pfnVertexAttribIFormat{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribIFormat", pfnVertexAttribIFormat, PFNGLVERTEXATTRIBIFORMATPROC);
	pfnVertexAttribIFormat(attribindex, size, type, relativeoffset);
}

void glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
	using PFNGLVERTEXATTRIBLFORMATPROC = void(APIENTRY *)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static PFNGLVERTEXATTRIBLFORMATPROC pfnVertexAttribLFormat{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribLFormat", pfnVertexAttribLFormat, PFNGLVERTEXATTRIBLFORMATPROC);
	pfnVertexAttribLFormat(attribindex, size, type, relativeoffset);
}

void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
	using PFNGLVERTEXATTRIBBINDINGPROC = void(APIENTRY *)(GLuint attribindex, GLuint bindingindex);
	static PFNGLVERTEXATTRIBBINDINGPROC pfnVertexAttribBinding{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribBinding", pfnVertexAttribBinding, PFNGLVERTEXATTRIBBINDINGPROC);
	pfnVertexAttribBinding(attribindex, bindingindex);
}

void glVertexBindingDivisor(GLuint bindingindex, GLuint divisor)
{
	using PFNGLVERTEXBINDINGDIVISORPROC = void(APIENTRY *)(GLuint bindingindex, GLuint divisor);
	static PFNGLVERTEXBINDINGDIVISORPROC pfnVertexBindingDivisor{nullptr};
	LOAD_ENTRYPOINT("glVertexBindingDivisor", pfnVertexBindingDivisor, PFNGLVERTEXBINDINGDIVISORPROC);
	pfnVertexBindingDivisor(bindingindex, divisor);
}

void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
	using PFNGLDEBUGMESSAGECONTROLPROC = void(APIENTRY *)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
	static PFNGLDEBUGMESSAGECONTROLPROC pfnDebugMessageControl{nullptr};
	LOAD_ENTRYPOINT("glDebugMessageControl", pfnDebugMessageControl, PFNGLDEBUGMESSAGECONTROLPROC);
	pfnDebugMessageControl(source, type, severity, count, ids, enabled);
}

void glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
	using PFNGLDEBUGMESSAGEINSERTPROC = void(APIENTRY *)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
	static PFNGLDEBUGMESSAGEINSERTPROC pfnDebugMessageInsert{nullptr};
	LOAD_ENTRYPOINT("glDebugMessageInsert", pfnDebugMessageInsert, PFNGLDEBUGMESSAGEINSERTPROC);
	pfnDebugMessageInsert(source, type, id, severity, length, buf);
}

void glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam)
{
	using PFNGLDEBUGMESSAGECALLBACKPROC = void(APIENTRY *)(GLDEBUGPROC callback, const void* userParam);
	static PFNGLDEBUGMESSAGECALLBACKPROC pfnDebugMessageCallback{nullptr};
	LOAD_ENTRYPOINT("glDebugMessageCallback", pfnDebugMessageCallback, PFNGLDEBUGMESSAGECALLBACKPROC);
	pfnDebugMessageCallback(callback, userParam);
}

GLuint glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
	using PFNGLGETDEBUGMESSAGELOGPROC = GLuint(APIENTRY *)(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
	static PFNGLGETDEBUGMESSAGELOGPROC pfnGetDebugMessageLog{nullptr};
	LOAD_ENTRYPOINT("glGetDebugMessageLog", pfnGetDebugMessageLog, PFNGLGETDEBUGMESSAGELOGPROC);
	return pfnGetDebugMessageLog(count, bufSize, sources, types, ids, severities, lengths, messageLog);
}

void glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar* message)
{
	using PFNGLPUSHDEBUGGROUPPROC = void(APIENTRY *)(GLenum source, GLuint id, GLsizei length, const GLchar* message);
	static PFNGLPUSHDEBUGGROUPPROC pfnPushDebugGroup{nullptr};
	LOAD_ENTRYPOINT("glPushDebugGroup", pfnPushDebugGroup, PFNGLPUSHDEBUGGROUPPROC);
	pfnPushDebugGroup(source, id, length, message);
}

void glPopDebugGroup(void)
{
	using PFNGLPOPDEBUGGROUPPROC = void(APIENTRY *)(void);
	static PFNGLPOPDEBUGGROUPPROC pfnPopDebugGroup{nullptr};
	LOAD_ENTRYPOINT("glPopDebugGroup", pfnPopDebugGroup, PFNGLPOPDEBUGGROUPPROC);
	pfnPopDebugGroup();
}

void glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label)
{
	using PFNGLOBJECTLABELPROC = void(APIENTRY *)(GLenum identifier, GLuint name, GLsizei length, const GLchar* label);
	static PFNGLOBJECTLABELPROC pfnObjectLabel{nullptr};
	LOAD_ENTRYPOINT("glObjectLabel", pfnObjectLabel, PFNGLOBJECTLABELPROC);
	pfnObjectLabel(identifier, name, length, label);
}

void glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label)
{
	using PFNGLGETOBJECTLABELPROC = void(APIENTRY *)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label);
	static PFNGLGETOBJECTLABELPROC pfnGetObjectLabel{nullptr};
	LOAD_ENTRYPOINT("glGetObjectLabel", pfnGetObjectLabel, PFNGLGETOBJECTLABELPROC);
	pfnGetObjectLabel(identifier, name, bufSize, length, label);
}

void glObjectPtrLabel(const void* ptr, GLsizei length, const GLchar* label)
{
	using PFNGLOBJECTPTRLABELPROC = void(APIENTRY *)(const void* ptr, GLsizei length, const GLchar* label);
	static PFNGLOBJECTPTRLABELPROC pfnObjectPtrLabel{nullptr};
	LOAD_ENTRYPOINT("glObjectPtrLabel", pfnObjectPtrLabel, PFNGLOBJECTPTRLABELPROC);
	pfnObjectPtrLabel(ptr, length, label);
}

void glGetObjectPtrLabel(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label)
{
	using PFNGLGETOBJECTPTRLABELPROC = void(APIENTRY *)(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label);
	static PFNGLGETOBJECTPTRLABELPROC pfnGetObjectPtrLabel{nullptr};
	LOAD_ENTRYPOINT("glGetObjectPtrLabel", pfnGetObjectPtrLabel, PFNGLGETOBJECTPTRLABELPROC);
	pfnGetObjectPtrLabel(ptr, bufSize, length, label);
}

//
// GL_VERSION_4_4
//

void glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags)
{
	using PFNGLBUFFERSTORAGEPROC = void(APIENTRY *)(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags);
	static PFNGLBUFFERSTORAGEPROC pfnBufferStorage{nullptr};
	LOAD_ENTRYPOINT("glBufferStorage", pfnBufferStorage, PFNGLBUFFERSTORAGEPROC);
	pfnBufferStorage(target, size, data, flags);
}

void glClearTexImage(GLuint texture, GLint level, GLenum format, GLenum type, const void* data)
{
	using PFNGLCLEARTEXIMAGEPROC = void(APIENTRY *)(GLuint texture, GLint level, GLenum format, GLenum type, const void* data);
	static PFNGLCLEARTEXIMAGEPROC pfnClearTexImage{nullptr};
	LOAD_ENTRYPOINT("glClearTexImage", pfnClearTexImage, PFNGLCLEARTEXIMAGEPROC);
	pfnClearTexImage(texture, level, format, type, data);
}

void glClearTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* data)
{
	using PFNGLCLEARTEXSUBIMAGEPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* data);
	static PFNGLCLEARTEXSUBIMAGEPROC pfnClearTexSubImage{nullptr};
	LOAD_ENTRYPOINT("glClearTexSubImage", pfnClearTexSubImage, PFNGLCLEARTEXSUBIMAGEPROC);
	pfnClearTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data);
}

void glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint* buffers)
{
	using PFNGLBINDBUFFERSBASEPROC = void(APIENTRY *)(GLenum target, GLuint first, GLsizei count, const GLuint* buffers);
	static PFNGLBINDBUFFERSBASEPROC pfnBindBuffersBase{nullptr};
	LOAD_ENTRYPOINT("glBindBuffersBase", pfnBindBuffersBase, PFNGLBINDBUFFERSBASEPROC);
	pfnBindBuffersBase(target, first, count, buffers);
}

void glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes)
{
	using PFNGLBINDBUFFERSRANGEPROC = void(APIENTRY *)(GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes);
	static PFNGLBINDBUFFERSRANGEPROC pfnBindBuffersRange{nullptr};
	LOAD_ENTRYPOINT("glBindBuffersRange", pfnBindBuffersRange, PFNGLBINDBUFFERSRANGEPROC);
	pfnBindBuffersRange(target, first, count, buffers, offsets, sizes);
}

void glBindTextures(GLuint first, GLsizei count, const GLuint* textures)
{
	using PFNGLBINDTEXTURESPROC = void(APIENTRY *)(GLuint first, GLsizei count, const GLuint* textures);
	static PFNGLBINDTEXTURESPROC pfnBindTextures{nullptr};
	LOAD_ENTRYPOINT("glBindTextures", pfnBindTextures, PFNGLBINDTEXTURESPROC);
	pfnBindTextures(first, count, textures);
}

void glBindSamplers(GLuint first, GLsizei count, const GLuint* samplers)
{
	using PFNGLBINDSAMPLERSPROC = void(APIENTRY *)(GLuint first, GLsizei count, const GLuint* samplers);
	static PFNGLBINDSAMPLERSPROC pfnBindSamplers{nullptr};
	LOAD_ENTRYPOINT("glBindSamplers", pfnBindSamplers, PFNGLBINDSAMPLERSPROC);
	pfnBindSamplers(first, count, samplers);
}

void glBindImageTextures(GLuint first, GLsizei count, const GLuint* textures)
{
	using PFNGLBINDIMAGETEXTURESPROC = void(APIENTRY *)(GLuint first, GLsizei count, const GLuint* textures);
	static PFNGLBINDIMAGETEXTURESPROC pfnBindImageTextures{nullptr};
	LOAD_ENTRYPOINT("glBindImageTextures", pfnBindImageTextures, PFNGLBINDIMAGETEXTURESPROC);
	pfnBindImageTextures(first, count, textures);
}

void glBindVertexBuffers(GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides)
{
	using PFNGLBINDVERTEXBUFFERSPROC = void(APIENTRY *)(GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides);
	static PFNGLBINDVERTEXBUFFERSPROC pfnBindVertexBuffers{nullptr};
	LOAD_ENTRYPOINT("glBindVertexBuffers", pfnBindVertexBuffers, PFNGLBINDVERTEXBUFFERSPROC);
	pfnBindVertexBuffers(first, count, buffers, offsets, strides);
}

//
// GL_VERSION_4_5
//

void glClipControl(GLenum origin, GLenum depth)
{
	using PFNGLCLIPCONTROLPROC = void(APIENTRY *)(GLenum origin, GLenum depth);
	static PFNGLCLIPCONTROLPROC pfnClipControl{nullptr};
	LOAD_ENTRYPOINT("glClipControl", pfnClipControl, PFNGLCLIPCONTROLPROC);
	pfnClipControl(origin, depth);
}

void glCreateTransformFeedbacks(GLsizei n, GLuint* ids)
{
	using PFNGLCREATETRANSFORMFEEDBACKSPROC = void(APIENTRY *)(GLsizei n, GLuint* ids);
	static PFNGLCREATETRANSFORMFEEDBACKSPROC pfnCreateTransformFeedbacks{nullptr};
	LOAD_ENTRYPOINT("glCreateTransformFeedbacks", pfnCreateTransformFeedbacks, PFNGLCREATETRANSFORMFEEDBACKSPROC);
	pfnCreateTransformFeedbacks(n, ids);
}

void glTransformFeedbackBufferBase(GLuint xfb, GLuint index, GLuint buffer)
{
	using PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC = void(APIENTRY *)(GLuint xfb, GLuint index, GLuint buffer);
	static PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC pfnTransformFeedbackBufferBase{nullptr};
	LOAD_ENTRYPOINT("glTransformFeedbackBufferBase", pfnTransformFeedbackBufferBase, PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC);
	pfnTransformFeedbackBufferBase(xfb, index, buffer);
}

void glTransformFeedbackBufferRange(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	using PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC = void(APIENTRY *)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC pfnTransformFeedbackBufferRange{nullptr};
	LOAD_ENTRYPOINT("glTransformFeedbackBufferRange", pfnTransformFeedbackBufferRange, PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC);
	pfnTransformFeedbackBufferRange(xfb, index, buffer, offset, size);
}

void glGetTransformFeedbackiv(GLuint xfb, GLenum pname, GLint* param)
{
	using PFNGLGETTRANSFORMFEEDBACKIVPROC = void(APIENTRY *)(GLuint xfb, GLenum pname, GLint* param);
	static PFNGLGETTRANSFORMFEEDBACKIVPROC pfnGetTransformFeedbackiv{nullptr};
	LOAD_ENTRYPOINT("glGetTransformFeedbackiv", pfnGetTransformFeedbackiv, PFNGLGETTRANSFORMFEEDBACKIVPROC);
	pfnGetTransformFeedbackiv(xfb, pname, param);
}

void glGetTransformFeedbacki_v(GLuint xfb, GLenum pname, GLuint index, GLint* param)
{
	using PFNGLGETTRANSFORMFEEDBACKI_VPROC = void(APIENTRY *)(GLuint xfb, GLenum pname, GLuint index, GLint* param);
	static PFNGLGETTRANSFORMFEEDBACKI_VPROC pfnGetTransformFeedbacki_v{nullptr};
	LOAD_ENTRYPOINT("glGetTransformFeedbacki_v", pfnGetTransformFeedbacki_v, PFNGLGETTRANSFORMFEEDBACKI_VPROC);
	pfnGetTransformFeedbacki_v(xfb, pname, index, param);
}

void glGetTransformFeedbacki64_v(GLuint xfb, GLenum pname, GLuint index, GLint64* param)
{
	using PFNGLGETTRANSFORMFEEDBACKI64_VPROC = void(APIENTRY *)(GLuint xfb, GLenum pname, GLuint index, GLint64* param);
	static PFNGLGETTRANSFORMFEEDBACKI64_VPROC pfnGetTransformFeedbacki64_v{nullptr};
	LOAD_ENTRYPOINT("glGetTransformFeedbacki64_v", pfnGetTransformFeedbacki64_v, PFNGLGETTRANSFORMFEEDBACKI64_VPROC);
	pfnGetTransformFeedbacki64_v(xfb, pname, index, param);
}

void glCreateBuffers(GLsizei n, GLuint* buffers)
{
	using PFNGLCREATEBUFFERSPROC = void(APIENTRY *)(GLsizei n, GLuint* buffers);
	static PFNGLCREATEBUFFERSPROC pfnCreateBuffers{nullptr};
	LOAD_ENTRYPOINT("glCreateBuffers", pfnCreateBuffers, PFNGLCREATEBUFFERSPROC);
	pfnCreateBuffers(n, buffers);
}

void glNamedBufferStorage(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags)
{
	using PFNGLNAMEDBUFFERSTORAGEPROC = void(APIENTRY *)(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags);
	static PFNGLNAMEDBUFFERSTORAGEPROC pfnNamedBufferStorage{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferStorage", pfnNamedBufferStorage, PFNGLNAMEDBUFFERSTORAGEPROC);
	pfnNamedBufferStorage(buffer, size, data, flags);
}

void glNamedBufferData(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage)
{
	using PFNGLNAMEDBUFFERDATAPROC = void(APIENTRY *)(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage);
	static PFNGLNAMEDBUFFERDATAPROC pfnNamedBufferData{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferData", pfnNamedBufferData, PFNGLNAMEDBUFFERDATAPROC);
	pfnNamedBufferData(buffer, size, data, usage);
}

void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data)
{
	using PFNGLNAMEDBUFFERSUBDATAPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data);
	static PFNGLNAMEDBUFFERSUBDATAPROC pfnNamedBufferSubData{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferSubData", pfnNamedBufferSubData, PFNGLNAMEDBUFFERSUBDATAPROC);
	pfnNamedBufferSubData(buffer, offset, size, data);
}

void glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	using PFNGLCOPYNAMEDBUFFERSUBDATAPROC = void(APIENTRY *)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	static PFNGLCOPYNAMEDBUFFERSUBDATAPROC pfnCopyNamedBufferSubData{nullptr};
	LOAD_ENTRYPOINT("glCopyNamedBufferSubData", pfnCopyNamedBufferSubData, PFNGLCOPYNAMEDBUFFERSUBDATAPROC);
	pfnCopyNamedBufferSubData(readBuffer, writeBuffer, readOffset, writeOffset, size);
}

void glClearNamedBufferData(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data)
{
	using PFNGLCLEARNAMEDBUFFERDATAPROC = void(APIENTRY *)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data);
	static PFNGLCLEARNAMEDBUFFERDATAPROC pfnClearNamedBufferData{nullptr};
	LOAD_ENTRYPOINT("glClearNamedBufferData", pfnClearNamedBufferData, PFNGLCLEARNAMEDBUFFERDATAPROC);
	pfnClearNamedBufferData(buffer, internalformat, format, type, data);
}

void glClearNamedBufferSubData(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data)
{
	using PFNGLCLEARNAMEDBUFFERSUBDATAPROC = void(APIENTRY *)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);
	static PFNGLCLEARNAMEDBUFFERSUBDATAPROC pfnClearNamedBufferSubData{nullptr};
	LOAD_ENTRYPOINT("glClearNamedBufferSubData", pfnClearNamedBufferSubData, PFNGLCLEARNAMEDBUFFERSUBDATAPROC);
	pfnClearNamedBufferSubData(buffer, internalformat, offset, size, format, type, data);
}

void* glMapNamedBuffer(GLuint buffer, GLenum access)
{
	using PFNGLMAPNAMEDBUFFERPROC = void*(APIENTRY *)(GLuint buffer, GLenum access);
	static PFNGLMAPNAMEDBUFFERPROC pfnMapNamedBuffer{nullptr};
	LOAD_ENTRYPOINT("glMapNamedBuffer", pfnMapNamedBuffer, PFNGLMAPNAMEDBUFFERPROC);
	return pfnMapNamedBuffer(buffer, access);
}

void* glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	using PFNGLMAPNAMEDBUFFERRANGEPROC = void*(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
	static PFNGLMAPNAMEDBUFFERRANGEPROC pfnMapNamedBufferRange{nullptr};
	LOAD_ENTRYPOINT("glMapNamedBufferRange", pfnMapNamedBufferRange, PFNGLMAPNAMEDBUFFERRANGEPROC);
	return pfnMapNamedBufferRange(buffer, offset, length, access);
}

GLboolean glUnmapNamedBuffer(GLuint buffer)
{
	using PFNGLUNMAPNAMEDBUFFERPROC = GLboolean(APIENTRY *)(GLuint buffer);
	static PFNGLUNMAPNAMEDBUFFERPROC pfnUnmapNamedBuffer{nullptr};
	LOAD_ENTRYPOINT("glUnmapNamedBuffer", pfnUnmapNamedBuffer, PFNGLUNMAPNAMEDBUFFERPROC);
	return pfnUnmapNamedBuffer(buffer);
}

void glFlushMappedNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
	using PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr length);
	static PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC pfnFlushMappedNamedBufferRange{nullptr};
	LOAD_ENTRYPOINT("glFlushMappedNamedBufferRange", pfnFlushMappedNamedBufferRange, PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC);
	pfnFlushMappedNamedBufferRange(buffer, offset, length);
}

void glGetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint* params)
{
	using PFNGLGETNAMEDBUFFERPARAMETERIVPROC = void(APIENTRY *)(GLuint buffer, GLenum pname, GLint* params);
	static PFNGLGETNAMEDBUFFERPARAMETERIVPROC pfnGetNamedBufferParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetNamedBufferParameteriv", pfnGetNamedBufferParameteriv, PFNGLGETNAMEDBUFFERPARAMETERIVPROC);
	pfnGetNamedBufferParameteriv(buffer, pname, params);
}

void glGetNamedBufferParameteri64v(GLuint buffer, GLenum pname, GLint64* params)
{
	using PFNGLGETNAMEDBUFFERPARAMETERI64VPROC = void(APIENTRY *)(GLuint buffer, GLenum pname, GLint64* params);
	static PFNGLGETNAMEDBUFFERPARAMETERI64VPROC pfnGetNamedBufferParameteri64v{nullptr};
	LOAD_ENTRYPOINT("glGetNamedBufferParameteri64v", pfnGetNamedBufferParameteri64v, PFNGLGETNAMEDBUFFERPARAMETERI64VPROC);
	pfnGetNamedBufferParameteri64v(buffer, pname, params);
}

void glGetNamedBufferPointerv(GLuint buffer, GLenum pname, void** params)
{
	using PFNGLGETNAMEDBUFFERPOINTERVPROC = void(APIENTRY *)(GLuint buffer, GLenum pname, void** params);
	static PFNGLGETNAMEDBUFFERPOINTERVPROC pfnGetNamedBufferPointerv{nullptr};
	LOAD_ENTRYPOINT("glGetNamedBufferPointerv", pfnGetNamedBufferPointerv, PFNGLGETNAMEDBUFFERPOINTERVPROC);
	pfnGetNamedBufferPointerv(buffer, pname, params);
}

void glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data)
{
	using PFNGLGETNAMEDBUFFERSUBDATAPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data);
	static PFNGLGETNAMEDBUFFERSUBDATAPROC pfnGetNamedBufferSubData{nullptr};
	LOAD_ENTRYPOINT("glGetNamedBufferSubData", pfnGetNamedBufferSubData, PFNGLGETNAMEDBUFFERSUBDATAPROC);
	pfnGetNamedBufferSubData(buffer, offset, size, data);
}

void glCreateFramebuffers(GLsizei n, GLuint* framebuffers)
{
	using PFNGLCREATEFRAMEBUFFERSPROC = void(APIENTRY *)(GLsizei n, GLuint* framebuffers);
	static PFNGLCREATEFRAMEBUFFERSPROC pfnCreateFramebuffers{nullptr};
	LOAD_ENTRYPOINT("glCreateFramebuffers", pfnCreateFramebuffers, PFNGLCREATEFRAMEBUFFERSPROC);
	pfnCreateFramebuffers(n, framebuffers);
}

void glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	using PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC pfnNamedFramebufferRenderbuffer{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferRenderbuffer", pfnNamedFramebufferRenderbuffer, PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC);
	pfnNamedFramebufferRenderbuffer(framebuffer, attachment, renderbuffertarget, renderbuffer);
}

void glNamedFramebufferParameteri(GLuint framebuffer, GLenum pname, GLint param)
{
	using PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC = void(APIENTRY *)(GLuint framebuffer, GLenum pname, GLint param);
	static PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC pfnNamedFramebufferParameteri{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferParameteri", pfnNamedFramebufferParameteri, PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC);
	pfnNamedFramebufferParameteri(framebuffer, pname, param);
}

void glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)
{
	using PFNGLNAMEDFRAMEBUFFERTEXTUREPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
	static PFNGLNAMEDFRAMEBUFFERTEXTUREPROC pfnNamedFramebufferTexture{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferTexture", pfnNamedFramebufferTexture, PFNGLNAMEDFRAMEBUFFERTEXTUREPROC);
	pfnNamedFramebufferTexture(framebuffer, attachment, texture, level);
}

void glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	using PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC pfnNamedFramebufferTextureLayer{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferTextureLayer", pfnNamedFramebufferTextureLayer, PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC);
	pfnNamedFramebufferTextureLayer(framebuffer, attachment, texture, level, layer);
}

void glNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buf)
{
	using PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC = void(APIENTRY *)(GLuint framebuffer, GLenum buf);
	static PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC pfnNamedFramebufferDrawBuffer{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferDrawBuffer", pfnNamedFramebufferDrawBuffer, PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC);
	pfnNamedFramebufferDrawBuffer(framebuffer, buf);
}

void glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum* bufs)
{
	using PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC = void(APIENTRY *)(GLuint framebuffer, GLsizei n, const GLenum* bufs);
	static PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC pfnNamedFramebufferDrawBuffers{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferDrawBuffers", pfnNamedFramebufferDrawBuffers, PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC);
	pfnNamedFramebufferDrawBuffers(framebuffer, n, bufs);
}

void glNamedFramebufferReadBuffer(GLuint framebuffer, GLenum src)
{
	using PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC = void(APIENTRY *)(GLuint framebuffer, GLenum src);
	static PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC pfnNamedFramebufferReadBuffer{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferReadBuffer", pfnNamedFramebufferReadBuffer, PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC);
	pfnNamedFramebufferReadBuffer(framebuffer, src);
}

void glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments)
{
	using PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC = void(APIENTRY *)(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments);
	static PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC pfnInvalidateNamedFramebufferData{nullptr};
	LOAD_ENTRYPOINT("glInvalidateNamedFramebufferData", pfnInvalidateNamedFramebufferData, PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC);
	pfnInvalidateNamedFramebufferData(framebuffer, numAttachments, attachments);
}

void glInvalidateNamedFramebufferSubData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC = void(APIENTRY *)(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC pfnInvalidateNamedFramebufferSubData{nullptr};
	LOAD_ENTRYPOINT("glInvalidateNamedFramebufferSubData", pfnInvalidateNamedFramebufferSubData, PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC);
	pfnInvalidateNamedFramebufferSubData(framebuffer, numAttachments, attachments, x, y, width, height);
}

void glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint* value)
{
	using PFNGLCLEARNAMEDFRAMEBUFFERIVPROC = void(APIENTRY *)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint* value);
	static PFNGLCLEARNAMEDFRAMEBUFFERIVPROC pfnClearNamedFramebufferiv{nullptr};
	LOAD_ENTRYPOINT("glClearNamedFramebufferiv", pfnClearNamedFramebufferiv, PFNGLCLEARNAMEDFRAMEBUFFERIVPROC);
	pfnClearNamedFramebufferiv(framebuffer, buffer, drawbuffer, value);
}

void glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint* value)
{
	using PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC = void(APIENTRY *)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint* value);
	static PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC pfnClearNamedFramebufferuiv{nullptr};
	LOAD_ENTRYPOINT("glClearNamedFramebufferuiv", pfnClearNamedFramebufferuiv, PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC);
	pfnClearNamedFramebufferuiv(framebuffer, buffer, drawbuffer, value);
}

void glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
	using PFNGLCLEARNAMEDFRAMEBUFFERFVPROC = void(APIENTRY *)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat* value);
	static PFNGLCLEARNAMEDFRAMEBUFFERFVPROC pfnClearNamedFramebufferfv{nullptr};
	LOAD_ENTRYPOINT("glClearNamedFramebufferfv", pfnClearNamedFramebufferfv, PFNGLCLEARNAMEDFRAMEBUFFERFVPROC);
	pfnClearNamedFramebufferfv(framebuffer, buffer, drawbuffer, value);
}

void glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	using PFNGLCLEARNAMEDFRAMEBUFFERFIPROC = void(APIENTRY *)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	static PFNGLCLEARNAMEDFRAMEBUFFERFIPROC pfnClearNamedFramebufferfi{nullptr};
	LOAD_ENTRYPOINT("glClearNamedFramebufferfi", pfnClearNamedFramebufferfi, PFNGLCLEARNAMEDFRAMEBUFFERFIPROC);
	pfnClearNamedFramebufferfi(framebuffer, buffer, drawbuffer, depth, stencil);
}

void glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	using PFNGLBLITNAMEDFRAMEBUFFERPROC = void(APIENTRY *)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	static PFNGLBLITNAMEDFRAMEBUFFERPROC pfnBlitNamedFramebuffer{nullptr};
	LOAD_ENTRYPOINT("glBlitNamedFramebuffer", pfnBlitNamedFramebuffer, PFNGLBLITNAMEDFRAMEBUFFERPROC);
	pfnBlitNamedFramebuffer(readFramebuffer, drawFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

GLenum glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target)
{
	using PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC = GLenum(APIENTRY *)(GLuint framebuffer, GLenum target);
	static PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC pfnCheckNamedFramebufferStatus{nullptr};
	LOAD_ENTRYPOINT("glCheckNamedFramebufferStatus", pfnCheckNamedFramebufferStatus, PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC);
	return pfnCheckNamedFramebufferStatus(framebuffer, target);
}

void glGetNamedFramebufferParameteriv(GLuint framebuffer, GLenum pname, GLint* param)
{
	using PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC = void(APIENTRY *)(GLuint framebuffer, GLenum pname, GLint* param);
	static PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC pfnGetNamedFramebufferParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetNamedFramebufferParameteriv", pfnGetNamedFramebufferParameteriv, PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC);
	pfnGetNamedFramebufferParameteriv(framebuffer, pname, param);
}

void glGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params)
{
	using PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params);
	static PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC pfnGetNamedFramebufferAttachmentParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetNamedFramebufferAttachmentParameteriv", pfnGetNamedFramebufferAttachmentParameteriv, PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC);
	pfnGetNamedFramebufferAttachmentParameteriv(framebuffer, attachment, pname, params);
}

void glCreateRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
	using PFNGLCREATERENDERBUFFERSPROC = void(APIENTRY *)(GLsizei n, GLuint* renderbuffers);
	static PFNGLCREATERENDERBUFFERSPROC pfnCreateRenderbuffers{nullptr};
	LOAD_ENTRYPOINT("glCreateRenderbuffers", pfnCreateRenderbuffers, PFNGLCREATERENDERBUFFERSPROC);
	pfnCreateRenderbuffers(n, renderbuffers);
}

void glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLNAMEDRENDERBUFFERSTORAGEPROC = void(APIENTRY *)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLNAMEDRENDERBUFFERSTORAGEPROC pfnNamedRenderbufferStorage{nullptr};
	LOAD_ENTRYPOINT("glNamedRenderbufferStorage", pfnNamedRenderbufferStorage, PFNGLNAMEDRENDERBUFFERSTORAGEPROC);
	pfnNamedRenderbufferStorage(renderbuffer, internalformat, width, height);
}

void glNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC = void(APIENTRY *)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC pfnNamedRenderbufferStorageMultisample{nullptr};
	LOAD_ENTRYPOINT("glNamedRenderbufferStorageMultisample", pfnNamedRenderbufferStorageMultisample, PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC);
	pfnNamedRenderbufferStorageMultisample(renderbuffer, samples, internalformat, width, height);
}

void glGetNamedRenderbufferParameteriv(GLuint renderbuffer, GLenum pname, GLint* params)
{
	using PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC = void(APIENTRY *)(GLuint renderbuffer, GLenum pname, GLint* params);
	static PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC pfnGetNamedRenderbufferParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetNamedRenderbufferParameteriv", pfnGetNamedRenderbufferParameteriv, PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC);
	pfnGetNamedRenderbufferParameteriv(renderbuffer, pname, params);
}

void glCreateTextures(GLenum target, GLsizei n, GLuint* textures)
{
	using PFNGLCREATETEXTURESPROC = void(APIENTRY *)(GLenum target, GLsizei n, GLuint* textures);
	static PFNGLCREATETEXTURESPROC pfnCreateTextures{nullptr};
	LOAD_ENTRYPOINT("glCreateTextures", pfnCreateTextures, PFNGLCREATETEXTURESPROC);
	pfnCreateTextures(target, n, textures);
}

void glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer)
{
	using PFNGLTEXTUREBUFFERPROC = void(APIENTRY *)(GLuint texture, GLenum internalformat, GLuint buffer);
	static PFNGLTEXTUREBUFFERPROC pfnTextureBuffer{nullptr};
	LOAD_ENTRYPOINT("glTextureBuffer", pfnTextureBuffer, PFNGLTEXTUREBUFFERPROC);
	pfnTextureBuffer(texture, internalformat, buffer);
}

void glTextureBufferRange(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	using PFNGLTEXTUREBUFFERRANGEPROC = void(APIENTRY *)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static PFNGLTEXTUREBUFFERRANGEPROC pfnTextureBufferRange{nullptr};
	LOAD_ENTRYPOINT("glTextureBufferRange", pfnTextureBufferRange, PFNGLTEXTUREBUFFERRANGEPROC);
	pfnTextureBufferRange(texture, internalformat, buffer, offset, size);
}

void glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)
{
	using PFNGLTEXTURESTORAGE1DPROC = void(APIENTRY *)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
	static PFNGLTEXTURESTORAGE1DPROC pfnTextureStorage1D{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage1D", pfnTextureStorage1D, PFNGLTEXTURESTORAGE1DPROC);
	pfnTextureStorage1D(texture, levels, internalformat, width);
}

void glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLTEXTURESTORAGE2DPROC = void(APIENTRY *)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLTEXTURESTORAGE2DPROC pfnTextureStorage2D{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage2D", pfnTextureStorage2D, PFNGLTEXTURESTORAGE2DPROC);
	pfnTextureStorage2D(texture, levels, internalformat, width, height);
}

void glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	using PFNGLTEXTURESTORAGE3DPROC = void(APIENTRY *)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	static PFNGLTEXTURESTORAGE3DPROC pfnTextureStorage3D{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage3D", pfnTextureStorage3D, PFNGLTEXTURESTORAGE3DPROC);
	pfnTextureStorage3D(texture, levels, internalformat, width, height, depth);
}

void glTextureStorage2DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	using PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC = void(APIENTRY *)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC pfnTextureStorage2DMultisample{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage2DMultisample", pfnTextureStorage2DMultisample, PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC);
	pfnTextureStorage2DMultisample(texture, samples, internalformat, width, height, fixedsamplelocations);
}

void glTextureStorage3DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	using PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC = void(APIENTRY *)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC pfnTextureStorage3DMultisample{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage3DMultisample", pfnTextureStorage3DMultisample, PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC);
	pfnTextureStorage3DMultisample(texture, samples, internalformat, width, height, depth, fixedsamplelocations);
}

void glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXTURESUBIMAGE1DPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXTURESUBIMAGE1DPROC pfnTextureSubImage1D{nullptr};
	LOAD_ENTRYPOINT("glTextureSubImage1D", pfnTextureSubImage1D, PFNGLTEXTURESUBIMAGE1DPROC);
	pfnTextureSubImage1D(texture, level, xoffset, width, format, type, pixels);
}

void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXTURESUBIMAGE2DPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXTURESUBIMAGE2DPROC pfnTextureSubImage2D{nullptr};
	LOAD_ENTRYPOINT("glTextureSubImage2D", pfnTextureSubImage2D, PFNGLTEXTURESUBIMAGE2DPROC);
	pfnTextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, type, pixels);
}

void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXTURESUBIMAGE3DPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXTURESUBIMAGE3DPROC pfnTextureSubImage3D{nullptr};
	LOAD_ENTRYPOINT("glTextureSubImage3D", pfnTextureSubImage3D, PFNGLTEXTURESUBIMAGE3DPROC);
	pfnTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data)
{
	using PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data);
	static PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC pfnCompressedTextureSubImage1D{nullptr};
	LOAD_ENTRYPOINT("glCompressedTextureSubImage1D", pfnCompressedTextureSubImage1D, PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC);
	pfnCompressedTextureSubImage1D(texture, level, xoffset, width, format, imageSize, data);
}

void glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
	using PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
	static PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC pfnCompressedTextureSubImage2D{nullptr};
	LOAD_ENTRYPOINT("glCompressedTextureSubImage2D", pfnCompressedTextureSubImage2D, PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC);
	pfnCompressedTextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
	using PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);
	static PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC pfnCompressedTextureSubImage3D{nullptr};
	LOAD_ENTRYPOINT("glCompressedTextureSubImage3D", pfnCompressedTextureSubImage3D, PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC);
	pfnCompressedTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

void glCopyTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	using PFNGLCOPYTEXTURESUBIMAGE1DPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	static PFNGLCOPYTEXTURESUBIMAGE1DPROC pfnCopyTextureSubImage1D{nullptr};
	LOAD_ENTRYPOINT("glCopyTextureSubImage1D", pfnCopyTextureSubImage1D, PFNGLCOPYTEXTURESUBIMAGE1DPROC);
	pfnCopyTextureSubImage1D(texture, level, xoffset, x, y, width);
}

void glCopyTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLCOPYTEXTURESUBIMAGE2DPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLCOPYTEXTURESUBIMAGE2DPROC pfnCopyTextureSubImage2D{nullptr};
	LOAD_ENTRYPOINT("glCopyTextureSubImage2D", pfnCopyTextureSubImage2D, PFNGLCOPYTEXTURESUBIMAGE2DPROC);
	pfnCopyTextureSubImage2D(texture, level, xoffset, yoffset, x, y, width, height);
}

void glCopyTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLCOPYTEXTURESUBIMAGE3DPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLCOPYTEXTURESUBIMAGE3DPROC pfnCopyTextureSubImage3D{nullptr};
	LOAD_ENTRYPOINT("glCopyTextureSubImage3D", pfnCopyTextureSubImage3D, PFNGLCOPYTEXTURESUBIMAGE3DPROC);
	pfnCopyTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, x, y, width, height);
}

void glTextureParameterf(GLuint texture, GLenum pname, GLfloat param)
{
	using PFNGLTEXTUREPARAMETERFPROC = void(APIENTRY *)(GLuint texture, GLenum pname, GLfloat param);
	static PFNGLTEXTUREPARAMETERFPROC pfnTextureParameterf{nullptr};
	LOAD_ENTRYPOINT("glTextureParameterf", pfnTextureParameterf, PFNGLTEXTUREPARAMETERFPROC);
	pfnTextureParameterf(texture, pname, param);
}

void glTextureParameterfv(GLuint texture, GLenum pname, const GLfloat* param)
{
	using PFNGLTEXTUREPARAMETERFVPROC = void(APIENTRY *)(GLuint texture, GLenum pname, const GLfloat* param);
	static PFNGLTEXTUREPARAMETERFVPROC pfnTextureParameterfv{nullptr};
	LOAD_ENTRYPOINT("glTextureParameterfv", pfnTextureParameterfv, PFNGLTEXTUREPARAMETERFVPROC);
	pfnTextureParameterfv(texture, pname, param);
}

void glTextureParameteri(GLuint texture, GLenum pname, GLint param)
{
	using PFNGLTEXTUREPARAMETERIPROC = void(APIENTRY *)(GLuint texture, GLenum pname, GLint param);
	static PFNGLTEXTUREPARAMETERIPROC pfnTextureParameteri{nullptr};
	LOAD_ENTRYPOINT("glTextureParameteri", pfnTextureParameteri, PFNGLTEXTUREPARAMETERIPROC);
	pfnTextureParameteri(texture, pname, param);
}

void glTextureParameterIiv(GLuint texture, GLenum pname, const GLint* params)
{
	using PFNGLTEXTUREPARAMETERIIVPROC = void(APIENTRY *)(GLuint texture, GLenum pname, const GLint* params);
	static PFNGLTEXTUREPARAMETERIIVPROC pfnTextureParameterIiv{nullptr};
	LOAD_ENTRYPOINT("glTextureParameterIiv", pfnTextureParameterIiv, PFNGLTEXTUREPARAMETERIIVPROC);
	pfnTextureParameterIiv(texture, pname, params);
}

void glTextureParameterIuiv(GLuint texture, GLenum pname, const GLuint* params)
{
	using PFNGLTEXTUREPARAMETERIUIVPROC = void(APIENTRY *)(GLuint texture, GLenum pname, const GLuint* params);
	static PFNGLTEXTUREPARAMETERIUIVPROC pfnTextureParameterIuiv{nullptr};
	LOAD_ENTRYPOINT("glTextureParameterIuiv", pfnTextureParameterIuiv, PFNGLTEXTUREPARAMETERIUIVPROC);
	pfnTextureParameterIuiv(texture, pname, params);
}

void glTextureParameteriv(GLuint texture, GLenum pname, const GLint* param)
{
	using PFNGLTEXTUREPARAMETERIVPROC = void(APIENTRY *)(GLuint texture, GLenum pname, const GLint* param);
	static PFNGLTEXTUREPARAMETERIVPROC pfnTextureParameteriv{nullptr};
	LOAD_ENTRYPOINT("glTextureParameteriv", pfnTextureParameteriv, PFNGLTEXTUREPARAMETERIVPROC);
	pfnTextureParameteriv(texture, pname, param);
}

void glGenerateTextureMipmap(GLuint texture)
{
	using PFNGLGENERATETEXTUREMIPMAPPROC = void(APIENTRY *)(GLuint texture);
	static PFNGLGENERATETEXTUREMIPMAPPROC pfnGenerateTextureMipmap{nullptr};
	LOAD_ENTRYPOINT("glGenerateTextureMipmap", pfnGenerateTextureMipmap, PFNGLGENERATETEXTUREMIPMAPPROC);
	pfnGenerateTextureMipmap(texture);
}

void glBindTextureUnit(GLuint unit, GLuint texture)
{
	using PFNGLBINDTEXTUREUNITPROC = void(APIENTRY *)(GLuint unit, GLuint texture);
	static PFNGLBINDTEXTUREUNITPROC pfnBindTextureUnit{nullptr};
	LOAD_ENTRYPOINT("glBindTextureUnit", pfnBindTextureUnit, PFNGLBINDTEXTUREUNITPROC);
	pfnBindTextureUnit(unit, texture);
}

void glGetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels)
{
	using PFNGLGETTEXTUREIMAGEPROC = void(APIENTRY *)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels);
	static PFNGLGETTEXTUREIMAGEPROC pfnGetTextureImage{nullptr};
	LOAD_ENTRYPOINT("glGetTextureImage", pfnGetTextureImage, PFNGLGETTEXTUREIMAGEPROC);
	pfnGetTextureImage(texture, level, format, type, bufSize, pixels);
}

void glGetCompressedTextureImage(GLuint texture, GLint level, GLsizei bufSize, void* pixels)
{
	using PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC = void(APIENTRY *)(GLuint texture, GLint level, GLsizei bufSize, void* pixels);
	static PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC pfnGetCompressedTextureImage{nullptr};
	LOAD_ENTRYPOINT("glGetCompressedTextureImage", pfnGetCompressedTextureImage, PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC);
	pfnGetCompressedTextureImage(texture, level, bufSize, pixels);
}

void glGetTextureLevelParameterfv(GLuint texture, GLint level, GLenum pname, GLfloat* params)
{
	using PFNGLGETTEXTURELEVELPARAMETERFVPROC = void(APIENTRY *)(GLuint texture, GLint level, GLenum pname, GLfloat* params);
	static PFNGLGETTEXTURELEVELPARAMETERFVPROC pfnGetTextureLevelParameterfv{nullptr};
	LOAD_ENTRYPOINT("glGetTextureLevelParameterfv", pfnGetTextureLevelParameterfv, PFNGLGETTEXTURELEVELPARAMETERFVPROC);
	pfnGetTextureLevelParameterfv(texture, level, pname, params);
}

void glGetTextureLevelParameteriv(GLuint texture, GLint level, GLenum pname, GLint* params)
{
	using PFNGLGETTEXTURELEVELPARAMETERIVPROC = void(APIENTRY *)(GLuint texture, GLint level, GLenum pname, GLint* params);
	static PFNGLGETTEXTURELEVELPARAMETERIVPROC pfnGetTextureLevelParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetTextureLevelParameteriv", pfnGetTextureLevelParameteriv, PFNGLGETTEXTURELEVELPARAMETERIVPROC);
	pfnGetTextureLevelParameteriv(texture, level, pname, params);
}

void glGetTextureParameterfv(GLuint texture, GLenum pname, GLfloat* params)
{
	using PFNGLGETTEXTUREPARAMETERFVPROC = void(APIENTRY *)(GLuint texture, GLenum pname, GLfloat* params);
	static PFNGLGETTEXTUREPARAMETERFVPROC pfnGetTextureParameterfv{nullptr};
	LOAD_ENTRYPOINT("glGetTextureParameterfv", pfnGetTextureParameterfv, PFNGLGETTEXTUREPARAMETERFVPROC);
	pfnGetTextureParameterfv(texture, pname, params);
}

void glGetTextureParameterIiv(GLuint texture, GLenum pname, GLint* params)
{
	using PFNGLGETTEXTUREPARAMETERIIVPROC = void(APIENTRY *)(GLuint texture, GLenum pname, GLint* params);
	static PFNGLGETTEXTUREPARAMETERIIVPROC pfnGetTextureParameterIiv{nullptr};
	LOAD_ENTRYPOINT("glGetTextureParameterIiv", pfnGetTextureParameterIiv, PFNGLGETTEXTUREPARAMETERIIVPROC);
	pfnGetTextureParameterIiv(texture, pname, params);
}

void glGetTextureParameterIuiv(GLuint texture, GLenum pname, GLuint* params)
{
	using PFNGLGETTEXTUREPARAMETERIUIVPROC = void(APIENTRY *)(GLuint texture, GLenum pname, GLuint* params);
	static PFNGLGETTEXTUREPARAMETERIUIVPROC pfnGetTextureParameterIuiv{nullptr};
	LOAD_ENTRYPOINT("glGetTextureParameterIuiv", pfnGetTextureParameterIuiv, PFNGLGETTEXTUREPARAMETERIUIVPROC);
	pfnGetTextureParameterIuiv(texture, pname, params);
}

void glGetTextureParameteriv(GLuint texture, GLenum pname, GLint* params)
{
	using PFNGLGETTEXTUREPARAMETERIVPROC = void(APIENTRY *)(GLuint texture, GLenum pname, GLint* params);
	static PFNGLGETTEXTUREPARAMETERIVPROC pfnGetTextureParameteriv{nullptr};
	LOAD_ENTRYPOINT("glGetTextureParameteriv", pfnGetTextureParameteriv, PFNGLGETTEXTUREPARAMETERIVPROC);
	pfnGetTextureParameteriv(texture, pname, params);
}

void glCreateVertexArrays(GLsizei n, GLuint* arrays)
{
	using PFNGLCREATEVERTEXARRAYSPROC = void(APIENTRY *)(GLsizei n, GLuint* arrays);
	static PFNGLCREATEVERTEXARRAYSPROC pfnCreateVertexArrays{nullptr};
	LOAD_ENTRYPOINT("glCreateVertexArrays", pfnCreateVertexArrays, PFNGLCREATEVERTEXARRAYSPROC);
	pfnCreateVertexArrays(n, arrays);
}

void glDisableVertexArrayAttrib(GLuint vaobj, GLuint index)
{
	using PFNGLDISABLEVERTEXARRAYATTRIBPROC = void(APIENTRY *)(GLuint vaobj, GLuint index);
	static PFNGLDISABLEVERTEXARRAYATTRIBPROC pfnDisableVertexArrayAttrib{nullptr};
	LOAD_ENTRYPOINT("glDisableVertexArrayAttrib", pfnDisableVertexArrayAttrib, PFNGLDISABLEVERTEXARRAYATTRIBPROC);
	pfnDisableVertexArrayAttrib(vaobj, index);
}

void glEnableVertexArrayAttrib(GLuint vaobj, GLuint index)
{
	using PFNGLENABLEVERTEXARRAYATTRIBPROC = void(APIENTRY *)(GLuint vaobj, GLuint index);
	static PFNGLENABLEVERTEXARRAYATTRIBPROC pfnEnableVertexArrayAttrib{nullptr};
	LOAD_ENTRYPOINT("glEnableVertexArrayAttrib", pfnEnableVertexArrayAttrib, PFNGLENABLEVERTEXARRAYATTRIBPROC);
	pfnEnableVertexArrayAttrib(vaobj, index);
}

void glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer)
{
	using PFNGLVERTEXARRAYELEMENTBUFFERPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer);
	static PFNGLVERTEXARRAYELEMENTBUFFERPROC pfnVertexArrayElementBuffer{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayElementBuffer", pfnVertexArrayElementBuffer, PFNGLVERTEXARRAYELEMENTBUFFERPROC);
	pfnVertexArrayElementBuffer(vaobj, buffer);
}

void glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
	using PFNGLVERTEXARRAYVERTEXBUFFERPROC = void(APIENTRY *)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
	static PFNGLVERTEXARRAYVERTEXBUFFERPROC pfnVertexArrayVertexBuffer{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexBuffer", pfnVertexArrayVertexBuffer, PFNGLVERTEXARRAYVERTEXBUFFERPROC);
	pfnVertexArrayVertexBuffer(vaobj, bindingindex, buffer, offset, stride);
}

void glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides)
{
	using PFNGLVERTEXARRAYVERTEXBUFFERSPROC = void(APIENTRY *)(GLuint vaobj, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides);
	static PFNGLVERTEXARRAYVERTEXBUFFERSPROC pfnVertexArrayVertexBuffers{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexBuffers", pfnVertexArrayVertexBuffers, PFNGLVERTEXARRAYVERTEXBUFFERSPROC);
	pfnVertexArrayVertexBuffers(vaobj, first, count, buffers, offsets, strides);
}

void glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex)
{
	using PFNGLVERTEXARRAYATTRIBBINDINGPROC = void(APIENTRY *)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
	static PFNGLVERTEXARRAYATTRIBBINDINGPROC pfnVertexArrayAttribBinding{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayAttribBinding", pfnVertexArrayAttribBinding, PFNGLVERTEXARRAYATTRIBBINDINGPROC);
	pfnVertexArrayAttribBinding(vaobj, attribindex, bindingindex);
}

void glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
	using PFNGLVERTEXARRAYATTRIBFORMATPROC = void(APIENTRY *)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
	static PFNGLVERTEXARRAYATTRIBFORMATPROC pfnVertexArrayAttribFormat{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayAttribFormat", pfnVertexArrayAttribFormat, PFNGLVERTEXARRAYATTRIBFORMATPROC);
	pfnVertexArrayAttribFormat(vaobj, attribindex, size, type, normalized, relativeoffset);
}

void glVertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
	using PFNGLVERTEXARRAYATTRIBIFORMATPROC = void(APIENTRY *)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static PFNGLVERTEXARRAYATTRIBIFORMATPROC pfnVertexArrayAttribIFormat{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayAttribIFormat", pfnVertexArrayAttribIFormat, PFNGLVERTEXARRAYATTRIBIFORMATPROC);
	pfnVertexArrayAttribIFormat(vaobj, attribindex, size, type, relativeoffset);
}

void glVertexArrayAttribLFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
	using PFNGLVERTEXARRAYATTRIBLFORMATPROC = void(APIENTRY *)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static PFNGLVERTEXARRAYATTRIBLFORMATPROC pfnVertexArrayAttribLFormat{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayAttribLFormat", pfnVertexArrayAttribLFormat, PFNGLVERTEXARRAYATTRIBLFORMATPROC);
	pfnVertexArrayAttribLFormat(vaobj, attribindex, size, type, relativeoffset);
}

void glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor)
{
	using PFNGLVERTEXARRAYBINDINGDIVISORPROC = void(APIENTRY *)(GLuint vaobj, GLuint bindingindex, GLuint divisor);
	static PFNGLVERTEXARRAYBINDINGDIVISORPROC pfnVertexArrayBindingDivisor{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayBindingDivisor", pfnVertexArrayBindingDivisor, PFNGLVERTEXARRAYBINDINGDIVISORPROC);
	pfnVertexArrayBindingDivisor(vaobj, bindingindex, divisor);
}

void glGetVertexArrayiv(GLuint vaobj, GLenum pname, GLint* param)
{
	using PFNGLGETVERTEXARRAYIVPROC = void(APIENTRY *)(GLuint vaobj, GLenum pname, GLint* param);
	static PFNGLGETVERTEXARRAYIVPROC pfnGetVertexArrayiv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexArrayiv", pfnGetVertexArrayiv, PFNGLGETVERTEXARRAYIVPROC);
	pfnGetVertexArrayiv(vaobj, pname, param);
}

void glGetVertexArrayIndexediv(GLuint vaobj, GLuint index, GLenum pname, GLint* param)
{
	using PFNGLGETVERTEXARRAYINDEXEDIVPROC = void(APIENTRY *)(GLuint vaobj, GLuint index, GLenum pname, GLint* param);
	static PFNGLGETVERTEXARRAYINDEXEDIVPROC pfnGetVertexArrayIndexediv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexArrayIndexediv", pfnGetVertexArrayIndexediv, PFNGLGETVERTEXARRAYINDEXEDIVPROC);
	pfnGetVertexArrayIndexediv(vaobj, index, pname, param);
}

void glGetVertexArrayIndexed64iv(GLuint vaobj, GLuint index, GLenum pname, GLint64* param)
{
	using PFNGLGETVERTEXARRAYINDEXED64IVPROC = void(APIENTRY *)(GLuint vaobj, GLuint index, GLenum pname, GLint64* param);
	static PFNGLGETVERTEXARRAYINDEXED64IVPROC pfnGetVertexArrayIndexed64iv{nullptr};
	LOAD_ENTRYPOINT("glGetVertexArrayIndexed64iv", pfnGetVertexArrayIndexed64iv, PFNGLGETVERTEXARRAYINDEXED64IVPROC);
	pfnGetVertexArrayIndexed64iv(vaobj, index, pname, param);
}

void glCreateSamplers(GLsizei n, GLuint* samplers)
{
	using PFNGLCREATESAMPLERSPROC = void(APIENTRY *)(GLsizei n, GLuint* samplers);
	static PFNGLCREATESAMPLERSPROC pfnCreateSamplers{nullptr};
	LOAD_ENTRYPOINT("glCreateSamplers", pfnCreateSamplers, PFNGLCREATESAMPLERSPROC);
	pfnCreateSamplers(n, samplers);
}

void glCreateProgramPipelines(GLsizei n, GLuint* pipelines)
{
	using PFNGLCREATEPROGRAMPIPELINESPROC = void(APIENTRY *)(GLsizei n, GLuint* pipelines);
	static PFNGLCREATEPROGRAMPIPELINESPROC pfnCreateProgramPipelines{nullptr};
	LOAD_ENTRYPOINT("glCreateProgramPipelines", pfnCreateProgramPipelines, PFNGLCREATEPROGRAMPIPELINESPROC);
	pfnCreateProgramPipelines(n, pipelines);
}

void glCreateQueries(GLenum target, GLsizei n, GLuint* ids)
{
	using PFNGLCREATEQUERIESPROC = void(APIENTRY *)(GLenum target, GLsizei n, GLuint* ids);
	static PFNGLCREATEQUERIESPROC pfnCreateQueries{nullptr};
	LOAD_ENTRYPOINT("glCreateQueries", pfnCreateQueries, PFNGLCREATEQUERIESPROC);
	pfnCreateQueries(target, n, ids);
}

void glGetQueryBufferObjecti64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
	using PFNGLGETQUERYBUFFEROBJECTI64VPROC = void(APIENTRY *)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static PFNGLGETQUERYBUFFEROBJECTI64VPROC pfnGetQueryBufferObjecti64v{nullptr};
	LOAD_ENTRYPOINT("glGetQueryBufferObjecti64v", pfnGetQueryBufferObjecti64v, PFNGLGETQUERYBUFFEROBJECTI64VPROC);
	pfnGetQueryBufferObjecti64v(id, buffer, pname, offset);
}

void glGetQueryBufferObjectiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
	using PFNGLGETQUERYBUFFEROBJECTIVPROC = void(APIENTRY *)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static PFNGLGETQUERYBUFFEROBJECTIVPROC pfnGetQueryBufferObjectiv{nullptr};
	LOAD_ENTRYPOINT("glGetQueryBufferObjectiv", pfnGetQueryBufferObjectiv, PFNGLGETQUERYBUFFEROBJECTIVPROC);
	pfnGetQueryBufferObjectiv(id, buffer, pname, offset);
}

void glGetQueryBufferObjectui64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
	using PFNGLGETQUERYBUFFEROBJECTUI64VPROC = void(APIENTRY *)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static PFNGLGETQUERYBUFFEROBJECTUI64VPROC pfnGetQueryBufferObjectui64v{nullptr};
	LOAD_ENTRYPOINT("glGetQueryBufferObjectui64v", pfnGetQueryBufferObjectui64v, PFNGLGETQUERYBUFFEROBJECTUI64VPROC);
	pfnGetQueryBufferObjectui64v(id, buffer, pname, offset);
}

void glGetQueryBufferObjectuiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
	using PFNGLGETQUERYBUFFEROBJECTUIVPROC = void(APIENTRY *)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
	static PFNGLGETQUERYBUFFEROBJECTUIVPROC pfnGetQueryBufferObjectuiv{nullptr};
	LOAD_ENTRYPOINT("glGetQueryBufferObjectuiv", pfnGetQueryBufferObjectuiv, PFNGLGETQUERYBUFFEROBJECTUIVPROC);
	pfnGetQueryBufferObjectuiv(id, buffer, pname, offset);
}

void glMemoryBarrierByRegion(GLbitfield barriers)
{
	using PFNGLMEMORYBARRIERBYREGIONPROC = void(APIENTRY *)(GLbitfield barriers);
	static PFNGLMEMORYBARRIERBYREGIONPROC pfnMemoryBarrierByRegion{nullptr};
	LOAD_ENTRYPOINT("glMemoryBarrierByRegion", pfnMemoryBarrierByRegion, PFNGLMEMORYBARRIERBYREGIONPROC);
	pfnMemoryBarrierByRegion(barriers);
}

void glGetTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void* pixels)
{
	using PFNGLGETTEXTURESUBIMAGEPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void* pixels);
	static PFNGLGETTEXTURESUBIMAGEPROC pfnGetTextureSubImage{nullptr};
	LOAD_ENTRYPOINT("glGetTextureSubImage", pfnGetTextureSubImage, PFNGLGETTEXTURESUBIMAGEPROC);
	pfnGetTextureSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, bufSize, pixels);
}

void glGetCompressedTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void* pixels)
{
	using PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void* pixels);
	static PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC pfnGetCompressedTextureSubImage{nullptr};
	LOAD_ENTRYPOINT("glGetCompressedTextureSubImage", pfnGetCompressedTextureSubImage, PFNGLGETCOMPRESSEDTEXTURESUBIMAGEPROC);
	pfnGetCompressedTextureSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, bufSize, pixels);
}

GLenum glGetGraphicsResetStatus(void)
{
	using PFNGLGETGRAPHICSRESETSTATUSPROC = GLenum(APIENTRY *)(void);
	static PFNGLGETGRAPHICSRESETSTATUSPROC pfnGetGraphicsResetStatus{nullptr};
	LOAD_ENTRYPOINT("glGetGraphicsResetStatus", pfnGetGraphicsResetStatus, PFNGLGETGRAPHICSRESETSTATUSPROC);
	return pfnGetGraphicsResetStatus();
}

void glGetnCompressedTexImage(GLenum target, GLint lod, GLsizei bufSize, void* pixels)
{
	using PFNGLGETNCOMPRESSEDTEXIMAGEPROC = void(APIENTRY *)(GLenum target, GLint lod, GLsizei bufSize, void* pixels);
	static PFNGLGETNCOMPRESSEDTEXIMAGEPROC pfnGetnCompressedTexImage{nullptr};
	LOAD_ENTRYPOINT("glGetnCompressedTexImage", pfnGetnCompressedTexImage, PFNGLGETNCOMPRESSEDTEXIMAGEPROC);
	pfnGetnCompressedTexImage(target, lod, bufSize, pixels);
}

void glGetnTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels)
{
	using PFNGLGETNTEXIMAGEPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels);
	static PFNGLGETNTEXIMAGEPROC pfnGetnTexImage{nullptr};
	LOAD_ENTRYPOINT("glGetnTexImage", pfnGetnTexImage, PFNGLGETNTEXIMAGEPROC);
	pfnGetnTexImage(target, level, format, type, bufSize, pixels);
}

void glGetnUniformdv(GLuint program, GLint location, GLsizei bufSize, GLdouble* params)
{
	using PFNGLGETNUNIFORMDVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLdouble* params);
	static PFNGLGETNUNIFORMDVPROC pfnGetnUniformdv{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformdv", pfnGetnUniformdv, PFNGLGETNUNIFORMDVPROC);
	pfnGetnUniformdv(program, location, bufSize, params);
}

void glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat* params)
{
	using PFNGLGETNUNIFORMFVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLfloat* params);
	static PFNGLGETNUNIFORMFVPROC pfnGetnUniformfv{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformfv", pfnGetnUniformfv, PFNGLGETNUNIFORMFVPROC);
	pfnGetnUniformfv(program, location, bufSize, params);
}

void glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint* params)
{
	using PFNGLGETNUNIFORMIVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLint* params);
	static PFNGLGETNUNIFORMIVPROC pfnGetnUniformiv{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformiv", pfnGetnUniformiv, PFNGLGETNUNIFORMIVPROC);
	pfnGetnUniformiv(program, location, bufSize, params);
}

void glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint* params)
{
	using PFNGLGETNUNIFORMUIVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLuint* params);
	static PFNGLGETNUNIFORMUIVPROC pfnGetnUniformuiv{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformuiv", pfnGetnUniformuiv, PFNGLGETNUNIFORMUIVPROC);
	pfnGetnUniformuiv(program, location, bufSize, params);
}

void glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data)
{
	using PFNGLREADNPIXELSPROC = void(APIENTRY *)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data);
	static PFNGLREADNPIXELSPROC pfnReadnPixels{nullptr};
	LOAD_ENTRYPOINT("glReadnPixels", pfnReadnPixels, PFNGLREADNPIXELSPROC);
	pfnReadnPixels(x, y, width, height, format, type, bufSize, data);
}

void glTextureBarrier(void)
{
	using PFNGLTEXTUREBARRIERPROC = void(APIENTRY *)(void);
	static PFNGLTEXTUREBARRIERPROC pfnTextureBarrier{nullptr};
	LOAD_ENTRYPOINT("glTextureBarrier", pfnTextureBarrier, PFNGLTEXTUREBARRIERPROC);
	pfnTextureBarrier();
}

//
// GL_VERSION_4_6
//

void glSpecializeShader(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue)
{
	using PFNGLSPECIALIZESHADERPROC = void(APIENTRY *)(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue);
	static PFNGLSPECIALIZESHADERPROC pfnSpecializeShader{nullptr};
	LOAD_ENTRYPOINT("glSpecializeShader", pfnSpecializeShader, PFNGLSPECIALIZESHADERPROC);
	pfnSpecializeShader(shader, pEntryPoint, numSpecializationConstants, pConstantIndex, pConstantValue);
}

void glMultiDrawArraysIndirectCount(GLenum mode, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
	using PFNGLMULTIDRAWARRAYSINDIRECTCOUNTPROC = void(APIENTRY *)(GLenum mode, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
	static PFNGLMULTIDRAWARRAYSINDIRECTCOUNTPROC pfnMultiDrawArraysIndirectCount{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawArraysIndirectCount", pfnMultiDrawArraysIndirectCount, PFNGLMULTIDRAWARRAYSINDIRECTCOUNTPROC);
	pfnMultiDrawArraysIndirectCount(mode, indirect, drawcount, maxdrawcount, stride);
}

void glMultiDrawElementsIndirectCount(GLenum mode, GLenum type, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
	using PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTPROC = void(APIENTRY *)(GLenum mode, GLenum type, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
	static PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTPROC pfnMultiDrawElementsIndirectCount{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawElementsIndirectCount", pfnMultiDrawElementsIndirectCount, PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTPROC);
	pfnMultiDrawElementsIndirectCount(mode, type, indirect, drawcount, maxdrawcount, stride);
}

void glPolygonOffsetClamp(GLfloat factor, GLfloat units, GLfloat clamp)
{
	using PFNGLPOLYGONOFFSETCLAMPPROC = void(APIENTRY *)(GLfloat factor, GLfloat units, GLfloat clamp);
	static PFNGLPOLYGONOFFSETCLAMPPROC pfnPolygonOffsetClamp{nullptr};
	LOAD_ENTRYPOINT("glPolygonOffsetClamp", pfnPolygonOffsetClamp, PFNGLPOLYGONOFFSETCLAMPPROC);
	pfnPolygonOffsetClamp(factor, units, clamp);
}

//
// GL_ARB_ES3_2_compatibility
//

void glPrimitiveBoundingBoxARB(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
	using PFNGLPRIMITIVEBOUNDINGBOXARBPROC = void(APIENTRY *)(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);
	static PFNGLPRIMITIVEBOUNDINGBOXARBPROC pfnPrimitiveBoundingBoxARB{nullptr};
	LOAD_ENTRYPOINT("glPrimitiveBoundingBoxARB", pfnPrimitiveBoundingBoxARB, PFNGLPRIMITIVEBOUNDINGBOXARBPROC);
	pfnPrimitiveBoundingBoxARB(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
}

//
// GL_ARB_bindless_texture
//

GLuint64 glGetTextureHandleARB(GLuint texture)
{
	using PFNGLGETTEXTUREHANDLEARBPROC = GLuint64(APIENTRY *)(GLuint texture);
	static PFNGLGETTEXTUREHANDLEARBPROC pfnGetTextureHandleARB{nullptr};
	LOAD_ENTRYPOINT("glGetTextureHandleARB", pfnGetTextureHandleARB, PFNGLGETTEXTUREHANDLEARBPROC);
	return pfnGetTextureHandleARB(texture);
}

GLuint64 glGetTextureSamplerHandleARB(GLuint texture, GLuint sampler)
{
	using PFNGLGETTEXTURESAMPLERHANDLEARBPROC = GLuint64(APIENTRY *)(GLuint texture, GLuint sampler);
	static PFNGLGETTEXTURESAMPLERHANDLEARBPROC pfnGetTextureSamplerHandleARB{nullptr};
	LOAD_ENTRYPOINT("glGetTextureSamplerHandleARB", pfnGetTextureSamplerHandleARB, PFNGLGETTEXTURESAMPLERHANDLEARBPROC);
	return pfnGetTextureSamplerHandleARB(texture, sampler);
}

void glMakeTextureHandleResidentARB(GLuint64 handle)
{
	using PFNGLMAKETEXTUREHANDLERESIDENTARBPROC = void(APIENTRY *)(GLuint64 handle);
	static PFNGLMAKETEXTUREHANDLERESIDENTARBPROC pfnMakeTextureHandleResidentARB{nullptr};
	LOAD_ENTRYPOINT("glMakeTextureHandleResidentARB", pfnMakeTextureHandleResidentARB, PFNGLMAKETEXTUREHANDLERESIDENTARBPROC);
	pfnMakeTextureHandleResidentARB(handle);
}

void glMakeTextureHandleNonResidentARB(GLuint64 handle)
{
	using PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC = void(APIENTRY *)(GLuint64 handle);
	static PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC pfnMakeTextureHandleNonResidentARB{nullptr};
	LOAD_ENTRYPOINT("glMakeTextureHandleNonResidentARB", pfnMakeTextureHandleNonResidentARB, PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC);
	pfnMakeTextureHandleNonResidentARB(handle);
}

GLuint64 glGetImageHandleARB(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format)
{
	using PFNGLGETIMAGEHANDLEARBPROC = GLuint64(APIENTRY *)(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format);
	static PFNGLGETIMAGEHANDLEARBPROC pfnGetImageHandleARB{nullptr};
	LOAD_ENTRYPOINT("glGetImageHandleARB", pfnGetImageHandleARB, PFNGLGETIMAGEHANDLEARBPROC);
	return pfnGetImageHandleARB(texture, level, layered, layer, format);
}

void glMakeImageHandleResidentARB(GLuint64 handle, GLenum access)
{
	using PFNGLMAKEIMAGEHANDLERESIDENTARBPROC = void(APIENTRY *)(GLuint64 handle, GLenum access);
	static PFNGLMAKEIMAGEHANDLERESIDENTARBPROC pfnMakeImageHandleResidentARB{nullptr};
	LOAD_ENTRYPOINT("glMakeImageHandleResidentARB", pfnMakeImageHandleResidentARB, PFNGLMAKEIMAGEHANDLERESIDENTARBPROC);
	pfnMakeImageHandleResidentARB(handle, access);
}

void glMakeImageHandleNonResidentARB(GLuint64 handle)
{
	using PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC = void(APIENTRY *)(GLuint64 handle);
	static PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC pfnMakeImageHandleNonResidentARB{nullptr};
	LOAD_ENTRYPOINT("glMakeImageHandleNonResidentARB", pfnMakeImageHandleNonResidentARB, PFNGLMAKEIMAGEHANDLENONRESIDENTARBPROC);
	pfnMakeImageHandleNonResidentARB(handle);
}

void glUniformHandleui64ARB(GLint location, GLuint64 value)
{
	using PFNGLUNIFORMHANDLEUI64ARBPROC = void(APIENTRY *)(GLint location, GLuint64 value);
	static PFNGLUNIFORMHANDLEUI64ARBPROC pfnUniformHandleui64ARB{nullptr};
	LOAD_ENTRYPOINT("glUniformHandleui64ARB", pfnUniformHandleui64ARB, PFNGLUNIFORMHANDLEUI64ARBPROC);
	pfnUniformHandleui64ARB(location, value);
}

void glUniformHandleui64vARB(GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLUNIFORMHANDLEUI64VARBPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64* value);
	static PFNGLUNIFORMHANDLEUI64VARBPROC pfnUniformHandleui64vARB{nullptr};
	LOAD_ENTRYPOINT("glUniformHandleui64vARB", pfnUniformHandleui64vARB, PFNGLUNIFORMHANDLEUI64VARBPROC);
	pfnUniformHandleui64vARB(location, count, value);
}

void glProgramUniformHandleui64ARB(GLuint program, GLint location, GLuint64 value)
{
	using PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64 value);
	static PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC pfnProgramUniformHandleui64ARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformHandleui64ARB", pfnProgramUniformHandleui64ARB, PFNGLPROGRAMUNIFORMHANDLEUI64ARBPROC);
	pfnProgramUniformHandleui64ARB(program, location, value);
}

void glProgramUniformHandleui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* values)
{
	using PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64* values);
	static PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC pfnProgramUniformHandleui64vARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformHandleui64vARB", pfnProgramUniformHandleui64vARB, PFNGLPROGRAMUNIFORMHANDLEUI64VARBPROC);
	pfnProgramUniformHandleui64vARB(program, location, count, values);
}

GLboolean glIsTextureHandleResidentARB(GLuint64 handle)
{
	using PFNGLISTEXTUREHANDLERESIDENTARBPROC = GLboolean(APIENTRY *)(GLuint64 handle);
	static PFNGLISTEXTUREHANDLERESIDENTARBPROC pfnIsTextureHandleResidentARB{nullptr};
	LOAD_ENTRYPOINT("glIsTextureHandleResidentARB", pfnIsTextureHandleResidentARB, PFNGLISTEXTUREHANDLERESIDENTARBPROC);
	return pfnIsTextureHandleResidentARB(handle);
}

GLboolean glIsImageHandleResidentARB(GLuint64 handle)
{
	using PFNGLISIMAGEHANDLERESIDENTARBPROC = GLboolean(APIENTRY *)(GLuint64 handle);
	static PFNGLISIMAGEHANDLERESIDENTARBPROC pfnIsImageHandleResidentARB{nullptr};
	LOAD_ENTRYPOINT("glIsImageHandleResidentARB", pfnIsImageHandleResidentARB, PFNGLISIMAGEHANDLERESIDENTARBPROC);
	return pfnIsImageHandleResidentARB(handle);
}

void glVertexAttribL1ui64ARB(GLuint index, GLuint64EXT x)
{
	using PFNGLVERTEXATTRIBL1UI64ARBPROC = void(APIENTRY *)(GLuint index, GLuint64EXT x);
	static PFNGLVERTEXATTRIBL1UI64ARBPROC pfnVertexAttribL1ui64ARB{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL1ui64ARB", pfnVertexAttribL1ui64ARB, PFNGLVERTEXATTRIBL1UI64ARBPROC);
	pfnVertexAttribL1ui64ARB(index, x);
}

void glVertexAttribL1ui64vARB(GLuint index, const GLuint64EXT* v)
{
	using PFNGLVERTEXATTRIBL1UI64VARBPROC = void(APIENTRY *)(GLuint index, const GLuint64EXT* v);
	static PFNGLVERTEXATTRIBL1UI64VARBPROC pfnVertexAttribL1ui64vARB{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL1ui64vARB", pfnVertexAttribL1ui64vARB, PFNGLVERTEXATTRIBL1UI64VARBPROC);
	pfnVertexAttribL1ui64vARB(index, v);
}

void glGetVertexAttribLui64vARB(GLuint index, GLenum pname, GLuint64EXT* params)
{
	using PFNGLGETVERTEXATTRIBLUI64VARBPROC = void(APIENTRY *)(GLuint index, GLenum pname, GLuint64EXT* params);
	static PFNGLGETVERTEXATTRIBLUI64VARBPROC pfnGetVertexAttribLui64vARB{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribLui64vARB", pfnGetVertexAttribLui64vARB, PFNGLGETVERTEXATTRIBLUI64VARBPROC);
	pfnGetVertexAttribLui64vARB(index, pname, params);
}

//
// GL_ARB_cl_event
//

GLsync glCreateSyncFromCLeventARB(struct _cl_context* context, struct _cl_event* event, GLbitfield flags)
{
	using PFNGLCREATESYNCFROMCLEVENTARBPROC = GLsync(APIENTRY *)(struct _cl_context* context, struct _cl_event* event, GLbitfield flags);
	static PFNGLCREATESYNCFROMCLEVENTARBPROC pfnCreateSyncFromCLeventARB{nullptr};
	LOAD_ENTRYPOINT("glCreateSyncFromCLeventARB", pfnCreateSyncFromCLeventARB, PFNGLCREATESYNCFROMCLEVENTARBPROC);
	return pfnCreateSyncFromCLeventARB(context, event, flags);
}

//
// GL_ARB_compute_variable_group_size
//

void glDispatchComputeGroupSizeARB(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z)
{
	using PFNGLDISPATCHCOMPUTEGROUPSIZEARBPROC = void(APIENTRY *)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z);
	static PFNGLDISPATCHCOMPUTEGROUPSIZEARBPROC pfnDispatchComputeGroupSizeARB{nullptr};
	LOAD_ENTRYPOINT("glDispatchComputeGroupSizeARB", pfnDispatchComputeGroupSizeARB, PFNGLDISPATCHCOMPUTEGROUPSIZEARBPROC);
	pfnDispatchComputeGroupSizeARB(num_groups_x, num_groups_y, num_groups_z, group_size_x, group_size_y, group_size_z);
}

//
// GL_ARB_debug_output
//

void glDebugMessageControlARB(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
	using PFNGLDEBUGMESSAGECONTROLARBPROC = void(APIENTRY *)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
	static PFNGLDEBUGMESSAGECONTROLARBPROC pfnDebugMessageControlARB{nullptr};
	LOAD_ENTRYPOINT("glDebugMessageControlARB", pfnDebugMessageControlARB, PFNGLDEBUGMESSAGECONTROLARBPROC);
	pfnDebugMessageControlARB(source, type, severity, count, ids, enabled);
}

void glDebugMessageInsertARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
	using PFNGLDEBUGMESSAGEINSERTARBPROC = void(APIENTRY *)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
	static PFNGLDEBUGMESSAGEINSERTARBPROC pfnDebugMessageInsertARB{nullptr};
	LOAD_ENTRYPOINT("glDebugMessageInsertARB", pfnDebugMessageInsertARB, PFNGLDEBUGMESSAGEINSERTARBPROC);
	pfnDebugMessageInsertARB(source, type, id, severity, length, buf);
}

void glDebugMessageCallbackARB(GLDEBUGPROCARB callback, const void* userParam)
{
	using PFNGLDEBUGMESSAGECALLBACKARBPROC = void(APIENTRY *)(GLDEBUGPROCARB callback, const void* userParam);
	static PFNGLDEBUGMESSAGECALLBACKARBPROC pfnDebugMessageCallbackARB{nullptr};
	LOAD_ENTRYPOINT("glDebugMessageCallbackARB", pfnDebugMessageCallbackARB, PFNGLDEBUGMESSAGECALLBACKARBPROC);
	pfnDebugMessageCallbackARB(callback, userParam);
}

GLuint glGetDebugMessageLogARB(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
	using PFNGLGETDEBUGMESSAGELOGARBPROC = GLuint(APIENTRY *)(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
	static PFNGLGETDEBUGMESSAGELOGARBPROC pfnGetDebugMessageLogARB{nullptr};
	LOAD_ENTRYPOINT("glGetDebugMessageLogARB", pfnGetDebugMessageLogARB, PFNGLGETDEBUGMESSAGELOGARBPROC);
	return pfnGetDebugMessageLogARB(count, bufSize, sources, types, ids, severities, lengths, messageLog);
}

//
// GL_ARB_draw_buffers_blend
//

void glBlendEquationiARB(GLuint buf, GLenum mode)
{
	using PFNGLBLENDEQUATIONIARBPROC = void(APIENTRY *)(GLuint buf, GLenum mode);
	static PFNGLBLENDEQUATIONIARBPROC pfnBlendEquationiARB{nullptr};
	LOAD_ENTRYPOINT("glBlendEquationiARB", pfnBlendEquationiARB, PFNGLBLENDEQUATIONIARBPROC);
	pfnBlendEquationiARB(buf, mode);
}

void glBlendEquationSeparateiARB(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
	using PFNGLBLENDEQUATIONSEPARATEIARBPROC = void(APIENTRY *)(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
	static PFNGLBLENDEQUATIONSEPARATEIARBPROC pfnBlendEquationSeparateiARB{nullptr};
	LOAD_ENTRYPOINT("glBlendEquationSeparateiARB", pfnBlendEquationSeparateiARB, PFNGLBLENDEQUATIONSEPARATEIARBPROC);
	pfnBlendEquationSeparateiARB(buf, modeRGB, modeAlpha);
}

void glBlendFunciARB(GLuint buf, GLenum src, GLenum dst)
{
	using PFNGLBLENDFUNCIARBPROC = void(APIENTRY *)(GLuint buf, GLenum src, GLenum dst);
	static PFNGLBLENDFUNCIARBPROC pfnBlendFunciARB{nullptr};
	LOAD_ENTRYPOINT("glBlendFunciARB", pfnBlendFunciARB, PFNGLBLENDFUNCIARBPROC);
	pfnBlendFunciARB(buf, src, dst);
}

void glBlendFuncSeparateiARB(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
	using PFNGLBLENDFUNCSEPARATEIARBPROC = void(APIENTRY *)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
	static PFNGLBLENDFUNCSEPARATEIARBPROC pfnBlendFuncSeparateiARB{nullptr};
	LOAD_ENTRYPOINT("glBlendFuncSeparateiARB", pfnBlendFuncSeparateiARB, PFNGLBLENDFUNCSEPARATEIARBPROC);
	pfnBlendFuncSeparateiARB(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

//
// GL_ARB_draw_instanced
//

void glDrawArraysInstancedARB(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
	using PFNGLDRAWARRAYSINSTANCEDARBPROC = void(APIENTRY *)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
	static PFNGLDRAWARRAYSINSTANCEDARBPROC pfnDrawArraysInstancedARB{nullptr};
	LOAD_ENTRYPOINT("glDrawArraysInstancedARB", pfnDrawArraysInstancedARB, PFNGLDRAWARRAYSINSTANCEDARBPROC);
	pfnDrawArraysInstancedARB(mode, first, count, primcount);
}

void glDrawElementsInstancedARB(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount)
{
	using PFNGLDRAWELEMENTSINSTANCEDARBPROC = void(APIENTRY *)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount);
	static PFNGLDRAWELEMENTSINSTANCEDARBPROC pfnDrawElementsInstancedARB{nullptr};
	LOAD_ENTRYPOINT("glDrawElementsInstancedARB", pfnDrawElementsInstancedARB, PFNGLDRAWELEMENTSINSTANCEDARBPROC);
	pfnDrawElementsInstancedARB(mode, count, type, indices, primcount);
}

//
// GL_ARB_geometry_shader4
//

void glProgramParameteriARB(GLuint program, GLenum pname, GLint value)
{
	using PFNGLPROGRAMPARAMETERIARBPROC = void(APIENTRY *)(GLuint program, GLenum pname, GLint value);
	static PFNGLPROGRAMPARAMETERIARBPROC pfnProgramParameteriARB{nullptr};
	LOAD_ENTRYPOINT("glProgramParameteriARB", pfnProgramParameteriARB, PFNGLPROGRAMPARAMETERIARBPROC);
	pfnProgramParameteriARB(program, pname, value);
}

void glFramebufferTextureARB(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	using PFNGLFRAMEBUFFERTEXTUREARBPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTUREARBPROC pfnFramebufferTextureARB{nullptr};
	LOAD_ENTRYPOINT("glFramebufferTextureARB", pfnFramebufferTextureARB, PFNGLFRAMEBUFFERTEXTUREARBPROC);
	pfnFramebufferTextureARB(target, attachment, texture, level);
}

void glFramebufferTextureLayerARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	using PFNGLFRAMEBUFFERTEXTURELAYERARBPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static PFNGLFRAMEBUFFERTEXTURELAYERARBPROC pfnFramebufferTextureLayerARB{nullptr};
	LOAD_ENTRYPOINT("glFramebufferTextureLayerARB", pfnFramebufferTextureLayerARB, PFNGLFRAMEBUFFERTEXTURELAYERARBPROC);
	pfnFramebufferTextureLayerARB(target, attachment, texture, level, layer);
}

void glFramebufferTextureFaceARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
	using PFNGLFRAMEBUFFERTEXTUREFACEARBPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
	static PFNGLFRAMEBUFFERTEXTUREFACEARBPROC pfnFramebufferTextureFaceARB{nullptr};
	LOAD_ENTRYPOINT("glFramebufferTextureFaceARB", pfnFramebufferTextureFaceARB, PFNGLFRAMEBUFFERTEXTUREFACEARBPROC);
	pfnFramebufferTextureFaceARB(target, attachment, texture, level, face);
}

//
// GL_ARB_gl_spirv
//

void glSpecializeShaderARB(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue)
{
	using PFNGLSPECIALIZESHADERARBPROC = void(APIENTRY *)(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue);
	static PFNGLSPECIALIZESHADERARBPROC pfnSpecializeShaderARB{nullptr};
	LOAD_ENTRYPOINT("glSpecializeShaderARB", pfnSpecializeShaderARB, PFNGLSPECIALIZESHADERARBPROC);
	pfnSpecializeShaderARB(shader, pEntryPoint, numSpecializationConstants, pConstantIndex, pConstantValue);
}

//
// GL_ARB_gpu_shader_int64
//

void glUniform1i64ARB(GLint location, GLint64 x)
{
	using PFNGLUNIFORM1I64ARBPROC = void(APIENTRY *)(GLint location, GLint64 x);
	static PFNGLUNIFORM1I64ARBPROC pfnUniform1i64ARB{nullptr};
	LOAD_ENTRYPOINT("glUniform1i64ARB", pfnUniform1i64ARB, PFNGLUNIFORM1I64ARBPROC);
	pfnUniform1i64ARB(location, x);
}

void glUniform2i64ARB(GLint location, GLint64 x, GLint64 y)
{
	using PFNGLUNIFORM2I64ARBPROC = void(APIENTRY *)(GLint location, GLint64 x, GLint64 y);
	static PFNGLUNIFORM2I64ARBPROC pfnUniform2i64ARB{nullptr};
	LOAD_ENTRYPOINT("glUniform2i64ARB", pfnUniform2i64ARB, PFNGLUNIFORM2I64ARBPROC);
	pfnUniform2i64ARB(location, x, y);
}

void glUniform3i64ARB(GLint location, GLint64 x, GLint64 y, GLint64 z)
{
	using PFNGLUNIFORM3I64ARBPROC = void(APIENTRY *)(GLint location, GLint64 x, GLint64 y, GLint64 z);
	static PFNGLUNIFORM3I64ARBPROC pfnUniform3i64ARB{nullptr};
	LOAD_ENTRYPOINT("glUniform3i64ARB", pfnUniform3i64ARB, PFNGLUNIFORM3I64ARBPROC);
	pfnUniform3i64ARB(location, x, y, z);
}

void glUniform4i64ARB(GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w)
{
	using PFNGLUNIFORM4I64ARBPROC = void(APIENTRY *)(GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w);
	static PFNGLUNIFORM4I64ARBPROC pfnUniform4i64ARB{nullptr};
	LOAD_ENTRYPOINT("glUniform4i64ARB", pfnUniform4i64ARB, PFNGLUNIFORM4I64ARBPROC);
	pfnUniform4i64ARB(location, x, y, z, w);
}

void glUniform1i64vARB(GLint location, GLsizei count, const GLint64* value)
{
	using PFNGLUNIFORM1I64VARBPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint64* value);
	static PFNGLUNIFORM1I64VARBPROC pfnUniform1i64vARB{nullptr};
	LOAD_ENTRYPOINT("glUniform1i64vARB", pfnUniform1i64vARB, PFNGLUNIFORM1I64VARBPROC);
	pfnUniform1i64vARB(location, count, value);
}

void glUniform2i64vARB(GLint location, GLsizei count, const GLint64* value)
{
	using PFNGLUNIFORM2I64VARBPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint64* value);
	static PFNGLUNIFORM2I64VARBPROC pfnUniform2i64vARB{nullptr};
	LOAD_ENTRYPOINT("glUniform2i64vARB", pfnUniform2i64vARB, PFNGLUNIFORM2I64VARBPROC);
	pfnUniform2i64vARB(location, count, value);
}

void glUniform3i64vARB(GLint location, GLsizei count, const GLint64* value)
{
	using PFNGLUNIFORM3I64VARBPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint64* value);
	static PFNGLUNIFORM3I64VARBPROC pfnUniform3i64vARB{nullptr};
	LOAD_ENTRYPOINT("glUniform3i64vARB", pfnUniform3i64vARB, PFNGLUNIFORM3I64VARBPROC);
	pfnUniform3i64vARB(location, count, value);
}

void glUniform4i64vARB(GLint location, GLsizei count, const GLint64* value)
{
	using PFNGLUNIFORM4I64VARBPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint64* value);
	static PFNGLUNIFORM4I64VARBPROC pfnUniform4i64vARB{nullptr};
	LOAD_ENTRYPOINT("glUniform4i64vARB", pfnUniform4i64vARB, PFNGLUNIFORM4I64VARBPROC);
	pfnUniform4i64vARB(location, count, value);
}

void glUniform1ui64ARB(GLint location, GLuint64 x)
{
	using PFNGLUNIFORM1UI64ARBPROC = void(APIENTRY *)(GLint location, GLuint64 x);
	static PFNGLUNIFORM1UI64ARBPROC pfnUniform1ui64ARB{nullptr};
	LOAD_ENTRYPOINT("glUniform1ui64ARB", pfnUniform1ui64ARB, PFNGLUNIFORM1UI64ARBPROC);
	pfnUniform1ui64ARB(location, x);
}

void glUniform2ui64ARB(GLint location, GLuint64 x, GLuint64 y)
{
	using PFNGLUNIFORM2UI64ARBPROC = void(APIENTRY *)(GLint location, GLuint64 x, GLuint64 y);
	static PFNGLUNIFORM2UI64ARBPROC pfnUniform2ui64ARB{nullptr};
	LOAD_ENTRYPOINT("glUniform2ui64ARB", pfnUniform2ui64ARB, PFNGLUNIFORM2UI64ARBPROC);
	pfnUniform2ui64ARB(location, x, y);
}

void glUniform3ui64ARB(GLint location, GLuint64 x, GLuint64 y, GLuint64 z)
{
	using PFNGLUNIFORM3UI64ARBPROC = void(APIENTRY *)(GLint location, GLuint64 x, GLuint64 y, GLuint64 z);
	static PFNGLUNIFORM3UI64ARBPROC pfnUniform3ui64ARB{nullptr};
	LOAD_ENTRYPOINT("glUniform3ui64ARB", pfnUniform3ui64ARB, PFNGLUNIFORM3UI64ARBPROC);
	pfnUniform3ui64ARB(location, x, y, z);
}

void glUniform4ui64ARB(GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w)
{
	using PFNGLUNIFORM4UI64ARBPROC = void(APIENTRY *)(GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w);
	static PFNGLUNIFORM4UI64ARBPROC pfnUniform4ui64ARB{nullptr};
	LOAD_ENTRYPOINT("glUniform4ui64ARB", pfnUniform4ui64ARB, PFNGLUNIFORM4UI64ARBPROC);
	pfnUniform4ui64ARB(location, x, y, z, w);
}

void glUniform1ui64vARB(GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLUNIFORM1UI64VARBPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64* value);
	static PFNGLUNIFORM1UI64VARBPROC pfnUniform1ui64vARB{nullptr};
	LOAD_ENTRYPOINT("glUniform1ui64vARB", pfnUniform1ui64vARB, PFNGLUNIFORM1UI64VARBPROC);
	pfnUniform1ui64vARB(location, count, value);
}

void glUniform2ui64vARB(GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLUNIFORM2UI64VARBPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64* value);
	static PFNGLUNIFORM2UI64VARBPROC pfnUniform2ui64vARB{nullptr};
	LOAD_ENTRYPOINT("glUniform2ui64vARB", pfnUniform2ui64vARB, PFNGLUNIFORM2UI64VARBPROC);
	pfnUniform2ui64vARB(location, count, value);
}

void glUniform3ui64vARB(GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLUNIFORM3UI64VARBPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64* value);
	static PFNGLUNIFORM3UI64VARBPROC pfnUniform3ui64vARB{nullptr};
	LOAD_ENTRYPOINT("glUniform3ui64vARB", pfnUniform3ui64vARB, PFNGLUNIFORM3UI64VARBPROC);
	pfnUniform3ui64vARB(location, count, value);
}

void glUniform4ui64vARB(GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLUNIFORM4UI64VARBPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64* value);
	static PFNGLUNIFORM4UI64VARBPROC pfnUniform4ui64vARB{nullptr};
	LOAD_ENTRYPOINT("glUniform4ui64vARB", pfnUniform4ui64vARB, PFNGLUNIFORM4UI64VARBPROC);
	pfnUniform4ui64vARB(location, count, value);
}

void glGetUniformi64vARB(GLuint program, GLint location, GLint64* params)
{
	using PFNGLGETUNIFORMI64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64* params);
	static PFNGLGETUNIFORMI64VARBPROC pfnGetUniformi64vARB{nullptr};
	LOAD_ENTRYPOINT("glGetUniformi64vARB", pfnGetUniformi64vARB, PFNGLGETUNIFORMI64VARBPROC);
	pfnGetUniformi64vARB(program, location, params);
}

void glGetUniformui64vARB(GLuint program, GLint location, GLuint64* params)
{
	using PFNGLGETUNIFORMUI64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64* params);
	static PFNGLGETUNIFORMUI64VARBPROC pfnGetUniformui64vARB{nullptr};
	LOAD_ENTRYPOINT("glGetUniformui64vARB", pfnGetUniformui64vARB, PFNGLGETUNIFORMUI64VARBPROC);
	pfnGetUniformui64vARB(program, location, params);
}

void glGetnUniformi64vARB(GLuint program, GLint location, GLsizei bufSize, GLint64* params)
{
	using PFNGLGETNUNIFORMI64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLint64* params);
	static PFNGLGETNUNIFORMI64VARBPROC pfnGetnUniformi64vARB{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformi64vARB", pfnGetnUniformi64vARB, PFNGLGETNUNIFORMI64VARBPROC);
	pfnGetnUniformi64vARB(program, location, bufSize, params);
}

void glGetnUniformui64vARB(GLuint program, GLint location, GLsizei bufSize, GLuint64* params)
{
	using PFNGLGETNUNIFORMUI64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLuint64* params);
	static PFNGLGETNUNIFORMUI64VARBPROC pfnGetnUniformui64vARB{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformui64vARB", pfnGetnUniformui64vARB, PFNGLGETNUNIFORMUI64VARBPROC);
	pfnGetnUniformui64vARB(program, location, bufSize, params);
}

void glProgramUniform1i64ARB(GLuint program, GLint location, GLint64 x)
{
	using PFNGLPROGRAMUNIFORM1I64ARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64 x);
	static PFNGLPROGRAMUNIFORM1I64ARBPROC pfnProgramUniform1i64ARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1i64ARB", pfnProgramUniform1i64ARB, PFNGLPROGRAMUNIFORM1I64ARBPROC);
	pfnProgramUniform1i64ARB(program, location, x);
}

void glProgramUniform2i64ARB(GLuint program, GLint location, GLint64 x, GLint64 y)
{
	using PFNGLPROGRAMUNIFORM2I64ARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64 x, GLint64 y);
	static PFNGLPROGRAMUNIFORM2I64ARBPROC pfnProgramUniform2i64ARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2i64ARB", pfnProgramUniform2i64ARB, PFNGLPROGRAMUNIFORM2I64ARBPROC);
	pfnProgramUniform2i64ARB(program, location, x, y);
}

void glProgramUniform3i64ARB(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z)
{
	using PFNGLPROGRAMUNIFORM3I64ARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z);
	static PFNGLPROGRAMUNIFORM3I64ARBPROC pfnProgramUniform3i64ARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3i64ARB", pfnProgramUniform3i64ARB, PFNGLPROGRAMUNIFORM3I64ARBPROC);
	pfnProgramUniform3i64ARB(program, location, x, y, z);
}

void glProgramUniform4i64ARB(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w)
{
	using PFNGLPROGRAMUNIFORM4I64ARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w);
	static PFNGLPROGRAMUNIFORM4I64ARBPROC pfnProgramUniform4i64ARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4i64ARB", pfnProgramUniform4i64ARB, PFNGLPROGRAMUNIFORM4I64ARBPROC);
	pfnProgramUniform4i64ARB(program, location, x, y, z, w);
}

void glProgramUniform1i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value)
{
	using PFNGLPROGRAMUNIFORM1I64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint64* value);
	static PFNGLPROGRAMUNIFORM1I64VARBPROC pfnProgramUniform1i64vARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1i64vARB", pfnProgramUniform1i64vARB, PFNGLPROGRAMUNIFORM1I64VARBPROC);
	pfnProgramUniform1i64vARB(program, location, count, value);
}

void glProgramUniform2i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value)
{
	using PFNGLPROGRAMUNIFORM2I64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint64* value);
	static PFNGLPROGRAMUNIFORM2I64VARBPROC pfnProgramUniform2i64vARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2i64vARB", pfnProgramUniform2i64vARB, PFNGLPROGRAMUNIFORM2I64VARBPROC);
	pfnProgramUniform2i64vARB(program, location, count, value);
}

void glProgramUniform3i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value)
{
	using PFNGLPROGRAMUNIFORM3I64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint64* value);
	static PFNGLPROGRAMUNIFORM3I64VARBPROC pfnProgramUniform3i64vARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3i64vARB", pfnProgramUniform3i64vARB, PFNGLPROGRAMUNIFORM3I64VARBPROC);
	pfnProgramUniform3i64vARB(program, location, count, value);
}

void glProgramUniform4i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value)
{
	using PFNGLPROGRAMUNIFORM4I64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint64* value);
	static PFNGLPROGRAMUNIFORM4I64VARBPROC pfnProgramUniform4i64vARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4i64vARB", pfnProgramUniform4i64vARB, PFNGLPROGRAMUNIFORM4I64VARBPROC);
	pfnProgramUniform4i64vARB(program, location, count, value);
}

void glProgramUniform1ui64ARB(GLuint program, GLint location, GLuint64 x)
{
	using PFNGLPROGRAMUNIFORM1UI64ARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64 x);
	static PFNGLPROGRAMUNIFORM1UI64ARBPROC pfnProgramUniform1ui64ARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1ui64ARB", pfnProgramUniform1ui64ARB, PFNGLPROGRAMUNIFORM1UI64ARBPROC);
	pfnProgramUniform1ui64ARB(program, location, x);
}

void glProgramUniform2ui64ARB(GLuint program, GLint location, GLuint64 x, GLuint64 y)
{
	using PFNGLPROGRAMUNIFORM2UI64ARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64 x, GLuint64 y);
	static PFNGLPROGRAMUNIFORM2UI64ARBPROC pfnProgramUniform2ui64ARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2ui64ARB", pfnProgramUniform2ui64ARB, PFNGLPROGRAMUNIFORM2UI64ARBPROC);
	pfnProgramUniform2ui64ARB(program, location, x, y);
}

void glProgramUniform3ui64ARB(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z)
{
	using PFNGLPROGRAMUNIFORM3UI64ARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z);
	static PFNGLPROGRAMUNIFORM3UI64ARBPROC pfnProgramUniform3ui64ARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3ui64ARB", pfnProgramUniform3ui64ARB, PFNGLPROGRAMUNIFORM3UI64ARBPROC);
	pfnProgramUniform3ui64ARB(program, location, x, y, z);
}

void glProgramUniform4ui64ARB(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w)
{
	using PFNGLPROGRAMUNIFORM4UI64ARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w);
	static PFNGLPROGRAMUNIFORM4UI64ARBPROC pfnProgramUniform4ui64ARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4ui64ARB", pfnProgramUniform4ui64ARB, PFNGLPROGRAMUNIFORM4UI64ARBPROC);
	pfnProgramUniform4ui64ARB(program, location, x, y, z, w);
}

void glProgramUniform1ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLPROGRAMUNIFORM1UI64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64* value);
	static PFNGLPROGRAMUNIFORM1UI64VARBPROC pfnProgramUniform1ui64vARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1ui64vARB", pfnProgramUniform1ui64vARB, PFNGLPROGRAMUNIFORM1UI64VARBPROC);
	pfnProgramUniform1ui64vARB(program, location, count, value);
}

void glProgramUniform2ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLPROGRAMUNIFORM2UI64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64* value);
	static PFNGLPROGRAMUNIFORM2UI64VARBPROC pfnProgramUniform2ui64vARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2ui64vARB", pfnProgramUniform2ui64vARB, PFNGLPROGRAMUNIFORM2UI64VARBPROC);
	pfnProgramUniform2ui64vARB(program, location, count, value);
}

void glProgramUniform3ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLPROGRAMUNIFORM3UI64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64* value);
	static PFNGLPROGRAMUNIFORM3UI64VARBPROC pfnProgramUniform3ui64vARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3ui64vARB", pfnProgramUniform3ui64vARB, PFNGLPROGRAMUNIFORM3UI64VARBPROC);
	pfnProgramUniform3ui64vARB(program, location, count, value);
}

void glProgramUniform4ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLPROGRAMUNIFORM4UI64VARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64* value);
	static PFNGLPROGRAMUNIFORM4UI64VARBPROC pfnProgramUniform4ui64vARB{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4ui64vARB", pfnProgramUniform4ui64vARB, PFNGLPROGRAMUNIFORM4UI64VARBPROC);
	pfnProgramUniform4ui64vARB(program, location, count, value);
}

//
// GL_ARB_indirect_parameters
//

void glMultiDrawArraysIndirectCountARB(GLenum mode, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
	using PFNGLMULTIDRAWARRAYSINDIRECTCOUNTARBPROC = void(APIENTRY *)(GLenum mode, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
	static PFNGLMULTIDRAWARRAYSINDIRECTCOUNTARBPROC pfnMultiDrawArraysIndirectCountARB{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawArraysIndirectCountARB", pfnMultiDrawArraysIndirectCountARB, PFNGLMULTIDRAWARRAYSINDIRECTCOUNTARBPROC);
	pfnMultiDrawArraysIndirectCountARB(mode, indirect, drawcount, maxdrawcount, stride);
}

void glMultiDrawElementsIndirectCountARB(GLenum mode, GLenum type, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
	using PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTARBPROC = void(APIENTRY *)(GLenum mode, GLenum type, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
	static PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTARBPROC pfnMultiDrawElementsIndirectCountARB{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawElementsIndirectCountARB", pfnMultiDrawElementsIndirectCountARB, PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTARBPROC);
	pfnMultiDrawElementsIndirectCountARB(mode, type, indirect, drawcount, maxdrawcount, stride);
}

//
// GL_ARB_instanced_arrays
//

void glVertexAttribDivisorARB(GLuint index, GLuint divisor)
{
	using PFNGLVERTEXATTRIBDIVISORARBPROC = void(APIENTRY *)(GLuint index, GLuint divisor);
	static PFNGLVERTEXATTRIBDIVISORARBPROC pfnVertexAttribDivisorARB{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribDivisorARB", pfnVertexAttribDivisorARB, PFNGLVERTEXATTRIBDIVISORARBPROC);
	pfnVertexAttribDivisorARB(index, divisor);
}

//
// GL_ARB_parallel_shader_compile
//

void glMaxShaderCompilerThreadsARB(GLuint count)
{
	using PFNGLMAXSHADERCOMPILERTHREADSARBPROC = void(APIENTRY *)(GLuint count);
	static PFNGLMAXSHADERCOMPILERTHREADSARBPROC pfnMaxShaderCompilerThreadsARB{nullptr};
	LOAD_ENTRYPOINT("glMaxShaderCompilerThreadsARB", pfnMaxShaderCompilerThreadsARB, PFNGLMAXSHADERCOMPILERTHREADSARBPROC);
	pfnMaxShaderCompilerThreadsARB(count);
}

//
// GL_ARB_robustness
//

GLenum glGetGraphicsResetStatusARB(void)
{
	using PFNGLGETGRAPHICSRESETSTATUSARBPROC = GLenum(APIENTRY *)(void);
	static PFNGLGETGRAPHICSRESETSTATUSARBPROC pfnGetGraphicsResetStatusARB{nullptr};
	LOAD_ENTRYPOINT("glGetGraphicsResetStatusARB", pfnGetGraphicsResetStatusARB, PFNGLGETGRAPHICSRESETSTATUSARBPROC);
	return pfnGetGraphicsResetStatusARB();
}

void glGetnTexImageARB(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* img)
{
	using PFNGLGETNTEXIMAGEARBPROC = void(APIENTRY *)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* img);
	static PFNGLGETNTEXIMAGEARBPROC pfnGetnTexImageARB{nullptr};
	LOAD_ENTRYPOINT("glGetnTexImageARB", pfnGetnTexImageARB, PFNGLGETNTEXIMAGEARBPROC);
	pfnGetnTexImageARB(target, level, format, type, bufSize, img);
}

void glReadnPixelsARB(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data)
{
	using PFNGLREADNPIXELSARBPROC = void(APIENTRY *)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data);
	static PFNGLREADNPIXELSARBPROC pfnReadnPixelsARB{nullptr};
	LOAD_ENTRYPOINT("glReadnPixelsARB", pfnReadnPixelsARB, PFNGLREADNPIXELSARBPROC);
	pfnReadnPixelsARB(x, y, width, height, format, type, bufSize, data);
}

void glGetnCompressedTexImageARB(GLenum target, GLint lod, GLsizei bufSize, void* img)
{
	using PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC = void(APIENTRY *)(GLenum target, GLint lod, GLsizei bufSize, void* img);
	static PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC pfnGetnCompressedTexImageARB{nullptr};
	LOAD_ENTRYPOINT("glGetnCompressedTexImageARB", pfnGetnCompressedTexImageARB, PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC);
	pfnGetnCompressedTexImageARB(target, lod, bufSize, img);
}

void glGetnUniformfvARB(GLuint program, GLint location, GLsizei bufSize, GLfloat* params)
{
	using PFNGLGETNUNIFORMFVARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLfloat* params);
	static PFNGLGETNUNIFORMFVARBPROC pfnGetnUniformfvARB{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformfvARB", pfnGetnUniformfvARB, PFNGLGETNUNIFORMFVARBPROC);
	pfnGetnUniformfvARB(program, location, bufSize, params);
}

void glGetnUniformivARB(GLuint program, GLint location, GLsizei bufSize, GLint* params)
{
	using PFNGLGETNUNIFORMIVARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLint* params);
	static PFNGLGETNUNIFORMIVARBPROC pfnGetnUniformivARB{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformivARB", pfnGetnUniformivARB, PFNGLGETNUNIFORMIVARBPROC);
	pfnGetnUniformivARB(program, location, bufSize, params);
}

void glGetnUniformuivARB(GLuint program, GLint location, GLsizei bufSize, GLuint* params)
{
	using PFNGLGETNUNIFORMUIVARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLuint* params);
	static PFNGLGETNUNIFORMUIVARBPROC pfnGetnUniformuivARB{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformuivARB", pfnGetnUniformuivARB, PFNGLGETNUNIFORMUIVARBPROC);
	pfnGetnUniformuivARB(program, location, bufSize, params);
}

void glGetnUniformdvARB(GLuint program, GLint location, GLsizei bufSize, GLdouble* params)
{
	using PFNGLGETNUNIFORMDVARBPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei bufSize, GLdouble* params);
	static PFNGLGETNUNIFORMDVARBPROC pfnGetnUniformdvARB{nullptr};
	LOAD_ENTRYPOINT("glGetnUniformdvARB", pfnGetnUniformdvARB, PFNGLGETNUNIFORMDVARBPROC);
	pfnGetnUniformdvARB(program, location, bufSize, params);
}

//
// GL_ARB_sample_locations
//

void glFramebufferSampleLocationsfvARB(GLenum target, GLuint start, GLsizei count, const GLfloat* v)
{
	using PFNGLFRAMEBUFFERSAMPLELOCATIONSFVARBPROC = void(APIENTRY *)(GLenum target, GLuint start, GLsizei count, const GLfloat* v);
	static PFNGLFRAMEBUFFERSAMPLELOCATIONSFVARBPROC pfnFramebufferSampleLocationsfvARB{nullptr};
	LOAD_ENTRYPOINT("glFramebufferSampleLocationsfvARB", pfnFramebufferSampleLocationsfvARB, PFNGLFRAMEBUFFERSAMPLELOCATIONSFVARBPROC);
	pfnFramebufferSampleLocationsfvARB(target, start, count, v);
}

void glNamedFramebufferSampleLocationsfvARB(GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v)
{
	using PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVARBPROC = void(APIENTRY *)(GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v);
	static PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVARBPROC pfnNamedFramebufferSampleLocationsfvARB{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferSampleLocationsfvARB", pfnNamedFramebufferSampleLocationsfvARB, PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVARBPROC);
	pfnNamedFramebufferSampleLocationsfvARB(framebuffer, start, count, v);
}

void glEvaluateDepthValuesARB(void)
{
	using PFNGLEVALUATEDEPTHVALUESARBPROC = void(APIENTRY *)(void);
	static PFNGLEVALUATEDEPTHVALUESARBPROC pfnEvaluateDepthValuesARB{nullptr};
	LOAD_ENTRYPOINT("glEvaluateDepthValuesARB", pfnEvaluateDepthValuesARB, PFNGLEVALUATEDEPTHVALUESARBPROC);
	pfnEvaluateDepthValuesARB();
}

//
// GL_ARB_sample_shading
//

void glMinSampleShadingARB(GLfloat value)
{
	using PFNGLMINSAMPLESHADINGARBPROC = void(APIENTRY *)(GLfloat value);
	static PFNGLMINSAMPLESHADINGARBPROC pfnMinSampleShadingARB{nullptr};
	LOAD_ENTRYPOINT("glMinSampleShadingARB", pfnMinSampleShadingARB, PFNGLMINSAMPLESHADINGARBPROC);
	pfnMinSampleShadingARB(value);
}

//
// GL_ARB_shading_language_include
//

void glNamedStringARB(GLenum type, GLint namelen, const GLchar* name, GLint stringlen, const GLchar* string)
{
	using PFNGLNAMEDSTRINGARBPROC = void(APIENTRY *)(GLenum type, GLint namelen, const GLchar* name, GLint stringlen, const GLchar* string);
	static PFNGLNAMEDSTRINGARBPROC pfnNamedStringARB{nullptr};
	LOAD_ENTRYPOINT("glNamedStringARB", pfnNamedStringARB, PFNGLNAMEDSTRINGARBPROC);
	pfnNamedStringARB(type, namelen, name, stringlen, string);
}

void glDeleteNamedStringARB(GLint namelen, const GLchar* name)
{
	using PFNGLDELETENAMEDSTRINGARBPROC = void(APIENTRY *)(GLint namelen, const GLchar* name);
	static PFNGLDELETENAMEDSTRINGARBPROC pfnDeleteNamedStringARB{nullptr};
	LOAD_ENTRYPOINT("glDeleteNamedStringARB", pfnDeleteNamedStringARB, PFNGLDELETENAMEDSTRINGARBPROC);
	pfnDeleteNamedStringARB(namelen, name);
}

void glCompileShaderIncludeARB(GLuint shader, GLsizei count, const GLchar* const* path, const GLint* length)
{
	using PFNGLCOMPILESHADERINCLUDEARBPROC = void(APIENTRY *)(GLuint shader, GLsizei count, const GLchar* const* path, const GLint* length);
	static PFNGLCOMPILESHADERINCLUDEARBPROC pfnCompileShaderIncludeARB{nullptr};
	LOAD_ENTRYPOINT("glCompileShaderIncludeARB", pfnCompileShaderIncludeARB, PFNGLCOMPILESHADERINCLUDEARBPROC);
	pfnCompileShaderIncludeARB(shader, count, path, length);
}

GLboolean glIsNamedStringARB(GLint namelen, const GLchar* name)
{
	using PFNGLISNAMEDSTRINGARBPROC = GLboolean(APIENTRY *)(GLint namelen, const GLchar* name);
	static PFNGLISNAMEDSTRINGARBPROC pfnIsNamedStringARB{nullptr};
	LOAD_ENTRYPOINT("glIsNamedStringARB", pfnIsNamedStringARB, PFNGLISNAMEDSTRINGARBPROC);
	return pfnIsNamedStringARB(namelen, name);
}

void glGetNamedStringARB(GLint namelen, const GLchar* name, GLsizei bufSize, GLint* stringlen, GLchar* string)
{
	using PFNGLGETNAMEDSTRINGARBPROC = void(APIENTRY *)(GLint namelen, const GLchar* name, GLsizei bufSize, GLint* stringlen, GLchar* string);
	static PFNGLGETNAMEDSTRINGARBPROC pfnGetNamedStringARB{nullptr};
	LOAD_ENTRYPOINT("glGetNamedStringARB", pfnGetNamedStringARB, PFNGLGETNAMEDSTRINGARBPROC);
	pfnGetNamedStringARB(namelen, name, bufSize, stringlen, string);
}

void glGetNamedStringivARB(GLint namelen, const GLchar* name, GLenum pname, GLint* params)
{
	using PFNGLGETNAMEDSTRINGIVARBPROC = void(APIENTRY *)(GLint namelen, const GLchar* name, GLenum pname, GLint* params);
	static PFNGLGETNAMEDSTRINGIVARBPROC pfnGetNamedStringivARB{nullptr};
	LOAD_ENTRYPOINT("glGetNamedStringivARB", pfnGetNamedStringivARB, PFNGLGETNAMEDSTRINGIVARBPROC);
	pfnGetNamedStringivARB(namelen, name, pname, params);
}

//
// GL_ARB_sparse_buffer
//

void glBufferPageCommitmentARB(GLenum target, GLintptr offset, GLsizeiptr size, GLboolean commit)
{
	using PFNGLBUFFERPAGECOMMITMENTARBPROC = void(APIENTRY *)(GLenum target, GLintptr offset, GLsizeiptr size, GLboolean commit);
	static PFNGLBUFFERPAGECOMMITMENTARBPROC pfnBufferPageCommitmentARB{nullptr};
	LOAD_ENTRYPOINT("glBufferPageCommitmentARB", pfnBufferPageCommitmentARB, PFNGLBUFFERPAGECOMMITMENTARBPROC);
	pfnBufferPageCommitmentARB(target, offset, size, commit);
}

void glNamedBufferPageCommitmentEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit)
{
	using PFNGLNAMEDBUFFERPAGECOMMITMENTEXTPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit);
	static PFNGLNAMEDBUFFERPAGECOMMITMENTEXTPROC pfnNamedBufferPageCommitmentEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferPageCommitmentEXT", pfnNamedBufferPageCommitmentEXT, PFNGLNAMEDBUFFERPAGECOMMITMENTEXTPROC);
	pfnNamedBufferPageCommitmentEXT(buffer, offset, size, commit);
}

void glNamedBufferPageCommitmentARB(GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit)
{
	using PFNGLNAMEDBUFFERPAGECOMMITMENTARBPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit);
	static PFNGLNAMEDBUFFERPAGECOMMITMENTARBPROC pfnNamedBufferPageCommitmentARB{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferPageCommitmentARB", pfnNamedBufferPageCommitmentARB, PFNGLNAMEDBUFFERPAGECOMMITMENTARBPROC);
	pfnNamedBufferPageCommitmentARB(buffer, offset, size, commit);
}

//
// GL_ARB_sparse_texture
//

void glTexPageCommitmentARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit)
{
	using PFNGLTEXPAGECOMMITMENTARBPROC = void(APIENTRY *)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
	static PFNGLTEXPAGECOMMITMENTARBPROC pfnTexPageCommitmentARB{nullptr};
	LOAD_ENTRYPOINT("glTexPageCommitmentARB", pfnTexPageCommitmentARB, PFNGLTEXPAGECOMMITMENTARBPROC);
	pfnTexPageCommitmentARB(target, level, xoffset, yoffset, zoffset, width, height, depth, commit);
}

//
// GL_ARB_texture_buffer_object
//

void glTexBufferARB(GLenum target, GLenum internalformat, GLuint buffer)
{
	using PFNGLTEXBUFFERARBPROC = void(APIENTRY *)(GLenum target, GLenum internalformat, GLuint buffer);
	static PFNGLTEXBUFFERARBPROC pfnTexBufferARB{nullptr};
	LOAD_ENTRYPOINT("glTexBufferARB", pfnTexBufferARB, PFNGLTEXBUFFERARBPROC);
	pfnTexBufferARB(target, internalformat, buffer);
}

//
// GL_ARB_viewport_array
//

void glDepthRangeArraydvNV(GLuint first, GLsizei count, const GLdouble* v)
{
	using PFNGLDEPTHRANGEARRAYDVNVPROC = void(APIENTRY *)(GLuint first, GLsizei count, const GLdouble* v);
	static PFNGLDEPTHRANGEARRAYDVNVPROC pfnDepthRangeArraydvNV{nullptr};
	LOAD_ENTRYPOINT("glDepthRangeArraydvNV", pfnDepthRangeArraydvNV, PFNGLDEPTHRANGEARRAYDVNVPROC);
	pfnDepthRangeArraydvNV(first, count, v);
}

void glDepthRangeIndexeddNV(GLuint index, GLdouble n, GLdouble f)
{
	using PFNGLDEPTHRANGEINDEXEDDNVPROC = void(APIENTRY *)(GLuint index, GLdouble n, GLdouble f);
	static PFNGLDEPTHRANGEINDEXEDDNVPROC pfnDepthRangeIndexeddNV{nullptr};
	LOAD_ENTRYPOINT("glDepthRangeIndexeddNV", pfnDepthRangeIndexeddNV, PFNGLDEPTHRANGEINDEXEDDNVPROC);
	pfnDepthRangeIndexeddNV(index, n, f);
}

//
// GL_KHR_blend_equation_advanced
//

void glBlendBarrierKHR(void)
{
	using PFNGLBLENDBARRIERKHRPROC = void(APIENTRY *)(void);
	static PFNGLBLENDBARRIERKHRPROC pfnBlendBarrierKHR{nullptr};
	LOAD_ENTRYPOINT("glBlendBarrierKHR", pfnBlendBarrierKHR, PFNGLBLENDBARRIERKHRPROC);
	pfnBlendBarrierKHR();
}

//
// GL_KHR_parallel_shader_compile
//

void glMaxShaderCompilerThreadsKHR(GLuint count)
{
	using PFNGLMAXSHADERCOMPILERTHREADSKHRPROC = void(APIENTRY *)(GLuint count);
	static PFNGLMAXSHADERCOMPILERTHREADSKHRPROC pfnMaxShaderCompilerThreadsKHR{nullptr};
	LOAD_ENTRYPOINT("glMaxShaderCompilerThreadsKHR", pfnMaxShaderCompilerThreadsKHR, PFNGLMAXSHADERCOMPILERTHREADSKHRPROC);
	pfnMaxShaderCompilerThreadsKHR(count);
}

//
// GL_AMD_framebuffer_multisample_advanced
//

void glRenderbufferStorageMultisampleAdvancedAMD(GLenum target, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC = void(APIENTRY *)(GLenum target, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC pfnRenderbufferStorageMultisampleAdvancedAMD{nullptr};
	LOAD_ENTRYPOINT("glRenderbufferStorageMultisampleAdvancedAMD", pfnRenderbufferStorageMultisampleAdvancedAMD, PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC);
	pfnRenderbufferStorageMultisampleAdvancedAMD(target, samples, storageSamples, internalformat, width, height);
}

void glNamedRenderbufferStorageMultisampleAdvancedAMD(GLuint renderbuffer, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC = void(APIENTRY *)(GLuint renderbuffer, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC pfnNamedRenderbufferStorageMultisampleAdvancedAMD{nullptr};
	LOAD_ENTRYPOINT("glNamedRenderbufferStorageMultisampleAdvancedAMD", pfnNamedRenderbufferStorageMultisampleAdvancedAMD, PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC);
	pfnNamedRenderbufferStorageMultisampleAdvancedAMD(renderbuffer, samples, storageSamples, internalformat, width, height);
}

//
// GL_AMD_performance_monitor
//

void glGetPerfMonitorGroupsAMD(GLint* numGroups, GLsizei groupsSize, GLuint* groups)
{
	using PFNGLGETPERFMONITORGROUPSAMDPROC = void(APIENTRY *)(GLint* numGroups, GLsizei groupsSize, GLuint* groups);
	static PFNGLGETPERFMONITORGROUPSAMDPROC pfnGetPerfMonitorGroupsAMD{nullptr};
	LOAD_ENTRYPOINT("glGetPerfMonitorGroupsAMD", pfnGetPerfMonitorGroupsAMD, PFNGLGETPERFMONITORGROUPSAMDPROC);
	pfnGetPerfMonitorGroupsAMD(numGroups, groupsSize, groups);
}

void glGetPerfMonitorCountersAMD(GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters)
{
	using PFNGLGETPERFMONITORCOUNTERSAMDPROC = void(APIENTRY *)(GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters);
	static PFNGLGETPERFMONITORCOUNTERSAMDPROC pfnGetPerfMonitorCountersAMD{nullptr};
	LOAD_ENTRYPOINT("glGetPerfMonitorCountersAMD", pfnGetPerfMonitorCountersAMD, PFNGLGETPERFMONITORCOUNTERSAMDPROC);
	pfnGetPerfMonitorCountersAMD(group, numCounters, maxActiveCounters, counterSize, counters);
}

void glGetPerfMonitorGroupStringAMD(GLuint group, GLsizei bufSize, GLsizei* length, GLchar* groupString)
{
	using PFNGLGETPERFMONITORGROUPSTRINGAMDPROC = void(APIENTRY *)(GLuint group, GLsizei bufSize, GLsizei* length, GLchar* groupString);
	static PFNGLGETPERFMONITORGROUPSTRINGAMDPROC pfnGetPerfMonitorGroupStringAMD{nullptr};
	LOAD_ENTRYPOINT("glGetPerfMonitorGroupStringAMD", pfnGetPerfMonitorGroupStringAMD, PFNGLGETPERFMONITORGROUPSTRINGAMDPROC);
	pfnGetPerfMonitorGroupStringAMD(group, bufSize, length, groupString);
}

void glGetPerfMonitorCounterStringAMD(GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, GLchar* counterString)
{
	using PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC = void(APIENTRY *)(GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, GLchar* counterString);
	static PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC pfnGetPerfMonitorCounterStringAMD{nullptr};
	LOAD_ENTRYPOINT("glGetPerfMonitorCounterStringAMD", pfnGetPerfMonitorCounterStringAMD, PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC);
	pfnGetPerfMonitorCounterStringAMD(group, counter, bufSize, length, counterString);
}

void glGetPerfMonitorCounterInfoAMD(GLuint group, GLuint counter, GLenum pname, void* data)
{
	using PFNGLGETPERFMONITORCOUNTERINFOAMDPROC = void(APIENTRY *)(GLuint group, GLuint counter, GLenum pname, void* data);
	static PFNGLGETPERFMONITORCOUNTERINFOAMDPROC pfnGetPerfMonitorCounterInfoAMD{nullptr};
	LOAD_ENTRYPOINT("glGetPerfMonitorCounterInfoAMD", pfnGetPerfMonitorCounterInfoAMD, PFNGLGETPERFMONITORCOUNTERINFOAMDPROC);
	pfnGetPerfMonitorCounterInfoAMD(group, counter, pname, data);
}

void glGenPerfMonitorsAMD(GLsizei n, GLuint* monitors)
{
	using PFNGLGENPERFMONITORSAMDPROC = void(APIENTRY *)(GLsizei n, GLuint* monitors);
	static PFNGLGENPERFMONITORSAMDPROC pfnGenPerfMonitorsAMD{nullptr};
	LOAD_ENTRYPOINT("glGenPerfMonitorsAMD", pfnGenPerfMonitorsAMD, PFNGLGENPERFMONITORSAMDPROC);
	pfnGenPerfMonitorsAMD(n, monitors);
}

void glDeletePerfMonitorsAMD(GLsizei n, GLuint* monitors)
{
	using PFNGLDELETEPERFMONITORSAMDPROC = void(APIENTRY *)(GLsizei n, GLuint* monitors);
	static PFNGLDELETEPERFMONITORSAMDPROC pfnDeletePerfMonitorsAMD{nullptr};
	LOAD_ENTRYPOINT("glDeletePerfMonitorsAMD", pfnDeletePerfMonitorsAMD, PFNGLDELETEPERFMONITORSAMDPROC);
	pfnDeletePerfMonitorsAMD(n, monitors);
}

void glSelectPerfMonitorCountersAMD(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* counterList)
{
	using PFNGLSELECTPERFMONITORCOUNTERSAMDPROC = void(APIENTRY *)(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* counterList);
	static PFNGLSELECTPERFMONITORCOUNTERSAMDPROC pfnSelectPerfMonitorCountersAMD{nullptr};
	LOAD_ENTRYPOINT("glSelectPerfMonitorCountersAMD", pfnSelectPerfMonitorCountersAMD, PFNGLSELECTPERFMONITORCOUNTERSAMDPROC);
	pfnSelectPerfMonitorCountersAMD(monitor, enable, group, numCounters, counterList);
}

void glBeginPerfMonitorAMD(GLuint monitor)
{
	using PFNGLBEGINPERFMONITORAMDPROC = void(APIENTRY *)(GLuint monitor);
	static PFNGLBEGINPERFMONITORAMDPROC pfnBeginPerfMonitorAMD{nullptr};
	LOAD_ENTRYPOINT("glBeginPerfMonitorAMD", pfnBeginPerfMonitorAMD, PFNGLBEGINPERFMONITORAMDPROC);
	pfnBeginPerfMonitorAMD(monitor);
}

void glEndPerfMonitorAMD(GLuint monitor)
{
	using PFNGLENDPERFMONITORAMDPROC = void(APIENTRY *)(GLuint monitor);
	static PFNGLENDPERFMONITORAMDPROC pfnEndPerfMonitorAMD{nullptr};
	LOAD_ENTRYPOINT("glEndPerfMonitorAMD", pfnEndPerfMonitorAMD, PFNGLENDPERFMONITORAMDPROC);
	pfnEndPerfMonitorAMD(monitor);
}

void glGetPerfMonitorCounterDataAMD(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten)
{
	using PFNGLGETPERFMONITORCOUNTERDATAAMDPROC = void(APIENTRY *)(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten);
	static PFNGLGETPERFMONITORCOUNTERDATAAMDPROC pfnGetPerfMonitorCounterDataAMD{nullptr};
	LOAD_ENTRYPOINT("glGetPerfMonitorCounterDataAMD", pfnGetPerfMonitorCounterDataAMD, PFNGLGETPERFMONITORCOUNTERDATAAMDPROC);
	pfnGetPerfMonitorCounterDataAMD(monitor, pname, dataSize, data, bytesWritten);
}

//
// GL_EXT_EGL_image_storage
//

void glEGLImageTargetTexStorageEXT(GLenum target, GLeglImageOES image, const GLint* attrib_list)
{
	using PFNGLEGLIMAGETARGETTEXSTORAGEEXTPROC = void(APIENTRY *)(GLenum target, GLeglImageOES image, const GLint* attrib_list);
	static PFNGLEGLIMAGETARGETTEXSTORAGEEXTPROC pfnEGLImageTargetTexStorageEXT{nullptr};
	LOAD_ENTRYPOINT("glEGLImageTargetTexStorageEXT", pfnEGLImageTargetTexStorageEXT, PFNGLEGLIMAGETARGETTEXSTORAGEEXTPROC);
	pfnEGLImageTargetTexStorageEXT(target, image, attrib_list);
}

void glEGLImageTargetTextureStorageEXT(GLuint texture, GLeglImageOES image, const GLint* attrib_list)
{
	using PFNGLEGLIMAGETARGETTEXTURESTORAGEEXTPROC = void(APIENTRY *)(GLuint texture, GLeglImageOES image, const GLint* attrib_list);
	static PFNGLEGLIMAGETARGETTEXTURESTORAGEEXTPROC pfnEGLImageTargetTextureStorageEXT{nullptr};
	LOAD_ENTRYPOINT("glEGLImageTargetTextureStorageEXT", pfnEGLImageTargetTextureStorageEXT, PFNGLEGLIMAGETARGETTEXTURESTORAGEEXTPROC);
	pfnEGLImageTargetTextureStorageEXT(texture, image, attrib_list);
}

//
// GL_EXT_debug_label
//

void glLabelObjectEXT(GLenum type, GLuint object, GLsizei length, const GLchar* label)
{
	using PFNGLLABELOBJECTEXTPROC = void(APIENTRY *)(GLenum type, GLuint object, GLsizei length, const GLchar* label);
	static PFNGLLABELOBJECTEXTPROC pfnLabelObjectEXT{nullptr};
	LOAD_ENTRYPOINT("glLabelObjectEXT", pfnLabelObjectEXT, PFNGLLABELOBJECTEXTPROC);
	pfnLabelObjectEXT(type, object, length, label);
}

void glGetObjectLabelEXT(GLenum type, GLuint object, GLsizei bufSize, GLsizei* length, GLchar* label)
{
	using PFNGLGETOBJECTLABELEXTPROC = void(APIENTRY *)(GLenum type, GLuint object, GLsizei bufSize, GLsizei* length, GLchar* label);
	static PFNGLGETOBJECTLABELEXTPROC pfnGetObjectLabelEXT{nullptr};
	LOAD_ENTRYPOINT("glGetObjectLabelEXT", pfnGetObjectLabelEXT, PFNGLGETOBJECTLABELEXTPROC);
	pfnGetObjectLabelEXT(type, object, bufSize, length, label);
}

//
// GL_EXT_debug_marker
//

void glInsertEventMarkerEXT(GLsizei length, const GLchar* marker)
{
	using PFNGLINSERTEVENTMARKEREXTPROC = void(APIENTRY *)(GLsizei length, const GLchar* marker);
	static PFNGLINSERTEVENTMARKEREXTPROC pfnInsertEventMarkerEXT{nullptr};
	LOAD_ENTRYPOINT("glInsertEventMarkerEXT", pfnInsertEventMarkerEXT, PFNGLINSERTEVENTMARKEREXTPROC);
	pfnInsertEventMarkerEXT(length, marker);
}

void glPushGroupMarkerEXT(GLsizei length, const GLchar* marker)
{
	using PFNGLPUSHGROUPMARKEREXTPROC = void(APIENTRY *)(GLsizei length, const GLchar* marker);
	static PFNGLPUSHGROUPMARKEREXTPROC pfnPushGroupMarkerEXT{nullptr};
	LOAD_ENTRYPOINT("glPushGroupMarkerEXT", pfnPushGroupMarkerEXT, PFNGLPUSHGROUPMARKEREXTPROC);
	pfnPushGroupMarkerEXT(length, marker);
}

void glPopGroupMarkerEXT(void)
{
	using PFNGLPOPGROUPMARKEREXTPROC = void(APIENTRY *)(void);
	static PFNGLPOPGROUPMARKEREXTPROC pfnPopGroupMarkerEXT{nullptr};
	LOAD_ENTRYPOINT("glPopGroupMarkerEXT", pfnPopGroupMarkerEXT, PFNGLPOPGROUPMARKEREXTPROC);
	pfnPopGroupMarkerEXT();
}

//
// GL_EXT_direct_state_access
//

void glMatrixLoadfEXT(GLenum mode, const GLfloat* m)
{
	using PFNGLMATRIXLOADFEXTPROC = void(APIENTRY *)(GLenum mode, const GLfloat* m);
	static PFNGLMATRIXLOADFEXTPROC pfnMatrixLoadfEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixLoadfEXT", pfnMatrixLoadfEXT, PFNGLMATRIXLOADFEXTPROC);
	pfnMatrixLoadfEXT(mode, m);
}

void glMatrixLoaddEXT(GLenum mode, const GLdouble* m)
{
	using PFNGLMATRIXLOADDEXTPROC = void(APIENTRY *)(GLenum mode, const GLdouble* m);
	static PFNGLMATRIXLOADDEXTPROC pfnMatrixLoaddEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixLoaddEXT", pfnMatrixLoaddEXT, PFNGLMATRIXLOADDEXTPROC);
	pfnMatrixLoaddEXT(mode, m);
}

void glMatrixMultfEXT(GLenum mode, const GLfloat* m)
{
	using PFNGLMATRIXMULTFEXTPROC = void(APIENTRY *)(GLenum mode, const GLfloat* m);
	static PFNGLMATRIXMULTFEXTPROC pfnMatrixMultfEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixMultfEXT", pfnMatrixMultfEXT, PFNGLMATRIXMULTFEXTPROC);
	pfnMatrixMultfEXT(mode, m);
}

void glMatrixMultdEXT(GLenum mode, const GLdouble* m)
{
	using PFNGLMATRIXMULTDEXTPROC = void(APIENTRY *)(GLenum mode, const GLdouble* m);
	static PFNGLMATRIXMULTDEXTPROC pfnMatrixMultdEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixMultdEXT", pfnMatrixMultdEXT, PFNGLMATRIXMULTDEXTPROC);
	pfnMatrixMultdEXT(mode, m);
}

void glMatrixLoadIdentityEXT(GLenum mode)
{
	using PFNGLMATRIXLOADIDENTITYEXTPROC = void(APIENTRY *)(GLenum mode);
	static PFNGLMATRIXLOADIDENTITYEXTPROC pfnMatrixLoadIdentityEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixLoadIdentityEXT", pfnMatrixLoadIdentityEXT, PFNGLMATRIXLOADIDENTITYEXTPROC);
	pfnMatrixLoadIdentityEXT(mode);
}

void glMatrixRotatefEXT(GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	using PFNGLMATRIXROTATEFEXTPROC = void(APIENTRY *)(GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	static PFNGLMATRIXROTATEFEXTPROC pfnMatrixRotatefEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixRotatefEXT", pfnMatrixRotatefEXT, PFNGLMATRIXROTATEFEXTPROC);
	pfnMatrixRotatefEXT(mode, angle, x, y, z);
}

void glMatrixRotatedEXT(GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	using PFNGLMATRIXROTATEDEXTPROC = void(APIENTRY *)(GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLMATRIXROTATEDEXTPROC pfnMatrixRotatedEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixRotatedEXT", pfnMatrixRotatedEXT, PFNGLMATRIXROTATEDEXTPROC);
	pfnMatrixRotatedEXT(mode, angle, x, y, z);
}

void glMatrixScalefEXT(GLenum mode, GLfloat x, GLfloat y, GLfloat z)
{
	using PFNGLMATRIXSCALEFEXTPROC = void(APIENTRY *)(GLenum mode, GLfloat x, GLfloat y, GLfloat z);
	static PFNGLMATRIXSCALEFEXTPROC pfnMatrixScalefEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixScalefEXT", pfnMatrixScalefEXT, PFNGLMATRIXSCALEFEXTPROC);
	pfnMatrixScalefEXT(mode, x, y, z);
}

void glMatrixScaledEXT(GLenum mode, GLdouble x, GLdouble y, GLdouble z)
{
	using PFNGLMATRIXSCALEDEXTPROC = void(APIENTRY *)(GLenum mode, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLMATRIXSCALEDEXTPROC pfnMatrixScaledEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixScaledEXT", pfnMatrixScaledEXT, PFNGLMATRIXSCALEDEXTPROC);
	pfnMatrixScaledEXT(mode, x, y, z);
}

void glMatrixTranslatefEXT(GLenum mode, GLfloat x, GLfloat y, GLfloat z)
{
	using PFNGLMATRIXTRANSLATEFEXTPROC = void(APIENTRY *)(GLenum mode, GLfloat x, GLfloat y, GLfloat z);
	static PFNGLMATRIXTRANSLATEFEXTPROC pfnMatrixTranslatefEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixTranslatefEXT", pfnMatrixTranslatefEXT, PFNGLMATRIXTRANSLATEFEXTPROC);
	pfnMatrixTranslatefEXT(mode, x, y, z);
}

void glMatrixTranslatedEXT(GLenum mode, GLdouble x, GLdouble y, GLdouble z)
{
	using PFNGLMATRIXTRANSLATEDEXTPROC = void(APIENTRY *)(GLenum mode, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLMATRIXTRANSLATEDEXTPROC pfnMatrixTranslatedEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixTranslatedEXT", pfnMatrixTranslatedEXT, PFNGLMATRIXTRANSLATEDEXTPROC);
	pfnMatrixTranslatedEXT(mode, x, y, z);
}

void glMatrixFrustumEXT(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	using PFNGLMATRIXFRUSTUMEXTPROC = void(APIENTRY *)(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	static PFNGLMATRIXFRUSTUMEXTPROC pfnMatrixFrustumEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixFrustumEXT", pfnMatrixFrustumEXT, PFNGLMATRIXFRUSTUMEXTPROC);
	pfnMatrixFrustumEXT(mode, left, right, bottom, top, zNear, zFar);
}

void glMatrixOrthoEXT(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	using PFNGLMATRIXORTHOEXTPROC = void(APIENTRY *)(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	static PFNGLMATRIXORTHOEXTPROC pfnMatrixOrthoEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixOrthoEXT", pfnMatrixOrthoEXT, PFNGLMATRIXORTHOEXTPROC);
	pfnMatrixOrthoEXT(mode, left, right, bottom, top, zNear, zFar);
}

void glMatrixPopEXT(GLenum mode)
{
	using PFNGLMATRIXPOPEXTPROC = void(APIENTRY *)(GLenum mode);
	static PFNGLMATRIXPOPEXTPROC pfnMatrixPopEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixPopEXT", pfnMatrixPopEXT, PFNGLMATRIXPOPEXTPROC);
	pfnMatrixPopEXT(mode);
}

void glMatrixPushEXT(GLenum mode)
{
	using PFNGLMATRIXPUSHEXTPROC = void(APIENTRY *)(GLenum mode);
	static PFNGLMATRIXPUSHEXTPROC pfnMatrixPushEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixPushEXT", pfnMatrixPushEXT, PFNGLMATRIXPUSHEXTPROC);
	pfnMatrixPushEXT(mode);
}

void glClientAttribDefaultEXT(GLbitfield mask)
{
	using PFNGLCLIENTATTRIBDEFAULTEXTPROC = void(APIENTRY *)(GLbitfield mask);
	static PFNGLCLIENTATTRIBDEFAULTEXTPROC pfnClientAttribDefaultEXT{nullptr};
	LOAD_ENTRYPOINT("glClientAttribDefaultEXT", pfnClientAttribDefaultEXT, PFNGLCLIENTATTRIBDEFAULTEXTPROC);
	pfnClientAttribDefaultEXT(mask);
}

void glPushClientAttribDefaultEXT(GLbitfield mask)
{
	using PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC = void(APIENTRY *)(GLbitfield mask);
	static PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC pfnPushClientAttribDefaultEXT{nullptr};
	LOAD_ENTRYPOINT("glPushClientAttribDefaultEXT", pfnPushClientAttribDefaultEXT, PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC);
	pfnPushClientAttribDefaultEXT(mask);
}

void glTextureParameterfEXT(GLuint texture, GLenum target, GLenum pname, GLfloat param)
{
	using PFNGLTEXTUREPARAMETERFEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, GLfloat param);
	static PFNGLTEXTUREPARAMETERFEXTPROC pfnTextureParameterfEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureParameterfEXT", pfnTextureParameterfEXT, PFNGLTEXTUREPARAMETERFEXTPROC);
	pfnTextureParameterfEXT(texture, target, pname, param);
}

void glTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, const GLfloat* params)
{
	using PFNGLTEXTUREPARAMETERFVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, const GLfloat* params);
	static PFNGLTEXTUREPARAMETERFVEXTPROC pfnTextureParameterfvEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureParameterfvEXT", pfnTextureParameterfvEXT, PFNGLTEXTUREPARAMETERFVEXTPROC);
	pfnTextureParameterfvEXT(texture, target, pname, params);
}

void glTextureParameteriEXT(GLuint texture, GLenum target, GLenum pname, GLint param)
{
	using PFNGLTEXTUREPARAMETERIEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, GLint param);
	static PFNGLTEXTUREPARAMETERIEXTPROC pfnTextureParameteriEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureParameteriEXT", pfnTextureParameteriEXT, PFNGLTEXTUREPARAMETERIEXTPROC);
	pfnTextureParameteriEXT(texture, target, pname, param);
}

void glTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, const GLint* params)
{
	using PFNGLTEXTUREPARAMETERIVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, const GLint* params);
	static PFNGLTEXTUREPARAMETERIVEXTPROC pfnTextureParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureParameterivEXT", pfnTextureParameterivEXT, PFNGLTEXTUREPARAMETERIVEXTPROC);
	pfnTextureParameterivEXT(texture, target, pname, params);
}

void glTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXTUREIMAGE1DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXTUREIMAGE1DEXTPROC pfnTextureImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureImage1DEXT", pfnTextureImage1DEXT, PFNGLTEXTUREIMAGE1DEXTPROC);
	pfnTextureImage1DEXT(texture, target, level, internalformat, width, border, format, type, pixels);
}

void glTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXTUREIMAGE2DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXTUREIMAGE2DEXTPROC pfnTextureImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureImage2DEXT", pfnTextureImage2DEXT, PFNGLTEXTUREIMAGE2DEXTPROC);
	pfnTextureImage2DEXT(texture, target, level, internalformat, width, height, border, format, type, pixels);
}

void glTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXTURESUBIMAGE1DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXTURESUBIMAGE1DEXTPROC pfnTextureSubImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureSubImage1DEXT", pfnTextureSubImage1DEXT, PFNGLTEXTURESUBIMAGE1DEXTPROC);
	pfnTextureSubImage1DEXT(texture, target, level, xoffset, width, format, type, pixels);
}

void glTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXTURESUBIMAGE2DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXTURESUBIMAGE2DEXTPROC pfnTextureSubImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureSubImage2DEXT", pfnTextureSubImage2DEXT, PFNGLTEXTURESUBIMAGE2DEXTPROC);
	pfnTextureSubImage2DEXT(texture, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void glCopyTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	using PFNGLCOPYTEXTUREIMAGE1DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
	static PFNGLCOPYTEXTUREIMAGE1DEXTPROC pfnCopyTextureImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyTextureImage1DEXT", pfnCopyTextureImage1DEXT, PFNGLCOPYTEXTUREIMAGE1DEXTPROC);
	pfnCopyTextureImage1DEXT(texture, target, level, internalformat, x, y, width, border);
}

void glCopyTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	using PFNGLCOPYTEXTUREIMAGE2DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	static PFNGLCOPYTEXTUREIMAGE2DEXTPROC pfnCopyTextureImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyTextureImage2DEXT", pfnCopyTextureImage2DEXT, PFNGLCOPYTEXTUREIMAGE2DEXTPROC);
	pfnCopyTextureImage2DEXT(texture, target, level, internalformat, x, y, width, height, border);
}

void glCopyTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	using PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	static PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC pfnCopyTextureSubImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyTextureSubImage1DEXT", pfnCopyTextureSubImage1DEXT, PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC);
	pfnCopyTextureSubImage1DEXT(texture, target, level, xoffset, x, y, width);
}

void glCopyTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC pfnCopyTextureSubImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyTextureSubImage2DEXT", pfnCopyTextureSubImage2DEXT, PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC);
	pfnCopyTextureSubImage2DEXT(texture, target, level, xoffset, yoffset, x, y, width, height);
}

void glGetTextureImageEXT(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
	using PFNGLGETTEXTUREIMAGEEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, void* pixels);
	static PFNGLGETTEXTUREIMAGEEXTPROC pfnGetTextureImageEXT{nullptr};
	LOAD_ENTRYPOINT("glGetTextureImageEXT", pfnGetTextureImageEXT, PFNGLGETTEXTUREIMAGEEXTPROC);
	pfnGetTextureImageEXT(texture, target, level, format, type, pixels);
}

void glGetTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, GLfloat* params)
{
	using PFNGLGETTEXTUREPARAMETERFVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, GLfloat* params);
	static PFNGLGETTEXTUREPARAMETERFVEXTPROC pfnGetTextureParameterfvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetTextureParameterfvEXT", pfnGetTextureParameterfvEXT, PFNGLGETTEXTUREPARAMETERFVEXTPROC);
	pfnGetTextureParameterfvEXT(texture, target, pname, params);
}

void glGetTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETTEXTUREPARAMETERIVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, GLint* params);
	static PFNGLGETTEXTUREPARAMETERIVEXTPROC pfnGetTextureParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetTextureParameterivEXT", pfnGetTextureParameterivEXT, PFNGLGETTEXTUREPARAMETERIVEXTPROC);
	pfnGetTextureParameterivEXT(texture, target, pname, params);
}

void glGetTextureLevelParameterfvEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat* params)
{
	using PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat* params);
	static PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC pfnGetTextureLevelParameterfvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetTextureLevelParameterfvEXT", pfnGetTextureLevelParameterfvEXT, PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC);
	pfnGetTextureLevelParameterfvEXT(texture, target, level, pname, params);
}

void glGetTextureLevelParameterivEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLint* params)
{
	using PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLenum pname, GLint* params);
	static PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC pfnGetTextureLevelParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetTextureLevelParameterivEXT", pfnGetTextureLevelParameterivEXT, PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC);
	pfnGetTextureLevelParameterivEXT(texture, target, level, pname, params);
}

void glTextureImage3DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXTUREIMAGE3DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXTUREIMAGE3DEXTPROC pfnTextureImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureImage3DEXT", pfnTextureImage3DEXT, PFNGLTEXTUREIMAGE3DEXTPROC);
	pfnTextureImage3DEXT(texture, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void glTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLTEXTURESUBIMAGE3DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
	static PFNGLTEXTURESUBIMAGE3DEXTPROC pfnTextureSubImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureSubImage3DEXT", pfnTextureSubImage3DEXT, PFNGLTEXTURESUBIMAGE3DEXTPROC);
	pfnTextureSubImage3DEXT(texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void glCopyTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC pfnCopyTextureSubImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyTextureSubImage3DEXT", pfnCopyTextureSubImage3DEXT, PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC);
	pfnCopyTextureSubImage3DEXT(texture, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

void glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture)
{
	using PFNGLBINDMULTITEXTUREEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLuint texture);
	static PFNGLBINDMULTITEXTUREEXTPROC pfnBindMultiTextureEXT{nullptr};
	LOAD_ENTRYPOINT("glBindMultiTextureEXT", pfnBindMultiTextureEXT, PFNGLBINDMULTITEXTUREEXTPROC);
	pfnBindMultiTextureEXT(texunit, target, texture);
}

void glMultiTexCoordPointerEXT(GLenum texunit, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	using PFNGLMULTITEXCOORDPOINTEREXTPROC = void(APIENTRY *)(GLenum texunit, GLint size, GLenum type, GLsizei stride, const void* pointer);
	static PFNGLMULTITEXCOORDPOINTEREXTPROC pfnMultiTexCoordPointerEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexCoordPointerEXT", pfnMultiTexCoordPointerEXT, PFNGLMULTITEXCOORDPOINTEREXTPROC);
	pfnMultiTexCoordPointerEXT(texunit, size, type, stride, pointer);
}

void glMultiTexEnvfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param)
{
	using PFNGLMULTITEXENVFEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
	static PFNGLMULTITEXENVFEXTPROC pfnMultiTexEnvfEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexEnvfEXT", pfnMultiTexEnvfEXT, PFNGLMULTITEXENVFEXTPROC);
	pfnMultiTexEnvfEXT(texunit, target, pname, param);
}

void glMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params)
{
	using PFNGLMULTITEXENVFVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params);
	static PFNGLMULTITEXENVFVEXTPROC pfnMultiTexEnvfvEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexEnvfvEXT", pfnMultiTexEnvfvEXT, PFNGLMULTITEXENVFVEXTPROC);
	pfnMultiTexEnvfvEXT(texunit, target, pname, params);
}

void glMultiTexEnviEXT(GLenum texunit, GLenum target, GLenum pname, GLint param)
{
	using PFNGLMULTITEXENVIEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLint param);
	static PFNGLMULTITEXENVIEXTPROC pfnMultiTexEnviEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexEnviEXT", pfnMultiTexEnviEXT, PFNGLMULTITEXENVIEXTPROC);
	pfnMultiTexEnviEXT(texunit, target, pname, param);
}

void glMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params)
{
	using PFNGLMULTITEXENVIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, const GLint* params);
	static PFNGLMULTITEXENVIVEXTPROC pfnMultiTexEnvivEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexEnvivEXT", pfnMultiTexEnvivEXT, PFNGLMULTITEXENVIVEXTPROC);
	pfnMultiTexEnvivEXT(texunit, target, pname, params);
}

void glMultiTexGendEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble param)
{
	using PFNGLMULTITEXGENDEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum coord, GLenum pname, GLdouble param);
	static PFNGLMULTITEXGENDEXTPROC pfnMultiTexGendEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexGendEXT", pfnMultiTexGendEXT, PFNGLMULTITEXGENDEXTPROC);
	pfnMultiTexGendEXT(texunit, coord, pname, param);
}

void glMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLdouble* params)
{
	using PFNGLMULTITEXGENDVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum coord, GLenum pname, const GLdouble* params);
	static PFNGLMULTITEXGENDVEXTPROC pfnMultiTexGendvEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexGendvEXT", pfnMultiTexGendvEXT, PFNGLMULTITEXGENDVEXTPROC);
	pfnMultiTexGendvEXT(texunit, coord, pname, params);
}

void glMultiTexGenfEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat param)
{
	using PFNGLMULTITEXGENFEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum coord, GLenum pname, GLfloat param);
	static PFNGLMULTITEXGENFEXTPROC pfnMultiTexGenfEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexGenfEXT", pfnMultiTexGenfEXT, PFNGLMULTITEXGENFEXTPROC);
	pfnMultiTexGenfEXT(texunit, coord, pname, param);
}

void glMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLfloat* params)
{
	using PFNGLMULTITEXGENFVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum coord, GLenum pname, const GLfloat* params);
	static PFNGLMULTITEXGENFVEXTPROC pfnMultiTexGenfvEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexGenfvEXT", pfnMultiTexGenfvEXT, PFNGLMULTITEXGENFVEXTPROC);
	pfnMultiTexGenfvEXT(texunit, coord, pname, params);
}

void glMultiTexGeniEXT(GLenum texunit, GLenum coord, GLenum pname, GLint param)
{
	using PFNGLMULTITEXGENIEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum coord, GLenum pname, GLint param);
	static PFNGLMULTITEXGENIEXTPROC pfnMultiTexGeniEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexGeniEXT", pfnMultiTexGeniEXT, PFNGLMULTITEXGENIEXTPROC);
	pfnMultiTexGeniEXT(texunit, coord, pname, param);
}

void glMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, const GLint* params)
{
	using PFNGLMULTITEXGENIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum coord, GLenum pname, const GLint* params);
	static PFNGLMULTITEXGENIVEXTPROC pfnMultiTexGenivEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexGenivEXT", pfnMultiTexGenivEXT, PFNGLMULTITEXGENIVEXTPROC);
	pfnMultiTexGenivEXT(texunit, coord, pname, params);
}

void glGetMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params)
{
	using PFNGLGETMULTITEXENVFVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLfloat* params);
	static PFNGLGETMULTITEXENVFVEXTPROC pfnGetMultiTexEnvfvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexEnvfvEXT", pfnGetMultiTexEnvfvEXT, PFNGLGETMULTITEXENVFVEXTPROC);
	pfnGetMultiTexEnvfvEXT(texunit, target, pname, params);
}

void glGetMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETMULTITEXENVIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLint* params);
	static PFNGLGETMULTITEXENVIVEXTPROC pfnGetMultiTexEnvivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexEnvivEXT", pfnGetMultiTexEnvivEXT, PFNGLGETMULTITEXENVIVEXTPROC);
	pfnGetMultiTexEnvivEXT(texunit, target, pname, params);
}

void glGetMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble* params)
{
	using PFNGLGETMULTITEXGENDVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum coord, GLenum pname, GLdouble* params);
	static PFNGLGETMULTITEXGENDVEXTPROC pfnGetMultiTexGendvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexGendvEXT", pfnGetMultiTexGendvEXT, PFNGLGETMULTITEXGENDVEXTPROC);
	pfnGetMultiTexGendvEXT(texunit, coord, pname, params);
}

void glGetMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat* params)
{
	using PFNGLGETMULTITEXGENFVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum coord, GLenum pname, GLfloat* params);
	static PFNGLGETMULTITEXGENFVEXTPROC pfnGetMultiTexGenfvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexGenfvEXT", pfnGetMultiTexGenfvEXT, PFNGLGETMULTITEXGENFVEXTPROC);
	pfnGetMultiTexGenfvEXT(texunit, coord, pname, params);
}

void glGetMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, GLint* params)
{
	using PFNGLGETMULTITEXGENIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum coord, GLenum pname, GLint* params);
	static PFNGLGETMULTITEXGENIVEXTPROC pfnGetMultiTexGenivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexGenivEXT", pfnGetMultiTexGenivEXT, PFNGLGETMULTITEXGENIVEXTPROC);
	pfnGetMultiTexGenivEXT(texunit, coord, pname, params);
}

void glMultiTexParameteriEXT(GLenum texunit, GLenum target, GLenum pname, GLint param)
{
	using PFNGLMULTITEXPARAMETERIEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLint param);
	static PFNGLMULTITEXPARAMETERIEXTPROC pfnMultiTexParameteriEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexParameteriEXT", pfnMultiTexParameteriEXT, PFNGLMULTITEXPARAMETERIEXTPROC);
	pfnMultiTexParameteriEXT(texunit, target, pname, param);
}

void glMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params)
{
	using PFNGLMULTITEXPARAMETERIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, const GLint* params);
	static PFNGLMULTITEXPARAMETERIVEXTPROC pfnMultiTexParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexParameterivEXT", pfnMultiTexParameterivEXT, PFNGLMULTITEXPARAMETERIVEXTPROC);
	pfnMultiTexParameterivEXT(texunit, target, pname, params);
}

void glMultiTexParameterfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param)
{
	using PFNGLMULTITEXPARAMETERFEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
	static PFNGLMULTITEXPARAMETERFEXTPROC pfnMultiTexParameterfEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexParameterfEXT", pfnMultiTexParameterfEXT, PFNGLMULTITEXPARAMETERFEXTPROC);
	pfnMultiTexParameterfEXT(texunit, target, pname, param);
}

void glMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params)
{
	using PFNGLMULTITEXPARAMETERFVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params);
	static PFNGLMULTITEXPARAMETERFVEXTPROC pfnMultiTexParameterfvEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexParameterfvEXT", pfnMultiTexParameterfvEXT, PFNGLMULTITEXPARAMETERFVEXTPROC);
	pfnMultiTexParameterfvEXT(texunit, target, pname, params);
}

void glMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLMULTITEXIMAGE1DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels);
	static PFNGLMULTITEXIMAGE1DEXTPROC pfnMultiTexImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexImage1DEXT", pfnMultiTexImage1DEXT, PFNGLMULTITEXIMAGE1DEXTPROC);
	pfnMultiTexImage1DEXT(texunit, target, level, internalformat, width, border, format, type, pixels);
}

void glMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLMULTITEXIMAGE2DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
	static PFNGLMULTITEXIMAGE2DEXTPROC pfnMultiTexImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexImage2DEXT", pfnMultiTexImage2DEXT, PFNGLMULTITEXIMAGE2DEXTPROC);
	pfnMultiTexImage2DEXT(texunit, target, level, internalformat, width, height, border, format, type, pixels);
}

void glMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLMULTITEXSUBIMAGE1DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
	static PFNGLMULTITEXSUBIMAGE1DEXTPROC pfnMultiTexSubImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexSubImage1DEXT", pfnMultiTexSubImage1DEXT, PFNGLMULTITEXSUBIMAGE1DEXTPROC);
	pfnMultiTexSubImage1DEXT(texunit, target, level, xoffset, width, format, type, pixels);
}

void glMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLMULTITEXSUBIMAGE2DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
	static PFNGLMULTITEXSUBIMAGE2DEXTPROC pfnMultiTexSubImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexSubImage2DEXT", pfnMultiTexSubImage2DEXT, PFNGLMULTITEXSUBIMAGE2DEXTPROC);
	pfnMultiTexSubImage2DEXT(texunit, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void glCopyMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	using PFNGLCOPYMULTITEXIMAGE1DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
	static PFNGLCOPYMULTITEXIMAGE1DEXTPROC pfnCopyMultiTexImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyMultiTexImage1DEXT", pfnCopyMultiTexImage1DEXT, PFNGLCOPYMULTITEXIMAGE1DEXTPROC);
	pfnCopyMultiTexImage1DEXT(texunit, target, level, internalformat, x, y, width, border);
}

void glCopyMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	using PFNGLCOPYMULTITEXIMAGE2DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	static PFNGLCOPYMULTITEXIMAGE2DEXTPROC pfnCopyMultiTexImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyMultiTexImage2DEXT", pfnCopyMultiTexImage2DEXT, PFNGLCOPYMULTITEXIMAGE2DEXTPROC);
	pfnCopyMultiTexImage2DEXT(texunit, target, level, internalformat, x, y, width, height, border);
}

void glCopyMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	using PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
	static PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC pfnCopyMultiTexSubImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyMultiTexSubImage1DEXT", pfnCopyMultiTexSubImage1DEXT, PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC);
	pfnCopyMultiTexSubImage1DEXT(texunit, target, level, xoffset, x, y, width);
}

void glCopyMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC pfnCopyMultiTexSubImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyMultiTexSubImage2DEXT", pfnCopyMultiTexSubImage2DEXT, PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC);
	pfnCopyMultiTexSubImage2DEXT(texunit, target, level, xoffset, yoffset, x, y, width, height);
}

void glGetMultiTexImageEXT(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
	using PFNGLGETMULTITEXIMAGEEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, void* pixels);
	static PFNGLGETMULTITEXIMAGEEXTPROC pfnGetMultiTexImageEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexImageEXT", pfnGetMultiTexImageEXT, PFNGLGETMULTITEXIMAGEEXTPROC);
	pfnGetMultiTexImageEXT(texunit, target, level, format, type, pixels);
}

void glGetMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params)
{
	using PFNGLGETMULTITEXPARAMETERFVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLfloat* params);
	static PFNGLGETMULTITEXPARAMETERFVEXTPROC pfnGetMultiTexParameterfvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexParameterfvEXT", pfnGetMultiTexParameterfvEXT, PFNGLGETMULTITEXPARAMETERFVEXTPROC);
	pfnGetMultiTexParameterfvEXT(texunit, target, pname, params);
}

void glGetMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETMULTITEXPARAMETERIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLint* params);
	static PFNGLGETMULTITEXPARAMETERIVEXTPROC pfnGetMultiTexParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexParameterivEXT", pfnGetMultiTexParameterivEXT, PFNGLGETMULTITEXPARAMETERIVEXTPROC);
	pfnGetMultiTexParameterivEXT(texunit, target, pname, params);
}

void glGetMultiTexLevelParameterfvEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat* params)
{
	using PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat* params);
	static PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC pfnGetMultiTexLevelParameterfvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexLevelParameterfvEXT", pfnGetMultiTexLevelParameterfvEXT, PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC);
	pfnGetMultiTexLevelParameterfvEXT(texunit, target, level, pname, params);
}

void glGetMultiTexLevelParameterivEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint* params)
{
	using PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint* params);
	static PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC pfnGetMultiTexLevelParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexLevelParameterivEXT", pfnGetMultiTexLevelParameterivEXT, PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC);
	pfnGetMultiTexLevelParameterivEXT(texunit, target, level, pname, params);
}

void glMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLMULTITEXIMAGE3DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
	static PFNGLMULTITEXIMAGE3DEXTPROC pfnMultiTexImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexImage3DEXT", pfnMultiTexImage3DEXT, PFNGLMULTITEXIMAGE3DEXTPROC);
	pfnMultiTexImage3DEXT(texunit, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void glMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
	using PFNGLMULTITEXSUBIMAGE3DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
	static PFNGLMULTITEXSUBIMAGE3DEXTPROC pfnMultiTexSubImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexSubImage3DEXT", pfnMultiTexSubImage3DEXT, PFNGLMULTITEXSUBIMAGE3DEXTPROC);
	pfnMultiTexSubImage3DEXT(texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void glCopyMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC pfnCopyMultiTexSubImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glCopyMultiTexSubImage3DEXT", pfnCopyMultiTexSubImage3DEXT, PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC);
	pfnCopyMultiTexSubImage3DEXT(texunit, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

void glEnableClientStateIndexedEXT(GLenum array, GLuint index)
{
	using PFNGLENABLECLIENTSTATEINDEXEDEXTPROC = void(APIENTRY *)(GLenum array, GLuint index);
	static PFNGLENABLECLIENTSTATEINDEXEDEXTPROC pfnEnableClientStateIndexedEXT{nullptr};
	LOAD_ENTRYPOINT("glEnableClientStateIndexedEXT", pfnEnableClientStateIndexedEXT, PFNGLENABLECLIENTSTATEINDEXEDEXTPROC);
	pfnEnableClientStateIndexedEXT(array, index);
}

void glDisableClientStateIndexedEXT(GLenum array, GLuint index)
{
	using PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC = void(APIENTRY *)(GLenum array, GLuint index);
	static PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC pfnDisableClientStateIndexedEXT{nullptr};
	LOAD_ENTRYPOINT("glDisableClientStateIndexedEXT", pfnDisableClientStateIndexedEXT, PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC);
	pfnDisableClientStateIndexedEXT(array, index);
}

void glGetFloatIndexedvEXT(GLenum target, GLuint index, GLfloat* data)
{
	using PFNGLGETFLOATINDEXEDVEXTPROC = void(APIENTRY *)(GLenum target, GLuint index, GLfloat* data);
	static PFNGLGETFLOATINDEXEDVEXTPROC pfnGetFloatIndexedvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetFloatIndexedvEXT", pfnGetFloatIndexedvEXT, PFNGLGETFLOATINDEXEDVEXTPROC);
	pfnGetFloatIndexedvEXT(target, index, data);
}

void glGetDoubleIndexedvEXT(GLenum target, GLuint index, GLdouble* data)
{
	using PFNGLGETDOUBLEINDEXEDVEXTPROC = void(APIENTRY *)(GLenum target, GLuint index, GLdouble* data);
	static PFNGLGETDOUBLEINDEXEDVEXTPROC pfnGetDoubleIndexedvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetDoubleIndexedvEXT", pfnGetDoubleIndexedvEXT, PFNGLGETDOUBLEINDEXEDVEXTPROC);
	pfnGetDoubleIndexedvEXT(target, index, data);
}

void glGetPointerIndexedvEXT(GLenum target, GLuint index, void** data)
{
	using PFNGLGETPOINTERINDEXEDVEXTPROC = void(APIENTRY *)(GLenum target, GLuint index, void** data);
	static PFNGLGETPOINTERINDEXEDVEXTPROC pfnGetPointerIndexedvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetPointerIndexedvEXT", pfnGetPointerIndexedvEXT, PFNGLGETPOINTERINDEXEDVEXTPROC);
	pfnGetPointerIndexedvEXT(target, index, data);
}

void glEnableIndexedEXT(GLenum target, GLuint index)
{
	using PFNGLENABLEINDEXEDEXTPROC = void(APIENTRY *)(GLenum target, GLuint index);
	static PFNGLENABLEINDEXEDEXTPROC pfnEnableIndexedEXT{nullptr};
	LOAD_ENTRYPOINT("glEnableIndexedEXT", pfnEnableIndexedEXT, PFNGLENABLEINDEXEDEXTPROC);
	pfnEnableIndexedEXT(target, index);
}

void glDisableIndexedEXT(GLenum target, GLuint index)
{
	using PFNGLDISABLEINDEXEDEXTPROC = void(APIENTRY *)(GLenum target, GLuint index);
	static PFNGLDISABLEINDEXEDEXTPROC pfnDisableIndexedEXT{nullptr};
	LOAD_ENTRYPOINT("glDisableIndexedEXT", pfnDisableIndexedEXT, PFNGLDISABLEINDEXEDEXTPROC);
	pfnDisableIndexedEXT(target, index);
}

GLboolean glIsEnabledIndexedEXT(GLenum target, GLuint index)
{
	using PFNGLISENABLEDINDEXEDEXTPROC = GLboolean(APIENTRY *)(GLenum target, GLuint index);
	static PFNGLISENABLEDINDEXEDEXTPROC pfnIsEnabledIndexedEXT{nullptr};
	LOAD_ENTRYPOINT("glIsEnabledIndexedEXT", pfnIsEnabledIndexedEXT, PFNGLISENABLEDINDEXEDEXTPROC);
	return pfnIsEnabledIndexedEXT(target, index);
}

void glGetIntegerIndexedvEXT(GLenum target, GLuint index, GLint* data)
{
	using PFNGLGETINTEGERINDEXEDVEXTPROC = void(APIENTRY *)(GLenum target, GLuint index, GLint* data);
	static PFNGLGETINTEGERINDEXEDVEXTPROC pfnGetIntegerIndexedvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetIntegerIndexedvEXT", pfnGetIntegerIndexedvEXT, PFNGLGETINTEGERINDEXEDVEXTPROC);
	pfnGetIntegerIndexedvEXT(target, index, data);
}

void glGetBooleanIndexedvEXT(GLenum target, GLuint index, GLboolean* data)
{
	using PFNGLGETBOOLEANINDEXEDVEXTPROC = void(APIENTRY *)(GLenum target, GLuint index, GLboolean* data);
	static PFNGLGETBOOLEANINDEXEDVEXTPROC pfnGetBooleanIndexedvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetBooleanIndexedvEXT", pfnGetBooleanIndexedvEXT, PFNGLGETBOOLEANINDEXEDVEXTPROC);
	pfnGetBooleanIndexedvEXT(target, index, data);
}

void glCompressedTextureImage3DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC pfnCompressedTextureImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedTextureImage3DEXT", pfnCompressedTextureImage3DEXT, PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC);
	pfnCompressedTextureImage3DEXT(texture, target, level, internalformat, width, height, depth, border, imageSize, bits);
}

void glCompressedTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC pfnCompressedTextureImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedTextureImage2DEXT", pfnCompressedTextureImage2DEXT, PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC);
	pfnCompressedTextureImage2DEXT(texture, target, level, internalformat, width, height, border, imageSize, bits);
}

void glCompressedTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC pfnCompressedTextureImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedTextureImage1DEXT", pfnCompressedTextureImage1DEXT, PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC);
	pfnCompressedTextureImage1DEXT(texture, target, level, internalformat, width, border, imageSize, bits);
}

void glCompressedTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC pfnCompressedTextureSubImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedTextureSubImage3DEXT", pfnCompressedTextureSubImage3DEXT, PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC);
	pfnCompressedTextureSubImage3DEXT(texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits);
}

void glCompressedTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC pfnCompressedTextureSubImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedTextureSubImage2DEXT", pfnCompressedTextureSubImage2DEXT, PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC);
	pfnCompressedTextureSubImage2DEXT(texture, target, level, xoffset, yoffset, width, height, format, imageSize, bits);
}

void glCompressedTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC pfnCompressedTextureSubImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedTextureSubImage1DEXT", pfnCompressedTextureSubImage1DEXT, PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC);
	pfnCompressedTextureSubImage1DEXT(texture, target, level, xoffset, width, format, imageSize, bits);
}

void glGetCompressedTextureImageEXT(GLuint texture, GLenum target, GLint lod, void* img)
{
	using PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLint lod, void* img);
	static PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC pfnGetCompressedTextureImageEXT{nullptr};
	LOAD_ENTRYPOINT("glGetCompressedTextureImageEXT", pfnGetCompressedTextureImageEXT, PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC);
	pfnGetCompressedTextureImageEXT(texture, target, lod, img);
}

void glCompressedMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC pfnCompressedMultiTexImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedMultiTexImage3DEXT", pfnCompressedMultiTexImage3DEXT, PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC);
	pfnCompressedMultiTexImage3DEXT(texunit, target, level, internalformat, width, height, depth, border, imageSize, bits);
}

void glCompressedMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC pfnCompressedMultiTexImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedMultiTexImage2DEXT", pfnCompressedMultiTexImage2DEXT, PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC);
	pfnCompressedMultiTexImage2DEXT(texunit, target, level, internalformat, width, height, border, imageSize, bits);
}

void glCompressedMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC pfnCompressedMultiTexImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedMultiTexImage1DEXT", pfnCompressedMultiTexImage1DEXT, PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC);
	pfnCompressedMultiTexImage1DEXT(texunit, target, level, internalformat, width, border, imageSize, bits);
}

void glCompressedMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC pfnCompressedMultiTexSubImage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedMultiTexSubImage3DEXT", pfnCompressedMultiTexSubImage3DEXT, PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC);
	pfnCompressedMultiTexSubImage3DEXT(texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits);
}

void glCompressedMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC pfnCompressedMultiTexSubImage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedMultiTexSubImage2DEXT", pfnCompressedMultiTexSubImage2DEXT, PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC);
	pfnCompressedMultiTexSubImage2DEXT(texunit, target, level, xoffset, yoffset, width, height, format, imageSize, bits);
}

void glCompressedMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits)
{
	using PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits);
	static PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC pfnCompressedMultiTexSubImage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glCompressedMultiTexSubImage1DEXT", pfnCompressedMultiTexSubImage1DEXT, PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC);
	pfnCompressedMultiTexSubImage1DEXT(texunit, target, level, xoffset, width, format, imageSize, bits);
}

void glGetCompressedMultiTexImageEXT(GLenum texunit, GLenum target, GLint lod, void* img)
{
	using PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLint lod, void* img);
	static PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC pfnGetCompressedMultiTexImageEXT{nullptr};
	LOAD_ENTRYPOINT("glGetCompressedMultiTexImageEXT", pfnGetCompressedMultiTexImageEXT, PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC);
	pfnGetCompressedMultiTexImageEXT(texunit, target, lod, img);
}

void glMatrixLoadTransposefEXT(GLenum mode, const GLfloat* m)
{
	using PFNGLMATRIXLOADTRANSPOSEFEXTPROC = void(APIENTRY *)(GLenum mode, const GLfloat* m);
	static PFNGLMATRIXLOADTRANSPOSEFEXTPROC pfnMatrixLoadTransposefEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixLoadTransposefEXT", pfnMatrixLoadTransposefEXT, PFNGLMATRIXLOADTRANSPOSEFEXTPROC);
	pfnMatrixLoadTransposefEXT(mode, m);
}

void glMatrixLoadTransposedEXT(GLenum mode, const GLdouble* m)
{
	using PFNGLMATRIXLOADTRANSPOSEDEXTPROC = void(APIENTRY *)(GLenum mode, const GLdouble* m);
	static PFNGLMATRIXLOADTRANSPOSEDEXTPROC pfnMatrixLoadTransposedEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixLoadTransposedEXT", pfnMatrixLoadTransposedEXT, PFNGLMATRIXLOADTRANSPOSEDEXTPROC);
	pfnMatrixLoadTransposedEXT(mode, m);
}

void glMatrixMultTransposefEXT(GLenum mode, const GLfloat* m)
{
	using PFNGLMATRIXMULTTRANSPOSEFEXTPROC = void(APIENTRY *)(GLenum mode, const GLfloat* m);
	static PFNGLMATRIXMULTTRANSPOSEFEXTPROC pfnMatrixMultTransposefEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixMultTransposefEXT", pfnMatrixMultTransposefEXT, PFNGLMATRIXMULTTRANSPOSEFEXTPROC);
	pfnMatrixMultTransposefEXT(mode, m);
}

void glMatrixMultTransposedEXT(GLenum mode, const GLdouble* m)
{
	using PFNGLMATRIXMULTTRANSPOSEDEXTPROC = void(APIENTRY *)(GLenum mode, const GLdouble* m);
	static PFNGLMATRIXMULTTRANSPOSEDEXTPROC pfnMatrixMultTransposedEXT{nullptr};
	LOAD_ENTRYPOINT("glMatrixMultTransposedEXT", pfnMatrixMultTransposedEXT, PFNGLMATRIXMULTTRANSPOSEDEXTPROC);
	pfnMatrixMultTransposedEXT(mode, m);
}

void glNamedBufferDataEXT(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage)
{
	using PFNGLNAMEDBUFFERDATAEXTPROC = void(APIENTRY *)(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage);
	static PFNGLNAMEDBUFFERDATAEXTPROC pfnNamedBufferDataEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferDataEXT", pfnNamedBufferDataEXT, PFNGLNAMEDBUFFERDATAEXTPROC);
	pfnNamedBufferDataEXT(buffer, size, data, usage);
}

void glNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data)
{
	using PFNGLNAMEDBUFFERSUBDATAEXTPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data);
	static PFNGLNAMEDBUFFERSUBDATAEXTPROC pfnNamedBufferSubDataEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferSubDataEXT", pfnNamedBufferSubDataEXT, PFNGLNAMEDBUFFERSUBDATAEXTPROC);
	pfnNamedBufferSubDataEXT(buffer, offset, size, data);
}

void* glMapNamedBufferEXT(GLuint buffer, GLenum access)
{
	using PFNGLMAPNAMEDBUFFEREXTPROC = void*(APIENTRY *)(GLuint buffer, GLenum access);
	static PFNGLMAPNAMEDBUFFEREXTPROC pfnMapNamedBufferEXT{nullptr};
	LOAD_ENTRYPOINT("glMapNamedBufferEXT", pfnMapNamedBufferEXT, PFNGLMAPNAMEDBUFFEREXTPROC);
	return pfnMapNamedBufferEXT(buffer, access);
}

GLboolean glUnmapNamedBufferEXT(GLuint buffer)
{
	using PFNGLUNMAPNAMEDBUFFEREXTPROC = GLboolean(APIENTRY *)(GLuint buffer);
	static PFNGLUNMAPNAMEDBUFFEREXTPROC pfnUnmapNamedBufferEXT{nullptr};
	LOAD_ENTRYPOINT("glUnmapNamedBufferEXT", pfnUnmapNamedBufferEXT, PFNGLUNMAPNAMEDBUFFEREXTPROC);
	return pfnUnmapNamedBufferEXT(buffer);
}

void glGetNamedBufferParameterivEXT(GLuint buffer, GLenum pname, GLint* params)
{
	using PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC = void(APIENTRY *)(GLuint buffer, GLenum pname, GLint* params);
	static PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC pfnGetNamedBufferParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedBufferParameterivEXT", pfnGetNamedBufferParameterivEXT, PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC);
	pfnGetNamedBufferParameterivEXT(buffer, pname, params);
}

void glGetNamedBufferPointervEXT(GLuint buffer, GLenum pname, void** params)
{
	using PFNGLGETNAMEDBUFFERPOINTERVEXTPROC = void(APIENTRY *)(GLuint buffer, GLenum pname, void** params);
	static PFNGLGETNAMEDBUFFERPOINTERVEXTPROC pfnGetNamedBufferPointervEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedBufferPointervEXT", pfnGetNamedBufferPointervEXT, PFNGLGETNAMEDBUFFERPOINTERVEXTPROC);
	pfnGetNamedBufferPointervEXT(buffer, pname, params);
}

void glGetNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data)
{
	using PFNGLGETNAMEDBUFFERSUBDATAEXTPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data);
	static PFNGLGETNAMEDBUFFERSUBDATAEXTPROC pfnGetNamedBufferSubDataEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedBufferSubDataEXT", pfnGetNamedBufferSubDataEXT, PFNGLGETNAMEDBUFFERSUBDATAEXTPROC);
	pfnGetNamedBufferSubDataEXT(buffer, offset, size, data);
}

void glProgramUniform1fEXT(GLuint program, GLint location, GLfloat v0)
{
	using PFNGLPROGRAMUNIFORM1FEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLfloat v0);
	static PFNGLPROGRAMUNIFORM1FEXTPROC pfnProgramUniform1fEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1fEXT", pfnProgramUniform1fEXT, PFNGLPROGRAMUNIFORM1FEXTPROC);
	pfnProgramUniform1fEXT(program, location, v0);
}

void glProgramUniform2fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
	using PFNGLPROGRAMUNIFORM2FEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
	static PFNGLPROGRAMUNIFORM2FEXTPROC pfnProgramUniform2fEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2fEXT", pfnProgramUniform2fEXT, PFNGLPROGRAMUNIFORM2FEXTPROC);
	pfnProgramUniform2fEXT(program, location, v0, v1);
}

void glProgramUniform3fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	using PFNGLPROGRAMUNIFORM3FEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	static PFNGLPROGRAMUNIFORM3FEXTPROC pfnProgramUniform3fEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3fEXT", pfnProgramUniform3fEXT, PFNGLPROGRAMUNIFORM3FEXTPROC);
	pfnProgramUniform3fEXT(program, location, v0, v1, v2);
}

void glProgramUniform4fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	using PFNGLPROGRAMUNIFORM4FEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	static PFNGLPROGRAMUNIFORM4FEXTPROC pfnProgramUniform4fEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4fEXT", pfnProgramUniform4fEXT, PFNGLPROGRAMUNIFORM4FEXTPROC);
	pfnProgramUniform4fEXT(program, location, v0, v1, v2, v3);
}

void glProgramUniform1iEXT(GLuint program, GLint location, GLint v0)
{
	using PFNGLPROGRAMUNIFORM1IEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLint v0);
	static PFNGLPROGRAMUNIFORM1IEXTPROC pfnProgramUniform1iEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1iEXT", pfnProgramUniform1iEXT, PFNGLPROGRAMUNIFORM1IEXTPROC);
	pfnProgramUniform1iEXT(program, location, v0);
}

void glProgramUniform2iEXT(GLuint program, GLint location, GLint v0, GLint v1)
{
	using PFNGLPROGRAMUNIFORM2IEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLint v0, GLint v1);
	static PFNGLPROGRAMUNIFORM2IEXTPROC pfnProgramUniform2iEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2iEXT", pfnProgramUniform2iEXT, PFNGLPROGRAMUNIFORM2IEXTPROC);
	pfnProgramUniform2iEXT(program, location, v0, v1);
}

void glProgramUniform3iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
	using PFNGLPROGRAMUNIFORM3IEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
	static PFNGLPROGRAMUNIFORM3IEXTPROC pfnProgramUniform3iEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3iEXT", pfnProgramUniform3iEXT, PFNGLPROGRAMUNIFORM3IEXTPROC);
	pfnProgramUniform3iEXT(program, location, v0, v1, v2);
}

void glProgramUniform4iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	using PFNGLPROGRAMUNIFORM4IEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	static PFNGLPROGRAMUNIFORM4IEXTPROC pfnProgramUniform4iEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4iEXT", pfnProgramUniform4iEXT, PFNGLPROGRAMUNIFORM4IEXTPROC);
	pfnProgramUniform4iEXT(program, location, v0, v1, v2, v3);
}

void glProgramUniform1fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORM1FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static PFNGLPROGRAMUNIFORM1FVEXTPROC pfnProgramUniform1fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1fvEXT", pfnProgramUniform1fvEXT, PFNGLPROGRAMUNIFORM1FVEXTPROC);
	pfnProgramUniform1fvEXT(program, location, count, value);
}

void glProgramUniform2fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORM2FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static PFNGLPROGRAMUNIFORM2FVEXTPROC pfnProgramUniform2fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2fvEXT", pfnProgramUniform2fvEXT, PFNGLPROGRAMUNIFORM2FVEXTPROC);
	pfnProgramUniform2fvEXT(program, location, count, value);
}

void glProgramUniform3fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORM3FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static PFNGLPROGRAMUNIFORM3FVEXTPROC pfnProgramUniform3fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3fvEXT", pfnProgramUniform3fvEXT, PFNGLPROGRAMUNIFORM3FVEXTPROC);
	pfnProgramUniform3fvEXT(program, location, count, value);
}

void glProgramUniform4fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORM4FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLfloat* value);
	static PFNGLPROGRAMUNIFORM4FVEXTPROC pfnProgramUniform4fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4fvEXT", pfnProgramUniform4fvEXT, PFNGLPROGRAMUNIFORM4FVEXTPROC);
	pfnProgramUniform4fvEXT(program, location, count, value);
}

void glProgramUniform1ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	using PFNGLPROGRAMUNIFORM1IVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint* value);
	static PFNGLPROGRAMUNIFORM1IVEXTPROC pfnProgramUniform1ivEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1ivEXT", pfnProgramUniform1ivEXT, PFNGLPROGRAMUNIFORM1IVEXTPROC);
	pfnProgramUniform1ivEXT(program, location, count, value);
}

void glProgramUniform2ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	using PFNGLPROGRAMUNIFORM2IVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint* value);
	static PFNGLPROGRAMUNIFORM2IVEXTPROC pfnProgramUniform2ivEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2ivEXT", pfnProgramUniform2ivEXT, PFNGLPROGRAMUNIFORM2IVEXTPROC);
	pfnProgramUniform2ivEXT(program, location, count, value);
}

void glProgramUniform3ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	using PFNGLPROGRAMUNIFORM3IVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint* value);
	static PFNGLPROGRAMUNIFORM3IVEXTPROC pfnProgramUniform3ivEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3ivEXT", pfnProgramUniform3ivEXT, PFNGLPROGRAMUNIFORM3IVEXTPROC);
	pfnProgramUniform3ivEXT(program, location, count, value);
}

void glProgramUniform4ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value)
{
	using PFNGLPROGRAMUNIFORM4IVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint* value);
	static PFNGLPROGRAMUNIFORM4IVEXTPROC pfnProgramUniform4ivEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4ivEXT", pfnProgramUniform4ivEXT, PFNGLPROGRAMUNIFORM4IVEXTPROC);
	pfnProgramUniform4ivEXT(program, location, count, value);
}

void glProgramUniformMatrix2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC pfnProgramUniformMatrix2fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2fvEXT", pfnProgramUniformMatrix2fvEXT, PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC);
	pfnProgramUniformMatrix2fvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC pfnProgramUniformMatrix3fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3fvEXT", pfnProgramUniformMatrix3fvEXT, PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC);
	pfnProgramUniformMatrix3fvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC pfnProgramUniformMatrix4fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4fvEXT", pfnProgramUniformMatrix4fvEXT, PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC);
	pfnProgramUniformMatrix4fvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC pfnProgramUniformMatrix2x3fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x3fvEXT", pfnProgramUniformMatrix2x3fvEXT, PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC);
	pfnProgramUniformMatrix2x3fvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC pfnProgramUniformMatrix3x2fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x2fvEXT", pfnProgramUniformMatrix3x2fvEXT, PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC);
	pfnProgramUniformMatrix3x2fvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC pfnProgramUniformMatrix2x4fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x4fvEXT", pfnProgramUniformMatrix2x4fvEXT, PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC);
	pfnProgramUniformMatrix2x4fvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC pfnProgramUniformMatrix4x2fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x2fvEXT", pfnProgramUniformMatrix4x2fvEXT, PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC);
	pfnProgramUniformMatrix4x2fvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC pfnProgramUniformMatrix3x4fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x4fvEXT", pfnProgramUniformMatrix3x4fvEXT, PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC);
	pfnProgramUniformMatrix3x4fvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
	static PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC pfnProgramUniformMatrix4x3fvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x3fvEXT", pfnProgramUniformMatrix4x3fvEXT, PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC);
	pfnProgramUniformMatrix4x3fvEXT(program, location, count, transpose, value);
}

void glTextureBufferEXT(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer)
{
	using PFNGLTEXTUREBUFFEREXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
	static PFNGLTEXTUREBUFFEREXTPROC pfnTextureBufferEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureBufferEXT", pfnTextureBufferEXT, PFNGLTEXTUREBUFFEREXTPROC);
	pfnTextureBufferEXT(texture, target, internalformat, buffer);
}

void glMultiTexBufferEXT(GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer)
{
	using PFNGLMULTITEXBUFFEREXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer);
	static PFNGLMULTITEXBUFFEREXTPROC pfnMultiTexBufferEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexBufferEXT", pfnMultiTexBufferEXT, PFNGLMULTITEXBUFFEREXTPROC);
	pfnMultiTexBufferEXT(texunit, target, internalformat, buffer);
}

void glTextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname, const GLint* params)
{
	using PFNGLTEXTUREPARAMETERIIVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, const GLint* params);
	static PFNGLTEXTUREPARAMETERIIVEXTPROC pfnTextureParameterIivEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureParameterIivEXT", pfnTextureParameterIivEXT, PFNGLTEXTUREPARAMETERIIVEXTPROC);
	pfnTextureParameterIivEXT(texture, target, pname, params);
}

void glTextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname, const GLuint* params)
{
	using PFNGLTEXTUREPARAMETERIUIVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, const GLuint* params);
	static PFNGLTEXTUREPARAMETERIUIVEXTPROC pfnTextureParameterIuivEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureParameterIuivEXT", pfnTextureParameterIuivEXT, PFNGLTEXTUREPARAMETERIUIVEXTPROC);
	pfnTextureParameterIuivEXT(texture, target, pname, params);
}

void glGetTextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETTEXTUREPARAMETERIIVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, GLint* params);
	static PFNGLGETTEXTUREPARAMETERIIVEXTPROC pfnGetTextureParameterIivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetTextureParameterIivEXT", pfnGetTextureParameterIivEXT, PFNGLGETTEXTUREPARAMETERIIVEXTPROC);
	pfnGetTextureParameterIivEXT(texture, target, pname, params);
}

void glGetTextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname, GLuint* params)
{
	using PFNGLGETTEXTUREPARAMETERIUIVEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum pname, GLuint* params);
	static PFNGLGETTEXTUREPARAMETERIUIVEXTPROC pfnGetTextureParameterIuivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetTextureParameterIuivEXT", pfnGetTextureParameterIuivEXT, PFNGLGETTEXTUREPARAMETERIUIVEXTPROC);
	pfnGetTextureParameterIuivEXT(texture, target, pname, params);
}

void glMultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params)
{
	using PFNGLMULTITEXPARAMETERIIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, const GLint* params);
	static PFNGLMULTITEXPARAMETERIIVEXTPROC pfnMultiTexParameterIivEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexParameterIivEXT", pfnMultiTexParameterIivEXT, PFNGLMULTITEXPARAMETERIIVEXTPROC);
	pfnMultiTexParameterIivEXT(texunit, target, pname, params);
}

void glMultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname, const GLuint* params)
{
	using PFNGLMULTITEXPARAMETERIUIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, const GLuint* params);
	static PFNGLMULTITEXPARAMETERIUIVEXTPROC pfnMultiTexParameterIuivEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexParameterIuivEXT", pfnMultiTexParameterIuivEXT, PFNGLMULTITEXPARAMETERIUIVEXTPROC);
	pfnMultiTexParameterIuivEXT(texunit, target, pname, params);
}

void glGetMultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETMULTITEXPARAMETERIIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLint* params);
	static PFNGLGETMULTITEXPARAMETERIIVEXTPROC pfnGetMultiTexParameterIivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexParameterIivEXT", pfnGetMultiTexParameterIivEXT, PFNGLGETMULTITEXPARAMETERIIVEXTPROC);
	pfnGetMultiTexParameterIivEXT(texunit, target, pname, params);
}

void glGetMultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname, GLuint* params)
{
	using PFNGLGETMULTITEXPARAMETERIUIVEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLenum pname, GLuint* params);
	static PFNGLGETMULTITEXPARAMETERIUIVEXTPROC pfnGetMultiTexParameterIuivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetMultiTexParameterIuivEXT", pfnGetMultiTexParameterIuivEXT, PFNGLGETMULTITEXPARAMETERIUIVEXTPROC);
	pfnGetMultiTexParameterIuivEXT(texunit, target, pname, params);
}

void glProgramUniform1uiEXT(GLuint program, GLint location, GLuint v0)
{
	using PFNGLPROGRAMUNIFORM1UIEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint v0);
	static PFNGLPROGRAMUNIFORM1UIEXTPROC pfnProgramUniform1uiEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1uiEXT", pfnProgramUniform1uiEXT, PFNGLPROGRAMUNIFORM1UIEXTPROC);
	pfnProgramUniform1uiEXT(program, location, v0);
}

void glProgramUniform2uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1)
{
	using PFNGLPROGRAMUNIFORM2UIEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint v0, GLuint v1);
	static PFNGLPROGRAMUNIFORM2UIEXTPROC pfnProgramUniform2uiEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2uiEXT", pfnProgramUniform2uiEXT, PFNGLPROGRAMUNIFORM2UIEXTPROC);
	pfnProgramUniform2uiEXT(program, location, v0, v1);
}

void glProgramUniform3uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	using PFNGLPROGRAMUNIFORM3UIEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
	static PFNGLPROGRAMUNIFORM3UIEXTPROC pfnProgramUniform3uiEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3uiEXT", pfnProgramUniform3uiEXT, PFNGLPROGRAMUNIFORM3UIEXTPROC);
	pfnProgramUniform3uiEXT(program, location, v0, v1, v2);
}

void glProgramUniform4uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	using PFNGLPROGRAMUNIFORM4UIEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static PFNGLPROGRAMUNIFORM4UIEXTPROC pfnProgramUniform4uiEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4uiEXT", pfnProgramUniform4uiEXT, PFNGLPROGRAMUNIFORM4UIEXTPROC);
	pfnProgramUniform4uiEXT(program, location, v0, v1, v2, v3);
}

void glProgramUniform1uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLPROGRAMUNIFORM1UIVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint* value);
	static PFNGLPROGRAMUNIFORM1UIVEXTPROC pfnProgramUniform1uivEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1uivEXT", pfnProgramUniform1uivEXT, PFNGLPROGRAMUNIFORM1UIVEXTPROC);
	pfnProgramUniform1uivEXT(program, location, count, value);
}

void glProgramUniform2uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLPROGRAMUNIFORM2UIVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint* value);
	static PFNGLPROGRAMUNIFORM2UIVEXTPROC pfnProgramUniform2uivEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2uivEXT", pfnProgramUniform2uivEXT, PFNGLPROGRAMUNIFORM2UIVEXTPROC);
	pfnProgramUniform2uivEXT(program, location, count, value);
}

void glProgramUniform3uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLPROGRAMUNIFORM3UIVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint* value);
	static PFNGLPROGRAMUNIFORM3UIVEXTPROC pfnProgramUniform3uivEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3uivEXT", pfnProgramUniform3uivEXT, PFNGLPROGRAMUNIFORM3UIVEXTPROC);
	pfnProgramUniform3uivEXT(program, location, count, value);
}

void glProgramUniform4uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
	using PFNGLPROGRAMUNIFORM4UIVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint* value);
	static PFNGLPROGRAMUNIFORM4UIVEXTPROC pfnProgramUniform4uivEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4uivEXT", pfnProgramUniform4uivEXT, PFNGLPROGRAMUNIFORM4UIVEXTPROC);
	pfnProgramUniform4uivEXT(program, location, count, value);
}

void glNamedProgramLocalParameters4fvEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat* params)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat* params);
	static PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC pfnNamedProgramLocalParameters4fvEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParameters4fvEXT", pfnNamedProgramLocalParameters4fvEXT, PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC);
	pfnNamedProgramLocalParameters4fvEXT(program, target, index, count, params);
}

void glNamedProgramLocalParameterI4iEXT(GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
	static PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC pfnNamedProgramLocalParameterI4iEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParameterI4iEXT", pfnNamedProgramLocalParameterI4iEXT, PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC);
	pfnNamedProgramLocalParameterI4iEXT(program, target, index, x, y, z, w);
}

void glNamedProgramLocalParameterI4ivEXT(GLuint program, GLenum target, GLuint index, const GLint* params)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, const GLint* params);
	static PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC pfnNamedProgramLocalParameterI4ivEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParameterI4ivEXT", pfnNamedProgramLocalParameterI4ivEXT, PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC);
	pfnNamedProgramLocalParameterI4ivEXT(program, target, index, params);
}

void glNamedProgramLocalParametersI4ivEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLint* params)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLsizei count, const GLint* params);
	static PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC pfnNamedProgramLocalParametersI4ivEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParametersI4ivEXT", pfnNamedProgramLocalParametersI4ivEXT, PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC);
	pfnNamedProgramLocalParametersI4ivEXT(program, target, index, count, params);
}

void glNamedProgramLocalParameterI4uiEXT(GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
	static PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC pfnNamedProgramLocalParameterI4uiEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParameterI4uiEXT", pfnNamedProgramLocalParameterI4uiEXT, PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC);
	pfnNamedProgramLocalParameterI4uiEXT(program, target, index, x, y, z, w);
}

void glNamedProgramLocalParameterI4uivEXT(GLuint program, GLenum target, GLuint index, const GLuint* params)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, const GLuint* params);
	static PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC pfnNamedProgramLocalParameterI4uivEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParameterI4uivEXT", pfnNamedProgramLocalParameterI4uivEXT, PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC);
	pfnNamedProgramLocalParameterI4uivEXT(program, target, index, params);
}

void glNamedProgramLocalParametersI4uivEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint* params)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint* params);
	static PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC pfnNamedProgramLocalParametersI4uivEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParametersI4uivEXT", pfnNamedProgramLocalParametersI4uivEXT, PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC);
	pfnNamedProgramLocalParametersI4uivEXT(program, target, index, count, params);
}

void glGetNamedProgramLocalParameterIivEXT(GLuint program, GLenum target, GLuint index, GLint* params)
{
	using PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLint* params);
	static PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC pfnGetNamedProgramLocalParameterIivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedProgramLocalParameterIivEXT", pfnGetNamedProgramLocalParameterIivEXT, PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC);
	pfnGetNamedProgramLocalParameterIivEXT(program, target, index, params);
}

void glGetNamedProgramLocalParameterIuivEXT(GLuint program, GLenum target, GLuint index, GLuint* params)
{
	using PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLuint* params);
	static PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC pfnGetNamedProgramLocalParameterIuivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedProgramLocalParameterIuivEXT", pfnGetNamedProgramLocalParameterIuivEXT, PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC);
	pfnGetNamedProgramLocalParameterIuivEXT(program, target, index, params);
}

void glEnableClientStateiEXT(GLenum array, GLuint index)
{
	using PFNGLENABLECLIENTSTATEIEXTPROC = void(APIENTRY *)(GLenum array, GLuint index);
	static PFNGLENABLECLIENTSTATEIEXTPROC pfnEnableClientStateiEXT{nullptr};
	LOAD_ENTRYPOINT("glEnableClientStateiEXT", pfnEnableClientStateiEXT, PFNGLENABLECLIENTSTATEIEXTPROC);
	pfnEnableClientStateiEXT(array, index);
}

void glDisableClientStateiEXT(GLenum array, GLuint index)
{
	using PFNGLDISABLECLIENTSTATEIEXTPROC = void(APIENTRY *)(GLenum array, GLuint index);
	static PFNGLDISABLECLIENTSTATEIEXTPROC pfnDisableClientStateiEXT{nullptr};
	LOAD_ENTRYPOINT("glDisableClientStateiEXT", pfnDisableClientStateiEXT, PFNGLDISABLECLIENTSTATEIEXTPROC);
	pfnDisableClientStateiEXT(array, index);
}

void glGetFloati_vEXT(GLenum pname, GLuint index, GLfloat* params)
{
	using PFNGLGETFLOATI_VEXTPROC = void(APIENTRY *)(GLenum pname, GLuint index, GLfloat* params);
	static PFNGLGETFLOATI_VEXTPROC pfnGetFloati_vEXT{nullptr};
	LOAD_ENTRYPOINT("glGetFloati_vEXT", pfnGetFloati_vEXT, PFNGLGETFLOATI_VEXTPROC);
	pfnGetFloati_vEXT(pname, index, params);
}

void glGetDoublei_vEXT(GLenum pname, GLuint index, GLdouble* params)
{
	using PFNGLGETDOUBLEI_VEXTPROC = void(APIENTRY *)(GLenum pname, GLuint index, GLdouble* params);
	static PFNGLGETDOUBLEI_VEXTPROC pfnGetDoublei_vEXT{nullptr};
	LOAD_ENTRYPOINT("glGetDoublei_vEXT", pfnGetDoublei_vEXT, PFNGLGETDOUBLEI_VEXTPROC);
	pfnGetDoublei_vEXT(pname, index, params);
}

void glGetPointeri_vEXT(GLenum pname, GLuint index, void** params)
{
	using PFNGLGETPOINTERI_VEXTPROC = void(APIENTRY *)(GLenum pname, GLuint index, void** params);
	static PFNGLGETPOINTERI_VEXTPROC pfnGetPointeri_vEXT{nullptr};
	LOAD_ENTRYPOINT("glGetPointeri_vEXT", pfnGetPointeri_vEXT, PFNGLGETPOINTERI_VEXTPROC);
	pfnGetPointeri_vEXT(pname, index, params);
}

void glNamedProgramStringEXT(GLuint program, GLenum target, GLenum format, GLsizei len, const void* string)
{
	using PFNGLNAMEDPROGRAMSTRINGEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLenum format, GLsizei len, const void* string);
	static PFNGLNAMEDPROGRAMSTRINGEXTPROC pfnNamedProgramStringEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramStringEXT", pfnNamedProgramStringEXT, PFNGLNAMEDPROGRAMSTRINGEXTPROC);
	pfnNamedProgramStringEXT(program, target, format, len, string);
}

void glNamedProgramLocalParameter4dEXT(GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC pfnNamedProgramLocalParameter4dEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParameter4dEXT", pfnNamedProgramLocalParameter4dEXT, PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC);
	pfnNamedProgramLocalParameter4dEXT(program, target, index, x, y, z, w);
}

void glNamedProgramLocalParameter4dvEXT(GLuint program, GLenum target, GLuint index, const GLdouble* params)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, const GLdouble* params);
	static PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC pfnNamedProgramLocalParameter4dvEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParameter4dvEXT", pfnNamedProgramLocalParameter4dvEXT, PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC);
	pfnNamedProgramLocalParameter4dvEXT(program, target, index, params);
}

void glNamedProgramLocalParameter4fEXT(GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	static PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC pfnNamedProgramLocalParameter4fEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParameter4fEXT", pfnNamedProgramLocalParameter4fEXT, PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC);
	pfnNamedProgramLocalParameter4fEXT(program, target, index, x, y, z, w);
}

void glNamedProgramLocalParameter4fvEXT(GLuint program, GLenum target, GLuint index, const GLfloat* params)
{
	using PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, const GLfloat* params);
	static PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC pfnNamedProgramLocalParameter4fvEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedProgramLocalParameter4fvEXT", pfnNamedProgramLocalParameter4fvEXT, PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC);
	pfnNamedProgramLocalParameter4fvEXT(program, target, index, params);
}

void glGetNamedProgramLocalParameterdvEXT(GLuint program, GLenum target, GLuint index, GLdouble* params)
{
	using PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLdouble* params);
	static PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC pfnGetNamedProgramLocalParameterdvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedProgramLocalParameterdvEXT", pfnGetNamedProgramLocalParameterdvEXT, PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC);
	pfnGetNamedProgramLocalParameterdvEXT(program, target, index, params);
}

void glGetNamedProgramLocalParameterfvEXT(GLuint program, GLenum target, GLuint index, GLfloat* params)
{
	using PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLuint index, GLfloat* params);
	static PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC pfnGetNamedProgramLocalParameterfvEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedProgramLocalParameterfvEXT", pfnGetNamedProgramLocalParameterfvEXT, PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC);
	pfnGetNamedProgramLocalParameterfvEXT(program, target, index, params);
}

void glGetNamedProgramivEXT(GLuint program, GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETNAMEDPROGRAMIVEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLenum pname, GLint* params);
	static PFNGLGETNAMEDPROGRAMIVEXTPROC pfnGetNamedProgramivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedProgramivEXT", pfnGetNamedProgramivEXT, PFNGLGETNAMEDPROGRAMIVEXTPROC);
	pfnGetNamedProgramivEXT(program, target, pname, params);
}

void glGetNamedProgramStringEXT(GLuint program, GLenum target, GLenum pname, void* string)
{
	using PFNGLGETNAMEDPROGRAMSTRINGEXTPROC = void(APIENTRY *)(GLuint program, GLenum target, GLenum pname, void* string);
	static PFNGLGETNAMEDPROGRAMSTRINGEXTPROC pfnGetNamedProgramStringEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedProgramStringEXT", pfnGetNamedProgramStringEXT, PFNGLGETNAMEDPROGRAMSTRINGEXTPROC);
	pfnGetNamedProgramStringEXT(program, target, pname, string);
}

void glNamedRenderbufferStorageEXT(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC = void(APIENTRY *)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC pfnNamedRenderbufferStorageEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedRenderbufferStorageEXT", pfnNamedRenderbufferStorageEXT, PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC);
	pfnNamedRenderbufferStorageEXT(renderbuffer, internalformat, width, height);
}

void glGetNamedRenderbufferParameterivEXT(GLuint renderbuffer, GLenum pname, GLint* params)
{
	using PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC = void(APIENTRY *)(GLuint renderbuffer, GLenum pname, GLint* params);
	static PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC pfnGetNamedRenderbufferParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedRenderbufferParameterivEXT", pfnGetNamedRenderbufferParameterivEXT, PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC);
	pfnGetNamedRenderbufferParameterivEXT(renderbuffer, pname, params);
}

void glNamedRenderbufferStorageMultisampleEXT(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC = void(APIENTRY *)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC pfnNamedRenderbufferStorageMultisampleEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedRenderbufferStorageMultisampleEXT", pfnNamedRenderbufferStorageMultisampleEXT, PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC);
	pfnNamedRenderbufferStorageMultisampleEXT(renderbuffer, samples, internalformat, width, height);
}

void glNamedRenderbufferStorageMultisampleCoverageEXT(GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC = void(APIENTRY *)(GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC pfnNamedRenderbufferStorageMultisampleCoverageEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedRenderbufferStorageMultisampleCoverageEXT", pfnNamedRenderbufferStorageMultisampleCoverageEXT, PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC);
	pfnNamedRenderbufferStorageMultisampleCoverageEXT(renderbuffer, coverageSamples, colorSamples, internalformat, width, height);
}

GLenum glCheckNamedFramebufferStatusEXT(GLuint framebuffer, GLenum target)
{
	using PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC = GLenum(APIENTRY *)(GLuint framebuffer, GLenum target);
	static PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC pfnCheckNamedFramebufferStatusEXT{nullptr};
	LOAD_ENTRYPOINT("glCheckNamedFramebufferStatusEXT", pfnCheckNamedFramebufferStatusEXT, PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC);
	return pfnCheckNamedFramebufferStatusEXT(framebuffer, target);
}

void glNamedFramebufferTexture1DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	using PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC pfnNamedFramebufferTexture1DEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferTexture1DEXT", pfnNamedFramebufferTexture1DEXT, PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC);
	pfnNamedFramebufferTexture1DEXT(framebuffer, attachment, textarget, texture, level);
}

void glNamedFramebufferTexture2DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	using PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC pfnNamedFramebufferTexture2DEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferTexture2DEXT", pfnNamedFramebufferTexture2DEXT, PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC);
	pfnNamedFramebufferTexture2DEXT(framebuffer, attachment, textarget, texture, level);
}

void glNamedFramebufferTexture3DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	using PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
	static PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC pfnNamedFramebufferTexture3DEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferTexture3DEXT", pfnNamedFramebufferTexture3DEXT, PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC);
	pfnNamedFramebufferTexture3DEXT(framebuffer, attachment, textarget, texture, level, zoffset);
}

void glNamedFramebufferRenderbufferEXT(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	using PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC pfnNamedFramebufferRenderbufferEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferRenderbufferEXT", pfnNamedFramebufferRenderbufferEXT, PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC);
	pfnNamedFramebufferRenderbufferEXT(framebuffer, attachment, renderbuffertarget, renderbuffer);
}

void glGetNamedFramebufferAttachmentParameterivEXT(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params)
{
	using PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params);
	static PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC pfnGetNamedFramebufferAttachmentParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedFramebufferAttachmentParameterivEXT", pfnGetNamedFramebufferAttachmentParameterivEXT, PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC);
	pfnGetNamedFramebufferAttachmentParameterivEXT(framebuffer, attachment, pname, params);
}

void glGenerateTextureMipmapEXT(GLuint texture, GLenum target)
{
	using PFNGLGENERATETEXTUREMIPMAPEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target);
	static PFNGLGENERATETEXTUREMIPMAPEXTPROC pfnGenerateTextureMipmapEXT{nullptr};
	LOAD_ENTRYPOINT("glGenerateTextureMipmapEXT", pfnGenerateTextureMipmapEXT, PFNGLGENERATETEXTUREMIPMAPEXTPROC);
	pfnGenerateTextureMipmapEXT(texture, target);
}

void glGenerateMultiTexMipmapEXT(GLenum texunit, GLenum target)
{
	using PFNGLGENERATEMULTITEXMIPMAPEXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target);
	static PFNGLGENERATEMULTITEXMIPMAPEXTPROC pfnGenerateMultiTexMipmapEXT{nullptr};
	LOAD_ENTRYPOINT("glGenerateMultiTexMipmapEXT", pfnGenerateMultiTexMipmapEXT, PFNGLGENERATEMULTITEXMIPMAPEXTPROC);
	pfnGenerateMultiTexMipmapEXT(texunit, target);
}

void glFramebufferDrawBufferEXT(GLuint framebuffer, GLenum mode)
{
	using PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum mode);
	static PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC pfnFramebufferDrawBufferEXT{nullptr};
	LOAD_ENTRYPOINT("glFramebufferDrawBufferEXT", pfnFramebufferDrawBufferEXT, PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC);
	pfnFramebufferDrawBufferEXT(framebuffer, mode);
}

void glFramebufferDrawBuffersEXT(GLuint framebuffer, GLsizei n, const GLenum* bufs)
{
	using PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLsizei n, const GLenum* bufs);
	static PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC pfnFramebufferDrawBuffersEXT{nullptr};
	LOAD_ENTRYPOINT("glFramebufferDrawBuffersEXT", pfnFramebufferDrawBuffersEXT, PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC);
	pfnFramebufferDrawBuffersEXT(framebuffer, n, bufs);
}

void glFramebufferReadBufferEXT(GLuint framebuffer, GLenum mode)
{
	using PFNGLFRAMEBUFFERREADBUFFEREXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum mode);
	static PFNGLFRAMEBUFFERREADBUFFEREXTPROC pfnFramebufferReadBufferEXT{nullptr};
	LOAD_ENTRYPOINT("glFramebufferReadBufferEXT", pfnFramebufferReadBufferEXT, PFNGLFRAMEBUFFERREADBUFFEREXTPROC);
	pfnFramebufferReadBufferEXT(framebuffer, mode);
}

void glGetFramebufferParameterivEXT(GLuint framebuffer, GLenum pname, GLint* params)
{
	using PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum pname, GLint* params);
	static PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC pfnGetFramebufferParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetFramebufferParameterivEXT", pfnGetFramebufferParameterivEXT, PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC);
	pfnGetFramebufferParameterivEXT(framebuffer, pname, params);
}

void glNamedCopyBufferSubDataEXT(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
	using PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC = void(APIENTRY *)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
	static PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC pfnNamedCopyBufferSubDataEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedCopyBufferSubDataEXT", pfnNamedCopyBufferSubDataEXT, PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC);
	pfnNamedCopyBufferSubDataEXT(readBuffer, writeBuffer, readOffset, writeOffset, size);
}

void glNamedFramebufferTextureEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)
{
	using PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
	static PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC pfnNamedFramebufferTextureEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferTextureEXT", pfnNamedFramebufferTextureEXT, PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC);
	pfnNamedFramebufferTextureEXT(framebuffer, attachment, texture, level);
}

void glNamedFramebufferTextureLayerEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	using PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC pfnNamedFramebufferTextureLayerEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferTextureLayerEXT", pfnNamedFramebufferTextureLayerEXT, PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC);
	pfnNamedFramebufferTextureLayerEXT(framebuffer, attachment, texture, level, layer);
}

void glNamedFramebufferTextureFaceEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face)
{
	using PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face);
	static PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC pfnNamedFramebufferTextureFaceEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferTextureFaceEXT", pfnNamedFramebufferTextureFaceEXT, PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC);
	pfnNamedFramebufferTextureFaceEXT(framebuffer, attachment, texture, level, face);
}

void glTextureRenderbufferEXT(GLuint texture, GLenum target, GLuint renderbuffer)
{
	using PFNGLTEXTURERENDERBUFFEREXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLuint renderbuffer);
	static PFNGLTEXTURERENDERBUFFEREXTPROC pfnTextureRenderbufferEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureRenderbufferEXT", pfnTextureRenderbufferEXT, PFNGLTEXTURERENDERBUFFEREXTPROC);
	pfnTextureRenderbufferEXT(texture, target, renderbuffer);
}

void glMultiTexRenderbufferEXT(GLenum texunit, GLenum target, GLuint renderbuffer)
{
	using PFNGLMULTITEXRENDERBUFFEREXTPROC = void(APIENTRY *)(GLenum texunit, GLenum target, GLuint renderbuffer);
	static PFNGLMULTITEXRENDERBUFFEREXTPROC pfnMultiTexRenderbufferEXT{nullptr};
	LOAD_ENTRYPOINT("glMultiTexRenderbufferEXT", pfnMultiTexRenderbufferEXT, PFNGLMULTITEXRENDERBUFFEREXTPROC);
	pfnMultiTexRenderbufferEXT(texunit, target, renderbuffer);
}

void glVertexArrayVertexOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYVERTEXOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYVERTEXOFFSETEXTPROC pfnVertexArrayVertexOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexOffsetEXT", pfnVertexArrayVertexOffsetEXT, PFNGLVERTEXARRAYVERTEXOFFSETEXTPROC);
	pfnVertexArrayVertexOffsetEXT(vaobj, buffer, size, type, stride, offset);
}

void glVertexArrayColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYCOLOROFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYCOLOROFFSETEXTPROC pfnVertexArrayColorOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayColorOffsetEXT", pfnVertexArrayColorOffsetEXT, PFNGLVERTEXARRAYCOLOROFFSETEXTPROC);
	pfnVertexArrayColorOffsetEXT(vaobj, buffer, size, type, stride, offset);
}

void glVertexArrayEdgeFlagOffsetEXT(GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYEDGEFLAGOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYEDGEFLAGOFFSETEXTPROC pfnVertexArrayEdgeFlagOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayEdgeFlagOffsetEXT", pfnVertexArrayEdgeFlagOffsetEXT, PFNGLVERTEXARRAYEDGEFLAGOFFSETEXTPROC);
	pfnVertexArrayEdgeFlagOffsetEXT(vaobj, buffer, stride, offset);
}

void glVertexArrayIndexOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYINDEXOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYINDEXOFFSETEXTPROC pfnVertexArrayIndexOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayIndexOffsetEXT", pfnVertexArrayIndexOffsetEXT, PFNGLVERTEXARRAYINDEXOFFSETEXTPROC);
	pfnVertexArrayIndexOffsetEXT(vaobj, buffer, type, stride, offset);
}

void glVertexArrayNormalOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYNORMALOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYNORMALOFFSETEXTPROC pfnVertexArrayNormalOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayNormalOffsetEXT", pfnVertexArrayNormalOffsetEXT, PFNGLVERTEXARRAYNORMALOFFSETEXTPROC);
	pfnVertexArrayNormalOffsetEXT(vaobj, buffer, type, stride, offset);
}

void glVertexArrayTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYTEXCOORDOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYTEXCOORDOFFSETEXTPROC pfnVertexArrayTexCoordOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayTexCoordOffsetEXT", pfnVertexArrayTexCoordOffsetEXT, PFNGLVERTEXARRAYTEXCOORDOFFSETEXTPROC);
	pfnVertexArrayTexCoordOffsetEXT(vaobj, buffer, size, type, stride, offset);
}

void glVertexArrayMultiTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYMULTITEXCOORDOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYMULTITEXCOORDOFFSETEXTPROC pfnVertexArrayMultiTexCoordOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayMultiTexCoordOffsetEXT", pfnVertexArrayMultiTexCoordOffsetEXT, PFNGLVERTEXARRAYMULTITEXCOORDOFFSETEXTPROC);
	pfnVertexArrayMultiTexCoordOffsetEXT(vaobj, buffer, texunit, size, type, stride, offset);
}

void glVertexArrayFogCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYFOGCOORDOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYFOGCOORDOFFSETEXTPROC pfnVertexArrayFogCoordOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayFogCoordOffsetEXT", pfnVertexArrayFogCoordOffsetEXT, PFNGLVERTEXARRAYFOGCOORDOFFSETEXTPROC);
	pfnVertexArrayFogCoordOffsetEXT(vaobj, buffer, type, stride, offset);
}

void glVertexArraySecondaryColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYSECONDARYCOLOROFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYSECONDARYCOLOROFFSETEXTPROC pfnVertexArraySecondaryColorOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArraySecondaryColorOffsetEXT", pfnVertexArraySecondaryColorOffsetEXT, PFNGLVERTEXARRAYSECONDARYCOLOROFFSETEXTPROC);
	pfnVertexArraySecondaryColorOffsetEXT(vaobj, buffer, size, type, stride, offset);
}

void glVertexArrayVertexAttribOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYVERTEXATTRIBOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYVERTEXATTRIBOFFSETEXTPROC pfnVertexArrayVertexAttribOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexAttribOffsetEXT", pfnVertexArrayVertexAttribOffsetEXT, PFNGLVERTEXARRAYVERTEXATTRIBOFFSETEXTPROC);
	pfnVertexArrayVertexAttribOffsetEXT(vaobj, buffer, index, size, type, normalized, stride, offset);
}

void glVertexArrayVertexAttribIOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYVERTEXATTRIBIOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYVERTEXATTRIBIOFFSETEXTPROC pfnVertexArrayVertexAttribIOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexAttribIOffsetEXT", pfnVertexArrayVertexAttribIOffsetEXT, PFNGLVERTEXARRAYVERTEXATTRIBIOFFSETEXTPROC);
	pfnVertexArrayVertexAttribIOffsetEXT(vaobj, buffer, index, size, type, stride, offset);
}

void glEnableVertexArrayEXT(GLuint vaobj, GLenum array)
{
	using PFNGLENABLEVERTEXARRAYEXTPROC = void(APIENTRY *)(GLuint vaobj, GLenum array);
	static PFNGLENABLEVERTEXARRAYEXTPROC pfnEnableVertexArrayEXT{nullptr};
	LOAD_ENTRYPOINT("glEnableVertexArrayEXT", pfnEnableVertexArrayEXT, PFNGLENABLEVERTEXARRAYEXTPROC);
	pfnEnableVertexArrayEXT(vaobj, array);
}

void glDisableVertexArrayEXT(GLuint vaobj, GLenum array)
{
	using PFNGLDISABLEVERTEXARRAYEXTPROC = void(APIENTRY *)(GLuint vaobj, GLenum array);
	static PFNGLDISABLEVERTEXARRAYEXTPROC pfnDisableVertexArrayEXT{nullptr};
	LOAD_ENTRYPOINT("glDisableVertexArrayEXT", pfnDisableVertexArrayEXT, PFNGLDISABLEVERTEXARRAYEXTPROC);
	pfnDisableVertexArrayEXT(vaobj, array);
}

void glEnableVertexArrayAttribEXT(GLuint vaobj, GLuint index)
{
	using PFNGLENABLEVERTEXARRAYATTRIBEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint index);
	static PFNGLENABLEVERTEXARRAYATTRIBEXTPROC pfnEnableVertexArrayAttribEXT{nullptr};
	LOAD_ENTRYPOINT("glEnableVertexArrayAttribEXT", pfnEnableVertexArrayAttribEXT, PFNGLENABLEVERTEXARRAYATTRIBEXTPROC);
	pfnEnableVertexArrayAttribEXT(vaobj, index);
}

void glDisableVertexArrayAttribEXT(GLuint vaobj, GLuint index)
{
	using PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint index);
	static PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC pfnDisableVertexArrayAttribEXT{nullptr};
	LOAD_ENTRYPOINT("glDisableVertexArrayAttribEXT", pfnDisableVertexArrayAttribEXT, PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC);
	pfnDisableVertexArrayAttribEXT(vaobj, index);
}

void glGetVertexArrayIntegervEXT(GLuint vaobj, GLenum pname, GLint* param)
{
	using PFNGLGETVERTEXARRAYINTEGERVEXTPROC = void(APIENTRY *)(GLuint vaobj, GLenum pname, GLint* param);
	static PFNGLGETVERTEXARRAYINTEGERVEXTPROC pfnGetVertexArrayIntegervEXT{nullptr};
	LOAD_ENTRYPOINT("glGetVertexArrayIntegervEXT", pfnGetVertexArrayIntegervEXT, PFNGLGETVERTEXARRAYINTEGERVEXTPROC);
	pfnGetVertexArrayIntegervEXT(vaobj, pname, param);
}

void glGetVertexArrayPointervEXT(GLuint vaobj, GLenum pname, void** param)
{
	using PFNGLGETVERTEXARRAYPOINTERVEXTPROC = void(APIENTRY *)(GLuint vaobj, GLenum pname, void** param);
	static PFNGLGETVERTEXARRAYPOINTERVEXTPROC pfnGetVertexArrayPointervEXT{nullptr};
	LOAD_ENTRYPOINT("glGetVertexArrayPointervEXT", pfnGetVertexArrayPointervEXT, PFNGLGETVERTEXARRAYPOINTERVEXTPROC);
	pfnGetVertexArrayPointervEXT(vaobj, pname, param);
}

void glGetVertexArrayIntegeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, GLint* param)
{
	using PFNGLGETVERTEXARRAYINTEGERI_VEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint index, GLenum pname, GLint* param);
	static PFNGLGETVERTEXARRAYINTEGERI_VEXTPROC pfnGetVertexArrayIntegeri_vEXT{nullptr};
	LOAD_ENTRYPOINT("glGetVertexArrayIntegeri_vEXT", pfnGetVertexArrayIntegeri_vEXT, PFNGLGETVERTEXARRAYINTEGERI_VEXTPROC);
	pfnGetVertexArrayIntegeri_vEXT(vaobj, index, pname, param);
}

void glGetVertexArrayPointeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, void** param)
{
	using PFNGLGETVERTEXARRAYPOINTERI_VEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint index, GLenum pname, void** param);
	static PFNGLGETVERTEXARRAYPOINTERI_VEXTPROC pfnGetVertexArrayPointeri_vEXT{nullptr};
	LOAD_ENTRYPOINT("glGetVertexArrayPointeri_vEXT", pfnGetVertexArrayPointeri_vEXT, PFNGLGETVERTEXARRAYPOINTERI_VEXTPROC);
	pfnGetVertexArrayPointeri_vEXT(vaobj, index, pname, param);
}

void* glMapNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	using PFNGLMAPNAMEDBUFFERRANGEEXTPROC = void*(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
	static PFNGLMAPNAMEDBUFFERRANGEEXTPROC pfnMapNamedBufferRangeEXT{nullptr};
	LOAD_ENTRYPOINT("glMapNamedBufferRangeEXT", pfnMapNamedBufferRangeEXT, PFNGLMAPNAMEDBUFFERRANGEEXTPROC);
	return pfnMapNamedBufferRangeEXT(buffer, offset, length, access);
}

void glFlushMappedNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
	using PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr length);
	static PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC pfnFlushMappedNamedBufferRangeEXT{nullptr};
	LOAD_ENTRYPOINT("glFlushMappedNamedBufferRangeEXT", pfnFlushMappedNamedBufferRangeEXT, PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC);
	pfnFlushMappedNamedBufferRangeEXT(buffer, offset, length);
}

void glNamedBufferStorageEXT(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags)
{
	using PFNGLNAMEDBUFFERSTORAGEEXTPROC = void(APIENTRY *)(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags);
	static PFNGLNAMEDBUFFERSTORAGEEXTPROC pfnNamedBufferStorageEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferStorageEXT", pfnNamedBufferStorageEXT, PFNGLNAMEDBUFFERSTORAGEEXTPROC);
	pfnNamedBufferStorageEXT(buffer, size, data, flags);
}

void glClearNamedBufferDataEXT(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data)
{
	using PFNGLCLEARNAMEDBUFFERDATAEXTPROC = void(APIENTRY *)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data);
	static PFNGLCLEARNAMEDBUFFERDATAEXTPROC pfnClearNamedBufferDataEXT{nullptr};
	LOAD_ENTRYPOINT("glClearNamedBufferDataEXT", pfnClearNamedBufferDataEXT, PFNGLCLEARNAMEDBUFFERDATAEXTPROC);
	pfnClearNamedBufferDataEXT(buffer, internalformat, format, type, data);
}

void glClearNamedBufferSubDataEXT(GLuint buffer, GLenum internalformat, GLsizeiptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data)
{
	using PFNGLCLEARNAMEDBUFFERSUBDATAEXTPROC = void(APIENTRY *)(GLuint buffer, GLenum internalformat, GLsizeiptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);
	static PFNGLCLEARNAMEDBUFFERSUBDATAEXTPROC pfnClearNamedBufferSubDataEXT{nullptr};
	LOAD_ENTRYPOINT("glClearNamedBufferSubDataEXT", pfnClearNamedBufferSubDataEXT, PFNGLCLEARNAMEDBUFFERSUBDATAEXTPROC);
	pfnClearNamedBufferSubDataEXT(buffer, internalformat, offset, size, format, type, data);
}

void glNamedFramebufferParameteriEXT(GLuint framebuffer, GLenum pname, GLint param)
{
	using PFNGLNAMEDFRAMEBUFFERPARAMETERIEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum pname, GLint param);
	static PFNGLNAMEDFRAMEBUFFERPARAMETERIEXTPROC pfnNamedFramebufferParameteriEXT{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferParameteriEXT", pfnNamedFramebufferParameteriEXT, PFNGLNAMEDFRAMEBUFFERPARAMETERIEXTPROC);
	pfnNamedFramebufferParameteriEXT(framebuffer, pname, param);
}

void glGetNamedFramebufferParameterivEXT(GLuint framebuffer, GLenum pname, GLint* params)
{
	using PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVEXTPROC = void(APIENTRY *)(GLuint framebuffer, GLenum pname, GLint* params);
	static PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVEXTPROC pfnGetNamedFramebufferParameterivEXT{nullptr};
	LOAD_ENTRYPOINT("glGetNamedFramebufferParameterivEXT", pfnGetNamedFramebufferParameterivEXT, PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVEXTPROC);
	pfnGetNamedFramebufferParameterivEXT(framebuffer, pname, params);
}

void glProgramUniform1dEXT(GLuint program, GLint location, GLdouble x)
{
	using PFNGLPROGRAMUNIFORM1DEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLdouble x);
	static PFNGLPROGRAMUNIFORM1DEXTPROC pfnProgramUniform1dEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1dEXT", pfnProgramUniform1dEXT, PFNGLPROGRAMUNIFORM1DEXTPROC);
	pfnProgramUniform1dEXT(program, location, x);
}

void glProgramUniform2dEXT(GLuint program, GLint location, GLdouble x, GLdouble y)
{
	using PFNGLPROGRAMUNIFORM2DEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLdouble x, GLdouble y);
	static PFNGLPROGRAMUNIFORM2DEXTPROC pfnProgramUniform2dEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2dEXT", pfnProgramUniform2dEXT, PFNGLPROGRAMUNIFORM2DEXTPROC);
	pfnProgramUniform2dEXT(program, location, x, y);
}

void glProgramUniform3dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	using PFNGLPROGRAMUNIFORM3DEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLPROGRAMUNIFORM3DEXTPROC pfnProgramUniform3dEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3dEXT", pfnProgramUniform3dEXT, PFNGLPROGRAMUNIFORM3DEXTPROC);
	pfnProgramUniform3dEXT(program, location, x, y, z);
}

void glProgramUniform4dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	using PFNGLPROGRAMUNIFORM4DEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static PFNGLPROGRAMUNIFORM4DEXTPROC pfnProgramUniform4dEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4dEXT", pfnProgramUniform4dEXT, PFNGLPROGRAMUNIFORM4DEXTPROC);
	pfnProgramUniform4dEXT(program, location, x, y, z, w);
}

void glProgramUniform1dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORM1DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
	static PFNGLPROGRAMUNIFORM1DVEXTPROC pfnProgramUniform1dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1dvEXT", pfnProgramUniform1dvEXT, PFNGLPROGRAMUNIFORM1DVEXTPROC);
	pfnProgramUniform1dvEXT(program, location, count, value);
}

void glProgramUniform2dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORM2DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
	static PFNGLPROGRAMUNIFORM2DVEXTPROC pfnProgramUniform2dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2dvEXT", pfnProgramUniform2dvEXT, PFNGLPROGRAMUNIFORM2DVEXTPROC);
	pfnProgramUniform2dvEXT(program, location, count, value);
}

void glProgramUniform3dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORM3DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
	static PFNGLPROGRAMUNIFORM3DVEXTPROC pfnProgramUniform3dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3dvEXT", pfnProgramUniform3dvEXT, PFNGLPROGRAMUNIFORM3DVEXTPROC);
	pfnProgramUniform3dvEXT(program, location, count, value);
}

void glProgramUniform4dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORM4DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLdouble* value);
	static PFNGLPROGRAMUNIFORM4DVEXTPROC pfnProgramUniform4dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4dvEXT", pfnProgramUniform4dvEXT, PFNGLPROGRAMUNIFORM4DVEXTPROC);
	pfnProgramUniform4dvEXT(program, location, count, value);
}

void glProgramUniformMatrix2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC pfnProgramUniformMatrix2dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2dvEXT", pfnProgramUniformMatrix2dvEXT, PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC);
	pfnProgramUniformMatrix2dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC pfnProgramUniformMatrix3dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3dvEXT", pfnProgramUniformMatrix3dvEXT, PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC);
	pfnProgramUniformMatrix3dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC pfnProgramUniformMatrix4dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4dvEXT", pfnProgramUniformMatrix4dvEXT, PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC);
	pfnProgramUniformMatrix4dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC pfnProgramUniformMatrix2x3dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x3dvEXT", pfnProgramUniformMatrix2x3dvEXT, PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC);
	pfnProgramUniformMatrix2x3dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC pfnProgramUniformMatrix2x4dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x4dvEXT", pfnProgramUniformMatrix2x4dvEXT, PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC);
	pfnProgramUniformMatrix2x4dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC pfnProgramUniformMatrix3x2dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x2dvEXT", pfnProgramUniformMatrix3x2dvEXT, PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC);
	pfnProgramUniformMatrix3x2dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC pfnProgramUniformMatrix3x4dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x4dvEXT", pfnProgramUniformMatrix3x4dvEXT, PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC);
	pfnProgramUniformMatrix3x4dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC pfnProgramUniformMatrix4x2dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x2dvEXT", pfnProgramUniformMatrix4x2dvEXT, PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC);
	pfnProgramUniformMatrix4x2dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value)
{
	using PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
	static PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC pfnProgramUniformMatrix4x3dvEXT{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x3dvEXT", pfnProgramUniformMatrix4x3dvEXT, PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC);
	pfnProgramUniformMatrix4x3dvEXT(program, location, count, transpose, value);
}

void glTextureBufferRangeEXT(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	using PFNGLTEXTUREBUFFERRANGEEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static PFNGLTEXTUREBUFFERRANGEEXTPROC pfnTextureBufferRangeEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureBufferRangeEXT", pfnTextureBufferRangeEXT, PFNGLTEXTUREBUFFERRANGEEXTPROC);
	pfnTextureBufferRangeEXT(texture, target, internalformat, buffer, offset, size);
}

void glTextureStorage1DEXT(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
	using PFNGLTEXTURESTORAGE1DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
	static PFNGLTEXTURESTORAGE1DEXTPROC pfnTextureStorage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage1DEXT", pfnTextureStorage1DEXT, PFNGLTEXTURESTORAGE1DEXTPROC);
	pfnTextureStorage1DEXT(texture, target, levels, internalformat, width);
}

void glTextureStorage2DEXT(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLTEXTURESTORAGE2DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLTEXTURESTORAGE2DEXTPROC pfnTextureStorage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage2DEXT", pfnTextureStorage2DEXT, PFNGLTEXTURESTORAGE2DEXTPROC);
	pfnTextureStorage2DEXT(texture, target, levels, internalformat, width, height);
}

void glTextureStorage3DEXT(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	using PFNGLTEXTURESTORAGE3DEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	static PFNGLTEXTURESTORAGE3DEXTPROC pfnTextureStorage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage3DEXT", pfnTextureStorage3DEXT, PFNGLTEXTURESTORAGE3DEXTPROC);
	pfnTextureStorage3DEXT(texture, target, levels, internalformat, width, height, depth);
}

void glTextureStorage2DMultisampleEXT(GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	using PFNGLTEXTURESTORAGE2DMULTISAMPLEEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static PFNGLTEXTURESTORAGE2DMULTISAMPLEEXTPROC pfnTextureStorage2DMultisampleEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage2DMultisampleEXT", pfnTextureStorage2DMultisampleEXT, PFNGLTEXTURESTORAGE2DMULTISAMPLEEXTPROC);
	pfnTextureStorage2DMultisampleEXT(texture, target, samples, internalformat, width, height, fixedsamplelocations);
}

void glTextureStorage3DMultisampleEXT(GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	using PFNGLTEXTURESTORAGE3DMULTISAMPLEEXTPROC = void(APIENTRY *)(GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static PFNGLTEXTURESTORAGE3DMULTISAMPLEEXTPROC pfnTextureStorage3DMultisampleEXT{nullptr};
	LOAD_ENTRYPOINT("glTextureStorage3DMultisampleEXT", pfnTextureStorage3DMultisampleEXT, PFNGLTEXTURESTORAGE3DMULTISAMPLEEXTPROC);
	pfnTextureStorage3DMultisampleEXT(texture, target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

void glVertexArrayBindVertexBufferEXT(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
	using PFNGLVERTEXARRAYBINDVERTEXBUFFEREXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
	static PFNGLVERTEXARRAYBINDVERTEXBUFFEREXTPROC pfnVertexArrayBindVertexBufferEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayBindVertexBufferEXT", pfnVertexArrayBindVertexBufferEXT, PFNGLVERTEXARRAYBINDVERTEXBUFFEREXTPROC);
	pfnVertexArrayBindVertexBufferEXT(vaobj, bindingindex, buffer, offset, stride);
}

void glVertexArrayVertexAttribFormatEXT(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
	using PFNGLVERTEXARRAYVERTEXATTRIBFORMATEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
	static PFNGLVERTEXARRAYVERTEXATTRIBFORMATEXTPROC pfnVertexArrayVertexAttribFormatEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexAttribFormatEXT", pfnVertexArrayVertexAttribFormatEXT, PFNGLVERTEXARRAYVERTEXATTRIBFORMATEXTPROC);
	pfnVertexArrayVertexAttribFormatEXT(vaobj, attribindex, size, type, normalized, relativeoffset);
}

void glVertexArrayVertexAttribIFormatEXT(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
	using PFNGLVERTEXARRAYVERTEXATTRIBIFORMATEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static PFNGLVERTEXARRAYVERTEXATTRIBIFORMATEXTPROC pfnVertexArrayVertexAttribIFormatEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexAttribIFormatEXT", pfnVertexArrayVertexAttribIFormatEXT, PFNGLVERTEXARRAYVERTEXATTRIBIFORMATEXTPROC);
	pfnVertexArrayVertexAttribIFormatEXT(vaobj, attribindex, size, type, relativeoffset);
}

void glVertexArrayVertexAttribLFormatEXT(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
	using PFNGLVERTEXARRAYVERTEXATTRIBLFORMATEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
	static PFNGLVERTEXARRAYVERTEXATTRIBLFORMATEXTPROC pfnVertexArrayVertexAttribLFormatEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexAttribLFormatEXT", pfnVertexArrayVertexAttribLFormatEXT, PFNGLVERTEXARRAYVERTEXATTRIBLFORMATEXTPROC);
	pfnVertexArrayVertexAttribLFormatEXT(vaobj, attribindex, size, type, relativeoffset);
}

void glVertexArrayVertexAttribBindingEXT(GLuint vaobj, GLuint attribindex, GLuint bindingindex)
{
	using PFNGLVERTEXARRAYVERTEXATTRIBBINDINGEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
	static PFNGLVERTEXARRAYVERTEXATTRIBBINDINGEXTPROC pfnVertexArrayVertexAttribBindingEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexAttribBindingEXT", pfnVertexArrayVertexAttribBindingEXT, PFNGLVERTEXARRAYVERTEXATTRIBBINDINGEXTPROC);
	pfnVertexArrayVertexAttribBindingEXT(vaobj, attribindex, bindingindex);
}

void glVertexArrayVertexBindingDivisorEXT(GLuint vaobj, GLuint bindingindex, GLuint divisor)
{
	using PFNGLVERTEXARRAYVERTEXBINDINGDIVISOREXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint bindingindex, GLuint divisor);
	static PFNGLVERTEXARRAYVERTEXBINDINGDIVISOREXTPROC pfnVertexArrayVertexBindingDivisorEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexBindingDivisorEXT", pfnVertexArrayVertexBindingDivisorEXT, PFNGLVERTEXARRAYVERTEXBINDINGDIVISOREXTPROC);
	pfnVertexArrayVertexBindingDivisorEXT(vaobj, bindingindex, divisor);
}

void glVertexArrayVertexAttribLOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset)
{
	using PFNGLVERTEXARRAYVERTEXATTRIBLOFFSETEXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset);
	static PFNGLVERTEXARRAYVERTEXATTRIBLOFFSETEXTPROC pfnVertexArrayVertexAttribLOffsetEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexAttribLOffsetEXT", pfnVertexArrayVertexAttribLOffsetEXT, PFNGLVERTEXARRAYVERTEXATTRIBLOFFSETEXTPROC);
	pfnVertexArrayVertexAttribLOffsetEXT(vaobj, buffer, index, size, type, stride, offset);
}

void glTexturePageCommitmentEXT(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit)
{
	using PFNGLTEXTUREPAGECOMMITMENTEXTPROC = void(APIENTRY *)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
	static PFNGLTEXTUREPAGECOMMITMENTEXTPROC pfnTexturePageCommitmentEXT{nullptr};
	LOAD_ENTRYPOINT("glTexturePageCommitmentEXT", pfnTexturePageCommitmentEXT, PFNGLTEXTUREPAGECOMMITMENTEXTPROC);
	pfnTexturePageCommitmentEXT(texture, level, xoffset, yoffset, zoffset, width, height, depth, commit);
}

void glVertexArrayVertexAttribDivisorEXT(GLuint vaobj, GLuint index, GLuint divisor)
{
	using PFNGLVERTEXARRAYVERTEXATTRIBDIVISOREXTPROC = void(APIENTRY *)(GLuint vaobj, GLuint index, GLuint divisor);
	static PFNGLVERTEXARRAYVERTEXATTRIBDIVISOREXTPROC pfnVertexArrayVertexAttribDivisorEXT{nullptr};
	LOAD_ENTRYPOINT("glVertexArrayVertexAttribDivisorEXT", pfnVertexArrayVertexAttribDivisorEXT, PFNGLVERTEXARRAYVERTEXATTRIBDIVISOREXTPROC);
	pfnVertexArrayVertexAttribDivisorEXT(vaobj, index, divisor);
}

//
// GL_EXT_draw_instanced
//

void glDrawArraysInstancedEXT(GLenum mode, GLint start, GLsizei count, GLsizei primcount)
{
	using PFNGLDRAWARRAYSINSTANCEDEXTPROC = void(APIENTRY *)(GLenum mode, GLint start, GLsizei count, GLsizei primcount);
	static PFNGLDRAWARRAYSINSTANCEDEXTPROC pfnDrawArraysInstancedEXT{nullptr};
	LOAD_ENTRYPOINT("glDrawArraysInstancedEXT", pfnDrawArraysInstancedEXT, PFNGLDRAWARRAYSINSTANCEDEXTPROC);
	pfnDrawArraysInstancedEXT(mode, start, count, primcount);
}

void glDrawElementsInstancedEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount)
{
	using PFNGLDRAWELEMENTSINSTANCEDEXTPROC = void(APIENTRY *)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount);
	static PFNGLDRAWELEMENTSINSTANCEDEXTPROC pfnDrawElementsInstancedEXT{nullptr};
	LOAD_ENTRYPOINT("glDrawElementsInstancedEXT", pfnDrawElementsInstancedEXT, PFNGLDRAWELEMENTSINSTANCEDEXTPROC);
	pfnDrawElementsInstancedEXT(mode, count, type, indices, primcount);
}

//
// GL_EXT_polygon_offset_clamp
//

void glPolygonOffsetClampEXT(GLfloat factor, GLfloat units, GLfloat clamp)
{
	using PFNGLPOLYGONOFFSETCLAMPEXTPROC = void(APIENTRY *)(GLfloat factor, GLfloat units, GLfloat clamp);
	static PFNGLPOLYGONOFFSETCLAMPEXTPROC pfnPolygonOffsetClampEXT{nullptr};
	LOAD_ENTRYPOINT("glPolygonOffsetClampEXT", pfnPolygonOffsetClampEXT, PFNGLPOLYGONOFFSETCLAMPEXTPROC);
	pfnPolygonOffsetClampEXT(factor, units, clamp);
}

//
// GL_EXT_raster_multisample
//

void glRasterSamplesEXT(GLuint samples, GLboolean fixedsamplelocations)
{
	using PFNGLRASTERSAMPLESEXTPROC = void(APIENTRY *)(GLuint samples, GLboolean fixedsamplelocations);
	static PFNGLRASTERSAMPLESEXTPROC pfnRasterSamplesEXT{nullptr};
	LOAD_ENTRYPOINT("glRasterSamplesEXT", pfnRasterSamplesEXT, PFNGLRASTERSAMPLESEXTPROC);
	pfnRasterSamplesEXT(samples, fixedsamplelocations);
}

//
// GL_EXT_separate_shader_objects
//

void glUseShaderProgramEXT(GLenum type, GLuint program)
{
	using PFNGLUSESHADERPROGRAMEXTPROC = void(APIENTRY *)(GLenum type, GLuint program);
	static PFNGLUSESHADERPROGRAMEXTPROC pfnUseShaderProgramEXT{nullptr};
	LOAD_ENTRYPOINT("glUseShaderProgramEXT", pfnUseShaderProgramEXT, PFNGLUSESHADERPROGRAMEXTPROC);
	pfnUseShaderProgramEXT(type, program);
}

void glActiveProgramEXT(GLuint program)
{
	using PFNGLACTIVEPROGRAMEXTPROC = void(APIENTRY *)(GLuint program);
	static PFNGLACTIVEPROGRAMEXTPROC pfnActiveProgramEXT{nullptr};
	LOAD_ENTRYPOINT("glActiveProgramEXT", pfnActiveProgramEXT, PFNGLACTIVEPROGRAMEXTPROC);
	pfnActiveProgramEXT(program);
}

GLuint glCreateShaderProgramEXT(GLenum type, const GLchar* string)
{
	using PFNGLCREATESHADERPROGRAMEXTPROC = GLuint(APIENTRY *)(GLenum type, const GLchar* string);
	static PFNGLCREATESHADERPROGRAMEXTPROC pfnCreateShaderProgramEXT{nullptr};
	LOAD_ENTRYPOINT("glCreateShaderProgramEXT", pfnCreateShaderProgramEXT, PFNGLCREATESHADERPROGRAMEXTPROC);
	return pfnCreateShaderProgramEXT(type, string);
}

//
// GL_EXT_shader_framebuffer_fetch_non_coherent
//

void glFramebufferFetchBarrierEXT(void)
{
	using PFNGLFRAMEBUFFERFETCHBARRIEREXTPROC = void(APIENTRY *)(void);
	static PFNGLFRAMEBUFFERFETCHBARRIEREXTPROC pfnFramebufferFetchBarrierEXT{nullptr};
	LOAD_ENTRYPOINT("glFramebufferFetchBarrierEXT", pfnFramebufferFetchBarrierEXT, PFNGLFRAMEBUFFERFETCHBARRIEREXTPROC);
	pfnFramebufferFetchBarrierEXT();
}

//
// GL_EXT_texture_storage
//

void glTexStorage1DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
	using PFNGLTEXSTORAGE1DEXTPROC = void(APIENTRY *)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
	static PFNGLTEXSTORAGE1DEXTPROC pfnTexStorage1DEXT{nullptr};
	LOAD_ENTRYPOINT("glTexStorage1DEXT", pfnTexStorage1DEXT, PFNGLTEXSTORAGE1DEXTPROC);
	pfnTexStorage1DEXT(target, levels, internalformat, width);
}

void glTexStorage2DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLTEXSTORAGE2DEXTPROC = void(APIENTRY *)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLTEXSTORAGE2DEXTPROC pfnTexStorage2DEXT{nullptr};
	LOAD_ENTRYPOINT("glTexStorage2DEXT", pfnTexStorage2DEXT, PFNGLTEXSTORAGE2DEXTPROC);
	pfnTexStorage2DEXT(target, levels, internalformat, width, height);
}

void glTexStorage3DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	using PFNGLTEXSTORAGE3DEXTPROC = void(APIENTRY *)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	static PFNGLTEXSTORAGE3DEXTPROC pfnTexStorage3DEXT{nullptr};
	LOAD_ENTRYPOINT("glTexStorage3DEXT", pfnTexStorage3DEXT, PFNGLTEXSTORAGE3DEXTPROC);
	pfnTexStorage3DEXT(target, levels, internalformat, width, height, depth);
}

//
// GL_EXT_window_rectangles
//

void glWindowRectanglesEXT(GLenum mode, GLsizei count, const GLint* box)
{
	using PFNGLWINDOWRECTANGLESEXTPROC = void(APIENTRY *)(GLenum mode, GLsizei count, const GLint* box);
	static PFNGLWINDOWRECTANGLESEXTPROC pfnWindowRectanglesEXT{nullptr};
	LOAD_ENTRYPOINT("glWindowRectanglesEXT", pfnWindowRectanglesEXT, PFNGLWINDOWRECTANGLESEXTPROC);
	pfnWindowRectanglesEXT(mode, count, box);
}

//
// GL_INTEL_framebuffer_CMAA
//

void glApplyFramebufferAttachmentCMAAINTEL(void)
{
	using PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAINTELPROC = void(APIENTRY *)(void);
	static PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAINTELPROC pfnApplyFramebufferAttachmentCMAAINTEL{nullptr};
	LOAD_ENTRYPOINT("glApplyFramebufferAttachmentCMAAINTEL", pfnApplyFramebufferAttachmentCMAAINTEL, PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAINTELPROC);
	pfnApplyFramebufferAttachmentCMAAINTEL();
}

//
// GL_INTEL_performance_query
//

void glBeginPerfQueryINTEL(GLuint queryHandle)
{
	using PFNGLBEGINPERFQUERYINTELPROC = void(APIENTRY *)(GLuint queryHandle);
	static PFNGLBEGINPERFQUERYINTELPROC pfnBeginPerfQueryINTEL{nullptr};
	LOAD_ENTRYPOINT("glBeginPerfQueryINTEL", pfnBeginPerfQueryINTEL, PFNGLBEGINPERFQUERYINTELPROC);
	pfnBeginPerfQueryINTEL(queryHandle);
}

void glCreatePerfQueryINTEL(GLuint queryId, GLuint* queryHandle)
{
	using PFNGLCREATEPERFQUERYINTELPROC = void(APIENTRY *)(GLuint queryId, GLuint* queryHandle);
	static PFNGLCREATEPERFQUERYINTELPROC pfnCreatePerfQueryINTEL{nullptr};
	LOAD_ENTRYPOINT("glCreatePerfQueryINTEL", pfnCreatePerfQueryINTEL, PFNGLCREATEPERFQUERYINTELPROC);
	pfnCreatePerfQueryINTEL(queryId, queryHandle);
}

void glDeletePerfQueryINTEL(GLuint queryHandle)
{
	using PFNGLDELETEPERFQUERYINTELPROC = void(APIENTRY *)(GLuint queryHandle);
	static PFNGLDELETEPERFQUERYINTELPROC pfnDeletePerfQueryINTEL{nullptr};
	LOAD_ENTRYPOINT("glDeletePerfQueryINTEL", pfnDeletePerfQueryINTEL, PFNGLDELETEPERFQUERYINTELPROC);
	pfnDeletePerfQueryINTEL(queryHandle);
}

void glEndPerfQueryINTEL(GLuint queryHandle)
{
	using PFNGLENDPERFQUERYINTELPROC = void(APIENTRY *)(GLuint queryHandle);
	static PFNGLENDPERFQUERYINTELPROC pfnEndPerfQueryINTEL{nullptr};
	LOAD_ENTRYPOINT("glEndPerfQueryINTEL", pfnEndPerfQueryINTEL, PFNGLENDPERFQUERYINTELPROC);
	pfnEndPerfQueryINTEL(queryHandle);
}

void glGetFirstPerfQueryIdINTEL(GLuint* queryId)
{
	using PFNGLGETFIRSTPERFQUERYIDINTELPROC = void(APIENTRY *)(GLuint* queryId);
	static PFNGLGETFIRSTPERFQUERYIDINTELPROC pfnGetFirstPerfQueryIdINTEL{nullptr};
	LOAD_ENTRYPOINT("glGetFirstPerfQueryIdINTEL", pfnGetFirstPerfQueryIdINTEL, PFNGLGETFIRSTPERFQUERYIDINTELPROC);
	pfnGetFirstPerfQueryIdINTEL(queryId);
}

void glGetNextPerfQueryIdINTEL(GLuint queryId, GLuint* nextQueryId)
{
	using PFNGLGETNEXTPERFQUERYIDINTELPROC = void(APIENTRY *)(GLuint queryId, GLuint* nextQueryId);
	static PFNGLGETNEXTPERFQUERYIDINTELPROC pfnGetNextPerfQueryIdINTEL{nullptr};
	LOAD_ENTRYPOINT("glGetNextPerfQueryIdINTEL", pfnGetNextPerfQueryIdINTEL, PFNGLGETNEXTPERFQUERYIDINTELPROC);
	pfnGetNextPerfQueryIdINTEL(queryId, nextQueryId);
}

void glGetPerfCounterInfoINTEL(GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar* counterName, GLuint counterDescLength, GLchar* counterDesc, GLuint* counterOffset, GLuint* counterDataSize, GLuint* counterTypeEnum, GLuint* counterDataTypeEnum, GLuint64* rawCounterMaxValue)
{
	using PFNGLGETPERFCOUNTERINFOINTELPROC = void(APIENTRY *)(GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar* counterName, GLuint counterDescLength, GLchar* counterDesc, GLuint* counterOffset, GLuint* counterDataSize, GLuint* counterTypeEnum, GLuint* counterDataTypeEnum, GLuint64* rawCounterMaxValue);
	static PFNGLGETPERFCOUNTERINFOINTELPROC pfnGetPerfCounterInfoINTEL{nullptr};
	LOAD_ENTRYPOINT("glGetPerfCounterInfoINTEL", pfnGetPerfCounterInfoINTEL, PFNGLGETPERFCOUNTERINFOINTELPROC);
	pfnGetPerfCounterInfoINTEL(queryId, counterId, counterNameLength, counterName, counterDescLength, counterDesc, counterOffset, counterDataSize, counterTypeEnum, counterDataTypeEnum, rawCounterMaxValue);
}

void glGetPerfQueryDataINTEL(GLuint queryHandle, GLuint flags, GLsizei dataSize, void* data, GLuint* bytesWritten)
{
	using PFNGLGETPERFQUERYDATAINTELPROC = void(APIENTRY *)(GLuint queryHandle, GLuint flags, GLsizei dataSize, void* data, GLuint* bytesWritten);
	static PFNGLGETPERFQUERYDATAINTELPROC pfnGetPerfQueryDataINTEL{nullptr};
	LOAD_ENTRYPOINT("glGetPerfQueryDataINTEL", pfnGetPerfQueryDataINTEL, PFNGLGETPERFQUERYDATAINTELPROC);
	pfnGetPerfQueryDataINTEL(queryHandle, flags, dataSize, data, bytesWritten);
}

void glGetPerfQueryIdByNameINTEL(GLchar* queryName, GLuint* queryId)
{
	using PFNGLGETPERFQUERYIDBYNAMEINTELPROC = void(APIENTRY *)(GLchar* queryName, GLuint* queryId);
	static PFNGLGETPERFQUERYIDBYNAMEINTELPROC pfnGetPerfQueryIdByNameINTEL{nullptr};
	LOAD_ENTRYPOINT("glGetPerfQueryIdByNameINTEL", pfnGetPerfQueryIdByNameINTEL, PFNGLGETPERFQUERYIDBYNAMEINTELPROC);
	pfnGetPerfQueryIdByNameINTEL(queryName, queryId);
}

void glGetPerfQueryInfoINTEL(GLuint queryId, GLuint queryNameLength, GLchar* queryName, GLuint* dataSize, GLuint* noCounters, GLuint* noInstances, GLuint* capsMask)
{
	using PFNGLGETPERFQUERYINFOINTELPROC = void(APIENTRY *)(GLuint queryId, GLuint queryNameLength, GLchar* queryName, GLuint* dataSize, GLuint* noCounters, GLuint* noInstances, GLuint* capsMask);
	static PFNGLGETPERFQUERYINFOINTELPROC pfnGetPerfQueryInfoINTEL{nullptr};
	LOAD_ENTRYPOINT("glGetPerfQueryInfoINTEL", pfnGetPerfQueryInfoINTEL, PFNGLGETPERFQUERYINFOINTELPROC);
	pfnGetPerfQueryInfoINTEL(queryId, queryNameLength, queryName, dataSize, noCounters, noInstances, capsMask);
}

//
// GL_MESA_framebuffer_flip_y
//

void glFramebufferParameteriMESA(GLenum target, GLenum pname, GLint param)
{
	using PFNGLFRAMEBUFFERPARAMETERIMESAPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint param);
	static PFNGLFRAMEBUFFERPARAMETERIMESAPROC pfnFramebufferParameteriMESA{nullptr};
	LOAD_ENTRYPOINT("glFramebufferParameteriMESA", pfnFramebufferParameteriMESA, PFNGLFRAMEBUFFERPARAMETERIMESAPROC);
	pfnFramebufferParameteriMESA(target, pname, param);
}

void glGetFramebufferParameterivMESA(GLenum target, GLenum pname, GLint* params)
{
	using PFNGLGETFRAMEBUFFERPARAMETERIVMESAPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLint* params);
	static PFNGLGETFRAMEBUFFERPARAMETERIVMESAPROC pfnGetFramebufferParameterivMESA{nullptr};
	LOAD_ENTRYPOINT("glGetFramebufferParameterivMESA", pfnGetFramebufferParameterivMESA, PFNGLGETFRAMEBUFFERPARAMETERIVMESAPROC);
	pfnGetFramebufferParameterivMESA(target, pname, params);
}

//
// GL_NV_bindless_multi_draw_indirect
//

void glMultiDrawArraysIndirectBindlessNV(GLenum mode, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount)
{
	using PFNGLMULTIDRAWARRAYSINDIRECTBINDLESSNVPROC = void(APIENTRY *)(GLenum mode, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount);
	static PFNGLMULTIDRAWARRAYSINDIRECTBINDLESSNVPROC pfnMultiDrawArraysIndirectBindlessNV{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawArraysIndirectBindlessNV", pfnMultiDrawArraysIndirectBindlessNV, PFNGLMULTIDRAWARRAYSINDIRECTBINDLESSNVPROC);
	pfnMultiDrawArraysIndirectBindlessNV(mode, indirect, drawCount, stride, vertexBufferCount);
}

void glMultiDrawElementsIndirectBindlessNV(GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount)
{
	using PFNGLMULTIDRAWELEMENTSINDIRECTBINDLESSNVPROC = void(APIENTRY *)(GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount);
	static PFNGLMULTIDRAWELEMENTSINDIRECTBINDLESSNVPROC pfnMultiDrawElementsIndirectBindlessNV{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawElementsIndirectBindlessNV", pfnMultiDrawElementsIndirectBindlessNV, PFNGLMULTIDRAWELEMENTSINDIRECTBINDLESSNVPROC);
	pfnMultiDrawElementsIndirectBindlessNV(mode, type, indirect, drawCount, stride, vertexBufferCount);
}

//
// GL_NV_bindless_multi_draw_indirect_count
//

void glMultiDrawArraysIndirectBindlessCountNV(GLenum mode, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount)
{
	using PFNGLMULTIDRAWARRAYSINDIRECTBINDLESSCOUNTNVPROC = void(APIENTRY *)(GLenum mode, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount);
	static PFNGLMULTIDRAWARRAYSINDIRECTBINDLESSCOUNTNVPROC pfnMultiDrawArraysIndirectBindlessCountNV{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawArraysIndirectBindlessCountNV", pfnMultiDrawArraysIndirectBindlessCountNV, PFNGLMULTIDRAWARRAYSINDIRECTBINDLESSCOUNTNVPROC);
	pfnMultiDrawArraysIndirectBindlessCountNV(mode, indirect, drawCount, maxDrawCount, stride, vertexBufferCount);
}

void glMultiDrawElementsIndirectBindlessCountNV(GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount)
{
	using PFNGLMULTIDRAWELEMENTSINDIRECTBINDLESSCOUNTNVPROC = void(APIENTRY *)(GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount);
	static PFNGLMULTIDRAWELEMENTSINDIRECTBINDLESSCOUNTNVPROC pfnMultiDrawElementsIndirectBindlessCountNV{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawElementsIndirectBindlessCountNV", pfnMultiDrawElementsIndirectBindlessCountNV, PFNGLMULTIDRAWELEMENTSINDIRECTBINDLESSCOUNTNVPROC);
	pfnMultiDrawElementsIndirectBindlessCountNV(mode, type, indirect, drawCount, maxDrawCount, stride, vertexBufferCount);
}

//
// GL_NV_bindless_texture
//

GLuint64 glGetTextureHandleNV(GLuint texture)
{
	using PFNGLGETTEXTUREHANDLENVPROC = GLuint64(APIENTRY *)(GLuint texture);
	static PFNGLGETTEXTUREHANDLENVPROC pfnGetTextureHandleNV{nullptr};
	LOAD_ENTRYPOINT("glGetTextureHandleNV", pfnGetTextureHandleNV, PFNGLGETTEXTUREHANDLENVPROC);
	return pfnGetTextureHandleNV(texture);
}

GLuint64 glGetTextureSamplerHandleNV(GLuint texture, GLuint sampler)
{
	using PFNGLGETTEXTURESAMPLERHANDLENVPROC = GLuint64(APIENTRY *)(GLuint texture, GLuint sampler);
	static PFNGLGETTEXTURESAMPLERHANDLENVPROC pfnGetTextureSamplerHandleNV{nullptr};
	LOAD_ENTRYPOINT("glGetTextureSamplerHandleNV", pfnGetTextureSamplerHandleNV, PFNGLGETTEXTURESAMPLERHANDLENVPROC);
	return pfnGetTextureSamplerHandleNV(texture, sampler);
}

void glMakeTextureHandleResidentNV(GLuint64 handle)
{
	using PFNGLMAKETEXTUREHANDLERESIDENTNVPROC = void(APIENTRY *)(GLuint64 handle);
	static PFNGLMAKETEXTUREHANDLERESIDENTNVPROC pfnMakeTextureHandleResidentNV{nullptr};
	LOAD_ENTRYPOINT("glMakeTextureHandleResidentNV", pfnMakeTextureHandleResidentNV, PFNGLMAKETEXTUREHANDLERESIDENTNVPROC);
	pfnMakeTextureHandleResidentNV(handle);
}

void glMakeTextureHandleNonResidentNV(GLuint64 handle)
{
	using PFNGLMAKETEXTUREHANDLENONRESIDENTNVPROC = void(APIENTRY *)(GLuint64 handle);
	static PFNGLMAKETEXTUREHANDLENONRESIDENTNVPROC pfnMakeTextureHandleNonResidentNV{nullptr};
	LOAD_ENTRYPOINT("glMakeTextureHandleNonResidentNV", pfnMakeTextureHandleNonResidentNV, PFNGLMAKETEXTUREHANDLENONRESIDENTNVPROC);
	pfnMakeTextureHandleNonResidentNV(handle);
}

GLuint64 glGetImageHandleNV(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format)
{
	using PFNGLGETIMAGEHANDLENVPROC = GLuint64(APIENTRY *)(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format);
	static PFNGLGETIMAGEHANDLENVPROC pfnGetImageHandleNV{nullptr};
	LOAD_ENTRYPOINT("glGetImageHandleNV", pfnGetImageHandleNV, PFNGLGETIMAGEHANDLENVPROC);
	return pfnGetImageHandleNV(texture, level, layered, layer, format);
}

void glMakeImageHandleResidentNV(GLuint64 handle, GLenum access)
{
	using PFNGLMAKEIMAGEHANDLERESIDENTNVPROC = void(APIENTRY *)(GLuint64 handle, GLenum access);
	static PFNGLMAKEIMAGEHANDLERESIDENTNVPROC pfnMakeImageHandleResidentNV{nullptr};
	LOAD_ENTRYPOINT("glMakeImageHandleResidentNV", pfnMakeImageHandleResidentNV, PFNGLMAKEIMAGEHANDLERESIDENTNVPROC);
	pfnMakeImageHandleResidentNV(handle, access);
}

void glMakeImageHandleNonResidentNV(GLuint64 handle)
{
	using PFNGLMAKEIMAGEHANDLENONRESIDENTNVPROC = void(APIENTRY *)(GLuint64 handle);
	static PFNGLMAKEIMAGEHANDLENONRESIDENTNVPROC pfnMakeImageHandleNonResidentNV{nullptr};
	LOAD_ENTRYPOINT("glMakeImageHandleNonResidentNV", pfnMakeImageHandleNonResidentNV, PFNGLMAKEIMAGEHANDLENONRESIDENTNVPROC);
	pfnMakeImageHandleNonResidentNV(handle);
}

void glUniformHandleui64NV(GLint location, GLuint64 value)
{
	using PFNGLUNIFORMHANDLEUI64NVPROC = void(APIENTRY *)(GLint location, GLuint64 value);
	static PFNGLUNIFORMHANDLEUI64NVPROC pfnUniformHandleui64NV{nullptr};
	LOAD_ENTRYPOINT("glUniformHandleui64NV", pfnUniformHandleui64NV, PFNGLUNIFORMHANDLEUI64NVPROC);
	pfnUniformHandleui64NV(location, value);
}

void glUniformHandleui64vNV(GLint location, GLsizei count, const GLuint64* value)
{
	using PFNGLUNIFORMHANDLEUI64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64* value);
	static PFNGLUNIFORMHANDLEUI64VNVPROC pfnUniformHandleui64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniformHandleui64vNV", pfnUniformHandleui64vNV, PFNGLUNIFORMHANDLEUI64VNVPROC);
	pfnUniformHandleui64vNV(location, count, value);
}

void glProgramUniformHandleui64NV(GLuint program, GLint location, GLuint64 value)
{
	using PFNGLPROGRAMUNIFORMHANDLEUI64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64 value);
	static PFNGLPROGRAMUNIFORMHANDLEUI64NVPROC pfnProgramUniformHandleui64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformHandleui64NV", pfnProgramUniformHandleui64NV, PFNGLPROGRAMUNIFORMHANDLEUI64NVPROC);
	pfnProgramUniformHandleui64NV(program, location, value);
}

void glProgramUniformHandleui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64* values)
{
	using PFNGLPROGRAMUNIFORMHANDLEUI64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64* values);
	static PFNGLPROGRAMUNIFORMHANDLEUI64VNVPROC pfnProgramUniformHandleui64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformHandleui64vNV", pfnProgramUniformHandleui64vNV, PFNGLPROGRAMUNIFORMHANDLEUI64VNVPROC);
	pfnProgramUniformHandleui64vNV(program, location, count, values);
}

GLboolean glIsTextureHandleResidentNV(GLuint64 handle)
{
	using PFNGLISTEXTUREHANDLERESIDENTNVPROC = GLboolean(APIENTRY *)(GLuint64 handle);
	static PFNGLISTEXTUREHANDLERESIDENTNVPROC pfnIsTextureHandleResidentNV{nullptr};
	LOAD_ENTRYPOINT("glIsTextureHandleResidentNV", pfnIsTextureHandleResidentNV, PFNGLISTEXTUREHANDLERESIDENTNVPROC);
	return pfnIsTextureHandleResidentNV(handle);
}

GLboolean glIsImageHandleResidentNV(GLuint64 handle)
{
	using PFNGLISIMAGEHANDLERESIDENTNVPROC = GLboolean(APIENTRY *)(GLuint64 handle);
	static PFNGLISIMAGEHANDLERESIDENTNVPROC pfnIsImageHandleResidentNV{nullptr};
	LOAD_ENTRYPOINT("glIsImageHandleResidentNV", pfnIsImageHandleResidentNV, PFNGLISIMAGEHANDLERESIDENTNVPROC);
	return pfnIsImageHandleResidentNV(handle);
}

//
// GL_NV_blend_equation_advanced
//

void glBlendParameteriNV(GLenum pname, GLint value)
{
	using PFNGLBLENDPARAMETERINVPROC = void(APIENTRY *)(GLenum pname, GLint value);
	static PFNGLBLENDPARAMETERINVPROC pfnBlendParameteriNV{nullptr};
	LOAD_ENTRYPOINT("glBlendParameteriNV", pfnBlendParameteriNV, PFNGLBLENDPARAMETERINVPROC);
	pfnBlendParameteriNV(pname, value);
}

void glBlendBarrierNV(void)
{
	using PFNGLBLENDBARRIERNVPROC = void(APIENTRY *)(void);
	static PFNGLBLENDBARRIERNVPROC pfnBlendBarrierNV{nullptr};
	LOAD_ENTRYPOINT("glBlendBarrierNV", pfnBlendBarrierNV, PFNGLBLENDBARRIERNVPROC);
	pfnBlendBarrierNV();
}

//
// GL_NV_clip_space_w_scaling
//

void glViewportPositionWScaleNV(GLuint index, GLfloat xcoeff, GLfloat ycoeff)
{
	using PFNGLVIEWPORTPOSITIONWSCALENVPROC = void(APIENTRY *)(GLuint index, GLfloat xcoeff, GLfloat ycoeff);
	static PFNGLVIEWPORTPOSITIONWSCALENVPROC pfnViewportPositionWScaleNV{nullptr};
	LOAD_ENTRYPOINT("glViewportPositionWScaleNV", pfnViewportPositionWScaleNV, PFNGLVIEWPORTPOSITIONWSCALENVPROC);
	pfnViewportPositionWScaleNV(index, xcoeff, ycoeff);
}

//
// GL_NV_command_list
//

void glCreateStatesNV(GLsizei n, GLuint* states)
{
	using PFNGLCREATESTATESNVPROC = void(APIENTRY *)(GLsizei n, GLuint* states);
	static PFNGLCREATESTATESNVPROC pfnCreateStatesNV{nullptr};
	LOAD_ENTRYPOINT("glCreateStatesNV", pfnCreateStatesNV, PFNGLCREATESTATESNVPROC);
	pfnCreateStatesNV(n, states);
}

void glDeleteStatesNV(GLsizei n, const GLuint* states)
{
	using PFNGLDELETESTATESNVPROC = void(APIENTRY *)(GLsizei n, const GLuint* states);
	static PFNGLDELETESTATESNVPROC pfnDeleteStatesNV{nullptr};
	LOAD_ENTRYPOINT("glDeleteStatesNV", pfnDeleteStatesNV, PFNGLDELETESTATESNVPROC);
	pfnDeleteStatesNV(n, states);
}

GLboolean glIsStateNV(GLuint state)
{
	using PFNGLISSTATENVPROC = GLboolean(APIENTRY *)(GLuint state);
	static PFNGLISSTATENVPROC pfnIsStateNV{nullptr};
	LOAD_ENTRYPOINT("glIsStateNV", pfnIsStateNV, PFNGLISSTATENVPROC);
	return pfnIsStateNV(state);
}

void glStateCaptureNV(GLuint state, GLenum mode)
{
	using PFNGLSTATECAPTURENVPROC = void(APIENTRY *)(GLuint state, GLenum mode);
	static PFNGLSTATECAPTURENVPROC pfnStateCaptureNV{nullptr};
	LOAD_ENTRYPOINT("glStateCaptureNV", pfnStateCaptureNV, PFNGLSTATECAPTURENVPROC);
	pfnStateCaptureNV(state, mode);
}

GLuint glGetCommandHeaderNV(GLenum tokenID, GLuint size)
{
	using PFNGLGETCOMMANDHEADERNVPROC = GLuint(APIENTRY *)(GLenum tokenID, GLuint size);
	static PFNGLGETCOMMANDHEADERNVPROC pfnGetCommandHeaderNV{nullptr};
	LOAD_ENTRYPOINT("glGetCommandHeaderNV", pfnGetCommandHeaderNV, PFNGLGETCOMMANDHEADERNVPROC);
	return pfnGetCommandHeaderNV(tokenID, size);
}

GLushort glGetStageIndexNV(GLenum shadertype)
{
	using PFNGLGETSTAGEINDEXNVPROC = GLushort(APIENTRY *)(GLenum shadertype);
	static PFNGLGETSTAGEINDEXNVPROC pfnGetStageIndexNV{nullptr};
	LOAD_ENTRYPOINT("glGetStageIndexNV", pfnGetStageIndexNV, PFNGLGETSTAGEINDEXNVPROC);
	return pfnGetStageIndexNV(shadertype);
}

void glDrawCommandsNV(GLenum primitiveMode, GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, GLuint count)
{
	using PFNGLDRAWCOMMANDSNVPROC = void(APIENTRY *)(GLenum primitiveMode, GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, GLuint count);
	static PFNGLDRAWCOMMANDSNVPROC pfnDrawCommandsNV{nullptr};
	LOAD_ENTRYPOINT("glDrawCommandsNV", pfnDrawCommandsNV, PFNGLDRAWCOMMANDSNVPROC);
	pfnDrawCommandsNV(primitiveMode, buffer, indirects, sizes, count);
}

void glDrawCommandsAddressNV(GLenum primitiveMode, const GLuint64* indirects, const GLsizei* sizes, GLuint count)
{
	using PFNGLDRAWCOMMANDSADDRESSNVPROC = void(APIENTRY *)(GLenum primitiveMode, const GLuint64* indirects, const GLsizei* sizes, GLuint count);
	static PFNGLDRAWCOMMANDSADDRESSNVPROC pfnDrawCommandsAddressNV{nullptr};
	LOAD_ENTRYPOINT("glDrawCommandsAddressNV", pfnDrawCommandsAddressNV, PFNGLDRAWCOMMANDSADDRESSNVPROC);
	pfnDrawCommandsAddressNV(primitiveMode, indirects, sizes, count);
}

void glDrawCommandsStatesNV(GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count)
{
	using PFNGLDRAWCOMMANDSSTATESNVPROC = void(APIENTRY *)(GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count);
	static PFNGLDRAWCOMMANDSSTATESNVPROC pfnDrawCommandsStatesNV{nullptr};
	LOAD_ENTRYPOINT("glDrawCommandsStatesNV", pfnDrawCommandsStatesNV, PFNGLDRAWCOMMANDSSTATESNVPROC);
	pfnDrawCommandsStatesNV(buffer, indirects, sizes, states, fbos, count);
}

void glDrawCommandsStatesAddressNV(const GLuint64* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count)
{
	using PFNGLDRAWCOMMANDSSTATESADDRESSNVPROC = void(APIENTRY *)(const GLuint64* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count);
	static PFNGLDRAWCOMMANDSSTATESADDRESSNVPROC pfnDrawCommandsStatesAddressNV{nullptr};
	LOAD_ENTRYPOINT("glDrawCommandsStatesAddressNV", pfnDrawCommandsStatesAddressNV, PFNGLDRAWCOMMANDSSTATESADDRESSNVPROC);
	pfnDrawCommandsStatesAddressNV(indirects, sizes, states, fbos, count);
}

void glCreateCommandListsNV(GLsizei n, GLuint* lists)
{
	using PFNGLCREATECOMMANDLISTSNVPROC = void(APIENTRY *)(GLsizei n, GLuint* lists);
	static PFNGLCREATECOMMANDLISTSNVPROC pfnCreateCommandListsNV{nullptr};
	LOAD_ENTRYPOINT("glCreateCommandListsNV", pfnCreateCommandListsNV, PFNGLCREATECOMMANDLISTSNVPROC);
	pfnCreateCommandListsNV(n, lists);
}

void glDeleteCommandListsNV(GLsizei n, const GLuint* lists)
{
	using PFNGLDELETECOMMANDLISTSNVPROC = void(APIENTRY *)(GLsizei n, const GLuint* lists);
	static PFNGLDELETECOMMANDLISTSNVPROC pfnDeleteCommandListsNV{nullptr};
	LOAD_ENTRYPOINT("glDeleteCommandListsNV", pfnDeleteCommandListsNV, PFNGLDELETECOMMANDLISTSNVPROC);
	pfnDeleteCommandListsNV(n, lists);
}

GLboolean glIsCommandListNV(GLuint list)
{
	using PFNGLISCOMMANDLISTNVPROC = GLboolean(APIENTRY *)(GLuint list);
	static PFNGLISCOMMANDLISTNVPROC pfnIsCommandListNV{nullptr};
	LOAD_ENTRYPOINT("glIsCommandListNV", pfnIsCommandListNV, PFNGLISCOMMANDLISTNVPROC);
	return pfnIsCommandListNV(list);
}

void glListDrawCommandsStatesClientNV(GLuint list, GLuint segment, const void** indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count)
{
	using PFNGLLISTDRAWCOMMANDSSTATESCLIENTNVPROC = void(APIENTRY *)(GLuint list, GLuint segment, const void** indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count);
	static PFNGLLISTDRAWCOMMANDSSTATESCLIENTNVPROC pfnListDrawCommandsStatesClientNV{nullptr};
	LOAD_ENTRYPOINT("glListDrawCommandsStatesClientNV", pfnListDrawCommandsStatesClientNV, PFNGLLISTDRAWCOMMANDSSTATESCLIENTNVPROC);
	pfnListDrawCommandsStatesClientNV(list, segment, indirects, sizes, states, fbos, count);
}

void glCommandListSegmentsNV(GLuint list, GLuint segments)
{
	using PFNGLCOMMANDLISTSEGMENTSNVPROC = void(APIENTRY *)(GLuint list, GLuint segments);
	static PFNGLCOMMANDLISTSEGMENTSNVPROC pfnCommandListSegmentsNV{nullptr};
	LOAD_ENTRYPOINT("glCommandListSegmentsNV", pfnCommandListSegmentsNV, PFNGLCOMMANDLISTSEGMENTSNVPROC);
	pfnCommandListSegmentsNV(list, segments);
}

void glCompileCommandListNV(GLuint list)
{
	using PFNGLCOMPILECOMMANDLISTNVPROC = void(APIENTRY *)(GLuint list);
	static PFNGLCOMPILECOMMANDLISTNVPROC pfnCompileCommandListNV{nullptr};
	LOAD_ENTRYPOINT("glCompileCommandListNV", pfnCompileCommandListNV, PFNGLCOMPILECOMMANDLISTNVPROC);
	pfnCompileCommandListNV(list);
}

void glCallCommandListNV(GLuint list)
{
	using PFNGLCALLCOMMANDLISTNVPROC = void(APIENTRY *)(GLuint list);
	static PFNGLCALLCOMMANDLISTNVPROC pfnCallCommandListNV{nullptr};
	LOAD_ENTRYPOINT("glCallCommandListNV", pfnCallCommandListNV, PFNGLCALLCOMMANDLISTNVPROC);
	pfnCallCommandListNV(list);
}

//
// GL_NV_conditional_render
//

void glBeginConditionalRenderNV(GLuint id, GLenum mode)
{
	using PFNGLBEGINCONDITIONALRENDERNVPROC = void(APIENTRY *)(GLuint id, GLenum mode);
	static PFNGLBEGINCONDITIONALRENDERNVPROC pfnBeginConditionalRenderNV{nullptr};
	LOAD_ENTRYPOINT("glBeginConditionalRenderNV", pfnBeginConditionalRenderNV, PFNGLBEGINCONDITIONALRENDERNVPROC);
	pfnBeginConditionalRenderNV(id, mode);
}

void glEndConditionalRenderNV(void)
{
	using PFNGLENDCONDITIONALRENDERNVPROC = void(APIENTRY *)(void);
	static PFNGLENDCONDITIONALRENDERNVPROC pfnEndConditionalRenderNV{nullptr};
	LOAD_ENTRYPOINT("glEndConditionalRenderNV", pfnEndConditionalRenderNV, PFNGLENDCONDITIONALRENDERNVPROC);
	pfnEndConditionalRenderNV();
}

//
// GL_NV_conservative_raster
//

void glSubpixelPrecisionBiasNV(GLuint xbits, GLuint ybits)
{
	using PFNGLSUBPIXELPRECISIONBIASNVPROC = void(APIENTRY *)(GLuint xbits, GLuint ybits);
	static PFNGLSUBPIXELPRECISIONBIASNVPROC pfnSubpixelPrecisionBiasNV{nullptr};
	LOAD_ENTRYPOINT("glSubpixelPrecisionBiasNV", pfnSubpixelPrecisionBiasNV, PFNGLSUBPIXELPRECISIONBIASNVPROC);
	pfnSubpixelPrecisionBiasNV(xbits, ybits);
}

//
// GL_NV_conservative_raster_dilate
//

void glConservativeRasterParameterfNV(GLenum pname, GLfloat value)
{
	using PFNGLCONSERVATIVERASTERPARAMETERFNVPROC = void(APIENTRY *)(GLenum pname, GLfloat value);
	static PFNGLCONSERVATIVERASTERPARAMETERFNVPROC pfnConservativeRasterParameterfNV{nullptr};
	LOAD_ENTRYPOINT("glConservativeRasterParameterfNV", pfnConservativeRasterParameterfNV, PFNGLCONSERVATIVERASTERPARAMETERFNVPROC);
	pfnConservativeRasterParameterfNV(pname, value);
}

//
// GL_NV_conservative_raster_pre_snap_triangles
//

void glConservativeRasterParameteriNV(GLenum pname, GLint param)
{
	using PFNGLCONSERVATIVERASTERPARAMETERINVPROC = void(APIENTRY *)(GLenum pname, GLint param);
	static PFNGLCONSERVATIVERASTERPARAMETERINVPROC pfnConservativeRasterParameteriNV{nullptr};
	LOAD_ENTRYPOINT("glConservativeRasterParameteriNV", pfnConservativeRasterParameteriNV, PFNGLCONSERVATIVERASTERPARAMETERINVPROC);
	pfnConservativeRasterParameteriNV(pname, param);
}

//
// GL_NV_depth_buffer_float
//

void glDepthRangedNV(GLdouble zNear, GLdouble zFar)
{
	using PFNGLDEPTHRANGEDNVPROC = void(APIENTRY *)(GLdouble zNear, GLdouble zFar);
	static PFNGLDEPTHRANGEDNVPROC pfnDepthRangedNV{nullptr};
	LOAD_ENTRYPOINT("glDepthRangedNV", pfnDepthRangedNV, PFNGLDEPTHRANGEDNVPROC);
	pfnDepthRangedNV(zNear, zFar);
}

void glClearDepthdNV(GLdouble depth)
{
	using PFNGLCLEARDEPTHDNVPROC = void(APIENTRY *)(GLdouble depth);
	static PFNGLCLEARDEPTHDNVPROC pfnClearDepthdNV{nullptr};
	LOAD_ENTRYPOINT("glClearDepthdNV", pfnClearDepthdNV, PFNGLCLEARDEPTHDNVPROC);
	pfnClearDepthdNV(depth);
}

void glDepthBoundsdNV(GLdouble zmin, GLdouble zmax)
{
	using PFNGLDEPTHBOUNDSDNVPROC = void(APIENTRY *)(GLdouble zmin, GLdouble zmax);
	static PFNGLDEPTHBOUNDSDNVPROC pfnDepthBoundsdNV{nullptr};
	LOAD_ENTRYPOINT("glDepthBoundsdNV", pfnDepthBoundsdNV, PFNGLDEPTHBOUNDSDNVPROC);
	pfnDepthBoundsdNV(zmin, zmax);
}

//
// GL_NV_draw_vulkan_image
//

void glDrawVkImageNV(GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1)
{
	using PFNGLDRAWVKIMAGENVPROC = void(APIENTRY *)(GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1);
	static PFNGLDRAWVKIMAGENVPROC pfnDrawVkImageNV{nullptr};
	LOAD_ENTRYPOINT("glDrawVkImageNV", pfnDrawVkImageNV, PFNGLDRAWVKIMAGENVPROC);
	pfnDrawVkImageNV(vkImage, sampler, x0, y0, x1, y1, z, s0, t0, s1, t1);
}

GLVULKANPROCNV glGetVkProcAddrNV(const GLchar* name)
{
	using PFNGLGETVKPROCADDRNVPROC = GLVULKANPROCNV(APIENTRY *)(const GLchar* name);
	static PFNGLGETVKPROCADDRNVPROC pfnGetVkProcAddrNV{nullptr};
	LOAD_ENTRYPOINT("glGetVkProcAddrNV", pfnGetVkProcAddrNV, PFNGLGETVKPROCADDRNVPROC);
	return pfnGetVkProcAddrNV(name);
}

void glWaitVkSemaphoreNV(GLuint64 vkSemaphore)
{
	using PFNGLWAITVKSEMAPHORENVPROC = void(APIENTRY *)(GLuint64 vkSemaphore);
	static PFNGLWAITVKSEMAPHORENVPROC pfnWaitVkSemaphoreNV{nullptr};
	LOAD_ENTRYPOINT("glWaitVkSemaphoreNV", pfnWaitVkSemaphoreNV, PFNGLWAITVKSEMAPHORENVPROC);
	pfnWaitVkSemaphoreNV(vkSemaphore);
}

void glSignalVkSemaphoreNV(GLuint64 vkSemaphore)
{
	using PFNGLSIGNALVKSEMAPHORENVPROC = void(APIENTRY *)(GLuint64 vkSemaphore);
	static PFNGLSIGNALVKSEMAPHORENVPROC pfnSignalVkSemaphoreNV{nullptr};
	LOAD_ENTRYPOINT("glSignalVkSemaphoreNV", pfnSignalVkSemaphoreNV, PFNGLSIGNALVKSEMAPHORENVPROC);
	pfnSignalVkSemaphoreNV(vkSemaphore);
}

void glSignalVkFenceNV(GLuint64 vkFence)
{
	using PFNGLSIGNALVKFENCENVPROC = void(APIENTRY *)(GLuint64 vkFence);
	static PFNGLSIGNALVKFENCENVPROC pfnSignalVkFenceNV{nullptr};
	LOAD_ENTRYPOINT("glSignalVkFenceNV", pfnSignalVkFenceNV, PFNGLSIGNALVKFENCENVPROC);
	pfnSignalVkFenceNV(vkFence);
}

//
// GL_NV_fragment_coverage_to_color
//

void glFragmentCoverageColorNV(GLuint color)
{
	using PFNGLFRAGMENTCOVERAGECOLORNVPROC = void(APIENTRY *)(GLuint color);
	static PFNGLFRAGMENTCOVERAGECOLORNVPROC pfnFragmentCoverageColorNV{nullptr};
	LOAD_ENTRYPOINT("glFragmentCoverageColorNV", pfnFragmentCoverageColorNV, PFNGLFRAGMENTCOVERAGECOLORNVPROC);
	pfnFragmentCoverageColorNV(color);
}

//
// GL_NV_framebuffer_mixed_samples
//

void glCoverageModulationTableNV(GLsizei n, const GLfloat* v)
{
	using PFNGLCOVERAGEMODULATIONTABLENVPROC = void(APIENTRY *)(GLsizei n, const GLfloat* v);
	static PFNGLCOVERAGEMODULATIONTABLENVPROC pfnCoverageModulationTableNV{nullptr};
	LOAD_ENTRYPOINT("glCoverageModulationTableNV", pfnCoverageModulationTableNV, PFNGLCOVERAGEMODULATIONTABLENVPROC);
	pfnCoverageModulationTableNV(n, v);
}

void glGetCoverageModulationTableNV(GLsizei bufSize, GLfloat* v)
{
	using PFNGLGETCOVERAGEMODULATIONTABLENVPROC = void(APIENTRY *)(GLsizei bufSize, GLfloat* v);
	static PFNGLGETCOVERAGEMODULATIONTABLENVPROC pfnGetCoverageModulationTableNV{nullptr};
	LOAD_ENTRYPOINT("glGetCoverageModulationTableNV", pfnGetCoverageModulationTableNV, PFNGLGETCOVERAGEMODULATIONTABLENVPROC);
	pfnGetCoverageModulationTableNV(bufSize, v);
}

void glCoverageModulationNV(GLenum components)
{
	using PFNGLCOVERAGEMODULATIONNVPROC = void(APIENTRY *)(GLenum components);
	static PFNGLCOVERAGEMODULATIONNVPROC pfnCoverageModulationNV{nullptr};
	LOAD_ENTRYPOINT("glCoverageModulationNV", pfnCoverageModulationNV, PFNGLCOVERAGEMODULATIONNVPROC);
	pfnCoverageModulationNV(components);
}

//
// GL_NV_framebuffer_multisample_coverage
//

void glRenderbufferStorageMultisampleCoverageNV(GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height)
{
	using PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC = void(APIENTRY *)(GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC pfnRenderbufferStorageMultisampleCoverageNV{nullptr};
	LOAD_ENTRYPOINT("glRenderbufferStorageMultisampleCoverageNV", pfnRenderbufferStorageMultisampleCoverageNV, PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC);
	pfnRenderbufferStorageMultisampleCoverageNV(target, coverageSamples, colorSamples, internalformat, width, height);
}

//
// GL_NV_gpu_shader5
//

void glUniform1i64NV(GLint location, GLint64EXT x)
{
	using PFNGLUNIFORM1I64NVPROC = void(APIENTRY *)(GLint location, GLint64EXT x);
	static PFNGLUNIFORM1I64NVPROC pfnUniform1i64NV{nullptr};
	LOAD_ENTRYPOINT("glUniform1i64NV", pfnUniform1i64NV, PFNGLUNIFORM1I64NVPROC);
	pfnUniform1i64NV(location, x);
}

void glUniform2i64NV(GLint location, GLint64EXT x, GLint64EXT y)
{
	using PFNGLUNIFORM2I64NVPROC = void(APIENTRY *)(GLint location, GLint64EXT x, GLint64EXT y);
	static PFNGLUNIFORM2I64NVPROC pfnUniform2i64NV{nullptr};
	LOAD_ENTRYPOINT("glUniform2i64NV", pfnUniform2i64NV, PFNGLUNIFORM2I64NVPROC);
	pfnUniform2i64NV(location, x, y);
}

void glUniform3i64NV(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)
{
	using PFNGLUNIFORM3I64NVPROC = void(APIENTRY *)(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z);
	static PFNGLUNIFORM3I64NVPROC pfnUniform3i64NV{nullptr};
	LOAD_ENTRYPOINT("glUniform3i64NV", pfnUniform3i64NV, PFNGLUNIFORM3I64NVPROC);
	pfnUniform3i64NV(location, x, y, z);
}

void glUniform4i64NV(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)
{
	using PFNGLUNIFORM4I64NVPROC = void(APIENTRY *)(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w);
	static PFNGLUNIFORM4I64NVPROC pfnUniform4i64NV{nullptr};
	LOAD_ENTRYPOINT("glUniform4i64NV", pfnUniform4i64NV, PFNGLUNIFORM4I64NVPROC);
	pfnUniform4i64NV(location, x, y, z, w);
}

void glUniform1i64vNV(GLint location, GLsizei count, const GLint64EXT* value)
{
	using PFNGLUNIFORM1I64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint64EXT* value);
	static PFNGLUNIFORM1I64VNVPROC pfnUniform1i64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniform1i64vNV", pfnUniform1i64vNV, PFNGLUNIFORM1I64VNVPROC);
	pfnUniform1i64vNV(location, count, value);
}

void glUniform2i64vNV(GLint location, GLsizei count, const GLint64EXT* value)
{
	using PFNGLUNIFORM2I64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint64EXT* value);
	static PFNGLUNIFORM2I64VNVPROC pfnUniform2i64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniform2i64vNV", pfnUniform2i64vNV, PFNGLUNIFORM2I64VNVPROC);
	pfnUniform2i64vNV(location, count, value);
}

void glUniform3i64vNV(GLint location, GLsizei count, const GLint64EXT* value)
{
	using PFNGLUNIFORM3I64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint64EXT* value);
	static PFNGLUNIFORM3I64VNVPROC pfnUniform3i64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniform3i64vNV", pfnUniform3i64vNV, PFNGLUNIFORM3I64VNVPROC);
	pfnUniform3i64vNV(location, count, value);
}

void glUniform4i64vNV(GLint location, GLsizei count, const GLint64EXT* value)
{
	using PFNGLUNIFORM4I64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLint64EXT* value);
	static PFNGLUNIFORM4I64VNVPROC pfnUniform4i64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniform4i64vNV", pfnUniform4i64vNV, PFNGLUNIFORM4I64VNVPROC);
	pfnUniform4i64vNV(location, count, value);
}

void glUniform1ui64NV(GLint location, GLuint64EXT x)
{
	using PFNGLUNIFORM1UI64NVPROC = void(APIENTRY *)(GLint location, GLuint64EXT x);
	static PFNGLUNIFORM1UI64NVPROC pfnUniform1ui64NV{nullptr};
	LOAD_ENTRYPOINT("glUniform1ui64NV", pfnUniform1ui64NV, PFNGLUNIFORM1UI64NVPROC);
	pfnUniform1ui64NV(location, x);
}

void glUniform2ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y)
{
	using PFNGLUNIFORM2UI64NVPROC = void(APIENTRY *)(GLint location, GLuint64EXT x, GLuint64EXT y);
	static PFNGLUNIFORM2UI64NVPROC pfnUniform2ui64NV{nullptr};
	LOAD_ENTRYPOINT("glUniform2ui64NV", pfnUniform2ui64NV, PFNGLUNIFORM2UI64NVPROC);
	pfnUniform2ui64NV(location, x, y);
}

void glUniform3ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)
{
	using PFNGLUNIFORM3UI64NVPROC = void(APIENTRY *)(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
	static PFNGLUNIFORM3UI64NVPROC pfnUniform3ui64NV{nullptr};
	LOAD_ENTRYPOINT("glUniform3ui64NV", pfnUniform3ui64NV, PFNGLUNIFORM3UI64NVPROC);
	pfnUniform3ui64NV(location, x, y, z);
}

void glUniform4ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)
{
	using PFNGLUNIFORM4UI64NVPROC = void(APIENTRY *)(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
	static PFNGLUNIFORM4UI64NVPROC pfnUniform4ui64NV{nullptr};
	LOAD_ENTRYPOINT("glUniform4ui64NV", pfnUniform4ui64NV, PFNGLUNIFORM4UI64NVPROC);
	pfnUniform4ui64NV(location, x, y, z, w);
}

void glUniform1ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLUNIFORM1UI64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLUNIFORM1UI64VNVPROC pfnUniform1ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniform1ui64vNV", pfnUniform1ui64vNV, PFNGLUNIFORM1UI64VNVPROC);
	pfnUniform1ui64vNV(location, count, value);
}

void glUniform2ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLUNIFORM2UI64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLUNIFORM2UI64VNVPROC pfnUniform2ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniform2ui64vNV", pfnUniform2ui64vNV, PFNGLUNIFORM2UI64VNVPROC);
	pfnUniform2ui64vNV(location, count, value);
}

void glUniform3ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLUNIFORM3UI64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLUNIFORM3UI64VNVPROC pfnUniform3ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniform3ui64vNV", pfnUniform3ui64vNV, PFNGLUNIFORM3UI64VNVPROC);
	pfnUniform3ui64vNV(location, count, value);
}

void glUniform4ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLUNIFORM4UI64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLUNIFORM4UI64VNVPROC pfnUniform4ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniform4ui64vNV", pfnUniform4ui64vNV, PFNGLUNIFORM4UI64VNVPROC);
	pfnUniform4ui64vNV(location, count, value);
}

void glGetUniformi64vNV(GLuint program, GLint location, GLint64EXT* params)
{
	using PFNGLGETUNIFORMI64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64EXT* params);
	static PFNGLGETUNIFORMI64VNVPROC pfnGetUniformi64vNV{nullptr};
	LOAD_ENTRYPOINT("glGetUniformi64vNV", pfnGetUniformi64vNV, PFNGLGETUNIFORMI64VNVPROC);
	pfnGetUniformi64vNV(program, location, params);
}

void glProgramUniform1i64NV(GLuint program, GLint location, GLint64EXT x)
{
	using PFNGLPROGRAMUNIFORM1I64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64EXT x);
	static PFNGLPROGRAMUNIFORM1I64NVPROC pfnProgramUniform1i64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1i64NV", pfnProgramUniform1i64NV, PFNGLPROGRAMUNIFORM1I64NVPROC);
	pfnProgramUniform1i64NV(program, location, x);
}

void glProgramUniform2i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y)
{
	using PFNGLPROGRAMUNIFORM2I64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64EXT x, GLint64EXT y);
	static PFNGLPROGRAMUNIFORM2I64NVPROC pfnProgramUniform2i64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2i64NV", pfnProgramUniform2i64NV, PFNGLPROGRAMUNIFORM2I64NVPROC);
	pfnProgramUniform2i64NV(program, location, x, y);
}

void glProgramUniform3i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)
{
	using PFNGLPROGRAMUNIFORM3I64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z);
	static PFNGLPROGRAMUNIFORM3I64NVPROC pfnProgramUniform3i64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3i64NV", pfnProgramUniform3i64NV, PFNGLPROGRAMUNIFORM3I64NVPROC);
	pfnProgramUniform3i64NV(program, location, x, y, z);
}

void glProgramUniform4i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)
{
	using PFNGLPROGRAMUNIFORM4I64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w);
	static PFNGLPROGRAMUNIFORM4I64NVPROC pfnProgramUniform4i64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4i64NV", pfnProgramUniform4i64NV, PFNGLPROGRAMUNIFORM4I64NVPROC);
	pfnProgramUniform4i64NV(program, location, x, y, z, w);
}

void glProgramUniform1i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value)
{
	using PFNGLPROGRAMUNIFORM1I64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint64EXT* value);
	static PFNGLPROGRAMUNIFORM1I64VNVPROC pfnProgramUniform1i64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1i64vNV", pfnProgramUniform1i64vNV, PFNGLPROGRAMUNIFORM1I64VNVPROC);
	pfnProgramUniform1i64vNV(program, location, count, value);
}

void glProgramUniform2i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value)
{
	using PFNGLPROGRAMUNIFORM2I64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint64EXT* value);
	static PFNGLPROGRAMUNIFORM2I64VNVPROC pfnProgramUniform2i64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2i64vNV", pfnProgramUniform2i64vNV, PFNGLPROGRAMUNIFORM2I64VNVPROC);
	pfnProgramUniform2i64vNV(program, location, count, value);
}

void glProgramUniform3i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value)
{
	using PFNGLPROGRAMUNIFORM3I64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint64EXT* value);
	static PFNGLPROGRAMUNIFORM3I64VNVPROC pfnProgramUniform3i64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3i64vNV", pfnProgramUniform3i64vNV, PFNGLPROGRAMUNIFORM3I64VNVPROC);
	pfnProgramUniform3i64vNV(program, location, count, value);
}

void glProgramUniform4i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value)
{
	using PFNGLPROGRAMUNIFORM4I64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLint64EXT* value);
	static PFNGLPROGRAMUNIFORM4I64VNVPROC pfnProgramUniform4i64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4i64vNV", pfnProgramUniform4i64vNV, PFNGLPROGRAMUNIFORM4I64VNVPROC);
	pfnProgramUniform4i64vNV(program, location, count, value);
}

void glProgramUniform1ui64NV(GLuint program, GLint location, GLuint64EXT x)
{
	using PFNGLPROGRAMUNIFORM1UI64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64EXT x);
	static PFNGLPROGRAMUNIFORM1UI64NVPROC pfnProgramUniform1ui64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1ui64NV", pfnProgramUniform1ui64NV, PFNGLPROGRAMUNIFORM1UI64NVPROC);
	pfnProgramUniform1ui64NV(program, location, x);
}

void glProgramUniform2ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y)
{
	using PFNGLPROGRAMUNIFORM2UI64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y);
	static PFNGLPROGRAMUNIFORM2UI64NVPROC pfnProgramUniform2ui64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2ui64NV", pfnProgramUniform2ui64NV, PFNGLPROGRAMUNIFORM2UI64NVPROC);
	pfnProgramUniform2ui64NV(program, location, x, y);
}

void glProgramUniform3ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)
{
	using PFNGLPROGRAMUNIFORM3UI64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
	static PFNGLPROGRAMUNIFORM3UI64NVPROC pfnProgramUniform3ui64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3ui64NV", pfnProgramUniform3ui64NV, PFNGLPROGRAMUNIFORM3UI64NVPROC);
	pfnProgramUniform3ui64NV(program, location, x, y, z);
}

void glProgramUniform4ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)
{
	using PFNGLPROGRAMUNIFORM4UI64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
	static PFNGLPROGRAMUNIFORM4UI64NVPROC pfnProgramUniform4ui64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4ui64NV", pfnProgramUniform4ui64NV, PFNGLPROGRAMUNIFORM4UI64NVPROC);
	pfnProgramUniform4ui64NV(program, location, x, y, z, w);
}

void glProgramUniform1ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLPROGRAMUNIFORM1UI64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLPROGRAMUNIFORM1UI64VNVPROC pfnProgramUniform1ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform1ui64vNV", pfnProgramUniform1ui64vNV, PFNGLPROGRAMUNIFORM1UI64VNVPROC);
	pfnProgramUniform1ui64vNV(program, location, count, value);
}

void glProgramUniform2ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLPROGRAMUNIFORM2UI64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLPROGRAMUNIFORM2UI64VNVPROC pfnProgramUniform2ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform2ui64vNV", pfnProgramUniform2ui64vNV, PFNGLPROGRAMUNIFORM2UI64VNVPROC);
	pfnProgramUniform2ui64vNV(program, location, count, value);
}

void glProgramUniform3ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLPROGRAMUNIFORM3UI64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLPROGRAMUNIFORM3UI64VNVPROC pfnProgramUniform3ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform3ui64vNV", pfnProgramUniform3ui64vNV, PFNGLPROGRAMUNIFORM3UI64VNVPROC);
	pfnProgramUniform3ui64vNV(program, location, count, value);
}

void glProgramUniform4ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLPROGRAMUNIFORM4UI64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLPROGRAMUNIFORM4UI64VNVPROC pfnProgramUniform4ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniform4ui64vNV", pfnProgramUniform4ui64vNV, PFNGLPROGRAMUNIFORM4UI64VNVPROC);
	pfnProgramUniform4ui64vNV(program, location, count, value);
}

//
// GL_NV_internalformat_sample_query
//

void glGetInternalformatSampleivNV(GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei count, GLint* params)
{
	using PFNGLGETINTERNALFORMATSAMPLEIVNVPROC = void(APIENTRY *)(GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei count, GLint* params);
	static PFNGLGETINTERNALFORMATSAMPLEIVNVPROC pfnGetInternalformatSampleivNV{nullptr};
	LOAD_ENTRYPOINT("glGetInternalformatSampleivNV", pfnGetInternalformatSampleivNV, PFNGLGETINTERNALFORMATSAMPLEIVNVPROC);
	pfnGetInternalformatSampleivNV(target, internalformat, samples, pname, count, params);
}

//
// GL_NV_memory_attachment
//

void glGetMemoryObjectDetachedResourcesuivNV(GLuint memory, GLenum pname, GLint first, GLsizei count, GLuint* params)
{
	using PFNGLGETMEMORYOBJECTDETACHEDRESOURCESUIVNVPROC = void(APIENTRY *)(GLuint memory, GLenum pname, GLint first, GLsizei count, GLuint* params);
	static PFNGLGETMEMORYOBJECTDETACHEDRESOURCESUIVNVPROC pfnGetMemoryObjectDetachedResourcesuivNV{nullptr};
	LOAD_ENTRYPOINT("glGetMemoryObjectDetachedResourcesuivNV", pfnGetMemoryObjectDetachedResourcesuivNV, PFNGLGETMEMORYOBJECTDETACHEDRESOURCESUIVNVPROC);
	pfnGetMemoryObjectDetachedResourcesuivNV(memory, pname, first, count, params);
}

void glResetMemoryObjectParameterNV(GLuint memory, GLenum pname)
{
	using PFNGLRESETMEMORYOBJECTPARAMETERNVPROC = void(APIENTRY *)(GLuint memory, GLenum pname);
	static PFNGLRESETMEMORYOBJECTPARAMETERNVPROC pfnResetMemoryObjectParameterNV{nullptr};
	LOAD_ENTRYPOINT("glResetMemoryObjectParameterNV", pfnResetMemoryObjectParameterNV, PFNGLRESETMEMORYOBJECTPARAMETERNVPROC);
	pfnResetMemoryObjectParameterNV(memory, pname);
}

void glTexAttachMemoryNV(GLenum target, GLuint memory, GLuint64 offset)
{
	using PFNGLTEXATTACHMEMORYNVPROC = void(APIENTRY *)(GLenum target, GLuint memory, GLuint64 offset);
	static PFNGLTEXATTACHMEMORYNVPROC pfnTexAttachMemoryNV{nullptr};
	LOAD_ENTRYPOINT("glTexAttachMemoryNV", pfnTexAttachMemoryNV, PFNGLTEXATTACHMEMORYNVPROC);
	pfnTexAttachMemoryNV(target, memory, offset);
}

void glBufferAttachMemoryNV(GLenum target, GLuint memory, GLuint64 offset)
{
	using PFNGLBUFFERATTACHMEMORYNVPROC = void(APIENTRY *)(GLenum target, GLuint memory, GLuint64 offset);
	static PFNGLBUFFERATTACHMEMORYNVPROC pfnBufferAttachMemoryNV{nullptr};
	LOAD_ENTRYPOINT("glBufferAttachMemoryNV", pfnBufferAttachMemoryNV, PFNGLBUFFERATTACHMEMORYNVPROC);
	pfnBufferAttachMemoryNV(target, memory, offset);
}

void glTextureAttachMemoryNV(GLuint texture, GLuint memory, GLuint64 offset)
{
	using PFNGLTEXTUREATTACHMEMORYNVPROC = void(APIENTRY *)(GLuint texture, GLuint memory, GLuint64 offset);
	static PFNGLTEXTUREATTACHMEMORYNVPROC pfnTextureAttachMemoryNV{nullptr};
	LOAD_ENTRYPOINT("glTextureAttachMemoryNV", pfnTextureAttachMemoryNV, PFNGLTEXTUREATTACHMEMORYNVPROC);
	pfnTextureAttachMemoryNV(texture, memory, offset);
}

void glNamedBufferAttachMemoryNV(GLuint buffer, GLuint memory, GLuint64 offset)
{
	using PFNGLNAMEDBUFFERATTACHMEMORYNVPROC = void(APIENTRY *)(GLuint buffer, GLuint memory, GLuint64 offset);
	static PFNGLNAMEDBUFFERATTACHMEMORYNVPROC pfnNamedBufferAttachMemoryNV{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferAttachMemoryNV", pfnNamedBufferAttachMemoryNV, PFNGLNAMEDBUFFERATTACHMEMORYNVPROC);
	pfnNamedBufferAttachMemoryNV(buffer, memory, offset);
}

//
// GL_NV_memory_object_sparse
//

void glBufferPageCommitmentMemNV(GLenum target, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit)
{
	using PFNGLBUFFERPAGECOMMITMENTMEMNVPROC = void(APIENTRY *)(GLenum target, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit);
	static PFNGLBUFFERPAGECOMMITMENTMEMNVPROC pfnBufferPageCommitmentMemNV{nullptr};
	LOAD_ENTRYPOINT("glBufferPageCommitmentMemNV", pfnBufferPageCommitmentMemNV, PFNGLBUFFERPAGECOMMITMENTMEMNVPROC);
	pfnBufferPageCommitmentMemNV(target, offset, size, memory, memOffset, commit);
}

void glTexPageCommitmentMemNV(GLenum target, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit)
{
	using PFNGLTEXPAGECOMMITMENTMEMNVPROC = void(APIENTRY *)(GLenum target, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit);
	static PFNGLTEXPAGECOMMITMENTMEMNVPROC pfnTexPageCommitmentMemNV{nullptr};
	LOAD_ENTRYPOINT("glTexPageCommitmentMemNV", pfnTexPageCommitmentMemNV, PFNGLTEXPAGECOMMITMENTMEMNVPROC);
	pfnTexPageCommitmentMemNV(target, layer, level, xoffset, yoffset, zoffset, width, height, depth, memory, offset, commit);
}

void glNamedBufferPageCommitmentMemNV(GLuint buffer, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit)
{
	using PFNGLNAMEDBUFFERPAGECOMMITMENTMEMNVPROC = void(APIENTRY *)(GLuint buffer, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit);
	static PFNGLNAMEDBUFFERPAGECOMMITMENTMEMNVPROC pfnNamedBufferPageCommitmentMemNV{nullptr};
	LOAD_ENTRYPOINT("glNamedBufferPageCommitmentMemNV", pfnNamedBufferPageCommitmentMemNV, PFNGLNAMEDBUFFERPAGECOMMITMENTMEMNVPROC);
	pfnNamedBufferPageCommitmentMemNV(buffer, offset, size, memory, memOffset, commit);
}

void glTexturePageCommitmentMemNV(GLuint texture, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit)
{
	using PFNGLTEXTUREPAGECOMMITMENTMEMNVPROC = void(APIENTRY *)(GLuint texture, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit);
	static PFNGLTEXTUREPAGECOMMITMENTMEMNVPROC pfnTexturePageCommitmentMemNV{nullptr};
	LOAD_ENTRYPOINT("glTexturePageCommitmentMemNV", pfnTexturePageCommitmentMemNV, PFNGLTEXTUREPAGECOMMITMENTMEMNVPROC);
	pfnTexturePageCommitmentMemNV(texture, layer, level, xoffset, yoffset, zoffset, width, height, depth, memory, offset, commit);
}

//
// GL_NV_mesh_shader
//

void glDrawMeshTasksNV(GLuint first, GLuint count)
{
	using PFNGLDRAWMESHTASKSNVPROC = void(APIENTRY *)(GLuint first, GLuint count);
	static PFNGLDRAWMESHTASKSNVPROC pfnDrawMeshTasksNV{nullptr};
	LOAD_ENTRYPOINT("glDrawMeshTasksNV", pfnDrawMeshTasksNV, PFNGLDRAWMESHTASKSNVPROC);
	pfnDrawMeshTasksNV(first, count);
}

void glDrawMeshTasksIndirectNV(GLintptr indirect)
{
	using PFNGLDRAWMESHTASKSINDIRECTNVPROC = void(APIENTRY *)(GLintptr indirect);
	static PFNGLDRAWMESHTASKSINDIRECTNVPROC pfnDrawMeshTasksIndirectNV{nullptr};
	LOAD_ENTRYPOINT("glDrawMeshTasksIndirectNV", pfnDrawMeshTasksIndirectNV, PFNGLDRAWMESHTASKSINDIRECTNVPROC);
	pfnDrawMeshTasksIndirectNV(indirect);
}

void glMultiDrawMeshTasksIndirectNV(GLintptr indirect, GLsizei drawcount, GLsizei stride)
{
	using PFNGLMULTIDRAWMESHTASKSINDIRECTNVPROC = void(APIENTRY *)(GLintptr indirect, GLsizei drawcount, GLsizei stride);
	static PFNGLMULTIDRAWMESHTASKSINDIRECTNVPROC pfnMultiDrawMeshTasksIndirectNV{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawMeshTasksIndirectNV", pfnMultiDrawMeshTasksIndirectNV, PFNGLMULTIDRAWMESHTASKSINDIRECTNVPROC);
	pfnMultiDrawMeshTasksIndirectNV(indirect, drawcount, stride);
}

void glMultiDrawMeshTasksIndirectCountNV(GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
	using PFNGLMULTIDRAWMESHTASKSINDIRECTCOUNTNVPROC = void(APIENTRY *)(GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
	static PFNGLMULTIDRAWMESHTASKSINDIRECTCOUNTNVPROC pfnMultiDrawMeshTasksIndirectCountNV{nullptr};
	LOAD_ENTRYPOINT("glMultiDrawMeshTasksIndirectCountNV", pfnMultiDrawMeshTasksIndirectCountNV, PFNGLMULTIDRAWMESHTASKSINDIRECTCOUNTNVPROC);
	pfnMultiDrawMeshTasksIndirectCountNV(indirect, drawcount, maxdrawcount, stride);
}

//
// GL_NV_path_rendering
//

GLuint glGenPathsNV(GLsizei range)
{
	using PFNGLGENPATHSNVPROC = GLuint(APIENTRY *)(GLsizei range);
	static PFNGLGENPATHSNVPROC pfnGenPathsNV{nullptr};
	LOAD_ENTRYPOINT("glGenPathsNV", pfnGenPathsNV, PFNGLGENPATHSNVPROC);
	return pfnGenPathsNV(range);
}

void glDeletePathsNV(GLuint path, GLsizei range)
{
	using PFNGLDELETEPATHSNVPROC = void(APIENTRY *)(GLuint path, GLsizei range);
	static PFNGLDELETEPATHSNVPROC pfnDeletePathsNV{nullptr};
	LOAD_ENTRYPOINT("glDeletePathsNV", pfnDeletePathsNV, PFNGLDELETEPATHSNVPROC);
	pfnDeletePathsNV(path, range);
}

GLboolean glIsPathNV(GLuint path)
{
	using PFNGLISPATHNVPROC = GLboolean(APIENTRY *)(GLuint path);
	static PFNGLISPATHNVPROC pfnIsPathNV{nullptr};
	LOAD_ENTRYPOINT("glIsPathNV", pfnIsPathNV, PFNGLISPATHNVPROC);
	return pfnIsPathNV(path);
}

void glPathCommandsNV(GLuint path, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords)
{
	using PFNGLPATHCOMMANDSNVPROC = void(APIENTRY *)(GLuint path, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords);
	static PFNGLPATHCOMMANDSNVPROC pfnPathCommandsNV{nullptr};
	LOAD_ENTRYPOINT("glPathCommandsNV", pfnPathCommandsNV, PFNGLPATHCOMMANDSNVPROC);
	pfnPathCommandsNV(path, numCommands, commands, numCoords, coordType, coords);
}

void glPathCoordsNV(GLuint path, GLsizei numCoords, GLenum coordType, const void* coords)
{
	using PFNGLPATHCOORDSNVPROC = void(APIENTRY *)(GLuint path, GLsizei numCoords, GLenum coordType, const void* coords);
	static PFNGLPATHCOORDSNVPROC pfnPathCoordsNV{nullptr};
	LOAD_ENTRYPOINT("glPathCoordsNV", pfnPathCoordsNV, PFNGLPATHCOORDSNVPROC);
	pfnPathCoordsNV(path, numCoords, coordType, coords);
}

void glPathSubCommandsNV(GLuint path, GLsizei commandStart, GLsizei commandsToDelete, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords)
{
	using PFNGLPATHSUBCOMMANDSNVPROC = void(APIENTRY *)(GLuint path, GLsizei commandStart, GLsizei commandsToDelete, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords);
	static PFNGLPATHSUBCOMMANDSNVPROC pfnPathSubCommandsNV{nullptr};
	LOAD_ENTRYPOINT("glPathSubCommandsNV", pfnPathSubCommandsNV, PFNGLPATHSUBCOMMANDSNVPROC);
	pfnPathSubCommandsNV(path, commandStart, commandsToDelete, numCommands, commands, numCoords, coordType, coords);
}

void glPathSubCoordsNV(GLuint path, GLsizei coordStart, GLsizei numCoords, GLenum coordType, const void* coords)
{
	using PFNGLPATHSUBCOORDSNVPROC = void(APIENTRY *)(GLuint path, GLsizei coordStart, GLsizei numCoords, GLenum coordType, const void* coords);
	static PFNGLPATHSUBCOORDSNVPROC pfnPathSubCoordsNV{nullptr};
	LOAD_ENTRYPOINT("glPathSubCoordsNV", pfnPathSubCoordsNV, PFNGLPATHSUBCOORDSNVPROC);
	pfnPathSubCoordsNV(path, coordStart, numCoords, coordType, coords);
}

void glPathStringNV(GLuint path, GLenum format, GLsizei length, const void* pathString)
{
	using PFNGLPATHSTRINGNVPROC = void(APIENTRY *)(GLuint path, GLenum format, GLsizei length, const void* pathString);
	static PFNGLPATHSTRINGNVPROC pfnPathStringNV{nullptr};
	LOAD_ENTRYPOINT("glPathStringNV", pfnPathStringNV, PFNGLPATHSTRINGNVPROC);
	pfnPathStringNV(path, format, length, pathString);
}

void glPathGlyphsNV(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLsizei numGlyphs, GLenum type, const void* charcodes, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale)
{
	using PFNGLPATHGLYPHSNVPROC = void(APIENTRY *)(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLsizei numGlyphs, GLenum type, const void* charcodes, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
	static PFNGLPATHGLYPHSNVPROC pfnPathGlyphsNV{nullptr};
	LOAD_ENTRYPOINT("glPathGlyphsNV", pfnPathGlyphsNV, PFNGLPATHGLYPHSNVPROC);
	pfnPathGlyphsNV(firstPathName, fontTarget, fontName, fontStyle, numGlyphs, type, charcodes, handleMissingGlyphs, pathParameterTemplate, emScale);
}

void glPathGlyphRangeNV(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyph, GLsizei numGlyphs, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale)
{
	using PFNGLPATHGLYPHRANGENVPROC = void(APIENTRY *)(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyph, GLsizei numGlyphs, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
	static PFNGLPATHGLYPHRANGENVPROC pfnPathGlyphRangeNV{nullptr};
	LOAD_ENTRYPOINT("glPathGlyphRangeNV", pfnPathGlyphRangeNV, PFNGLPATHGLYPHRANGENVPROC);
	pfnPathGlyphRangeNV(firstPathName, fontTarget, fontName, fontStyle, firstGlyph, numGlyphs, handleMissingGlyphs, pathParameterTemplate, emScale);
}

void glWeightPathsNV(GLuint resultPath, GLsizei numPaths, const GLuint* paths, const GLfloat* weights)
{
	using PFNGLWEIGHTPATHSNVPROC = void(APIENTRY *)(GLuint resultPath, GLsizei numPaths, const GLuint* paths, const GLfloat* weights);
	static PFNGLWEIGHTPATHSNVPROC pfnWeightPathsNV{nullptr};
	LOAD_ENTRYPOINT("glWeightPathsNV", pfnWeightPathsNV, PFNGLWEIGHTPATHSNVPROC);
	pfnWeightPathsNV(resultPath, numPaths, paths, weights);
}

void glCopyPathNV(GLuint resultPath, GLuint srcPath)
{
	using PFNGLCOPYPATHNVPROC = void(APIENTRY *)(GLuint resultPath, GLuint srcPath);
	static PFNGLCOPYPATHNVPROC pfnCopyPathNV{nullptr};
	LOAD_ENTRYPOINT("glCopyPathNV", pfnCopyPathNV, PFNGLCOPYPATHNVPROC);
	pfnCopyPathNV(resultPath, srcPath);
}

void glInterpolatePathsNV(GLuint resultPath, GLuint pathA, GLuint pathB, GLfloat weight)
{
	using PFNGLINTERPOLATEPATHSNVPROC = void(APIENTRY *)(GLuint resultPath, GLuint pathA, GLuint pathB, GLfloat weight);
	static PFNGLINTERPOLATEPATHSNVPROC pfnInterpolatePathsNV{nullptr};
	LOAD_ENTRYPOINT("glInterpolatePathsNV", pfnInterpolatePathsNV, PFNGLINTERPOLATEPATHSNVPROC);
	pfnInterpolatePathsNV(resultPath, pathA, pathB, weight);
}

void glTransformPathNV(GLuint resultPath, GLuint srcPath, GLenum transformType, const GLfloat* transformValues)
{
	using PFNGLTRANSFORMPATHNVPROC = void(APIENTRY *)(GLuint resultPath, GLuint srcPath, GLenum transformType, const GLfloat* transformValues);
	static PFNGLTRANSFORMPATHNVPROC pfnTransformPathNV{nullptr};
	LOAD_ENTRYPOINT("glTransformPathNV", pfnTransformPathNV, PFNGLTRANSFORMPATHNVPROC);
	pfnTransformPathNV(resultPath, srcPath, transformType, transformValues);
}

void glPathParameterivNV(GLuint path, GLenum pname, const GLint* value)
{
	using PFNGLPATHPARAMETERIVNVPROC = void(APIENTRY *)(GLuint path, GLenum pname, const GLint* value);
	static PFNGLPATHPARAMETERIVNVPROC pfnPathParameterivNV{nullptr};
	LOAD_ENTRYPOINT("glPathParameterivNV", pfnPathParameterivNV, PFNGLPATHPARAMETERIVNVPROC);
	pfnPathParameterivNV(path, pname, value);
}

void glPathParameteriNV(GLuint path, GLenum pname, GLint value)
{
	using PFNGLPATHPARAMETERINVPROC = void(APIENTRY *)(GLuint path, GLenum pname, GLint value);
	static PFNGLPATHPARAMETERINVPROC pfnPathParameteriNV{nullptr};
	LOAD_ENTRYPOINT("glPathParameteriNV", pfnPathParameteriNV, PFNGLPATHPARAMETERINVPROC);
	pfnPathParameteriNV(path, pname, value);
}

void glPathParameterfvNV(GLuint path, GLenum pname, const GLfloat* value)
{
	using PFNGLPATHPARAMETERFVNVPROC = void(APIENTRY *)(GLuint path, GLenum pname, const GLfloat* value);
	static PFNGLPATHPARAMETERFVNVPROC pfnPathParameterfvNV{nullptr};
	LOAD_ENTRYPOINT("glPathParameterfvNV", pfnPathParameterfvNV, PFNGLPATHPARAMETERFVNVPROC);
	pfnPathParameterfvNV(path, pname, value);
}

void glPathParameterfNV(GLuint path, GLenum pname, GLfloat value)
{
	using PFNGLPATHPARAMETERFNVPROC = void(APIENTRY *)(GLuint path, GLenum pname, GLfloat value);
	static PFNGLPATHPARAMETERFNVPROC pfnPathParameterfNV{nullptr};
	LOAD_ENTRYPOINT("glPathParameterfNV", pfnPathParameterfNV, PFNGLPATHPARAMETERFNVPROC);
	pfnPathParameterfNV(path, pname, value);
}

void glPathDashArrayNV(GLuint path, GLsizei dashCount, const GLfloat* dashArray)
{
	using PFNGLPATHDASHARRAYNVPROC = void(APIENTRY *)(GLuint path, GLsizei dashCount, const GLfloat* dashArray);
	static PFNGLPATHDASHARRAYNVPROC pfnPathDashArrayNV{nullptr};
	LOAD_ENTRYPOINT("glPathDashArrayNV", pfnPathDashArrayNV, PFNGLPATHDASHARRAYNVPROC);
	pfnPathDashArrayNV(path, dashCount, dashArray);
}

void glPathStencilFuncNV(GLenum func, GLint ref, GLuint mask)
{
	using PFNGLPATHSTENCILFUNCNVPROC = void(APIENTRY *)(GLenum func, GLint ref, GLuint mask);
	static PFNGLPATHSTENCILFUNCNVPROC pfnPathStencilFuncNV{nullptr};
	LOAD_ENTRYPOINT("glPathStencilFuncNV", pfnPathStencilFuncNV, PFNGLPATHSTENCILFUNCNVPROC);
	pfnPathStencilFuncNV(func, ref, mask);
}

void glPathStencilDepthOffsetNV(GLfloat factor, GLfloat units)
{
	using PFNGLPATHSTENCILDEPTHOFFSETNVPROC = void(APIENTRY *)(GLfloat factor, GLfloat units);
	static PFNGLPATHSTENCILDEPTHOFFSETNVPROC pfnPathStencilDepthOffsetNV{nullptr};
	LOAD_ENTRYPOINT("glPathStencilDepthOffsetNV", pfnPathStencilDepthOffsetNV, PFNGLPATHSTENCILDEPTHOFFSETNVPROC);
	pfnPathStencilDepthOffsetNV(factor, units);
}

void glStencilFillPathNV(GLuint path, GLenum fillMode, GLuint mask)
{
	using PFNGLSTENCILFILLPATHNVPROC = void(APIENTRY *)(GLuint path, GLenum fillMode, GLuint mask);
	static PFNGLSTENCILFILLPATHNVPROC pfnStencilFillPathNV{nullptr};
	LOAD_ENTRYPOINT("glStencilFillPathNV", pfnStencilFillPathNV, PFNGLSTENCILFILLPATHNVPROC);
	pfnStencilFillPathNV(path, fillMode, mask);
}

void glStencilStrokePathNV(GLuint path, GLint reference, GLuint mask)
{
	using PFNGLSTENCILSTROKEPATHNVPROC = void(APIENTRY *)(GLuint path, GLint reference, GLuint mask);
	static PFNGLSTENCILSTROKEPATHNVPROC pfnStencilStrokePathNV{nullptr};
	LOAD_ENTRYPOINT("glStencilStrokePathNV", pfnStencilStrokePathNV, PFNGLSTENCILSTROKEPATHNVPROC);
	pfnStencilStrokePathNV(path, reference, mask);
}

void glStencilFillPathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat* transformValues)
{
	using PFNGLSTENCILFILLPATHINSTANCEDNVPROC = void(APIENTRY *)(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat* transformValues);
	static PFNGLSTENCILFILLPATHINSTANCEDNVPROC pfnStencilFillPathInstancedNV{nullptr};
	LOAD_ENTRYPOINT("glStencilFillPathInstancedNV", pfnStencilFillPathInstancedNV, PFNGLSTENCILFILLPATHINSTANCEDNVPROC);
	pfnStencilFillPathInstancedNV(numPaths, pathNameType, paths, pathBase, fillMode, mask, transformType, transformValues);
}

void glStencilStrokePathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat* transformValues)
{
	using PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC = void(APIENTRY *)(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat* transformValues);
	static PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC pfnStencilStrokePathInstancedNV{nullptr};
	LOAD_ENTRYPOINT("glStencilStrokePathInstancedNV", pfnStencilStrokePathInstancedNV, PFNGLSTENCILSTROKEPATHINSTANCEDNVPROC);
	pfnStencilStrokePathInstancedNV(numPaths, pathNameType, paths, pathBase, reference, mask, transformType, transformValues);
}

void glPathCoverDepthFuncNV(GLenum func)
{
	using PFNGLPATHCOVERDEPTHFUNCNVPROC = void(APIENTRY *)(GLenum func);
	static PFNGLPATHCOVERDEPTHFUNCNVPROC pfnPathCoverDepthFuncNV{nullptr};
	LOAD_ENTRYPOINT("glPathCoverDepthFuncNV", pfnPathCoverDepthFuncNV, PFNGLPATHCOVERDEPTHFUNCNVPROC);
	pfnPathCoverDepthFuncNV(func);
}

void glCoverFillPathNV(GLuint path, GLenum coverMode)
{
	using PFNGLCOVERFILLPATHNVPROC = void(APIENTRY *)(GLuint path, GLenum coverMode);
	static PFNGLCOVERFILLPATHNVPROC pfnCoverFillPathNV{nullptr};
	LOAD_ENTRYPOINT("glCoverFillPathNV", pfnCoverFillPathNV, PFNGLCOVERFILLPATHNVPROC);
	pfnCoverFillPathNV(path, coverMode);
}

void glCoverStrokePathNV(GLuint path, GLenum coverMode)
{
	using PFNGLCOVERSTROKEPATHNVPROC = void(APIENTRY *)(GLuint path, GLenum coverMode);
	static PFNGLCOVERSTROKEPATHNVPROC pfnCoverStrokePathNV{nullptr};
	LOAD_ENTRYPOINT("glCoverStrokePathNV", pfnCoverStrokePathNV, PFNGLCOVERSTROKEPATHNVPROC);
	pfnCoverStrokePathNV(path, coverMode);
}

void glCoverFillPathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues)
{
	using PFNGLCOVERFILLPATHINSTANCEDNVPROC = void(APIENTRY *)(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues);
	static PFNGLCOVERFILLPATHINSTANCEDNVPROC pfnCoverFillPathInstancedNV{nullptr};
	LOAD_ENTRYPOINT("glCoverFillPathInstancedNV", pfnCoverFillPathInstancedNV, PFNGLCOVERFILLPATHINSTANCEDNVPROC);
	pfnCoverFillPathInstancedNV(numPaths, pathNameType, paths, pathBase, coverMode, transformType, transformValues);
}

void glCoverStrokePathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues)
{
	using PFNGLCOVERSTROKEPATHINSTANCEDNVPROC = void(APIENTRY *)(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues);
	static PFNGLCOVERSTROKEPATHINSTANCEDNVPROC pfnCoverStrokePathInstancedNV{nullptr};
	LOAD_ENTRYPOINT("glCoverStrokePathInstancedNV", pfnCoverStrokePathInstancedNV, PFNGLCOVERSTROKEPATHINSTANCEDNVPROC);
	pfnCoverStrokePathInstancedNV(numPaths, pathNameType, paths, pathBase, coverMode, transformType, transformValues);
}

void glGetPathParameterivNV(GLuint path, GLenum pname, GLint* value)
{
	using PFNGLGETPATHPARAMETERIVNVPROC = void(APIENTRY *)(GLuint path, GLenum pname, GLint* value);
	static PFNGLGETPATHPARAMETERIVNVPROC pfnGetPathParameterivNV{nullptr};
	LOAD_ENTRYPOINT("glGetPathParameterivNV", pfnGetPathParameterivNV, PFNGLGETPATHPARAMETERIVNVPROC);
	pfnGetPathParameterivNV(path, pname, value);
}

void glGetPathParameterfvNV(GLuint path, GLenum pname, GLfloat* value)
{
	using PFNGLGETPATHPARAMETERFVNVPROC = void(APIENTRY *)(GLuint path, GLenum pname, GLfloat* value);
	static PFNGLGETPATHPARAMETERFVNVPROC pfnGetPathParameterfvNV{nullptr};
	LOAD_ENTRYPOINT("glGetPathParameterfvNV", pfnGetPathParameterfvNV, PFNGLGETPATHPARAMETERFVNVPROC);
	pfnGetPathParameterfvNV(path, pname, value);
}

void glGetPathCommandsNV(GLuint path, GLubyte* commands)
{
	using PFNGLGETPATHCOMMANDSNVPROC = void(APIENTRY *)(GLuint path, GLubyte* commands);
	static PFNGLGETPATHCOMMANDSNVPROC pfnGetPathCommandsNV{nullptr};
	LOAD_ENTRYPOINT("glGetPathCommandsNV", pfnGetPathCommandsNV, PFNGLGETPATHCOMMANDSNVPROC);
	pfnGetPathCommandsNV(path, commands);
}

void glGetPathCoordsNV(GLuint path, GLfloat* coords)
{
	using PFNGLGETPATHCOORDSNVPROC = void(APIENTRY *)(GLuint path, GLfloat* coords);
	static PFNGLGETPATHCOORDSNVPROC pfnGetPathCoordsNV{nullptr};
	LOAD_ENTRYPOINT("glGetPathCoordsNV", pfnGetPathCoordsNV, PFNGLGETPATHCOORDSNVPROC);
	pfnGetPathCoordsNV(path, coords);
}

void glGetPathDashArrayNV(GLuint path, GLfloat* dashArray)
{
	using PFNGLGETPATHDASHARRAYNVPROC = void(APIENTRY *)(GLuint path, GLfloat* dashArray);
	static PFNGLGETPATHDASHARRAYNVPROC pfnGetPathDashArrayNV{nullptr};
	LOAD_ENTRYPOINT("glGetPathDashArrayNV", pfnGetPathDashArrayNV, PFNGLGETPATHDASHARRAYNVPROC);
	pfnGetPathDashArrayNV(path, dashArray);
}

void glGetPathMetricsNV(GLbitfield metricQueryMask, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLsizei stride, GLfloat* metrics)
{
	using PFNGLGETPATHMETRICSNVPROC = void(APIENTRY *)(GLbitfield metricQueryMask, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLsizei stride, GLfloat* metrics);
	static PFNGLGETPATHMETRICSNVPROC pfnGetPathMetricsNV{nullptr};
	LOAD_ENTRYPOINT("glGetPathMetricsNV", pfnGetPathMetricsNV, PFNGLGETPATHMETRICSNVPROC);
	pfnGetPathMetricsNV(metricQueryMask, numPaths, pathNameType, paths, pathBase, stride, metrics);
}

void glGetPathMetricRangeNV(GLbitfield metricQueryMask, GLuint firstPathName, GLsizei numPaths, GLsizei stride, GLfloat* metrics)
{
	using PFNGLGETPATHMETRICRANGENVPROC = void(APIENTRY *)(GLbitfield metricQueryMask, GLuint firstPathName, GLsizei numPaths, GLsizei stride, GLfloat* metrics);
	static PFNGLGETPATHMETRICRANGENVPROC pfnGetPathMetricRangeNV{nullptr};
	LOAD_ENTRYPOINT("glGetPathMetricRangeNV", pfnGetPathMetricRangeNV, PFNGLGETPATHMETRICRANGENVPROC);
	pfnGetPathMetricRangeNV(metricQueryMask, firstPathName, numPaths, stride, metrics);
}

void glGetPathSpacingNV(GLenum pathListMode, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLfloat advanceScale, GLfloat kerningScale, GLenum transformType, GLfloat* returnedSpacing)
{
	using PFNGLGETPATHSPACINGNVPROC = void(APIENTRY *)(GLenum pathListMode, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLfloat advanceScale, GLfloat kerningScale, GLenum transformType, GLfloat* returnedSpacing);
	static PFNGLGETPATHSPACINGNVPROC pfnGetPathSpacingNV{nullptr};
	LOAD_ENTRYPOINT("glGetPathSpacingNV", pfnGetPathSpacingNV, PFNGLGETPATHSPACINGNVPROC);
	pfnGetPathSpacingNV(pathListMode, numPaths, pathNameType, paths, pathBase, advanceScale, kerningScale, transformType, returnedSpacing);
}

GLboolean glIsPointInFillPathNV(GLuint path, GLuint mask, GLfloat x, GLfloat y)
{
	using PFNGLISPOINTINFILLPATHNVPROC = GLboolean(APIENTRY *)(GLuint path, GLuint mask, GLfloat x, GLfloat y);
	static PFNGLISPOINTINFILLPATHNVPROC pfnIsPointInFillPathNV{nullptr};
	LOAD_ENTRYPOINT("glIsPointInFillPathNV", pfnIsPointInFillPathNV, PFNGLISPOINTINFILLPATHNVPROC);
	return pfnIsPointInFillPathNV(path, mask, x, y);
}

GLboolean glIsPointInStrokePathNV(GLuint path, GLfloat x, GLfloat y)
{
	using PFNGLISPOINTINSTROKEPATHNVPROC = GLboolean(APIENTRY *)(GLuint path, GLfloat x, GLfloat y);
	static PFNGLISPOINTINSTROKEPATHNVPROC pfnIsPointInStrokePathNV{nullptr};
	LOAD_ENTRYPOINT("glIsPointInStrokePathNV", pfnIsPointInStrokePathNV, PFNGLISPOINTINSTROKEPATHNVPROC);
	return pfnIsPointInStrokePathNV(path, x, y);
}

GLfloat glGetPathLengthNV(GLuint path, GLsizei startSegment, GLsizei numSegments)
{
	using PFNGLGETPATHLENGTHNVPROC = GLfloat(APIENTRY *)(GLuint path, GLsizei startSegment, GLsizei numSegments);
	static PFNGLGETPATHLENGTHNVPROC pfnGetPathLengthNV{nullptr};
	LOAD_ENTRYPOINT("glGetPathLengthNV", pfnGetPathLengthNV, PFNGLGETPATHLENGTHNVPROC);
	return pfnGetPathLengthNV(path, startSegment, numSegments);
}

GLboolean glPointAlongPathNV(GLuint path, GLsizei startSegment, GLsizei numSegments, GLfloat distance, GLfloat* x, GLfloat* y, GLfloat* tangentX, GLfloat* tangentY)
{
	using PFNGLPOINTALONGPATHNVPROC = GLboolean(APIENTRY *)(GLuint path, GLsizei startSegment, GLsizei numSegments, GLfloat distance, GLfloat* x, GLfloat* y, GLfloat* tangentX, GLfloat* tangentY);
	static PFNGLPOINTALONGPATHNVPROC pfnPointAlongPathNV{nullptr};
	LOAD_ENTRYPOINT("glPointAlongPathNV", pfnPointAlongPathNV, PFNGLPOINTALONGPATHNVPROC);
	return pfnPointAlongPathNV(path, startSegment, numSegments, distance, x, y, tangentX, tangentY);
}

void glMatrixLoad3x2fNV(GLenum matrixMode, const GLfloat* m)
{
	using PFNGLMATRIXLOAD3X2FNVPROC = void(APIENTRY *)(GLenum matrixMode, const GLfloat* m);
	static PFNGLMATRIXLOAD3X2FNVPROC pfnMatrixLoad3x2fNV{nullptr};
	LOAD_ENTRYPOINT("glMatrixLoad3x2fNV", pfnMatrixLoad3x2fNV, PFNGLMATRIXLOAD3X2FNVPROC);
	pfnMatrixLoad3x2fNV(matrixMode, m);
}

void glMatrixLoad3x3fNV(GLenum matrixMode, const GLfloat* m)
{
	using PFNGLMATRIXLOAD3X3FNVPROC = void(APIENTRY *)(GLenum matrixMode, const GLfloat* m);
	static PFNGLMATRIXLOAD3X3FNVPROC pfnMatrixLoad3x3fNV{nullptr};
	LOAD_ENTRYPOINT("glMatrixLoad3x3fNV", pfnMatrixLoad3x3fNV, PFNGLMATRIXLOAD3X3FNVPROC);
	pfnMatrixLoad3x3fNV(matrixMode, m);
}

void glMatrixLoadTranspose3x3fNV(GLenum matrixMode, const GLfloat* m)
{
	using PFNGLMATRIXLOADTRANSPOSE3X3FNVPROC = void(APIENTRY *)(GLenum matrixMode, const GLfloat* m);
	static PFNGLMATRIXLOADTRANSPOSE3X3FNVPROC pfnMatrixLoadTranspose3x3fNV{nullptr};
	LOAD_ENTRYPOINT("glMatrixLoadTranspose3x3fNV", pfnMatrixLoadTranspose3x3fNV, PFNGLMATRIXLOADTRANSPOSE3X3FNVPROC);
	pfnMatrixLoadTranspose3x3fNV(matrixMode, m);
}

void glMatrixMult3x2fNV(GLenum matrixMode, const GLfloat* m)
{
	using PFNGLMATRIXMULT3X2FNVPROC = void(APIENTRY *)(GLenum matrixMode, const GLfloat* m);
	static PFNGLMATRIXMULT3X2FNVPROC pfnMatrixMult3x2fNV{nullptr};
	LOAD_ENTRYPOINT("glMatrixMult3x2fNV", pfnMatrixMult3x2fNV, PFNGLMATRIXMULT3X2FNVPROC);
	pfnMatrixMult3x2fNV(matrixMode, m);
}

void glMatrixMult3x3fNV(GLenum matrixMode, const GLfloat* m)
{
	using PFNGLMATRIXMULT3X3FNVPROC = void(APIENTRY *)(GLenum matrixMode, const GLfloat* m);
	static PFNGLMATRIXMULT3X3FNVPROC pfnMatrixMult3x3fNV{nullptr};
	LOAD_ENTRYPOINT("glMatrixMult3x3fNV", pfnMatrixMult3x3fNV, PFNGLMATRIXMULT3X3FNVPROC);
	pfnMatrixMult3x3fNV(matrixMode, m);
}

void glMatrixMultTranspose3x3fNV(GLenum matrixMode, const GLfloat* m)
{
	using PFNGLMATRIXMULTTRANSPOSE3X3FNVPROC = void(APIENTRY *)(GLenum matrixMode, const GLfloat* m);
	static PFNGLMATRIXMULTTRANSPOSE3X3FNVPROC pfnMatrixMultTranspose3x3fNV{nullptr};
	LOAD_ENTRYPOINT("glMatrixMultTranspose3x3fNV", pfnMatrixMultTranspose3x3fNV, PFNGLMATRIXMULTTRANSPOSE3X3FNVPROC);
	pfnMatrixMultTranspose3x3fNV(matrixMode, m);
}

void glStencilThenCoverFillPathNV(GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode)
{
	using PFNGLSTENCILTHENCOVERFILLPATHNVPROC = void(APIENTRY *)(GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode);
	static PFNGLSTENCILTHENCOVERFILLPATHNVPROC pfnStencilThenCoverFillPathNV{nullptr};
	LOAD_ENTRYPOINT("glStencilThenCoverFillPathNV", pfnStencilThenCoverFillPathNV, PFNGLSTENCILTHENCOVERFILLPATHNVPROC);
	pfnStencilThenCoverFillPathNV(path, fillMode, mask, coverMode);
}

void glStencilThenCoverStrokePathNV(GLuint path, GLint reference, GLuint mask, GLenum coverMode)
{
	using PFNGLSTENCILTHENCOVERSTROKEPATHNVPROC = void(APIENTRY *)(GLuint path, GLint reference, GLuint mask, GLenum coverMode);
	static PFNGLSTENCILTHENCOVERSTROKEPATHNVPROC pfnStencilThenCoverStrokePathNV{nullptr};
	LOAD_ENTRYPOINT("glStencilThenCoverStrokePathNV", pfnStencilThenCoverStrokePathNV, PFNGLSTENCILTHENCOVERSTROKEPATHNVPROC);
	pfnStencilThenCoverStrokePathNV(path, reference, mask, coverMode);
}

void glStencilThenCoverFillPathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues)
{
	using PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDNVPROC = void(APIENTRY *)(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues);
	static PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDNVPROC pfnStencilThenCoverFillPathInstancedNV{nullptr};
	LOAD_ENTRYPOINT("glStencilThenCoverFillPathInstancedNV", pfnStencilThenCoverFillPathInstancedNV, PFNGLSTENCILTHENCOVERFILLPATHINSTANCEDNVPROC);
	pfnStencilThenCoverFillPathInstancedNV(numPaths, pathNameType, paths, pathBase, fillMode, mask, coverMode, transformType, transformValues);
}

void glStencilThenCoverStrokePathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues)
{
	using PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDNVPROC = void(APIENTRY *)(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues);
	static PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDNVPROC pfnStencilThenCoverStrokePathInstancedNV{nullptr};
	LOAD_ENTRYPOINT("glStencilThenCoverStrokePathInstancedNV", pfnStencilThenCoverStrokePathInstancedNV, PFNGLSTENCILTHENCOVERSTROKEPATHINSTANCEDNVPROC);
	pfnStencilThenCoverStrokePathInstancedNV(numPaths, pathNameType, paths, pathBase, reference, mask, coverMode, transformType, transformValues);
}

GLenum glPathGlyphIndexRangeNV(GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint pathParameterTemplate, GLfloat emScale, GLuint* baseAndCount)
{
	using PFNGLPATHGLYPHINDEXRANGENVPROC = GLenum(APIENTRY *)(GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint pathParameterTemplate, GLfloat emScale, GLuint* baseAndCount);
	static PFNGLPATHGLYPHINDEXRANGENVPROC pfnPathGlyphIndexRangeNV{nullptr};
	LOAD_ENTRYPOINT("glPathGlyphIndexRangeNV", pfnPathGlyphIndexRangeNV, PFNGLPATHGLYPHINDEXRANGENVPROC);
	return pfnPathGlyphIndexRangeNV(fontTarget, fontName, fontStyle, pathParameterTemplate, emScale, baseAndCount);
}

GLenum glPathGlyphIndexArrayNV(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale)
{
	using PFNGLPATHGLYPHINDEXARRAYNVPROC = GLenum(APIENTRY *)(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
	static PFNGLPATHGLYPHINDEXARRAYNVPROC pfnPathGlyphIndexArrayNV{nullptr};
	LOAD_ENTRYPOINT("glPathGlyphIndexArrayNV", pfnPathGlyphIndexArrayNV, PFNGLPATHGLYPHINDEXARRAYNVPROC);
	return pfnPathGlyphIndexArrayNV(firstPathName, fontTarget, fontName, fontStyle, firstGlyphIndex, numGlyphs, pathParameterTemplate, emScale);
}

GLenum glPathMemoryGlyphIndexArrayNV(GLuint firstPathName, GLenum fontTarget, GLsizeiptr fontSize, const void* fontData, GLsizei faceIndex, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale)
{
	using PFNGLPATHMEMORYGLYPHINDEXARRAYNVPROC = GLenum(APIENTRY *)(GLuint firstPathName, GLenum fontTarget, GLsizeiptr fontSize, const void* fontData, GLsizei faceIndex, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
	static PFNGLPATHMEMORYGLYPHINDEXARRAYNVPROC pfnPathMemoryGlyphIndexArrayNV{nullptr};
	LOAD_ENTRYPOINT("glPathMemoryGlyphIndexArrayNV", pfnPathMemoryGlyphIndexArrayNV, PFNGLPATHMEMORYGLYPHINDEXARRAYNVPROC);
	return pfnPathMemoryGlyphIndexArrayNV(firstPathName, fontTarget, fontSize, fontData, faceIndex, firstGlyphIndex, numGlyphs, pathParameterTemplate, emScale);
}

void glProgramPathFragmentInputGenNV(GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat* coeffs)
{
	using PFNGLPROGRAMPATHFRAGMENTINPUTGENNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat* coeffs);
	static PFNGLPROGRAMPATHFRAGMENTINPUTGENNVPROC pfnProgramPathFragmentInputGenNV{nullptr};
	LOAD_ENTRYPOINT("glProgramPathFragmentInputGenNV", pfnProgramPathFragmentInputGenNV, PFNGLPROGRAMPATHFRAGMENTINPUTGENNVPROC);
	pfnProgramPathFragmentInputGenNV(program, location, genMode, components, coeffs);
}

void glGetProgramResourcefvNV(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei count, GLsizei* length, GLfloat* params)
{
	using PFNGLGETPROGRAMRESOURCEFVNVPROC = void(APIENTRY *)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei count, GLsizei* length, GLfloat* params);
	static PFNGLGETPROGRAMRESOURCEFVNVPROC pfnGetProgramResourcefvNV{nullptr};
	LOAD_ENTRYPOINT("glGetProgramResourcefvNV", pfnGetProgramResourcefvNV, PFNGLGETPROGRAMRESOURCEFVNVPROC);
	pfnGetProgramResourcefvNV(program, programInterface, index, propCount, props, count, length, params);
}

//
// GL_NV_sample_locations
//

void glFramebufferSampleLocationsfvNV(GLenum target, GLuint start, GLsizei count, const GLfloat* v)
{
	using PFNGLFRAMEBUFFERSAMPLELOCATIONSFVNVPROC = void(APIENTRY *)(GLenum target, GLuint start, GLsizei count, const GLfloat* v);
	static PFNGLFRAMEBUFFERSAMPLELOCATIONSFVNVPROC pfnFramebufferSampleLocationsfvNV{nullptr};
	LOAD_ENTRYPOINT("glFramebufferSampleLocationsfvNV", pfnFramebufferSampleLocationsfvNV, PFNGLFRAMEBUFFERSAMPLELOCATIONSFVNVPROC);
	pfnFramebufferSampleLocationsfvNV(target, start, count, v);
}

void glNamedFramebufferSampleLocationsfvNV(GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v)
{
	using PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVNVPROC = void(APIENTRY *)(GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v);
	static PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVNVPROC pfnNamedFramebufferSampleLocationsfvNV{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferSampleLocationsfvNV", pfnNamedFramebufferSampleLocationsfvNV, PFNGLNAMEDFRAMEBUFFERSAMPLELOCATIONSFVNVPROC);
	pfnNamedFramebufferSampleLocationsfvNV(framebuffer, start, count, v);
}

void glResolveDepthValuesNV(void)
{
	using PFNGLRESOLVEDEPTHVALUESNVPROC = void(APIENTRY *)(void);
	static PFNGLRESOLVEDEPTHVALUESNVPROC pfnResolveDepthValuesNV{nullptr};
	LOAD_ENTRYPOINT("glResolveDepthValuesNV", pfnResolveDepthValuesNV, PFNGLRESOLVEDEPTHVALUESNVPROC);
	pfnResolveDepthValuesNV();
}

//
// GL_NV_scissor_exclusive
//

void glScissorExclusiveNV(GLint x, GLint y, GLsizei width, GLsizei height)
{
	using PFNGLSCISSOREXCLUSIVENVPROC = void(APIENTRY *)(GLint x, GLint y, GLsizei width, GLsizei height);
	static PFNGLSCISSOREXCLUSIVENVPROC pfnScissorExclusiveNV{nullptr};
	LOAD_ENTRYPOINT("glScissorExclusiveNV", pfnScissorExclusiveNV, PFNGLSCISSOREXCLUSIVENVPROC);
	pfnScissorExclusiveNV(x, y, width, height);
}

void glScissorExclusiveArrayvNV(GLuint first, GLsizei count, const GLint* v)
{
	using PFNGLSCISSOREXCLUSIVEARRAYVNVPROC = void(APIENTRY *)(GLuint first, GLsizei count, const GLint* v);
	static PFNGLSCISSOREXCLUSIVEARRAYVNVPROC pfnScissorExclusiveArrayvNV{nullptr};
	LOAD_ENTRYPOINT("glScissorExclusiveArrayvNV", pfnScissorExclusiveArrayvNV, PFNGLSCISSOREXCLUSIVEARRAYVNVPROC);
	pfnScissorExclusiveArrayvNV(first, count, v);
}

//
// GL_NV_shader_buffer_load
//

void glMakeBufferResidentNV(GLenum target, GLenum access)
{
	using PFNGLMAKEBUFFERRESIDENTNVPROC = void(APIENTRY *)(GLenum target, GLenum access);
	static PFNGLMAKEBUFFERRESIDENTNVPROC pfnMakeBufferResidentNV{nullptr};
	LOAD_ENTRYPOINT("glMakeBufferResidentNV", pfnMakeBufferResidentNV, PFNGLMAKEBUFFERRESIDENTNVPROC);
	pfnMakeBufferResidentNV(target, access);
}

void glMakeBufferNonResidentNV(GLenum target)
{
	using PFNGLMAKEBUFFERNONRESIDENTNVPROC = void(APIENTRY *)(GLenum target);
	static PFNGLMAKEBUFFERNONRESIDENTNVPROC pfnMakeBufferNonResidentNV{nullptr};
	LOAD_ENTRYPOINT("glMakeBufferNonResidentNV", pfnMakeBufferNonResidentNV, PFNGLMAKEBUFFERNONRESIDENTNVPROC);
	pfnMakeBufferNonResidentNV(target);
}

GLboolean glIsBufferResidentNV(GLenum target)
{
	using PFNGLISBUFFERRESIDENTNVPROC = GLboolean(APIENTRY *)(GLenum target);
	static PFNGLISBUFFERRESIDENTNVPROC pfnIsBufferResidentNV{nullptr};
	LOAD_ENTRYPOINT("glIsBufferResidentNV", pfnIsBufferResidentNV, PFNGLISBUFFERRESIDENTNVPROC);
	return pfnIsBufferResidentNV(target);
}

void glMakeNamedBufferResidentNV(GLuint buffer, GLenum access)
{
	using PFNGLMAKENAMEDBUFFERRESIDENTNVPROC = void(APIENTRY *)(GLuint buffer, GLenum access);
	static PFNGLMAKENAMEDBUFFERRESIDENTNVPROC pfnMakeNamedBufferResidentNV{nullptr};
	LOAD_ENTRYPOINT("glMakeNamedBufferResidentNV", pfnMakeNamedBufferResidentNV, PFNGLMAKENAMEDBUFFERRESIDENTNVPROC);
	pfnMakeNamedBufferResidentNV(buffer, access);
}

void glMakeNamedBufferNonResidentNV(GLuint buffer)
{
	using PFNGLMAKENAMEDBUFFERNONRESIDENTNVPROC = void(APIENTRY *)(GLuint buffer);
	static PFNGLMAKENAMEDBUFFERNONRESIDENTNVPROC pfnMakeNamedBufferNonResidentNV{nullptr};
	LOAD_ENTRYPOINT("glMakeNamedBufferNonResidentNV", pfnMakeNamedBufferNonResidentNV, PFNGLMAKENAMEDBUFFERNONRESIDENTNVPROC);
	pfnMakeNamedBufferNonResidentNV(buffer);
}

GLboolean glIsNamedBufferResidentNV(GLuint buffer)
{
	using PFNGLISNAMEDBUFFERRESIDENTNVPROC = GLboolean(APIENTRY *)(GLuint buffer);
	static PFNGLISNAMEDBUFFERRESIDENTNVPROC pfnIsNamedBufferResidentNV{nullptr};
	LOAD_ENTRYPOINT("glIsNamedBufferResidentNV", pfnIsNamedBufferResidentNV, PFNGLISNAMEDBUFFERRESIDENTNVPROC);
	return pfnIsNamedBufferResidentNV(buffer);
}

void glGetBufferParameterui64vNV(GLenum target, GLenum pname, GLuint64EXT* params)
{
	using PFNGLGETBUFFERPARAMETERUI64VNVPROC = void(APIENTRY *)(GLenum target, GLenum pname, GLuint64EXT* params);
	static PFNGLGETBUFFERPARAMETERUI64VNVPROC pfnGetBufferParameterui64vNV{nullptr};
	LOAD_ENTRYPOINT("glGetBufferParameterui64vNV", pfnGetBufferParameterui64vNV, PFNGLGETBUFFERPARAMETERUI64VNVPROC);
	pfnGetBufferParameterui64vNV(target, pname, params);
}

void glGetNamedBufferParameterui64vNV(GLuint buffer, GLenum pname, GLuint64EXT* params)
{
	using PFNGLGETNAMEDBUFFERPARAMETERUI64VNVPROC = void(APIENTRY *)(GLuint buffer, GLenum pname, GLuint64EXT* params);
	static PFNGLGETNAMEDBUFFERPARAMETERUI64VNVPROC pfnGetNamedBufferParameterui64vNV{nullptr};
	LOAD_ENTRYPOINT("glGetNamedBufferParameterui64vNV", pfnGetNamedBufferParameterui64vNV, PFNGLGETNAMEDBUFFERPARAMETERUI64VNVPROC);
	pfnGetNamedBufferParameterui64vNV(buffer, pname, params);
}

void glGetIntegerui64vNV(GLenum value, GLuint64EXT* result)
{
	using PFNGLGETINTEGERUI64VNVPROC = void(APIENTRY *)(GLenum value, GLuint64EXT* result);
	static PFNGLGETINTEGERUI64VNVPROC pfnGetIntegerui64vNV{nullptr};
	LOAD_ENTRYPOINT("glGetIntegerui64vNV", pfnGetIntegerui64vNV, PFNGLGETINTEGERUI64VNVPROC);
	pfnGetIntegerui64vNV(value, result);
}

void glUniformui64NV(GLint location, GLuint64EXT value)
{
	using PFNGLUNIFORMUI64NVPROC = void(APIENTRY *)(GLint location, GLuint64EXT value);
	static PFNGLUNIFORMUI64NVPROC pfnUniformui64NV{nullptr};
	LOAD_ENTRYPOINT("glUniformui64NV", pfnUniformui64NV, PFNGLUNIFORMUI64NVPROC);
	pfnUniformui64NV(location, value);
}

void glUniformui64vNV(GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLUNIFORMUI64VNVPROC = void(APIENTRY *)(GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLUNIFORMUI64VNVPROC pfnUniformui64vNV{nullptr};
	LOAD_ENTRYPOINT("glUniformui64vNV", pfnUniformui64vNV, PFNGLUNIFORMUI64VNVPROC);
	pfnUniformui64vNV(location, count, value);
}

void glGetUniformui64vNV(GLuint program, GLint location, GLuint64EXT* params)
{
	using PFNGLGETUNIFORMUI64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64EXT* params);
	static PFNGLGETUNIFORMUI64VNVPROC pfnGetUniformui64vNV{nullptr};
	LOAD_ENTRYPOINT("glGetUniformui64vNV", pfnGetUniformui64vNV, PFNGLGETUNIFORMUI64VNVPROC);
	pfnGetUniformui64vNV(program, location, params);
}

void glProgramUniformui64NV(GLuint program, GLint location, GLuint64EXT value)
{
	using PFNGLPROGRAMUNIFORMUI64NVPROC = void(APIENTRY *)(GLuint program, GLint location, GLuint64EXT value);
	static PFNGLPROGRAMUNIFORMUI64NVPROC pfnProgramUniformui64NV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformui64NV", pfnProgramUniformui64NV, PFNGLPROGRAMUNIFORMUI64NVPROC);
	pfnProgramUniformui64NV(program, location, value);
}

void glProgramUniformui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value)
{
	using PFNGLPROGRAMUNIFORMUI64VNVPROC = void(APIENTRY *)(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);
	static PFNGLPROGRAMUNIFORMUI64VNVPROC pfnProgramUniformui64vNV{nullptr};
	LOAD_ENTRYPOINT("glProgramUniformui64vNV", pfnProgramUniformui64vNV, PFNGLPROGRAMUNIFORMUI64VNVPROC);
	pfnProgramUniformui64vNV(program, location, count, value);
}

//
// GL_NV_shading_rate_image
//

void glBindShadingRateImageNV(GLuint texture)
{
	using PFNGLBINDSHADINGRATEIMAGENVPROC = void(APIENTRY *)(GLuint texture);
	static PFNGLBINDSHADINGRATEIMAGENVPROC pfnBindShadingRateImageNV{nullptr};
	LOAD_ENTRYPOINT("glBindShadingRateImageNV", pfnBindShadingRateImageNV, PFNGLBINDSHADINGRATEIMAGENVPROC);
	pfnBindShadingRateImageNV(texture);
}

void glGetShadingRateImagePaletteNV(GLuint viewport, GLuint entry, GLenum* rate)
{
	using PFNGLGETSHADINGRATEIMAGEPALETTENVPROC = void(APIENTRY *)(GLuint viewport, GLuint entry, GLenum* rate);
	static PFNGLGETSHADINGRATEIMAGEPALETTENVPROC pfnGetShadingRateImagePaletteNV{nullptr};
	LOAD_ENTRYPOINT("glGetShadingRateImagePaletteNV", pfnGetShadingRateImagePaletteNV, PFNGLGETSHADINGRATEIMAGEPALETTENVPROC);
	pfnGetShadingRateImagePaletteNV(viewport, entry, rate);
}

void glGetShadingRateSampleLocationivNV(GLenum rate, GLuint samples, GLuint index, GLint* location)
{
	using PFNGLGETSHADINGRATESAMPLELOCATIONIVNVPROC = void(APIENTRY *)(GLenum rate, GLuint samples, GLuint index, GLint* location);
	static PFNGLGETSHADINGRATESAMPLELOCATIONIVNVPROC pfnGetShadingRateSampleLocationivNV{nullptr};
	LOAD_ENTRYPOINT("glGetShadingRateSampleLocationivNV", pfnGetShadingRateSampleLocationivNV, PFNGLGETSHADINGRATESAMPLELOCATIONIVNVPROC);
	pfnGetShadingRateSampleLocationivNV(rate, samples, index, location);
}

void glShadingRateImageBarrierNV(GLboolean synchronize)
{
	using PFNGLSHADINGRATEIMAGEBARRIERNVPROC = void(APIENTRY *)(GLboolean synchronize);
	static PFNGLSHADINGRATEIMAGEBARRIERNVPROC pfnShadingRateImageBarrierNV{nullptr};
	LOAD_ENTRYPOINT("glShadingRateImageBarrierNV", pfnShadingRateImageBarrierNV, PFNGLSHADINGRATEIMAGEBARRIERNVPROC);
	pfnShadingRateImageBarrierNV(synchronize);
}

void glShadingRateImagePaletteNV(GLuint viewport, GLuint first, GLsizei count, const GLenum* rates)
{
	using PFNGLSHADINGRATEIMAGEPALETTENVPROC = void(APIENTRY *)(GLuint viewport, GLuint first, GLsizei count, const GLenum* rates);
	static PFNGLSHADINGRATEIMAGEPALETTENVPROC pfnShadingRateImagePaletteNV{nullptr};
	LOAD_ENTRYPOINT("glShadingRateImagePaletteNV", pfnShadingRateImagePaletteNV, PFNGLSHADINGRATEIMAGEPALETTENVPROC);
	pfnShadingRateImagePaletteNV(viewport, first, count, rates);
}

void glShadingRateSampleOrderNV(GLenum order)
{
	using PFNGLSHADINGRATESAMPLEORDERNVPROC = void(APIENTRY *)(GLenum order);
	static PFNGLSHADINGRATESAMPLEORDERNVPROC pfnShadingRateSampleOrderNV{nullptr};
	LOAD_ENTRYPOINT("glShadingRateSampleOrderNV", pfnShadingRateSampleOrderNV, PFNGLSHADINGRATESAMPLEORDERNVPROC);
	pfnShadingRateSampleOrderNV(order);
}

void glShadingRateSampleOrderCustomNV(GLenum rate, GLuint samples, const GLint* locations)
{
	using PFNGLSHADINGRATESAMPLEORDERCUSTOMNVPROC = void(APIENTRY *)(GLenum rate, GLuint samples, const GLint* locations);
	static PFNGLSHADINGRATESAMPLEORDERCUSTOMNVPROC pfnShadingRateSampleOrderCustomNV{nullptr};
	LOAD_ENTRYPOINT("glShadingRateSampleOrderCustomNV", pfnShadingRateSampleOrderCustomNV, PFNGLSHADINGRATESAMPLEORDERCUSTOMNVPROC);
	pfnShadingRateSampleOrderCustomNV(rate, samples, locations);
}

//
// GL_NV_texture_barrier
//

void glTextureBarrierNV(void)
{
	using PFNGLTEXTUREBARRIERNVPROC = void(APIENTRY *)(void);
	static PFNGLTEXTUREBARRIERNVPROC pfnTextureBarrierNV{nullptr};
	LOAD_ENTRYPOINT("glTextureBarrierNV", pfnTextureBarrierNV, PFNGLTEXTUREBARRIERNVPROC);
	pfnTextureBarrierNV();
}

//
// GL_NV_vertex_attrib_integer_64bit
//

void glVertexAttribL1i64NV(GLuint index, GLint64EXT x)
{
	using PFNGLVERTEXATTRIBL1I64NVPROC = void(APIENTRY *)(GLuint index, GLint64EXT x);
	static PFNGLVERTEXATTRIBL1I64NVPROC pfnVertexAttribL1i64NV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL1i64NV", pfnVertexAttribL1i64NV, PFNGLVERTEXATTRIBL1I64NVPROC);
	pfnVertexAttribL1i64NV(index, x);
}

void glVertexAttribL2i64NV(GLuint index, GLint64EXT x, GLint64EXT y)
{
	using PFNGLVERTEXATTRIBL2I64NVPROC = void(APIENTRY *)(GLuint index, GLint64EXT x, GLint64EXT y);
	static PFNGLVERTEXATTRIBL2I64NVPROC pfnVertexAttribL2i64NV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL2i64NV", pfnVertexAttribL2i64NV, PFNGLVERTEXATTRIBL2I64NVPROC);
	pfnVertexAttribL2i64NV(index, x, y);
}

void glVertexAttribL3i64NV(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z)
{
	using PFNGLVERTEXATTRIBL3I64NVPROC = void(APIENTRY *)(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z);
	static PFNGLVERTEXATTRIBL3I64NVPROC pfnVertexAttribL3i64NV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL3i64NV", pfnVertexAttribL3i64NV, PFNGLVERTEXATTRIBL3I64NVPROC);
	pfnVertexAttribL3i64NV(index, x, y, z);
}

void glVertexAttribL4i64NV(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)
{
	using PFNGLVERTEXATTRIBL4I64NVPROC = void(APIENTRY *)(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w);
	static PFNGLVERTEXATTRIBL4I64NVPROC pfnVertexAttribL4i64NV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL4i64NV", pfnVertexAttribL4i64NV, PFNGLVERTEXATTRIBL4I64NVPROC);
	pfnVertexAttribL4i64NV(index, x, y, z, w);
}

void glVertexAttribL1i64vNV(GLuint index, const GLint64EXT* v)
{
	using PFNGLVERTEXATTRIBL1I64VNVPROC = void(APIENTRY *)(GLuint index, const GLint64EXT* v);
	static PFNGLVERTEXATTRIBL1I64VNVPROC pfnVertexAttribL1i64vNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL1i64vNV", pfnVertexAttribL1i64vNV, PFNGLVERTEXATTRIBL1I64VNVPROC);
	pfnVertexAttribL1i64vNV(index, v);
}

void glVertexAttribL2i64vNV(GLuint index, const GLint64EXT* v)
{
	using PFNGLVERTEXATTRIBL2I64VNVPROC = void(APIENTRY *)(GLuint index, const GLint64EXT* v);
	static PFNGLVERTEXATTRIBL2I64VNVPROC pfnVertexAttribL2i64vNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL2i64vNV", pfnVertexAttribL2i64vNV, PFNGLVERTEXATTRIBL2I64VNVPROC);
	pfnVertexAttribL2i64vNV(index, v);
}

void glVertexAttribL3i64vNV(GLuint index, const GLint64EXT* v)
{
	using PFNGLVERTEXATTRIBL3I64VNVPROC = void(APIENTRY *)(GLuint index, const GLint64EXT* v);
	static PFNGLVERTEXATTRIBL3I64VNVPROC pfnVertexAttribL3i64vNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL3i64vNV", pfnVertexAttribL3i64vNV, PFNGLVERTEXATTRIBL3I64VNVPROC);
	pfnVertexAttribL3i64vNV(index, v);
}

void glVertexAttribL4i64vNV(GLuint index, const GLint64EXT* v)
{
	using PFNGLVERTEXATTRIBL4I64VNVPROC = void(APIENTRY *)(GLuint index, const GLint64EXT* v);
	static PFNGLVERTEXATTRIBL4I64VNVPROC pfnVertexAttribL4i64vNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL4i64vNV", pfnVertexAttribL4i64vNV, PFNGLVERTEXATTRIBL4I64VNVPROC);
	pfnVertexAttribL4i64vNV(index, v);
}

void glVertexAttribL1ui64NV(GLuint index, GLuint64EXT x)
{
	using PFNGLVERTEXATTRIBL1UI64NVPROC = void(APIENTRY *)(GLuint index, GLuint64EXT x);
	static PFNGLVERTEXATTRIBL1UI64NVPROC pfnVertexAttribL1ui64NV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL1ui64NV", pfnVertexAttribL1ui64NV, PFNGLVERTEXATTRIBL1UI64NVPROC);
	pfnVertexAttribL1ui64NV(index, x);
}

void glVertexAttribL2ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y)
{
	using PFNGLVERTEXATTRIBL2UI64NVPROC = void(APIENTRY *)(GLuint index, GLuint64EXT x, GLuint64EXT y);
	static PFNGLVERTEXATTRIBL2UI64NVPROC pfnVertexAttribL2ui64NV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL2ui64NV", pfnVertexAttribL2ui64NV, PFNGLVERTEXATTRIBL2UI64NVPROC);
	pfnVertexAttribL2ui64NV(index, x, y);
}

void glVertexAttribL3ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)
{
	using PFNGLVERTEXATTRIBL3UI64NVPROC = void(APIENTRY *)(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
	static PFNGLVERTEXATTRIBL3UI64NVPROC pfnVertexAttribL3ui64NV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL3ui64NV", pfnVertexAttribL3ui64NV, PFNGLVERTEXATTRIBL3UI64NVPROC);
	pfnVertexAttribL3ui64NV(index, x, y, z);
}

void glVertexAttribL4ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)
{
	using PFNGLVERTEXATTRIBL4UI64NVPROC = void(APIENTRY *)(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
	static PFNGLVERTEXATTRIBL4UI64NVPROC pfnVertexAttribL4ui64NV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL4ui64NV", pfnVertexAttribL4ui64NV, PFNGLVERTEXATTRIBL4UI64NVPROC);
	pfnVertexAttribL4ui64NV(index, x, y, z, w);
}

void glVertexAttribL1ui64vNV(GLuint index, const GLuint64EXT* v)
{
	using PFNGLVERTEXATTRIBL1UI64VNVPROC = void(APIENTRY *)(GLuint index, const GLuint64EXT* v);
	static PFNGLVERTEXATTRIBL1UI64VNVPROC pfnVertexAttribL1ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL1ui64vNV", pfnVertexAttribL1ui64vNV, PFNGLVERTEXATTRIBL1UI64VNVPROC);
	pfnVertexAttribL1ui64vNV(index, v);
}

void glVertexAttribL2ui64vNV(GLuint index, const GLuint64EXT* v)
{
	using PFNGLVERTEXATTRIBL2UI64VNVPROC = void(APIENTRY *)(GLuint index, const GLuint64EXT* v);
	static PFNGLVERTEXATTRIBL2UI64VNVPROC pfnVertexAttribL2ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL2ui64vNV", pfnVertexAttribL2ui64vNV, PFNGLVERTEXATTRIBL2UI64VNVPROC);
	pfnVertexAttribL2ui64vNV(index, v);
}

void glVertexAttribL3ui64vNV(GLuint index, const GLuint64EXT* v)
{
	using PFNGLVERTEXATTRIBL3UI64VNVPROC = void(APIENTRY *)(GLuint index, const GLuint64EXT* v);
	static PFNGLVERTEXATTRIBL3UI64VNVPROC pfnVertexAttribL3ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL3ui64vNV", pfnVertexAttribL3ui64vNV, PFNGLVERTEXATTRIBL3UI64VNVPROC);
	pfnVertexAttribL3ui64vNV(index, v);
}

void glVertexAttribL4ui64vNV(GLuint index, const GLuint64EXT* v)
{
	using PFNGLVERTEXATTRIBL4UI64VNVPROC = void(APIENTRY *)(GLuint index, const GLuint64EXT* v);
	static PFNGLVERTEXATTRIBL4UI64VNVPROC pfnVertexAttribL4ui64vNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribL4ui64vNV", pfnVertexAttribL4ui64vNV, PFNGLVERTEXATTRIBL4UI64VNVPROC);
	pfnVertexAttribL4ui64vNV(index, v);
}

void glGetVertexAttribLi64vNV(GLuint index, GLenum pname, GLint64EXT* params)
{
	using PFNGLGETVERTEXATTRIBLI64VNVPROC = void(APIENTRY *)(GLuint index, GLenum pname, GLint64EXT* params);
	static PFNGLGETVERTEXATTRIBLI64VNVPROC pfnGetVertexAttribLi64vNV{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribLi64vNV", pfnGetVertexAttribLi64vNV, PFNGLGETVERTEXATTRIBLI64VNVPROC);
	pfnGetVertexAttribLi64vNV(index, pname, params);
}

void glGetVertexAttribLui64vNV(GLuint index, GLenum pname, GLuint64EXT* params)
{
	using PFNGLGETVERTEXATTRIBLUI64VNVPROC = void(APIENTRY *)(GLuint index, GLenum pname, GLuint64EXT* params);
	static PFNGLGETVERTEXATTRIBLUI64VNVPROC pfnGetVertexAttribLui64vNV{nullptr};
	LOAD_ENTRYPOINT("glGetVertexAttribLui64vNV", pfnGetVertexAttribLui64vNV, PFNGLGETVERTEXATTRIBLUI64VNVPROC);
	pfnGetVertexAttribLui64vNV(index, pname, params);
}

void glVertexAttribLFormatNV(GLuint index, GLint size, GLenum type, GLsizei stride)
{
	using PFNGLVERTEXATTRIBLFORMATNVPROC = void(APIENTRY *)(GLuint index, GLint size, GLenum type, GLsizei stride);
	static PFNGLVERTEXATTRIBLFORMATNVPROC pfnVertexAttribLFormatNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribLFormatNV", pfnVertexAttribLFormatNV, PFNGLVERTEXATTRIBLFORMATNVPROC);
	pfnVertexAttribLFormatNV(index, size, type, stride);
}

//
// GL_NV_vertex_buffer_unified_memory
//

void glBufferAddressRangeNV(GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length)
{
	using PFNGLBUFFERADDRESSRANGENVPROC = void(APIENTRY *)(GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length);
	static PFNGLBUFFERADDRESSRANGENVPROC pfnBufferAddressRangeNV{nullptr};
	LOAD_ENTRYPOINT("glBufferAddressRangeNV", pfnBufferAddressRangeNV, PFNGLBUFFERADDRESSRANGENVPROC);
	pfnBufferAddressRangeNV(pname, index, address, length);
}

void glVertexFormatNV(GLint size, GLenum type, GLsizei stride)
{
	using PFNGLVERTEXFORMATNVPROC = void(APIENTRY *)(GLint size, GLenum type, GLsizei stride);
	static PFNGLVERTEXFORMATNVPROC pfnVertexFormatNV{nullptr};
	LOAD_ENTRYPOINT("glVertexFormatNV", pfnVertexFormatNV, PFNGLVERTEXFORMATNVPROC);
	pfnVertexFormatNV(size, type, stride);
}

void glNormalFormatNV(GLenum type, GLsizei stride)
{
	using PFNGLNORMALFORMATNVPROC = void(APIENTRY *)(GLenum type, GLsizei stride);
	static PFNGLNORMALFORMATNVPROC pfnNormalFormatNV{nullptr};
	LOAD_ENTRYPOINT("glNormalFormatNV", pfnNormalFormatNV, PFNGLNORMALFORMATNVPROC);
	pfnNormalFormatNV(type, stride);
}

void glColorFormatNV(GLint size, GLenum type, GLsizei stride)
{
	using PFNGLCOLORFORMATNVPROC = void(APIENTRY *)(GLint size, GLenum type, GLsizei stride);
	static PFNGLCOLORFORMATNVPROC pfnColorFormatNV{nullptr};
	LOAD_ENTRYPOINT("glColorFormatNV", pfnColorFormatNV, PFNGLCOLORFORMATNVPROC);
	pfnColorFormatNV(size, type, stride);
}

void glIndexFormatNV(GLenum type, GLsizei stride)
{
	using PFNGLINDEXFORMATNVPROC = void(APIENTRY *)(GLenum type, GLsizei stride);
	static PFNGLINDEXFORMATNVPROC pfnIndexFormatNV{nullptr};
	LOAD_ENTRYPOINT("glIndexFormatNV", pfnIndexFormatNV, PFNGLINDEXFORMATNVPROC);
	pfnIndexFormatNV(type, stride);
}

void glTexCoordFormatNV(GLint size, GLenum type, GLsizei stride)
{
	using PFNGLTEXCOORDFORMATNVPROC = void(APIENTRY *)(GLint size, GLenum type, GLsizei stride);
	static PFNGLTEXCOORDFORMATNVPROC pfnTexCoordFormatNV{nullptr};
	LOAD_ENTRYPOINT("glTexCoordFormatNV", pfnTexCoordFormatNV, PFNGLTEXCOORDFORMATNVPROC);
	pfnTexCoordFormatNV(size, type, stride);
}

void glEdgeFlagFormatNV(GLsizei stride)
{
	using PFNGLEDGEFLAGFORMATNVPROC = void(APIENTRY *)(GLsizei stride);
	static PFNGLEDGEFLAGFORMATNVPROC pfnEdgeFlagFormatNV{nullptr};
	LOAD_ENTRYPOINT("glEdgeFlagFormatNV", pfnEdgeFlagFormatNV, PFNGLEDGEFLAGFORMATNVPROC);
	pfnEdgeFlagFormatNV(stride);
}

void glSecondaryColorFormatNV(GLint size, GLenum type, GLsizei stride)
{
	using PFNGLSECONDARYCOLORFORMATNVPROC = void(APIENTRY *)(GLint size, GLenum type, GLsizei stride);
	static PFNGLSECONDARYCOLORFORMATNVPROC pfnSecondaryColorFormatNV{nullptr};
	LOAD_ENTRYPOINT("glSecondaryColorFormatNV", pfnSecondaryColorFormatNV, PFNGLSECONDARYCOLORFORMATNVPROC);
	pfnSecondaryColorFormatNV(size, type, stride);
}

void glFogCoordFormatNV(GLenum type, GLsizei stride)
{
	using PFNGLFOGCOORDFORMATNVPROC = void(APIENTRY *)(GLenum type, GLsizei stride);
	static PFNGLFOGCOORDFORMATNVPROC pfnFogCoordFormatNV{nullptr};
	LOAD_ENTRYPOINT("glFogCoordFormatNV", pfnFogCoordFormatNV, PFNGLFOGCOORDFORMATNVPROC);
	pfnFogCoordFormatNV(type, stride);
}

void glVertexAttribFormatNV(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride)
{
	using PFNGLVERTEXATTRIBFORMATNVPROC = void(APIENTRY *)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride);
	static PFNGLVERTEXATTRIBFORMATNVPROC pfnVertexAttribFormatNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribFormatNV", pfnVertexAttribFormatNV, PFNGLVERTEXATTRIBFORMATNVPROC);
	pfnVertexAttribFormatNV(index, size, type, normalized, stride);
}

void glVertexAttribIFormatNV(GLuint index, GLint size, GLenum type, GLsizei stride)
{
	using PFNGLVERTEXATTRIBIFORMATNVPROC = void(APIENTRY *)(GLuint index, GLint size, GLenum type, GLsizei stride);
	static PFNGLVERTEXATTRIBIFORMATNVPROC pfnVertexAttribIFormatNV{nullptr};
	LOAD_ENTRYPOINT("glVertexAttribIFormatNV", pfnVertexAttribIFormatNV, PFNGLVERTEXATTRIBIFORMATNVPROC);
	pfnVertexAttribIFormatNV(index, size, type, stride);
}

void glGetIntegerui64i_vNV(GLenum value, GLuint index, GLuint64EXT* result)
{
	using PFNGLGETINTEGERUI64I_VNVPROC = void(APIENTRY *)(GLenum value, GLuint index, GLuint64EXT* result);
	static PFNGLGETINTEGERUI64I_VNVPROC pfnGetIntegerui64i_vNV{nullptr};
	LOAD_ENTRYPOINT("glGetIntegerui64i_vNV", pfnGetIntegerui64i_vNV, PFNGLGETINTEGERUI64I_VNVPROC);
	pfnGetIntegerui64i_vNV(value, index, result);
}

//
// GL_NV_viewport_swizzle
//

void glViewportSwizzleNV(GLuint index, GLenum swizzlex, GLenum swizzley, GLenum swizzlez, GLenum swizzlew)
{
	using PFNGLVIEWPORTSWIZZLENVPROC = void(APIENTRY *)(GLuint index, GLenum swizzlex, GLenum swizzley, GLenum swizzlez, GLenum swizzlew);
	static PFNGLVIEWPORTSWIZZLENVPROC pfnViewportSwizzleNV{nullptr};
	LOAD_ENTRYPOINT("glViewportSwizzleNV", pfnViewportSwizzleNV, PFNGLVIEWPORTSWIZZLENVPROC);
	pfnViewportSwizzleNV(index, swizzlex, swizzley, swizzlez, swizzlew);
}

//
// GL_OVR_multiview
//

void glFramebufferTextureMultiviewOVR(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews)
{
	using PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC = void(APIENTRY *)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
	static PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC pfnFramebufferTextureMultiviewOVR{nullptr};
	LOAD_ENTRYPOINT("glFramebufferTextureMultiviewOVR", pfnFramebufferTextureMultiviewOVR, PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC);
	pfnFramebufferTextureMultiviewOVR(target, attachment, texture, level, baseViewIndex, numViews);
}

void glNamedFramebufferTextureMultiviewOVR(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews)
{
	using PFNGLNAMEDFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC = void(APIENTRY *)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
	static PFNGLNAMEDFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC pfnNamedFramebufferTextureMultiviewOVR{nullptr};
	LOAD_ENTRYPOINT("glNamedFramebufferTextureMultiviewOVR", pfnNamedFramebufferTextureMultiviewOVR, PFNGLNAMEDFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC);
	pfnNamedFramebufferTextureMultiviewOVR(framebuffer, attachment, texture, level, baseViewIndex, numViews);
}