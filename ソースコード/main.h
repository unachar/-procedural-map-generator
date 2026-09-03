#pragma once
#define _HAS_STD_BYTE 0
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>

#define NOMINMAX
#include <windows.h>
#include <assert.h>
#include <functional>
#include <chrono>


#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")


#include <DirectXMath.h>
using namespace DirectX;
using namespace std;


#include "DirectXTex.h"

#if _DEBUG
#pragma comment(lib,"DirectXTex_Debug.lib")

#else
#pragma comment(lib,"DirectXTex_Release.lib")

#endif

#include "vector.h"


#pragma comment (lib, "winmm.lib")

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"


#define SCREEN_WIDTH	(1280)
#define SCREEN_HEIGHT	(720)


HWND GetWindow();
bool IsAppActive();
bool IsMouseCursorUnlocked();
bool ShouldResetMouseLook();
void ClearResetMouseLook();

void Invoke(function<void()> Function, int Time);


static vector<float> FPSHistory;
static chrono::high_resolution_clock::time_point LastTime;

static float g_CurrentFPS;
static float g_MinFPS;
static float g_MaxFPS;
static float g_AvgFPS;

static const int MAX_HISTORY_SIZE = 1800;


