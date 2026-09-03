#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "keyboard.h"
#include "mouse.h"



const char* CLASS_NAME = "AppClass";
const char* WINDOW_NAME = "DX11Game";


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


HWND g_Window;


HWND GetWindow()
{
	return g_Window;
}

static bool g_IsRunning = true;
static bool g_IsMouseUnlockRequested = false;
static bool g_ShouldResetMouseLook = false;

bool IsAppActive()
{
	return g_IsRunning;
}

bool IsMouseCursorUnlocked()
{
	return !g_IsRunning || g_IsMouseUnlockRequested;
}

bool ShouldResetMouseLook()
{
	return g_ShouldResetMouseLook;
}

void ClearResetMouseLook()
{
	g_ShouldResetMouseLook = false;
}




int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{


	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CLASS_NAME;
		wcex.hIconSm = nullptr;

		RegisterClassEx(&wcex);


		RECT rc = { 0, 0, (LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_Window = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	}

	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);


	Manager::Init();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(g_Window);
	ImGui_ImplDX11_Init(Renderer::GetDevice(), Renderer::GetDeviceContext());

	Keyboard_Initialize();
	Mouse_Initialize(g_Window);



	ShowWindow(g_Window, nCmdShow);
	UpdateWindow(g_Window);




	DWORD dwExecLastTime;
	DWORD dwCurrentTime;
	timeBeginPeriod(1);
	dwExecLastTime = timeGetTime();
	dwCurrentTime = 0;



	MSG msg;
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			dwCurrentTime = timeGetTime();

			if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))
			{
				dwExecLastTime = dwCurrentTime;

				if (g_IsRunning)
				{
					Manager::Update();

					Manager::Draw();

				}
				else
				{
					Sleep(1);
				}
		
			}
		}
	}

	timeEndPeriod(1);

	UnregisterClass(CLASS_NAME, wcex.hInstance);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Manager::Uninit();


	Mouse_Finalize();

	CoUninitialize();

	return (int)msg.wParam;
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
		return true;
	}

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		break;

	case WM_SETCURSOR:
		if (IsMouseCursorUnlocked())
		{
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		}
		else
		{
			SetCursor(NULL);
		}
		return TRUE;

	case WM_ACTIVATEAPP:
		g_IsRunning = (bool)wParam;
		if (!g_IsRunning)
		{
			g_IsMouseUnlockRequested = false;
			Mouse_SetMode(MOUSE_POSITION_MODE_ABSOLUTE);
			Mouse_SetVisible(true);
		}
		else
		{
			g_ShouldResetMouseLook = true;
			Mouse_SetPositionToCenter();
		}
		break;
	case WM_SYSKEYDOWN:
		if (wParam == VK_MENU)
		{
			g_IsMouseUnlockRequested = true;
			Mouse_SetMode(MOUSE_POSITION_MODE_ABSOLUTE);
			Mouse_SetVisible(true);
		}
		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam == VK_MENU)
		{
			g_IsMouseUnlockRequested = false;
			g_ShouldResetMouseLook = true;
		}
		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		if (uMsg == WM_SYSKEYUP && wParam == VK_MENU)
		{
			return 0;
		}
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse_ProcessMessage(uMsg, wParam, lParam);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}