import <windows.h>;
import <GL/glcorearb.h>;
import <exception>;
import <memory>;
import <string>;
import OpenGL;

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
// Note:
// VS2022 (v17.11.2) has an IntelliSense bug that causes it to report false errors
// if this copyright notice is placed at the top of the file before import statements.

// This Windows OpenGL application demonstrates how to use the OpenGLContext class
// to avoid having to statically link to OpenGL32.lib.
class GLApplication
{
public:
    // Unicode-based, general-purpose exception and error class.
    class Error
    {
    public:
        Error(const wchar_t *pszMessage) : m_message{pszMessage} {}
        Error(const std::wstring &message) : m_message{message} {}
        virtual ~Error() {}

        const wchar_t *what() const { return m_message.c_str(); }

    private:
        std::wstring m_message;
    };

	GLApplication();
    GLApplication(const wchar_t *pszWindowName);
    ~GLApplication();

	int run();

private:
	static LRESULT CALLBACK windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool create();
    void destroy();
	void init(int argc, wchar_t *argv[]);
    void initApplication(const wchar_t *pszWindowName);
    void initOpenGL();
	int mainLoop();
    void render() const;
    void shutdown();
    void update();
	LRESULT windowProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    WNDCLASSEXW m_wcl{};
    HWND m_hWnd{nullptr};
    HINSTANCE m_hInstance{GetModuleHandle(nullptr)};
    HDC m_hDC{nullptr};
    HGLRC m_hRC{nullptr};
    const wchar_t *m_pszWindowName{L""};
    int m_windowWidth{};
    int m_windowHeight{};
    std::shared_ptr<OpenGLContext> m_pContext{};
};

GLApplication::GLApplication()
{
    initApplication(L"");
}

GLApplication::GLApplication(const wchar_t *pszWindowName)
{
    initApplication(pszWindowName);
}

GLApplication::~GLApplication()
{
}

int GLApplication::run()
{
    int status{};
 
    try
    {
        if (create())
        {
            try
            {
                initOpenGL();
                init(__argc, __wargv);
                status = mainLoop();
				shutdown();
            }
            catch (...)
            {
				shutdown();
                throw;
            }
            
            destroy();
        }
    }
    catch (const GLApplication::Error &e)
    {
        destroy();
        status = EXIT_FAILURE;
        MessageBox(0, e.what(), L"GLApplication Unhandled Exception", MB_ICONERROR);
    }

    return status;
}

bool GLApplication::create()
{      
    if (!RegisterClassExW(&m_wcl))
        return false;

    // Create a window that is centered on the desktop.
    // It's exactly 1/4 the size of the desktop. Don't allow it to be resized.

    DWORD wndExStyle{WS_EX_OVERLAPPEDWINDOW};
    DWORD wndStyle{WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS};

    m_hWnd = CreateWindowExW(wndExStyle, m_wcl.lpszClassName, m_pszWindowName, wndStyle, 0, 0, 0, 0, 0, 0, m_wcl.hInstance, this);

    if (!m_hWnd)
    {
        UnregisterClassW(m_wcl.lpszClassName, m_hInstance);
        return false;
    }
    else
    {
        int screenWidth{GetSystemMetrics(SM_CXSCREEN)};
        int screenHeight{GetSystemMetrics(SM_CYSCREEN)};
        int halfScreenWidth{screenWidth / 2};
        int halfScreenHeight{screenHeight / 2};
        int left{(screenWidth - halfScreenWidth) / 2};
        int top{(screenHeight - halfScreenHeight) / 2};
        RECT rc{};

        SetRect(&rc, left, top, left + halfScreenWidth, top + halfScreenHeight);
        AdjustWindowRectEx(&rc, wndStyle, FALSE, wndExStyle);
        MoveWindow(m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

        GetClientRect(m_hWnd, &rc);
        m_windowWidth = rc.right - rc.left;
        m_windowHeight = rc.bottom - rc.top;   

        return true;
    }
}

void GLApplication::destroy()
{
    if (m_pContext && m_hRC)
    {
		m_pContext->wglMakeCurrent(m_hDC, nullptr);
		m_pContext->wglDeleteContext(m_hRC);
		m_hRC = nullptr;
    }
	
    if (m_hDC)
	{
		ReleaseDC(m_hWnd, m_hDC);
		m_hDC = nullptr;
	}
    
    UnregisterClassW(m_wcl.lpszClassName, m_hInstance);
}

void GLApplication::init(int argc, wchar_t *argv[])
{
}

void GLApplication::initApplication(const wchar_t *pszWindowName)
{
    m_pszWindowName = pszWindowName;
    
    m_wcl.cbSize = sizeof(m_wcl);
    m_wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    m_wcl.lpfnWndProc = windowProc;
    m_wcl.cbClsExtra = 0;
    m_wcl.cbWndExtra = 0;
    m_wcl.hInstance = m_hInstance;
    m_wcl.hIcon = LoadIcon(0, IDI_APPLICATION);
    m_wcl.hCursor = LoadCursor(0, IDC_ARROW);
    m_wcl.hbrBackground = 0;
    m_wcl.lpszMenuName = 0;
    m_wcl.lpszClassName = L"GLApplicationWindowClass";
    m_wcl.hIconSm = 0;
}

void GLApplication::initOpenGL()
{
    PIXELFORMATDESCRIPTOR pfd
    {
        .nSize = sizeof(pfd),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24,
        .cDepthBits = 16,
        .iLayerType = PFD_MAIN_PLANE,
    };
    	
    if (!(m_pContext = OpenGLContext::createForWindow(m_hWnd, pfd)))
        throw GLApplication::Error(L"GLContext::createForWindow() failed.");

    if (!(m_hDC = GetDC(m_hWnd)))
        throw GLApplication::Error(L"GetDC() failed.");

	if (!(m_hRC = m_pContext->wglCreateContext(m_hDC)))
		throw GLApplication::Error(L"GLContext::wglCreateContext() failed.");
	
	if (!m_pContext->wglMakeCurrent(m_hDC, m_hRC))
		throw GLApplication::Error(L"GLContext::wglMakeCurrent() failed.");
}

int GLApplication::mainLoop()
{
    MSG msg{};
    
    memset(&msg, 0, sizeof(msg));
    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);

    while (true)
    {
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT)
            break;

        update();
        render();
        m_pContext->SwapBuffers(m_hDC);
    }

    return static_cast<int>(msg.wParam);
}

void GLApplication::render() const
{
    glViewport(0, 0, m_windowWidth, m_windowHeight);
    glClearColor(0.3f, 0.5f, 0.9f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GLApplication::shutdown()
{
}

void GLApplication::update()
{
}

LRESULT CALLBACK GLApplication::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    GLApplication *pApplication{nullptr};

    if (msg == WM_NCCREATE)
    {
        pApplication = reinterpret_cast<GLApplication *>((reinterpret_cast<LPCREATESTRUCT>(lParam))->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApplication));
    }
    else
    {
        pApplication = reinterpret_cast<GLApplication *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (!pApplication)
        return DefWindowProc(hWnd, msg, wParam, lParam);

    pApplication->m_hWnd = hWnd;
    return pApplication->windowProcImpl(hWnd, msg, wParam, lParam);
}

LRESULT GLApplication::windowProcImpl(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
        m_windowWidth = static_cast<int>(LOWORD(lParam));
        m_windowHeight = static_cast<int>(HIWORD(lParam));
        break;

	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	GLApplication app{L"OpenGL Application"};
	return app.run();
}