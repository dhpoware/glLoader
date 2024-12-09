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
//
// OpenGL 4.6 API Core Profile.
// https://registry.khronos.org/OpenGL/api/GL/glcorearb.h
// 
// <GL/glcorearb.h> depends on the Khronos Shared Platform Header (<KHR/khrplatform.h>) from the EGL Registry.
// https://www.khronos.org/registry/EGL/api/KHR/khrplatform.h
// 
// This is a new dependency introduced in OpenGL Registry pull request 183 for increased compatibility between OpenGL and OpenGL ES headers.
//
// Copy <GL/glcorearb.h> to the same directory where <GL/gl.h> is located.
// Copy <KHR/khrplatform.h> to the same directory where <GL/glcorearb.h> is located.
//
// The following WGL extensions are supported:
//  WGL_ARB_create_context
//  WGL_EXT_swap_control

module;

#include <windows.h>
#include <GL/glcorearb.h>
#include <memory>

export module OpenGL;
import <string>;

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

    // Determines whether an OpenGL extension is supported.
    bool extensionSupported(const std::wstring extensionName);

    //
	// The following methods are replacements for the WGL functions in opengl32.dll:
    //

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

    //
    // WGL_ARB_create_context
    //

    HGLRC wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList);

    //
    // WGL_ARB_extensions_string
    //

    const char *wglGetExtensionsStringARB(HDC hDC);

    //
    // WGL_EXT_swap_control
    //

    BOOL wglSwapIntervalEXT(int interval);
	int wglGetSwapIntervalEXT(void);
            
private:
    OpenGLContext();
    
    static LRESULT CALLBACK dummyGLWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT CALLBACK dummyGLWndProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool createDummyGLWindow();
    void destroyDummyGLWindow() const;
    std::wstring toWideString(const char *pszStr) const;

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
    using PFNWGLCREATECONTEXTATTRIBSARBPROC = HGLRC(WINAPI*)(HDC hDC, HGLRC hShareContext, const int *attribList);
    using PFNWGLGETEXTENSIONSSTRINGARBPROC = const char *(WINAPI*)(HDC);
    using PFNWGLSWAPINTERVALEXTPROC = BOOL(WINAPI*)(int interval);
    using PFNWGLGETSWAPINTERVALEXTPROC = int(WINAPI*)(void);
    
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
    PFNWGLCREATECONTEXTATTRIBSARBPROC m_pfnWglCreateContextAttribsARB{nullptr};
    PFNWGLGETEXTENSIONSSTRINGARBPROC m_pfnWglGetExtensionsStringARB{nullptr};
    PFNWGLSWAPINTERVALEXTPROC m_pfnWglSwapIntervalEXT{nullptr};
    PFNWGLGETSWAPINTERVALEXTPROC m_pfnWglGetSwapIntervalEXT{nullptr};
    
    WNDCLASSEXW m_wcl{};
    HWND m_hWnd{nullptr};
    HDC m_hDC{nullptr};
    HGLRC m_hRC{nullptr};
    std::wstring m_extensions{};
};

extern "C"
{
	// This macro returns the byte offset into a buffer object in machine units. Used primarily for Buffer Objects.
	export constexpr GLubyte *BUFFER_OFFSET(size_t bytes) { return reinterpret_cast<GLubyte *>(0) + bytes; }

    //
    // WGL_ARB_create_context
    //

    export constexpr auto ERROR_INVALID_PROFILE_ARB					{0x2096};
	export constexpr auto ERROR_INVALID_VERSION_ARB					{0x2095};
	export constexpr auto WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB	{0x00000002};
	export constexpr auto WGL_CONTEXT_CORE_PROFILE_BIT_ARB			{0x00000001};
	export constexpr auto WGL_CONTEXT_DEBUG_BIT_ARB					{0x0001};
	export constexpr auto WGL_CONTEXT_FLAGS_ARB						{0x2094};
	export constexpr auto WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB	{0x0002};
	export constexpr auto WGL_CONTEXT_LAYER_PLANE_ARB				{0x2093};
	export constexpr auto WGL_CONTEXT_MAJOR_VERSION_ARB				{0x2091};
	export constexpr auto WGL_CONTEXT_MINOR_VERSION_ARB				{0x2092};
	export constexpr auto WGL_CONTEXT_PROFILE_MASK_ARB				{0x9126};

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

	//
    // GL_VERSION_1_2
    //

    export void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices);
    export void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
    export void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
    export void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

    //
    // GL_VERSION_1_3
    //

    export void glActiveTexture(GLenum texture);
    export void glSampleCoverage(GLfloat value, GLboolean invert);
    export void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data);
    export void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);
    export void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* data);
    export void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);
    export void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
    export void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data);
    export void glGetCompressedTexImage(GLenum target, GLint level, void* img);

    //
    // GL_VERSION_1_4
    //

    export void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
    export void glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount);
    export void glMultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount);
    export void glPointParameterf(GLenum pname, GLfloat param);
    export void glPointParameterfv(GLenum pname, const GLfloat* params);
    export void glPointParameteri(GLenum pname, GLint param);
    export void glPointParameteriv(GLenum pname, const GLint* params);
    export void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    export void glBlendEquation(GLenum mode);

    //
    // GL_VERSION_1_5
    //

    export void glGenQueries(GLsizei n, GLuint* ids);
    export void glDeleteQueries(GLsizei n, const GLuint* ids);
    export GLboolean glIsQuery(GLuint id);
    export void glBeginQuery(GLenum target, GLuint id);
    export void glEndQuery(GLenum target);
    export void glGetQueryiv(GLenum target, GLenum pname, GLint* params);
    export void glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params);
    export void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params);
    export void glBindBuffer(GLenum target, GLuint buffer);
    export void glDeleteBuffers(GLsizei n, const GLuint* buffers);
    export void glGenBuffers(GLsizei n, GLuint* buffers);
    export GLboolean glIsBuffer(GLuint buffer);
    export void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    export void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
    export void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data);
    export void* glMapBuffer(GLenum target, GLenum access);
    export GLboolean glUnmapBuffer(GLenum target);
    export void glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params);
    export void glGetBufferPointerv(GLenum target, GLenum pname, void** params);

    //
    // GL_VERSION_2_0
    //

    export void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
    export void glDrawBuffers(GLsizei n, const GLenum* bufs);
    export void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
    export void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
    export void glStencilMaskSeparate(GLenum face, GLuint mask);
    export void glAttachShader(GLuint program, GLuint shader);
    export void glBindAttribLocation(GLuint program, GLuint index, const GLchar* name);
    export void glCompileShader(GLuint shader);
    export GLuint glCreateProgram(void);
    export GLuint glCreateShader(GLenum type);
    export void glDeleteProgram(GLuint program);
    export void glDeleteShader(GLuint shader);
    export void glDetachShader(GLuint program, GLuint shader);
    export void glDisableVertexAttribArray(GLuint index);
    export void glEnableVertexAttribArray(GLuint index);
    export void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
    export void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
    export void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders);
    export GLint glGetAttribLocation(GLuint program, const GLchar* name);
    export void glGetProgramiv(GLuint program, GLenum pname, GLint* params);
    export void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    export void glGetShaderiv(GLuint shader, GLenum pname, GLint* params);
    export void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    export void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source);
    export GLint glGetUniformLocation(GLuint program, const GLchar* name);
    export void glGetUniformfv(GLuint program, GLint location, GLfloat* params);
    export void glGetUniformiv(GLuint program, GLint location, GLint* params);
    export void glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble* params);
    export void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params);
    export void glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params);
    export void glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer);
    export GLboolean glIsProgram(GLuint program);
    export GLboolean glIsShader(GLuint shader);
    export void glLinkProgram(GLuint program);
    export void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
    export void glUseProgram(GLuint program);
    export void glUniform1f(GLint location, GLfloat v0);
    export void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
    export void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    export void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    export void glUniform1i(GLint location, GLint v0);
    export void glUniform2i(GLint location, GLint v0, GLint v1);
    export void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
    export void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    export void glUniform1fv(GLint location, GLsizei count, const GLfloat* value);
    export void glUniform2fv(GLint location, GLsizei count, const GLfloat* value);
    export void glUniform3fv(GLint location, GLsizei count, const GLfloat* value);
    export void glUniform4fv(GLint location, GLsizei count, const GLfloat* value);
    export void glUniform1iv(GLint location, GLsizei count, const GLint* value);
    export void glUniform2iv(GLint location, GLsizei count, const GLint* value);
    export void glUniform3iv(GLint location, GLsizei count, const GLint* value);
    export void glUniform4iv(GLint location, GLsizei count, const GLint* value);
    export void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glValidateProgram(GLuint program);
    export void glVertexAttrib1d(GLuint index, GLdouble x);
    export void glVertexAttrib1dv(GLuint index, const GLdouble* v);
    export void glVertexAttrib1f(GLuint index, GLfloat x);
    export void glVertexAttrib1fv(GLuint index, const GLfloat* v);
    export void glVertexAttrib1s(GLuint index, GLshort x);
    export void glVertexAttrib1sv(GLuint index, const GLshort* v);
    export void glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y);
    export void glVertexAttrib2dv(GLuint index, const GLdouble* v);
    export void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y);
    export void glVertexAttrib2fv(GLuint index, const GLfloat* v);
    export void glVertexAttrib2s(GLuint index, GLshort x, GLshort y);
    export void glVertexAttrib2sv(GLuint index, const GLshort* v);
    export void glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    export void glVertexAttrib3dv(GLuint index, const GLdouble* v);
    export void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
    export void glVertexAttrib3fv(GLuint index, const GLfloat* v);
    export void glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z);
    export void glVertexAttrib3sv(GLuint index, const GLshort* v);
    export void glVertexAttrib4Nbv(GLuint index, const GLbyte* v);
    export void glVertexAttrib4Niv(GLuint index, const GLint* v);
    export void glVertexAttrib4Nsv(GLuint index, const GLshort* v);
    export void glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    export void glVertexAttrib4Nubv(GLuint index, const GLubyte* v);
    export void glVertexAttrib4Nuiv(GLuint index, const GLuint* v);
    export void glVertexAttrib4Nusv(GLuint index, const GLushort* v);
    export void glVertexAttrib4bv(GLuint index, const GLbyte* v);
    export void glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    export void glVertexAttrib4dv(GLuint index, const GLdouble* v);
    export void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    export void glVertexAttrib4fv(GLuint index, const GLfloat* v);
    export void glVertexAttrib4iv(GLuint index, const GLint* v);
    export void glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    export void glVertexAttrib4sv(GLuint index, const GLshort* v);
    export void glVertexAttrib4ubv(GLuint index, const GLubyte* v);
    export void glVertexAttrib4uiv(GLuint index, const GLuint* v);
    export void glVertexAttrib4usv(GLuint index, const GLushort* v);
    export void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

    //
    // GL_VERSION_2_1
    //

    export void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

    //
    // GL_VERSION_3_0
    //

    export void glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
    export void glGetBooleani_v(GLenum target, GLuint index, GLboolean* data);
    export void glGetIntegeri_v(GLenum target, GLuint index, GLint* data);
    export void glEnablei(GLenum target, GLuint index);
    export void glDisablei(GLenum target, GLuint index);
    export GLboolean glIsEnabledi(GLenum target, GLuint index);
    export void glBeginTransformFeedback(GLenum primitiveMode);
    export void glEndTransformFeedback(void);
    export void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    export void glBindBufferBase(GLenum target, GLuint index, GLuint buffer);
    export void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);
    export void glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name);
    export void glClampColor(GLenum target, GLenum clamp);
    export void glBeginConditionalRender(GLuint id, GLenum mode);
    export void glEndConditionalRender(void);
    export void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
    export void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint* params);
    export void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params);
    export void glVertexAttribI1i(GLuint index, GLint x);
    export void glVertexAttribI2i(GLuint index, GLint x, GLint y);
    export void glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z);
    export void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w);
    export void glVertexAttribI1ui(GLuint index, GLuint x);
    export void glVertexAttribI2ui(GLuint index, GLuint x, GLuint y);
    export void glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z);
    export void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
    export void glVertexAttribI1iv(GLuint index, const GLint* v);
    export void glVertexAttribI2iv(GLuint index, const GLint* v);
    export void glVertexAttribI3iv(GLuint index, const GLint* v);
    export void glVertexAttribI4iv(GLuint index, const GLint* v);
    export void glVertexAttribI1uiv(GLuint index, const GLuint* v);
    export void glVertexAttribI2uiv(GLuint index, const GLuint* v);
    export void glVertexAttribI3uiv(GLuint index, const GLuint* v);
    export void glVertexAttribI4uiv(GLuint index, const GLuint* v);
    export void glVertexAttribI4bv(GLuint index, const GLbyte* v);
    export void glVertexAttribI4sv(GLuint index, const GLshort* v);
    export void glVertexAttribI4ubv(GLuint index, const GLubyte* v);
    export void glVertexAttribI4usv(GLuint index, const GLushort* v);
    export void glGetUniformuiv(GLuint program, GLint location, GLuint* params);
    export void glBindFragDataLocation(GLuint program, GLuint color, const GLchar* name);
    export GLint glGetFragDataLocation(GLuint program, const GLchar* name);
    export void glUniform1ui(GLint location, GLuint v0);
    export void glUniform2ui(GLint location, GLuint v0, GLuint v1);
    export void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2);
    export void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    export void glUniform1uiv(GLint location, GLsizei count, const GLuint* value);
    export void glUniform2uiv(GLint location, GLsizei count, const GLuint* value);
    export void glUniform3uiv(GLint location, GLsizei count, const GLuint* value);
    export void glUniform4uiv(GLint location, GLsizei count, const GLuint* value);
    export void glTexParameterIiv(GLenum target, GLenum pname, const GLint* params);
    export void glTexParameterIuiv(GLenum target, GLenum pname, const GLuint* params);
    export void glGetTexParameterIiv(GLenum target, GLenum pname, GLint* params);
    export void glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint* params);
    export void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value);
    export void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value);
    export void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value);
    export void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
    export const GLubyte* glGetStringi(GLenum name, GLuint index);
    export GLboolean glIsRenderbuffer(GLuint renderbuffer);
    export void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
    export void glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
    export void glGenRenderbuffers(GLsizei n, GLuint* renderbuffers);
    export void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    export void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params);
    export GLboolean glIsFramebuffer(GLuint framebuffer);
    export void glBindFramebuffer(GLenum target, GLuint framebuffer);
    export void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
    export void glGenFramebuffers(GLsizei n, GLuint* framebuffers);
    export GLenum glCheckFramebufferStatus(GLenum target);
    export void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    export void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    export void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    export void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    export void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params);
    export void glGenerateMipmap(GLenum target);
    export void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    export void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    export void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
    export void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
    export void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
    export void glBindVertexArray(GLuint array);
    export void glDeleteVertexArrays(GLsizei n, const GLuint* arrays);
    export void glGenVertexArrays(GLsizei n, GLuint* arrays);
    export GLboolean glIsVertexArray(GLuint array);

    //
    // GL_VERSION_3_1
    //

    export void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
    export void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
    export void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer);
    export void glPrimitiveRestartIndex(GLuint index);
    export void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    export void glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices);
    export void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params);
    export void glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformName);
    export GLuint glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName);
    export void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
    export void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
    export void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);

    //
    // GL_VERSION_3_2
    //

    export void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex);
    export void glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices, GLint basevertex);
    export void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex);
    export void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei drawcount, const GLint* basevertex);
    export void glProvokingVertex(GLenum mode);
    export GLsync glFenceSync(GLenum condition, GLbitfield flags);
    export GLboolean glIsSync(GLsync sync);
    export void glDeleteSync(GLsync sync);
    export GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
    export void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
    export void glGetInteger64v(GLenum pname, GLint64* data);
    export void glGetSynciv(GLsync sync, GLenum pname, GLsizei count, GLsizei* length, GLint* values);
    export void glGetInteger64i_v(GLenum target, GLuint index, GLint64* data);
    export void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params);
    export void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
    export void glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    export void glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    export void glGetMultisamplefv(GLenum pname, GLuint index, GLfloat* val);
    export void glSampleMaski(GLuint maskNumber, GLbitfield mask);

    //
    // GL_VERSION_3_3
    //

    export void glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar* name);
    export GLint glGetFragDataIndex(GLuint program, const GLchar* name);
    export void glGenSamplers(GLsizei count, GLuint* samplers);
    export void glDeleteSamplers(GLsizei count, const GLuint* samplers);
    export GLboolean glIsSampler(GLuint sampler);
    export void glBindSampler(GLuint unit, GLuint sampler);
    export void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param);
    export void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param);
    export void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
    export void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param);
    export void glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint* param);
    export void glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint* param);
    export void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params);
    export void glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint* params);
    export void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params);
    export void glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint* params);
    export void glQueryCounter(GLuint id, GLenum target);
    export void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params);
    export void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params);
    export void glVertexAttribDivisor(GLuint index, GLuint divisor);
    export void glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    export void glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
    export void glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    export void glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
    export void glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    export void glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);
    export void glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    export void glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint* value);

    //
    // GL_VERSION_4_0
    //

    export void glMinSampleShading(GLfloat value);
    export void glBlendEquationi(GLuint buf, GLenum mode);
    export void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
    export void glBlendFunci(GLuint buf, GLenum src, GLenum dst);
    export void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
    export void glDrawArraysIndirect(GLenum mode, const void* indirect);
    export void glDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect);
    export void glUniform1d(GLint location, GLdouble x);
    export void glUniform2d(GLint location, GLdouble x, GLdouble y);
    export void glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z);
    export void glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    export void glUniform1dv(GLint location, GLsizei count, const GLdouble* value);
    export void glUniform2dv(GLint location, GLsizei count, const GLdouble* value);
    export void glUniform3dv(GLint location, GLsizei count, const GLdouble* value);
    export void glUniform4dv(GLint location, GLsizei count, const GLdouble* value);
    export void glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glGetUniformdv(GLuint program, GLint location, GLdouble* params);
    export GLint glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar* name);
    export GLuint glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar* name);
    export void glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint* values);
    export void glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name);
    export void glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name);
    export void glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint* indices);
    export void glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint* params);
    export void glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint* values);
    export void glPatchParameteri(GLenum pname, GLint value);
    export void glPatchParameterfv(GLenum pname, const GLfloat* values);
    export void glBindTransformFeedback(GLenum target, GLuint id);
    export void glDeleteTransformFeedbacks(GLsizei n, const GLuint* ids);
    export void glGenTransformFeedbacks(GLsizei n, GLuint* ids);
    export GLboolean glIsTransformFeedback(GLuint id);
    export void glPauseTransformFeedback(void);
    export void glResumeTransformFeedback(void);
    export void glDrawTransformFeedback(GLenum mode, GLuint id);
    export void glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream);
    export void glBeginQueryIndexed(GLenum target, GLuint index, GLuint id);
    export void glEndQueryIndexed(GLenum target, GLuint index);
    export void glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint* params);

    //
    // GL_VERSION_4_1
    //

    export void glReleaseShaderCompiler(void);
    export void glShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryFormat, const void* binary, GLsizei length);
    export void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
    export void glDepthRangef(GLfloat n, GLfloat f);
    export void glClearDepthf(GLfloat d);
    export void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary);
    export void glProgramBinary(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length);
    export void glProgramParameteri(GLuint program, GLenum pname, GLint value);
    export void glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program);
    export void glActiveShaderProgram(GLuint pipeline, GLuint program);
    export GLuint glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar* const* strings);
    export void glBindProgramPipeline(GLuint pipeline);
    export void glDeleteProgramPipelines(GLsizei n, const GLuint* pipelines);
    export void glGenProgramPipelines(GLsizei n, GLuint* pipelines);
    export GLboolean glIsProgramPipeline(GLuint pipeline);
    export void glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint* params);
    export void glProgramUniform1i(GLuint program, GLint location, GLint v0);
    export void glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint* value);
    export void glProgramUniform1f(GLuint program, GLint location, GLfloat v0);
    export void glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    export void glProgramUniform1d(GLuint program, GLint location, GLdouble v0);
    export void glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    export void glProgramUniform1ui(GLuint program, GLint location, GLuint v0);
    export void glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint* value);
    export void glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1);
    export void glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint* value);
    export void glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1);
    export void glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    export void glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1);
    export void glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    export void glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1);
    export void glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint* value);
    export void glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
    export void glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint* value);
    export void glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    export void glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    export void glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
    export void glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    export void glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
    export void glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint* value);
    export void glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    export void glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint* value);
    export void glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    export void glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    export void glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
    export void glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    export void glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    export void glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint* value);
    export void glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glValidateProgramPipeline(GLuint pipeline);
    export void glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    export void glVertexAttribL1d(GLuint index, GLdouble x);
    export void glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y);
    export void glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    export void glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    export void glVertexAttribL1dv(GLuint index, const GLdouble* v);
    export void glVertexAttribL2dv(GLuint index, const GLdouble* v);
    export void glVertexAttribL3dv(GLuint index, const GLdouble* v);
    export void glVertexAttribL4dv(GLuint index, const GLdouble* v);
    export void glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
    export void glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble* params);
    export void glViewportArrayv(GLuint first, GLsizei count, const GLfloat* v);
    export void glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
    export void glViewportIndexedfv(GLuint index, const GLfloat* v);
    export void glScissorArrayv(GLuint first, GLsizei count, const GLint* v);
    export void glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
    export void glScissorIndexedv(GLuint index, const GLint* v);
    export void glDepthRangeArrayv(GLuint first, GLsizei count, const GLdouble* v);
    export void glDepthRangeIndexed(GLuint index, GLdouble n, GLdouble f);
    export void glGetFloati_v(GLenum target, GLuint index, GLfloat* data);
    export void glGetDoublei_v(GLenum target, GLuint index, GLdouble* data);

    //
    // GL_VERSION_4_2
    //

    export void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);
    export void glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLuint baseinstance);
    export void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
    export void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint* params);
    export void glGetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex, GLenum pname, GLint* params);
    export void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
    export void glMemoryBarrier(GLbitfield barriers);
    export void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
    export void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    export void glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    export void glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei instancecount);
    export void glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);

    //
    // GL_VERSION_4_3
    //

    export void glClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void* data);
    export void glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);
    export void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
    export void glDispatchComputeIndirect(GLintptr indirect);
    export void glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
    export void glFramebufferParameteri(GLenum target, GLenum pname, GLint param);
    export void glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint* params);
    export void glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64* params);
    export void glInvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
    export void glInvalidateTexImage(GLuint texture, GLint level);
    export void glInvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length);
    export void glInvalidateBufferData(GLuint buffer);
    export void glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments);
    export void glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
    export void glMultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride);
    export void glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride);
    export void glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint* params);
    export GLuint glGetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar* name);
    export void glGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name);
    export void glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei count, GLsizei* length, GLint* params);
    export GLint glGetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar* name);
    export GLint glGetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar* name);
    export void glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
    export void glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    export void glTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    export void glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    export void glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
    export void glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    export void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    export void glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    export void glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    export void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex);
    export void glVertexBindingDivisor(GLuint bindingindex, GLuint divisor);
    export void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
    export void glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
    export void glDebugMessageCallback(GLDEBUGPROC callback, const void* userParam);
    export GLuint glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
    export void glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar* message);
    export void glPopDebugGroup(void);
    export void glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label);
    export void glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label);
    export void glObjectPtrLabel(const void* ptr, GLsizei length, const GLchar* label);
    export void glGetObjectPtrLabel(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label);

    //
    // GL_VERSION_4_4
    //

    export void glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags);
    export void glClearTexImage(GLuint texture, GLint level, GLenum format, GLenum type, const void* data);
    export void glClearTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* data);
    export void glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint* buffers);
    export void glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizeiptr* sizes);
    export void glBindTextures(GLuint first, GLsizei count, const GLuint* textures);
    export void glBindSamplers(GLuint first, GLsizei count, const GLuint* samplers);
    export void glBindImageTextures(GLuint first, GLsizei count, const GLuint* textures);
    export void glBindVertexBuffers(GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides);

    //
    // GL_VERSION_4_5
    //

    export void glClipControl(GLenum origin, GLenum depth);
    export void glCreateTransformFeedbacks(GLsizei n, GLuint* ids);
    export void glTransformFeedbackBufferBase(GLuint xfb, GLuint index, GLuint buffer);
    export void glTransformFeedbackBufferRange(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    export void glGetTransformFeedbackiv(GLuint xfb, GLenum pname, GLint* param);
    export void glGetTransformFeedbacki_v(GLuint xfb, GLenum pname, GLuint index, GLint* param);
    export void glGetTransformFeedbacki64_v(GLuint xfb, GLenum pname, GLuint index, GLint64* param);
    export void glCreateBuffers(GLsizei n, GLuint* buffers);
    export void glNamedBufferStorage(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags);
    export void glNamedBufferData(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage);
    export void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data);
    export void glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    export void glClearNamedBufferData(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data);
    export void glClearNamedBufferSubData(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);
    export void* glMapNamedBuffer(GLuint buffer, GLenum access);
    export void* glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
    export GLboolean glUnmapNamedBuffer(GLuint buffer);
    export void glFlushMappedNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length);
    export void glGetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint* params);
    export void glGetNamedBufferParameteri64v(GLuint buffer, GLenum pname, GLint64* params);
    export void glGetNamedBufferPointerv(GLuint buffer, GLenum pname, void** params);
    export void glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data);
    export void glCreateFramebuffers(GLsizei n, GLuint* framebuffers);
    export void glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    export void glNamedFramebufferParameteri(GLuint framebuffer, GLenum pname, GLint param);
    export void glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
    export void glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
    export void glNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buf);
    export void glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum* bufs);
    export void glNamedFramebufferReadBuffer(GLuint framebuffer, GLenum src);
    export void glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments);
    export void glInvalidateNamedFramebufferSubData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height);
    export void glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint* value);
    export void glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint* value);
    export void glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat* value);
    export void glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
    export void glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    export GLenum glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target);
    export void glGetNamedFramebufferParameteriv(GLuint framebuffer, GLenum pname, GLint* param);
    export void glGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params);
    export void glCreateRenderbuffers(GLsizei n, GLuint* renderbuffers);
    export void glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
    export void glNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    export void glGetNamedRenderbufferParameteriv(GLuint renderbuffer, GLenum pname, GLint* params);
    export void glCreateTextures(GLenum target, GLsizei n, GLuint* textures);
    export void glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer);
    export void glTextureBufferRange(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    export void glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
    export void glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    export void glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    export void glTextureStorage2DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    export void glTextureStorage3DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    export void glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
    export void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
    export void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
    export void glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* data);
    export void glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
    export void glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);
    export void glCopyTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    export void glCopyTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    export void glCopyTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    export void glTextureParameterf(GLuint texture, GLenum pname, GLfloat param);
    export void glTextureParameterfv(GLuint texture, GLenum pname, const GLfloat* param);
    export void glTextureParameteri(GLuint texture, GLenum pname, GLint param);
    export void glTextureParameterIiv(GLuint texture, GLenum pname, const GLint* params);
    export void glTextureParameterIuiv(GLuint texture, GLenum pname, const GLuint* params);
    export void glTextureParameteriv(GLuint texture, GLenum pname, const GLint* param);
    export void glGenerateTextureMipmap(GLuint texture);
    export void glBindTextureUnit(GLuint unit, GLuint texture);
    export void glGetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels);
    export void glGetCompressedTextureImage(GLuint texture, GLint level, GLsizei bufSize, void* pixels);
    export void glGetTextureLevelParameterfv(GLuint texture, GLint level, GLenum pname, GLfloat* params);
    export void glGetTextureLevelParameteriv(GLuint texture, GLint level, GLenum pname, GLint* params);
    export void glGetTextureParameterfv(GLuint texture, GLenum pname, GLfloat* params);
    export void glGetTextureParameterIiv(GLuint texture, GLenum pname, GLint* params);
    export void glGetTextureParameterIuiv(GLuint texture, GLenum pname, GLuint* params);
    export void glGetTextureParameteriv(GLuint texture, GLenum pname, GLint* params);
    export void glCreateVertexArrays(GLsizei n, GLuint* arrays);
    export void glDisableVertexArrayAttrib(GLuint vaobj, GLuint index);
    export void glEnableVertexArrayAttrib(GLuint vaobj, GLuint index);
    export void glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer);
    export void glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    export void glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint* buffers, const GLintptr* offsets, const GLsizei* strides);
    export void glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
    export void glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    export void glVertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    export void glVertexArrayAttribLFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    export void glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor);
    export void glGetVertexArrayiv(GLuint vaobj, GLenum pname, GLint* param);
    export void glGetVertexArrayIndexediv(GLuint vaobj, GLuint index, GLenum pname, GLint* param);
    export void glGetVertexArrayIndexed64iv(GLuint vaobj, GLuint index, GLenum pname, GLint64* param);
    export void glCreateSamplers(GLsizei n, GLuint* samplers);
    export void glCreateProgramPipelines(GLsizei n, GLuint* pipelines);
    export void glCreateQueries(GLenum target, GLsizei n, GLuint* ids);
    export void glGetQueryBufferObjecti64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
    export void glGetQueryBufferObjectiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
    export void glGetQueryBufferObjectui64v(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
    export void glGetQueryBufferObjectuiv(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);
    export void glMemoryBarrierByRegion(GLbitfield barriers);
    export void glGetTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void* pixels);
    export void glGetCompressedTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void* pixels);
    export GLenum glGetGraphicsResetStatus(void);
    export void glGetnCompressedTexImage(GLenum target, GLint lod, GLsizei bufSize, void* pixels);
    export void glGetnTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* pixels);
    export void glGetnUniformdv(GLuint program, GLint location, GLsizei bufSize, GLdouble* params);
    export void glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat* params);
    export void glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint* params);
    export void glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint* params);
    export void glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data);
    export void glTextureBarrier(void);

    //
    // GL_VERSION_4_6
    //

    export void glSpecializeShader(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue);
    export void glMultiDrawArraysIndirectCount(GLenum mode, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
    export void glMultiDrawElementsIndirectCount(GLenum mode, GLenum type, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
    export void glPolygonOffsetClamp(GLfloat factor, GLfloat units, GLfloat clamp);

    //
    // GL_ARB_ES3_2_compatibility
    //

    export void glPrimitiveBoundingBoxARB(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);

    //
    // GL_ARB_bindless_texture
    //

    export GLuint64 glGetTextureHandleARB(GLuint texture);
    export GLuint64 glGetTextureSamplerHandleARB(GLuint texture, GLuint sampler);
    export void glMakeTextureHandleResidentARB(GLuint64 handle);
    export void glMakeTextureHandleNonResidentARB(GLuint64 handle);
    export GLuint64 glGetImageHandleARB(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format);
    export void glMakeImageHandleResidentARB(GLuint64 handle, GLenum access);
    export void glMakeImageHandleNonResidentARB(GLuint64 handle);
    export void glUniformHandleui64ARB(GLint location, GLuint64 value);
    export void glUniformHandleui64vARB(GLint location, GLsizei count, const GLuint64* value);
    export void glProgramUniformHandleui64ARB(GLuint program, GLint location, GLuint64 value);
    export void glProgramUniformHandleui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* values);
    export GLboolean glIsTextureHandleResidentARB(GLuint64 handle);
    export GLboolean glIsImageHandleResidentARB(GLuint64 handle);
    export void glVertexAttribL1ui64ARB(GLuint index, GLuint64EXT x);
    export void glVertexAttribL1ui64vARB(GLuint index, const GLuint64EXT* v);
    export void glGetVertexAttribLui64vARB(GLuint index, GLenum pname, GLuint64EXT* params);

    //
    // GL_ARB_cl_event
    //

    export GLsync glCreateSyncFromCLeventARB(struct _cl_context* context, struct _cl_event* event, GLbitfield flags);

    //
    // GL_ARB_compute_variable_group_size
    //

    export void glDispatchComputeGroupSizeARB(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z);

    //
    // GL_ARB_debug_output
    //

    export void glDebugMessageControlARB(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
    export void glDebugMessageInsertARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
    export void glDebugMessageCallbackARB(GLDEBUGPROCARB callback, const void* userParam);
    export GLuint glGetDebugMessageLogARB(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);

    //
    // GL_ARB_draw_buffers_blend
    //

    export void glBlendEquationiARB(GLuint buf, GLenum mode);
    export void glBlendEquationSeparateiARB(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
    export void glBlendFunciARB(GLuint buf, GLenum src, GLenum dst);
    export void glBlendFuncSeparateiARB(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

    //
    // GL_ARB_draw_instanced
    //

    export void glDrawArraysInstancedARB(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
    export void glDrawElementsInstancedARB(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount);

    //
    // GL_ARB_geometry_shader4
    //

    export void glProgramParameteriARB(GLuint program, GLenum pname, GLint value);
    export void glFramebufferTextureARB(GLenum target, GLenum attachment, GLuint texture, GLint level);
    export void glFramebufferTextureLayerARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
    export void glFramebufferTextureFaceARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);

    //
    // GL_ARB_gl_spirv
    //

    export void glSpecializeShaderARB(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue);

    //
    // GL_ARB_gpu_shader_int64
    //

    export void glUniform1i64ARB(GLint location, GLint64 x);
    export void glUniform2i64ARB(GLint location, GLint64 x, GLint64 y);
    export void glUniform3i64ARB(GLint location, GLint64 x, GLint64 y, GLint64 z);
    export void glUniform4i64ARB(GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w);
    export void glUniform1i64vARB(GLint location, GLsizei count, const GLint64* value);
    export void glUniform2i64vARB(GLint location, GLsizei count, const GLint64* value);
    export void glUniform3i64vARB(GLint location, GLsizei count, const GLint64* value);
    export void glUniform4i64vARB(GLint location, GLsizei count, const GLint64* value);
    export void glUniform1ui64ARB(GLint location, GLuint64 x);
    export void glUniform2ui64ARB(GLint location, GLuint64 x, GLuint64 y);
    export void glUniform3ui64ARB(GLint location, GLuint64 x, GLuint64 y, GLuint64 z);
    export void glUniform4ui64ARB(GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w);
    export void glUniform1ui64vARB(GLint location, GLsizei count, const GLuint64* value);
    export void glUniform2ui64vARB(GLint location, GLsizei count, const GLuint64* value);
    export void glUniform3ui64vARB(GLint location, GLsizei count, const GLuint64* value);
    export void glUniform4ui64vARB(GLint location, GLsizei count, const GLuint64* value);
    export void glGetUniformi64vARB(GLuint program, GLint location, GLint64* params);
    export void glGetUniformui64vARB(GLuint program, GLint location, GLuint64* params);
    export void glGetnUniformi64vARB(GLuint program, GLint location, GLsizei bufSize, GLint64* params);
    export void glGetnUniformui64vARB(GLuint program, GLint location, GLsizei bufSize, GLuint64* params);
    export void glProgramUniform1i64ARB(GLuint program, GLint location, GLint64 x);
    export void glProgramUniform2i64ARB(GLuint program, GLint location, GLint64 x, GLint64 y);
    export void glProgramUniform3i64ARB(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z);
    export void glProgramUniform4i64ARB(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w);
    export void glProgramUniform1i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value);
    export void glProgramUniform2i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value);
    export void glProgramUniform3i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value);
    export void glProgramUniform4i64vARB(GLuint program, GLint location, GLsizei count, const GLint64* value);
    export void glProgramUniform1ui64ARB(GLuint program, GLint location, GLuint64 x);
    export void glProgramUniform2ui64ARB(GLuint program, GLint location, GLuint64 x, GLuint64 y);
    export void glProgramUniform3ui64ARB(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z);
    export void glProgramUniform4ui64ARB(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w);
    export void glProgramUniform1ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value);
    export void glProgramUniform2ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value);
    export void glProgramUniform3ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value);
    export void glProgramUniform4ui64vARB(GLuint program, GLint location, GLsizei count, const GLuint64* value);

    //
    // GL_ARB_indirect_parameters
    //

    export void glMultiDrawArraysIndirectCountARB(GLenum mode, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);
    export void glMultiDrawElementsIndirectCountARB(GLenum mode, GLenum type, const void* indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);

    //
    // GL_ARB_instanced_arrays
    //

    export void glVertexAttribDivisorARB(GLuint index, GLuint divisor);

    //
    // GL_ARB_parallel_shader_compile
    //

    export void glMaxShaderCompilerThreadsARB(GLuint count);

    //
    // GL_ARB_robustness
    //

    export GLenum glGetGraphicsResetStatusARB(void);
    export void glGetnTexImageARB(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void* img);
    export void glReadnPixelsARB(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data);
    export void glGetnCompressedTexImageARB(GLenum target, GLint lod, GLsizei bufSize, void* img);
    export void glGetnUniformfvARB(GLuint program, GLint location, GLsizei bufSize, GLfloat* params);
    export void glGetnUniformivARB(GLuint program, GLint location, GLsizei bufSize, GLint* params);
    export void glGetnUniformuivARB(GLuint program, GLint location, GLsizei bufSize, GLuint* params);
    export void glGetnUniformdvARB(GLuint program, GLint location, GLsizei bufSize, GLdouble* params);

    //
    // GL_ARB_sample_locations
    //

    export void glFramebufferSampleLocationsfvARB(GLenum target, GLuint start, GLsizei count, const GLfloat* v);
    export void glNamedFramebufferSampleLocationsfvARB(GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v);
    export void glEvaluateDepthValuesARB(void);

    //
    // GL_ARB_sample_shading
    //

    export void glMinSampleShadingARB(GLfloat value);

    //
    // GL_ARB_shading_language_include
    //

    export void glNamedStringARB(GLenum type, GLint namelen, const GLchar* name, GLint stringlen, const GLchar* string);
    export void glDeleteNamedStringARB(GLint namelen, const GLchar* name);
    export void glCompileShaderIncludeARB(GLuint shader, GLsizei count, const GLchar* const* path, const GLint* length);
    export GLboolean glIsNamedStringARB(GLint namelen, const GLchar* name);
    export void glGetNamedStringARB(GLint namelen, const GLchar* name, GLsizei bufSize, GLint* stringlen, GLchar* string);
    export void glGetNamedStringivARB(GLint namelen, const GLchar* name, GLenum pname, GLint* params);

    //
    // GL_ARB_sparse_buffer
    //

    export void glBufferPageCommitmentARB(GLenum target, GLintptr offset, GLsizeiptr size, GLboolean commit);
    export void glNamedBufferPageCommitmentEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit);
    export void glNamedBufferPageCommitmentARB(GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit);

    //
    // GL_ARB_sparse_texture
    //

    export void glTexPageCommitmentARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);

    //
    // GL_ARB_texture_buffer_object
    //

    export void glTexBufferARB(GLenum target, GLenum internalformat, GLuint buffer);

    //
    // GL_ARB_viewport_array
    //

    export void glDepthRangeArraydvNV(GLuint first, GLsizei count, const GLdouble* v);
    export void glDepthRangeIndexeddNV(GLuint index, GLdouble n, GLdouble f);

    //
    // GL_KHR_blend_equation_advanced
    //

    export void glBlendBarrierKHR(void);

    //
    // GL_KHR_parallel_shader_compile
    //

    export void glMaxShaderCompilerThreadsKHR(GLuint count);

    //
    // GL_AMD_framebuffer_multisample_advanced
    //

    export void glRenderbufferStorageMultisampleAdvancedAMD(GLenum target, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height);
    export void glNamedRenderbufferStorageMultisampleAdvancedAMD(GLuint renderbuffer, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height);

    //
    // GL_AMD_performance_monitor
    //

    export void glGetPerfMonitorGroupsAMD(GLint* numGroups, GLsizei groupsSize, GLuint* groups);
    export void glGetPerfMonitorCountersAMD(GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters);
    export void glGetPerfMonitorGroupStringAMD(GLuint group, GLsizei bufSize, GLsizei* length, GLchar* groupString);
    export void glGetPerfMonitorCounterStringAMD(GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, GLchar* counterString);
    export void glGetPerfMonitorCounterInfoAMD(GLuint group, GLuint counter, GLenum pname, void* data);
    export void glGenPerfMonitorsAMD(GLsizei n, GLuint* monitors);
    export void glDeletePerfMonitorsAMD(GLsizei n, GLuint* monitors);
    export void glSelectPerfMonitorCountersAMD(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* counterList);
    export void glBeginPerfMonitorAMD(GLuint monitor);
    export void glEndPerfMonitorAMD(GLuint monitor);
    export void glGetPerfMonitorCounterDataAMD(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten);

    //
    // GL_EXT_EGL_image_storage
    //

    export void glEGLImageTargetTexStorageEXT(GLenum target, GLeglImageOES image, const GLint* attrib_list);
    export void glEGLImageTargetTextureStorageEXT(GLuint texture, GLeglImageOES image, const GLint* attrib_list);

    //
    // GL_EXT_debug_label
    //

    export void glLabelObjectEXT(GLenum type, GLuint object, GLsizei length, const GLchar* label);
    export void glGetObjectLabelEXT(GLenum type, GLuint object, GLsizei bufSize, GLsizei* length, GLchar* label);

    //
    // GL_EXT_debug_marker
    //

    export void glInsertEventMarkerEXT(GLsizei length, const GLchar* marker);
    export void glPushGroupMarkerEXT(GLsizei length, const GLchar* marker);
    export void glPopGroupMarkerEXT(void);

    //
    // GL_EXT_direct_state_access
    //

    export void glMatrixLoadfEXT(GLenum mode, const GLfloat* m);
    export void glMatrixLoaddEXT(GLenum mode, const GLdouble* m);
    export void glMatrixMultfEXT(GLenum mode, const GLfloat* m);
    export void glMatrixMultdEXT(GLenum mode, const GLdouble* m);
    export void glMatrixLoadIdentityEXT(GLenum mode);
    export void glMatrixRotatefEXT(GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    export void glMatrixRotatedEXT(GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    export void glMatrixScalefEXT(GLenum mode, GLfloat x, GLfloat y, GLfloat z);
    export void glMatrixScaledEXT(GLenum mode, GLdouble x, GLdouble y, GLdouble z);
    export void glMatrixTranslatefEXT(GLenum mode, GLfloat x, GLfloat y, GLfloat z);
    export void glMatrixTranslatedEXT(GLenum mode, GLdouble x, GLdouble y, GLdouble z);
    export void glMatrixFrustumEXT(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    export void glMatrixOrthoEXT(GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    export void glMatrixPopEXT(GLenum mode);
    export void glMatrixPushEXT(GLenum mode);
    export void glClientAttribDefaultEXT(GLbitfield mask);
    export void glPushClientAttribDefaultEXT(GLbitfield mask);
    export void glTextureParameterfEXT(GLuint texture, GLenum target, GLenum pname, GLfloat param);
    export void glTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, const GLfloat* params);
    export void glTextureParameteriEXT(GLuint texture, GLenum target, GLenum pname, GLint param);
    export void glTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, const GLint* params);
    export void glTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels);
    export void glTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
    export void glTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
    export void glTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
    export void glCopyTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
    export void glCopyTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    export void glCopyTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    export void glCopyTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    export void glGetTextureImageEXT(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, void* pixels);
    export void glGetTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, GLfloat* params);
    export void glGetTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, GLint* params);
    export void glGetTextureLevelParameterfvEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat* params);
    export void glGetTextureLevelParameterivEXT(GLuint texture, GLenum target, GLint level, GLenum pname, GLint* params);
    export void glTextureImage3DEXT(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
    export void glTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
    export void glCopyTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    export void glBindMultiTextureEXT(GLenum texunit, GLenum target, GLuint texture);
    export void glMultiTexCoordPointerEXT(GLenum texunit, GLint size, GLenum type, GLsizei stride, const void* pointer);
    export void glMultiTexEnvfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
    export void glMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params);
    export void glMultiTexEnviEXT(GLenum texunit, GLenum target, GLenum pname, GLint param);
    export void glMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params);
    export void glMultiTexGendEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble param);
    export void glMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLdouble* params);
    export void glMultiTexGenfEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat param);
    export void glMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, const GLfloat* params);
    export void glMultiTexGeniEXT(GLenum texunit, GLenum coord, GLenum pname, GLint param);
    export void glMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, const GLint* params);
    export void glGetMultiTexEnvfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params);
    export void glGetMultiTexEnvivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params);
    export void glGetMultiTexGendvEXT(GLenum texunit, GLenum coord, GLenum pname, GLdouble* params);
    export void glGetMultiTexGenfvEXT(GLenum texunit, GLenum coord, GLenum pname, GLfloat* params);
    export void glGetMultiTexGenivEXT(GLenum texunit, GLenum coord, GLenum pname, GLint* params);
    export void glMultiTexParameteriEXT(GLenum texunit, GLenum target, GLenum pname, GLint param);
    export void glMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params);
    export void glMultiTexParameterfEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
    export void glMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat* params);
    export void glMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels);
    export void glMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
    export void glMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels);
    export void glMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
    export void glCopyMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
    export void glCopyMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    export void glCopyMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    export void glCopyMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    export void glGetMultiTexImageEXT(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, void* pixels);
    export void glGetMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat* params);
    export void glGetMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params);
    export void glGetMultiTexLevelParameterfvEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat* params);
    export void glGetMultiTexLevelParameterivEXT(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint* params);
    export void glMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
    export void glMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
    export void glCopyMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    export void glEnableClientStateIndexedEXT(GLenum array, GLuint index);
    export void glDisableClientStateIndexedEXT(GLenum array, GLuint index);
    export void glGetFloatIndexedvEXT(GLenum target, GLuint index, GLfloat* data);
    export void glGetDoubleIndexedvEXT(GLenum target, GLuint index, GLdouble* data);
    export void glGetPointerIndexedvEXT(GLenum target, GLuint index, void** data);
    export void glEnableIndexedEXT(GLenum target, GLuint index);
    export void glDisableIndexedEXT(GLenum target, GLuint index);
    export GLboolean glIsEnabledIndexedEXT(GLenum target, GLuint index);
    export void glGetIntegerIndexedvEXT(GLenum target, GLuint index, GLint* data);
    export void glGetBooleanIndexedvEXT(GLenum target, GLuint index, GLboolean* data);
    export void glCompressedTextureImage3DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits);
    export void glCompressedTextureImage2DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits);
    export void glCompressedTextureImage1DEXT(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits);
    export void glCompressedTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits);
    export void glCompressedTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits);
    export void glCompressedTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits);
    export void glGetCompressedTextureImageEXT(GLuint texture, GLenum target, GLint lod, void* img);
    export void glCompressedMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* bits);
    export void glCompressedMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* bits);
    export void glCompressedMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void* bits);
    export void glCompressedMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* bits);
    export void glCompressedMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* bits);
    export void glCompressedMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void* bits);
    export void glGetCompressedMultiTexImageEXT(GLenum texunit, GLenum target, GLint lod, void* img);
    export void glMatrixLoadTransposefEXT(GLenum mode, const GLfloat* m);
    export void glMatrixLoadTransposedEXT(GLenum mode, const GLdouble* m);
    export void glMatrixMultTransposefEXT(GLenum mode, const GLfloat* m);
    export void glMatrixMultTransposedEXT(GLenum mode, const GLdouble* m);
    export void glNamedBufferDataEXT(GLuint buffer, GLsizeiptr size, const void* data, GLenum usage);
    export void glNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, const void* data);
    export void* glMapNamedBufferEXT(GLuint buffer, GLenum access);
    export GLboolean glUnmapNamedBufferEXT(GLuint buffer);
    export void glGetNamedBufferParameterivEXT(GLuint buffer, GLenum pname, GLint* params);
    export void glGetNamedBufferPointervEXT(GLuint buffer, GLenum pname, void** params);
    export void glGetNamedBufferSubDataEXT(GLuint buffer, GLintptr offset, GLsizeiptr size, void* data);
    export void glProgramUniform1fEXT(GLuint program, GLint location, GLfloat v0);
    export void glProgramUniform2fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1);
    export void glProgramUniform3fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    export void glProgramUniform4fEXT(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    export void glProgramUniform1iEXT(GLuint program, GLint location, GLint v0);
    export void glProgramUniform2iEXT(GLuint program, GLint location, GLint v0, GLint v1);
    export void glProgramUniform3iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
    export void glProgramUniform4iEXT(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    export void glProgramUniform1fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    export void glProgramUniform2fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    export void glProgramUniform3fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    export void glProgramUniform4fvEXT(GLuint program, GLint location, GLsizei count, const GLfloat* value);
    export void glProgramUniform1ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value);
    export void glProgramUniform2ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value);
    export void glProgramUniform3ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value);
    export void glProgramUniform4ivEXT(GLuint program, GLint location, GLsizei count, const GLint* value);
    export void glProgramUniformMatrix2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix2x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix3x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix2x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix4x2fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix3x4fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glProgramUniformMatrix4x3fvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    export void glTextureBufferEXT(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
    export void glMultiTexBufferEXT(GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer);
    export void glTextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname, const GLint* params);
    export void glTextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname, const GLuint* params);
    export void glGetTextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname, GLint* params);
    export void glGetTextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname, GLuint* params);
    export void glMultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname, const GLint* params);
    export void glMultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname, const GLuint* params);
    export void glGetMultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname, GLint* params);
    export void glGetMultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname, GLuint* params);
    export void glProgramUniform1uiEXT(GLuint program, GLint location, GLuint v0);
    export void glProgramUniform2uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1);
    export void glProgramUniform3uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
    export void glProgramUniform4uiEXT(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    export void glProgramUniform1uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value);
    export void glProgramUniform2uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value);
    export void glProgramUniform3uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value);
    export void glProgramUniform4uivEXT(GLuint program, GLint location, GLsizei count, const GLuint* value);
    export void glNamedProgramLocalParameters4fvEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat* params);
    export void glNamedProgramLocalParameterI4iEXT(GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
    export void glNamedProgramLocalParameterI4ivEXT(GLuint program, GLenum target, GLuint index, const GLint* params);
    export void glNamedProgramLocalParametersI4ivEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLint* params);
    export void glNamedProgramLocalParameterI4uiEXT(GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
    export void glNamedProgramLocalParameterI4uivEXT(GLuint program, GLenum target, GLuint index, const GLuint* params);
    export void glNamedProgramLocalParametersI4uivEXT(GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint* params);
    export void glGetNamedProgramLocalParameterIivEXT(GLuint program, GLenum target, GLuint index, GLint* params);
    export void glGetNamedProgramLocalParameterIuivEXT(GLuint program, GLenum target, GLuint index, GLuint* params);
    export void glEnableClientStateiEXT(GLenum array, GLuint index);
    export void glDisableClientStateiEXT(GLenum array, GLuint index);
    export void glGetFloati_vEXT(GLenum pname, GLuint index, GLfloat* params);
    export void glGetDoublei_vEXT(GLenum pname, GLuint index, GLdouble* params);
    export void glGetPointeri_vEXT(GLenum pname, GLuint index, void** params);
    export void glNamedProgramStringEXT(GLuint program, GLenum target, GLenum format, GLsizei len, const void* string);
    export void glNamedProgramLocalParameter4dEXT(GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    export void glNamedProgramLocalParameter4dvEXT(GLuint program, GLenum target, GLuint index, const GLdouble* params);
    export void glNamedProgramLocalParameter4fEXT(GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    export void glNamedProgramLocalParameter4fvEXT(GLuint program, GLenum target, GLuint index, const GLfloat* params);
    export void glGetNamedProgramLocalParameterdvEXT(GLuint program, GLenum target, GLuint index, GLdouble* params);
    export void glGetNamedProgramLocalParameterfvEXT(GLuint program, GLenum target, GLuint index, GLfloat* params);
    export void glGetNamedProgramivEXT(GLuint program, GLenum target, GLenum pname, GLint* params);
    export void glGetNamedProgramStringEXT(GLuint program, GLenum target, GLenum pname, void* string);
    export void glNamedRenderbufferStorageEXT(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
    export void glGetNamedRenderbufferParameterivEXT(GLuint renderbuffer, GLenum pname, GLint* params);
    export void glNamedRenderbufferStorageMultisampleEXT(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    export void glNamedRenderbufferStorageMultisampleCoverageEXT(GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
    export GLenum glCheckNamedFramebufferStatusEXT(GLuint framebuffer, GLenum target);
    export void glNamedFramebufferTexture1DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    export void glNamedFramebufferTexture2DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    export void glNamedFramebufferTexture3DEXT(GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    export void glNamedFramebufferRenderbufferEXT(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    export void glGetNamedFramebufferAttachmentParameterivEXT(GLuint framebuffer, GLenum attachment, GLenum pname, GLint* params);
    export void glGenerateTextureMipmapEXT(GLuint texture, GLenum target);
    export void glGenerateMultiTexMipmapEXT(GLenum texunit, GLenum target);
    export void glFramebufferDrawBufferEXT(GLuint framebuffer, GLenum mode);
    export void glFramebufferDrawBuffersEXT(GLuint framebuffer, GLsizei n, const GLenum* bufs);
    export void glFramebufferReadBufferEXT(GLuint framebuffer, GLenum mode);
    export void glGetFramebufferParameterivEXT(GLuint framebuffer, GLenum pname, GLint* params);
    export void glNamedCopyBufferSubDataEXT(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    export void glNamedFramebufferTextureEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
    export void glNamedFramebufferTextureLayerEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
    export void glNamedFramebufferTextureFaceEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face);
    export void glTextureRenderbufferEXT(GLuint texture, GLenum target, GLuint renderbuffer);
    export void glMultiTexRenderbufferEXT(GLenum texunit, GLenum target, GLuint renderbuffer);
    export void glVertexArrayVertexOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
    export void glVertexArrayColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
    export void glVertexArrayEdgeFlagOffsetEXT(GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset);
    export void glVertexArrayIndexOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset);
    export void glVertexArrayNormalOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset);
    export void glVertexArrayTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
    export void glVertexArrayMultiTexCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset);
    export void glVertexArrayFogCoordOffsetEXT(GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset);
    export void glVertexArraySecondaryColorOffsetEXT(GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset);
    export void glVertexArrayVertexAttribOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset);
    export void glVertexArrayVertexAttribIOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset);
    export void glEnableVertexArrayEXT(GLuint vaobj, GLenum array);
    export void glDisableVertexArrayEXT(GLuint vaobj, GLenum array);
    export void glEnableVertexArrayAttribEXT(GLuint vaobj, GLuint index);
    export void glDisableVertexArrayAttribEXT(GLuint vaobj, GLuint index);
    export void glGetVertexArrayIntegervEXT(GLuint vaobj, GLenum pname, GLint* param);
    export void glGetVertexArrayPointervEXT(GLuint vaobj, GLenum pname, void** param);
    export void glGetVertexArrayIntegeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, GLint* param);
    export void glGetVertexArrayPointeri_vEXT(GLuint vaobj, GLuint index, GLenum pname, void** param);
    export void* glMapNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
    export void glFlushMappedNamedBufferRangeEXT(GLuint buffer, GLintptr offset, GLsizeiptr length);
    export void glNamedBufferStorageEXT(GLuint buffer, GLsizeiptr size, const void* data, GLbitfield flags);
    export void glClearNamedBufferDataEXT(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data);
    export void glClearNamedBufferSubDataEXT(GLuint buffer, GLenum internalformat, GLsizeiptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data);
    export void glNamedFramebufferParameteriEXT(GLuint framebuffer, GLenum pname, GLint param);
    export void glGetNamedFramebufferParameterivEXT(GLuint framebuffer, GLenum pname, GLint* params);
    export void glProgramUniform1dEXT(GLuint program, GLint location, GLdouble x);
    export void glProgramUniform2dEXT(GLuint program, GLint location, GLdouble x, GLdouble y);
    export void glProgramUniform3dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z);
    export void glProgramUniform4dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    export void glProgramUniform1dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    export void glProgramUniform2dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    export void glProgramUniform3dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    export void glProgramUniform4dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble* value);
    export void glProgramUniformMatrix2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix2x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix2x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix3x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix3x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix4x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glProgramUniformMatrix4x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value);
    export void glTextureBufferRangeEXT(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    export void glTextureStorage1DEXT(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
    export void glTextureStorage2DEXT(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    export void glTextureStorage3DEXT(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    export void glTextureStorage2DMultisampleEXT(GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    export void glTextureStorage3DMultisampleEXT(GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    export void glVertexArrayBindVertexBufferEXT(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    export void glVertexArrayVertexAttribFormatEXT(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    export void glVertexArrayVertexAttribIFormatEXT(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    export void glVertexArrayVertexAttribLFormatEXT(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    export void glVertexArrayVertexAttribBindingEXT(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
    export void glVertexArrayVertexBindingDivisorEXT(GLuint vaobj, GLuint bindingindex, GLuint divisor);
    export void glVertexArrayVertexAttribLOffsetEXT(GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset);
    export void glTexturePageCommitmentEXT(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit);
    export void glVertexArrayVertexAttribDivisorEXT(GLuint vaobj, GLuint index, GLuint divisor);

    //
    // GL_EXT_draw_instanced
    //

    export void glDrawArraysInstancedEXT(GLenum mode, GLint start, GLsizei count, GLsizei primcount);
    export void glDrawElementsInstancedEXT(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount);

    //
    // GL_EXT_polygon_offset_clamp
    //

    export void glPolygonOffsetClampEXT(GLfloat factor, GLfloat units, GLfloat clamp);

    //
    // GL_EXT_raster_multisample
    //

    export void glRasterSamplesEXT(GLuint samples, GLboolean fixedsamplelocations);

    //
    // GL_EXT_separate_shader_objects
    //

    export void glUseShaderProgramEXT(GLenum type, GLuint program);
    export void glActiveProgramEXT(GLuint program);
    export GLuint glCreateShaderProgramEXT(GLenum type, const GLchar* string);

    //
    // GL_EXT_shader_framebuffer_fetch_non_coherent
    //

    export void glFramebufferFetchBarrierEXT(void);

    //
    // GL_EXT_texture_storage
    //

    export void glTexStorage1DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
    export void glTexStorage2DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    export void glTexStorage3DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

    //
    // GL_EXT_window_rectangles
    //

    export void glWindowRectanglesEXT(GLenum mode, GLsizei count, const GLint* box);

    //
    // GL_INTEL_framebuffer_CMAA
    //

    export void glApplyFramebufferAttachmentCMAAINTEL(void);

    //
    // GL_INTEL_performance_query
    //

    export void glBeginPerfQueryINTEL(GLuint queryHandle);
    export void glCreatePerfQueryINTEL(GLuint queryId, GLuint* queryHandle);
    export void glDeletePerfQueryINTEL(GLuint queryHandle);
    export void glEndPerfQueryINTEL(GLuint queryHandle);
    export void glGetFirstPerfQueryIdINTEL(GLuint* queryId);
    export void glGetNextPerfQueryIdINTEL(GLuint queryId, GLuint* nextQueryId);
    export void glGetPerfCounterInfoINTEL(GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar* counterName, GLuint counterDescLength, GLchar* counterDesc, GLuint* counterOffset, GLuint* counterDataSize, GLuint* counterTypeEnum, GLuint* counterDataTypeEnum, GLuint64* rawCounterMaxValue);
    export void glGetPerfQueryDataINTEL(GLuint queryHandle, GLuint flags, GLsizei dataSize, void* data, GLuint* bytesWritten);
    export void glGetPerfQueryIdByNameINTEL(GLchar* queryName, GLuint* queryId);
    export void glGetPerfQueryInfoINTEL(GLuint queryId, GLuint queryNameLength, GLchar* queryName, GLuint* dataSize, GLuint* noCounters, GLuint* noInstances, GLuint* capsMask);

    //
    // GL_MESA_framebuffer_flip_y
    //

    export void glFramebufferParameteriMESA(GLenum target, GLenum pname, GLint param);
    export void glGetFramebufferParameterivMESA(GLenum target, GLenum pname, GLint* params);

    //
    // GL_NV_bindless_multi_draw_indirect
    //

    export void glMultiDrawArraysIndirectBindlessNV(GLenum mode, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount);
    export void glMultiDrawElementsIndirectBindlessNV(GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount);

    //
    // GL_NV_bindless_multi_draw_indirect_count
    //

    export void glMultiDrawArraysIndirectBindlessCountNV(GLenum mode, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount);
    export void glMultiDrawElementsIndirectBindlessCountNV(GLenum mode, GLenum type, const void* indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount);

    //
    // GL_NV_bindless_texture
    //

    export GLuint64 glGetTextureHandleNV(GLuint texture);
    export GLuint64 glGetTextureSamplerHandleNV(GLuint texture, GLuint sampler);
    export void glMakeTextureHandleResidentNV(GLuint64 handle);
    export void glMakeTextureHandleNonResidentNV(GLuint64 handle);
    export GLuint64 glGetImageHandleNV(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format);
    export void glMakeImageHandleResidentNV(GLuint64 handle, GLenum access);
    export void glMakeImageHandleNonResidentNV(GLuint64 handle);
    export void glUniformHandleui64NV(GLint location, GLuint64 value);
    export void glUniformHandleui64vNV(GLint location, GLsizei count, const GLuint64* value);
    export void glProgramUniformHandleui64NV(GLuint program, GLint location, GLuint64 value);
    export void glProgramUniformHandleui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64* values);
    export GLboolean glIsTextureHandleResidentNV(GLuint64 handle);
    export GLboolean glIsImageHandleResidentNV(GLuint64 handle);

    //
    // GL_NV_blend_equation_advanced
    //

    export void glBlendParameteriNV(GLenum pname, GLint value);
    export void glBlendBarrierNV(void);

    //
    // GL_NV_clip_space_w_scaling
    //

    export void glViewportPositionWScaleNV(GLuint index, GLfloat xcoeff, GLfloat ycoeff);

    //
    // GL_NV_command_list
    //

    export void glCreateStatesNV(GLsizei n, GLuint* states);
    export void glDeleteStatesNV(GLsizei n, const GLuint* states);
    export GLboolean glIsStateNV(GLuint state);
    export void glStateCaptureNV(GLuint state, GLenum mode);
    export GLuint glGetCommandHeaderNV(GLenum tokenID, GLuint size);
    export GLushort glGetStageIndexNV(GLenum shadertype);
    export void glDrawCommandsNV(GLenum primitiveMode, GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, GLuint count);
    export void glDrawCommandsAddressNV(GLenum primitiveMode, const GLuint64* indirects, const GLsizei* sizes, GLuint count);
    export void glDrawCommandsStatesNV(GLuint buffer, const GLintptr* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count);
    export void glDrawCommandsStatesAddressNV(const GLuint64* indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count);
    export void glCreateCommandListsNV(GLsizei n, GLuint* lists);
    export void glDeleteCommandListsNV(GLsizei n, const GLuint* lists);
    export GLboolean glIsCommandListNV(GLuint list);
    export void glListDrawCommandsStatesClientNV(GLuint list, GLuint segment, const void** indirects, const GLsizei* sizes, const GLuint* states, const GLuint* fbos, GLuint count);
    export void glCommandListSegmentsNV(GLuint list, GLuint segments);
    export void glCompileCommandListNV(GLuint list);
    export void glCallCommandListNV(GLuint list);

    //
    // GL_NV_conditional_render
    //

    export void glBeginConditionalRenderNV(GLuint id, GLenum mode);
    export void glEndConditionalRenderNV(void);

    //
    // GL_NV_conservative_raster
    //

    export void glSubpixelPrecisionBiasNV(GLuint xbits, GLuint ybits);

    //
    // GL_NV_conservative_raster_dilate
    //

    export void glConservativeRasterParameterfNV(GLenum pname, GLfloat value);

    //
    // GL_NV_conservative_raster_pre_snap_triangles
    //

    export void glConservativeRasterParameteriNV(GLenum pname, GLint param);

    //
    // GL_NV_depth_buffer_float
    //

    export void glDepthRangedNV(GLdouble zNear, GLdouble zFar);
    export void glClearDepthdNV(GLdouble depth);
    export void glDepthBoundsdNV(GLdouble zmin, GLdouble zmax);

    //
    // GL_NV_draw_vulkan_image
    //

    export void glDrawVkImageNV(GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1);
    export GLVULKANPROCNV glGetVkProcAddrNV(const GLchar* name);
    export void glWaitVkSemaphoreNV(GLuint64 vkSemaphore);
    export void glSignalVkSemaphoreNV(GLuint64 vkSemaphore);
    export void glSignalVkFenceNV(GLuint64 vkFence);

    //
    // GL_NV_fragment_coverage_to_color
    //

    export void glFragmentCoverageColorNV(GLuint color);

    //
    // GL_NV_framebuffer_mixed_samples
    //

    export void glCoverageModulationTableNV(GLsizei n, const GLfloat* v);
    export void glGetCoverageModulationTableNV(GLsizei bufSize, GLfloat* v);
    export void glCoverageModulationNV(GLenum components);

    //
    // GL_NV_framebuffer_multisample_coverage
    //

    export void glRenderbufferStorageMultisampleCoverageNV(GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);

    //
    // GL_NV_gpu_shader5
    //

    export void glUniform1i64NV(GLint location, GLint64EXT x);
    export void glUniform2i64NV(GLint location, GLint64EXT x, GLint64EXT y);
    export void glUniform3i64NV(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z);
    export void glUniform4i64NV(GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w);
    export void glUniform1i64vNV(GLint location, GLsizei count, const GLint64EXT* value);
    export void glUniform2i64vNV(GLint location, GLsizei count, const GLint64EXT* value);
    export void glUniform3i64vNV(GLint location, GLsizei count, const GLint64EXT* value);
    export void glUniform4i64vNV(GLint location, GLsizei count, const GLint64EXT* value);
    export void glUniform1ui64NV(GLint location, GLuint64EXT x);
    export void glUniform2ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y);
    export void glUniform3ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
    export void glUniform4ui64NV(GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
    export void glUniform1ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value);
    export void glUniform2ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value);
    export void glUniform3ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value);
    export void glUniform4ui64vNV(GLint location, GLsizei count, const GLuint64EXT* value);
    export void glGetUniformi64vNV(GLuint program, GLint location, GLint64EXT* params);
    export void glProgramUniform1i64NV(GLuint program, GLint location, GLint64EXT x);
    export void glProgramUniform2i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y);
    export void glProgramUniform3i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z);
    export void glProgramUniform4i64NV(GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w);
    export void glProgramUniform1i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value);
    export void glProgramUniform2i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value);
    export void glProgramUniform3i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value);
    export void glProgramUniform4i64vNV(GLuint program, GLint location, GLsizei count, const GLint64EXT* value);
    export void glProgramUniform1ui64NV(GLuint program, GLint location, GLuint64EXT x);
    export void glProgramUniform2ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y);
    export void glProgramUniform3ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
    export void glProgramUniform4ui64NV(GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
    export void glProgramUniform1ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);
    export void glProgramUniform2ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);
    export void glProgramUniform3ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);
    export void glProgramUniform4ui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);

    //
    // GL_NV_internalformat_sample_query
    //

    export void glGetInternalformatSampleivNV(GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei count, GLint* params);

    //
    // GL_NV_memory_attachment
    //

    export void glGetMemoryObjectDetachedResourcesuivNV(GLuint memory, GLenum pname, GLint first, GLsizei count, GLuint* params);
    export void glResetMemoryObjectParameterNV(GLuint memory, GLenum pname);
    export void glTexAttachMemoryNV(GLenum target, GLuint memory, GLuint64 offset);
    export void glBufferAttachMemoryNV(GLenum target, GLuint memory, GLuint64 offset);
    export void glTextureAttachMemoryNV(GLuint texture, GLuint memory, GLuint64 offset);
    export void glNamedBufferAttachMemoryNV(GLuint buffer, GLuint memory, GLuint64 offset);

    //
    // GL_NV_memory_object_sparse
    //

    export void glBufferPageCommitmentMemNV(GLenum target, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit);
    export void glTexPageCommitmentMemNV(GLenum target, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit);
    export void glNamedBufferPageCommitmentMemNV(GLuint buffer, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit);
    export void glTexturePageCommitmentMemNV(GLuint texture, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit);

    //
    // GL_NV_mesh_shader
    //

    export void glDrawMeshTasksNV(GLuint first, GLuint count);
    export void glDrawMeshTasksIndirectNV(GLintptr indirect);
    export void glMultiDrawMeshTasksIndirectNV(GLintptr indirect, GLsizei drawcount, GLsizei stride);
    export void glMultiDrawMeshTasksIndirectCountNV(GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);

    //
    // GL_NV_path_rendering
    //

    export GLuint glGenPathsNV(GLsizei range);
    export void glDeletePathsNV(GLuint path, GLsizei range);
    export GLboolean glIsPathNV(GLuint path);
    export void glPathCommandsNV(GLuint path, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords);
    export void glPathCoordsNV(GLuint path, GLsizei numCoords, GLenum coordType, const void* coords);
    export void glPathSubCommandsNV(GLuint path, GLsizei commandStart, GLsizei commandsToDelete, GLsizei numCommands, const GLubyte* commands, GLsizei numCoords, GLenum coordType, const void* coords);
    export void glPathSubCoordsNV(GLuint path, GLsizei coordStart, GLsizei numCoords, GLenum coordType, const void* coords);
    export void glPathStringNV(GLuint path, GLenum format, GLsizei length, const void* pathString);
    export void glPathGlyphsNV(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLsizei numGlyphs, GLenum type, const void* charcodes, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
    export void glPathGlyphRangeNV(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyph, GLsizei numGlyphs, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
    export void glWeightPathsNV(GLuint resultPath, GLsizei numPaths, const GLuint* paths, const GLfloat* weights);
    export void glCopyPathNV(GLuint resultPath, GLuint srcPath);
    export void glInterpolatePathsNV(GLuint resultPath, GLuint pathA, GLuint pathB, GLfloat weight);
    export void glTransformPathNV(GLuint resultPath, GLuint srcPath, GLenum transformType, const GLfloat* transformValues);
    export void glPathParameterivNV(GLuint path, GLenum pname, const GLint* value);
    export void glPathParameteriNV(GLuint path, GLenum pname, GLint value);
    export void glPathParameterfvNV(GLuint path, GLenum pname, const GLfloat* value);
    export void glPathParameterfNV(GLuint path, GLenum pname, GLfloat value);
    export void glPathDashArrayNV(GLuint path, GLsizei dashCount, const GLfloat* dashArray);
    export void glPathStencilFuncNV(GLenum func, GLint ref, GLuint mask);
    export void glPathStencilDepthOffsetNV(GLfloat factor, GLfloat units);
    export void glStencilFillPathNV(GLuint path, GLenum fillMode, GLuint mask);
    export void glStencilStrokePathNV(GLuint path, GLint reference, GLuint mask);
    export void glStencilFillPathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat* transformValues);
    export void glStencilStrokePathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat* transformValues);
    export void glPathCoverDepthFuncNV(GLenum func);
    export void glCoverFillPathNV(GLuint path, GLenum coverMode);
    export void glCoverStrokePathNV(GLuint path, GLenum coverMode);
    export void glCoverFillPathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues);
    export void glCoverStrokePathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat* transformValues);
    export void glGetPathParameterivNV(GLuint path, GLenum pname, GLint* value);
    export void glGetPathParameterfvNV(GLuint path, GLenum pname, GLfloat* value);
    export void glGetPathCommandsNV(GLuint path, GLubyte* commands);
    export void glGetPathCoordsNV(GLuint path, GLfloat* coords);
    export void glGetPathDashArrayNV(GLuint path, GLfloat* dashArray);
    export void glGetPathMetricsNV(GLbitfield metricQueryMask, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLsizei stride, GLfloat* metrics);
    export void glGetPathMetricRangeNV(GLbitfield metricQueryMask, GLuint firstPathName, GLsizei numPaths, GLsizei stride, GLfloat* metrics);
    export void glGetPathSpacingNV(GLenum pathListMode, GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLfloat advanceScale, GLfloat kerningScale, GLenum transformType, GLfloat* returnedSpacing);
    export GLboolean glIsPointInFillPathNV(GLuint path, GLuint mask, GLfloat x, GLfloat y);
    export GLboolean glIsPointInStrokePathNV(GLuint path, GLfloat x, GLfloat y);
    export GLfloat glGetPathLengthNV(GLuint path, GLsizei startSegment, GLsizei numSegments);
    export GLboolean glPointAlongPathNV(GLuint path, GLsizei startSegment, GLsizei numSegments, GLfloat distance, GLfloat* x, GLfloat* y, GLfloat* tangentX, GLfloat* tangentY);
    export void glMatrixLoad3x2fNV(GLenum matrixMode, const GLfloat* m);
    export void glMatrixLoad3x3fNV(GLenum matrixMode, const GLfloat* m);
    export void glMatrixLoadTranspose3x3fNV(GLenum matrixMode, const GLfloat* m);
    export void glMatrixMult3x2fNV(GLenum matrixMode, const GLfloat* m);
    export void glMatrixMult3x3fNV(GLenum matrixMode, const GLfloat* m);
    export void glMatrixMultTranspose3x3fNV(GLenum matrixMode, const GLfloat* m);
    export void glStencilThenCoverFillPathNV(GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode);
    export void glStencilThenCoverStrokePathNV(GLuint path, GLint reference, GLuint mask, GLenum coverMode);
    export void glStencilThenCoverFillPathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues);
    export void glStencilThenCoverStrokePathInstancedNV(GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat* transformValues);
    export GLenum glPathGlyphIndexRangeNV(GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint pathParameterTemplate, GLfloat emScale, GLuint* baseAndCount);
    export GLenum glPathGlyphIndexArrayNV(GLuint firstPathName, GLenum fontTarget, const void* fontName, GLbitfield fontStyle, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
    export GLenum glPathMemoryGlyphIndexArrayNV(GLuint firstPathName, GLenum fontTarget, GLsizeiptr fontSize, const void* fontData, GLsizei faceIndex, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale);
    export void glProgramPathFragmentInputGenNV(GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat* coeffs);
    export void glGetProgramResourcefvNV(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei count, GLsizei* length, GLfloat* params);

    //
    // GL_NV_sample_locations
    //

    export void glFramebufferSampleLocationsfvNV(GLenum target, GLuint start, GLsizei count, const GLfloat* v);
    export void glNamedFramebufferSampleLocationsfvNV(GLuint framebuffer, GLuint start, GLsizei count, const GLfloat* v);
    export void glResolveDepthValuesNV(void);

    //
    // GL_NV_scissor_exclusive
    //

    export void glScissorExclusiveNV(GLint x, GLint y, GLsizei width, GLsizei height);
    export void glScissorExclusiveArrayvNV(GLuint first, GLsizei count, const GLint* v);

    //
    // GL_NV_shader_buffer_load
    //

    export void glMakeBufferResidentNV(GLenum target, GLenum access);
    export void glMakeBufferNonResidentNV(GLenum target);
    export GLboolean glIsBufferResidentNV(GLenum target);
    export void glMakeNamedBufferResidentNV(GLuint buffer, GLenum access);
    export void glMakeNamedBufferNonResidentNV(GLuint buffer);
    export GLboolean glIsNamedBufferResidentNV(GLuint buffer);
    export void glGetBufferParameterui64vNV(GLenum target, GLenum pname, GLuint64EXT* params);
    export void glGetNamedBufferParameterui64vNV(GLuint buffer, GLenum pname, GLuint64EXT* params);
    export void glGetIntegerui64vNV(GLenum value, GLuint64EXT* result);
    export void glUniformui64NV(GLint location, GLuint64EXT value);
    export void glUniformui64vNV(GLint location, GLsizei count, const GLuint64EXT* value);
    export void glGetUniformui64vNV(GLuint program, GLint location, GLuint64EXT* params);
    export void glProgramUniformui64NV(GLuint program, GLint location, GLuint64EXT value);
    export void glProgramUniformui64vNV(GLuint program, GLint location, GLsizei count, const GLuint64EXT* value);

    //
    // GL_NV_shading_rate_image
    //

    export void glBindShadingRateImageNV(GLuint texture);
    export void glGetShadingRateImagePaletteNV(GLuint viewport, GLuint entry, GLenum* rate);
    export void glGetShadingRateSampleLocationivNV(GLenum rate, GLuint samples, GLuint index, GLint* location);
    export void glShadingRateImageBarrierNV(GLboolean synchronize);
    export void glShadingRateImagePaletteNV(GLuint viewport, GLuint first, GLsizei count, const GLenum* rates);
    export void glShadingRateSampleOrderNV(GLenum order);
    export void glShadingRateSampleOrderCustomNV(GLenum rate, GLuint samples, const GLint* locations);

    //
    // GL_NV_texture_barrier
    //

    export void glTextureBarrierNV(void);

    //
    // GL_NV_vertex_attrib_integer_64bit
    //

    export void glVertexAttribL1i64NV(GLuint index, GLint64EXT x);
    export void glVertexAttribL2i64NV(GLuint index, GLint64EXT x, GLint64EXT y);
    export void glVertexAttribL3i64NV(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z);
    export void glVertexAttribL4i64NV(GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w);
    export void glVertexAttribL1i64vNV(GLuint index, const GLint64EXT* v);
    export void glVertexAttribL2i64vNV(GLuint index, const GLint64EXT* v);
    export void glVertexAttribL3i64vNV(GLuint index, const GLint64EXT* v);
    export void glVertexAttribL4i64vNV(GLuint index, const GLint64EXT* v);
    export void glVertexAttribL1ui64NV(GLuint index, GLuint64EXT x);
    export void glVertexAttribL2ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y);
    export void glVertexAttribL3ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z);
    export void glVertexAttribL4ui64NV(GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w);
    export void glVertexAttribL1ui64vNV(GLuint index, const GLuint64EXT* v);
    export void glVertexAttribL2ui64vNV(GLuint index, const GLuint64EXT* v);
    export void glVertexAttribL3ui64vNV(GLuint index, const GLuint64EXT* v);
    export void glVertexAttribL4ui64vNV(GLuint index, const GLuint64EXT* v);
    export void glGetVertexAttribLi64vNV(GLuint index, GLenum pname, GLint64EXT* params);
    export void glGetVertexAttribLui64vNV(GLuint index, GLenum pname, GLuint64EXT* params);
    export void glVertexAttribLFormatNV(GLuint index, GLint size, GLenum type, GLsizei stride);

    //
    // GL_NV_vertex_buffer_unified_memory
    //

    export void glBufferAddressRangeNV(GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length);
    export void glVertexFormatNV(GLint size, GLenum type, GLsizei stride);
    export void glNormalFormatNV(GLenum type, GLsizei stride);
    export void glColorFormatNV(GLint size, GLenum type, GLsizei stride);
    export void glIndexFormatNV(GLenum type, GLsizei stride);
    export void glTexCoordFormatNV(GLint size, GLenum type, GLsizei stride);
    export void glEdgeFlagFormatNV(GLsizei stride);
    export void glSecondaryColorFormatNV(GLint size, GLenum type, GLsizei stride);
    export void glFogCoordFormatNV(GLenum type, GLsizei stride);
    export void glVertexAttribFormatNV(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride);
    export void glVertexAttribIFormatNV(GLuint index, GLint size, GLenum type, GLsizei stride);
    export void glGetIntegerui64i_vNV(GLenum value, GLuint index, GLuint64EXT* result);

    //
    // GL_NV_viewport_swizzle
    //

    export void glViewportSwizzleNV(GLuint index, GLenum swizzlex, GLenum swizzley, GLenum swizzlez, GLenum swizzlew);

    //
    // GL_OVR_multiview
    //

    export void glFramebufferTextureMultiviewOVR(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
    export void glNamedFramebufferTextureMultiviewOVR(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
}