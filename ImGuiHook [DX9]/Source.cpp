#include "Hook.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"

bool ShowMenu = true;
bool ImGui_Initialised = false;

namespace Process {
	DWORD ID;
	HANDLE Handle;
	HWND Hwnd;
	HMODULE Module;
	WNDPROC WndProc;
	int WindowWidth;
	int WindowHeight;
	LPCSTR Title;
	LPCSTR ClassName;
	LPCSTR Path;
}

void InputHandler() {
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;
	int button = -1;
	if (GetAsyncKeyState(VK_LBUTTON)) button = 0;
	if (button != -1) ImGui::GetIO().MouseDown[button] = true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ShowMenu) {
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
		return true;
	}
	return CallWindowProc(Process::WndProc, hwnd, uMsg, wParam, lParam);
}

HRESULT APIENTRY MJEndScene(IDirect3DDevice9* pDevice) {
	if (!ImGui_Initialised) {
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui_ImplWin32_Init(Process::Hwnd);
		ImGui_ImplDX9_Init(pDevice);
		ImGui_ImplDX9_CreateDeviceObjects();
		ImGui::GetIO().ImeWindowHandle = Process::Hwnd;
		Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
		ImGui_Initialised = true;
	}
	if (GetAsyncKeyState(VK_INSERT) & 1) ShowMenu = !ShowMenu;
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::GetIO().MouseDrawCursor = ShowMenu;
	if (ShowMenu == true) {
		InputHandler();
		ImGui::ShowDemoWindow();
	}
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	return oEndScene(pDevice);
}

HRESULT APIENTRY MJPresent(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) {
	return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT APIENTRY MJDrawIndexedPrimitive(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
	return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT APIENTRY MJDrawPrimitive(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
	return oDrawPrimitive(pDevice, PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT APIENTRY MJSetTexture(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, IDirect3DBaseTexture9* pTexture) {
	return oSetTexture(pDevice, Sampler, pTexture);
}

HRESULT APIENTRY MJReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	return oReset(pDevice, pPresentationParameters);;
}

HRESULT APIENTRY MJSetStreamSource(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride) {
	return oSetStreamSource(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

HRESULT APIENTRY MJSetVertexDeclaration(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9* pdecl) {
	return oSetVertexDeclaration(pDevice, pdecl);
}

HRESULT APIENTRY MJSetVertexShaderConstantF(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float* pConstantData, UINT Vector4fCount) {
	return oSetVertexShaderConstantF(pDevice, StartRegister, pConstantData, Vector4fCount);
}

HRESULT APIENTRY MJSetVertexShader(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9* veShader) {
	return oSetVertexShader(pDevice, veShader);
}

HRESULT APIENTRY MJSetPixelShader(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9* piShader) {
	return oSetPixelShader(pDevice, piShader);
}

DWORD WINAPI MainThread(LPVOID lpParameter) {
	bool WindowFocus = false;
	while (WindowFocus == false) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (GetCurrentProcessId() == ForegroundWindowProcessID) {

			Process::ID = GetCurrentProcessId();
			Process::Handle = GetCurrentProcess();
			Process::Hwnd = GetForegroundWindow();

			RECT TempRect;
			GetWindowRect(Process::Hwnd, &TempRect);
			Process::WindowWidth = TempRect.right - TempRect.left;
			Process::WindowHeight = TempRect.bottom - TempRect.top;

			char TempTitle[MAX_PATH];
			GetWindowText(Process::Hwnd, TempTitle, sizeof(TempTitle));
			Process::Title = TempTitle;

			char TempClassName[MAX_PATH];
			GetClassName(Process::Hwnd, TempClassName, sizeof(TempClassName));
			Process::ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(Process::Handle, NULL, TempPath, sizeof(TempPath));
			Process::Path = TempPath;

			WindowFocus = true;
		}
	}
	bool InitHook = false;
	while (InitHook == false) {
		WNDCLASSEX WindowClass;
		HWND WindowHwnd;

		WindowClass.cbSize = sizeof(WNDCLASSEX);
		WindowClass.style = CS_HREDRAW | CS_VREDRAW;
		WindowClass.lpfnWndProc = DefWindowProc;
		WindowClass.cbClsExtra = 0;
		WindowClass.cbWndExtra = 0;
		WindowClass.hInstance = Process::Module;
		WindowClass.hIcon = NULL;
		WindowClass.hCursor = NULL;
		WindowClass.hbrBackground = NULL;
		WindowClass.lpszMenuName = NULL;
		WindowClass.lpszClassName = "MJ";
		WindowClass.hIconSm = NULL;
		RegisterClassEx(&WindowClass);
		WindowHwnd = CreateWindow(WindowClass.lpszClassName, "DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, WindowClass.hInstance, NULL);
		if (WindowHwnd == NULL) {
			break;
		}

		LPDIRECT3D9 Direct3D9 = ((LPDIRECT3D9(__stdcall*)(uint32_t))(Direct3DCreate9))(D3D_SDK_VERSION);
		if (Direct3D9 == NULL) {
			DestroyWindow(WindowHwnd);
			UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
			break;
		}

		D3DDISPLAYMODE DisplayMode;
		if (Direct3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &DisplayMode) < 0) {
			DestroyWindow(WindowHwnd);
			UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
			break;
		}
			
		D3DPRESENT_PARAMETERS Params;
		Params.BackBufferWidth = 0;
		Params.BackBufferHeight = 0;
		Params.BackBufferFormat = DisplayMode.Format;
		Params.BackBufferCount = 0;
		Params.MultiSampleType = D3DMULTISAMPLE_NONE;
		Params.MultiSampleQuality = NULL;
		Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		Params.hDeviceWindow = WindowHwnd;
		Params.Windowed = 1;
		Params.EnableAutoDepthStencil = 0;
		Params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
		Params.Flags = NULL;
		Params.FullScreen_RefreshRateInHz = 0;
		Params.PresentationInterval = 0;

		LPDIRECT3DDEVICE9 Device;
		if (Direct3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, WindowHwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &Params, &Device) < 0) {
			Direct3D9->Release();
			DestroyWindow(WindowHwnd);
			UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
			break;
		}

		uintx_t* dVtable = (uintx_t*)Device;
		dVtable = (uintx_t*)dVtable[0];

		oEndScene = (EndScene)dVtable[42];
		oPresent = (Present)dVtable[17];
		oDrawIndexedPrimitive = (DrawIndexedPrimitive)dVtable[82];
		oDrawPrimitive = (DrawPrimitive)dVtable[81];
		oSetTexture = (SetTexture)dVtable[65];
		oReset = (Reset)dVtable[16];
		oSetStreamSource = (SetStreamSource)dVtable[100];
		oSetVertexDeclaration = (SetVertexDeclaration)dVtable[87];
		oSetVertexShaderConstantF = (SetVertexShaderConstantF)dVtable[94];
		oSetVertexShader = (SetVertexShader)dVtable[92];
		oSetPixelShader = (SetPixelShader)dVtable[107];

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(LPVOID&)oEndScene, (PBYTE)MJEndScene);
		DetourAttach(&(LPVOID&)oPresent, (PBYTE)MJPresent);
		DetourAttach(&(LPVOID&)oDrawIndexedPrimitive, (PBYTE)MJDrawIndexedPrimitive);
		DetourAttach(&(LPVOID&)oDrawPrimitive, (PBYTE)MJDrawPrimitive);
		DetourAttach(&(LPVOID&)oSetTexture, (PBYTE)MJSetTexture);
		DetourAttach(&(LPVOID&)oReset, (PBYTE)MJReset);
		DetourAttach(&(LPVOID&)oSetStreamSource, (PBYTE)MJSetStreamSource);
		DetourAttach(&(LPVOID&)oSetVertexDeclaration, (PBYTE)MJSetVertexDeclaration);
		DetourAttach(&(LPVOID&)oSetVertexShaderConstantF, (PBYTE)MJSetVertexShaderConstantF);
		DetourAttach(&(LPVOID&)oSetVertexShader, (PBYTE)MJSetVertexShader);
		DetourAttach(&(LPVOID&)oSetPixelShader, (PBYTE)MJSetPixelShader);
		
		DetourTransactionCommit();
		Direct3D9->Release();
		Direct3D9 = NULL;
		Device->Release();
		Device = NULL;
		DestroyWindow(WindowHwnd);
		UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
		InitHook = true;
	}
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		if (ChecktDirectXVersion(DirectXVersion.D3D9) == true) {
			Process::Module = hModule;
			CreateThread(0, 0, MainThread, 0, 0, 0);
		}
		break;
	case DLL_PROCESS_DETACH:
		FreeLibraryAndExitThread(hModule, TRUE);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	default:
		break;
	}
	return TRUE;
}