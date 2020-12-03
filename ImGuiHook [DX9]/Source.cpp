#include "Hook.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"

void InputHandler()
{
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;
	int button = -1;
	if (GetAsyncKeyState(VK_LBUTTON)) button = 0;
	if (button != -1) ImGui::GetIO().MouseDown[button] = true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ShowMenu && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}
	return CallWindowProc(ProcessWndProc, hWnd, msg, wParam, lParam);
}

HRESULT APIENTRY MJHook_EndScene(IDirect3DDevice9* pDevice){
	if (pDevice == nullptr) return EndScene_orig(pDevice);
	if (!ImGui_Initialised)
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		D3DDEVICE_CREATION_PARAMETERS d3dcp;
		pDevice->GetCreationParameters(&d3dcp);
		ProcessHwnd = d3dcp.hFocusWindow;
		if (ProcessHwnd != NULL){
			ProcessWndProc = (WNDPROC)SetWindowLongPtr(ProcessHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			ImGui_ImplWin32_Init(ProcessHwnd);
			ImGui_ImplDX9_Init(pDevice);
			ImGui::GetIO().ImeWindowHandle = ProcessHwnd;
			ImGui_Initialised = true;
		}
	}
	if (GetAsyncKeyState(VK_INSERT) & 1) ShowMenu = !ShowMenu;
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (ShowMenu == true) {
		InputHandler();
		ImGui::ShowDemoWindow();
		ImGui::GetIO().MouseDrawCursor = 1;
	}
	else {
		ImGui::GetIO().MouseDrawCursor = 0;
	}
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	return EndScene_orig(pDevice);
}

HRESULT APIENTRY MJHook_Present(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion){
	return Present_orig(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT APIENTRY MJHook_DrawIndexedPrimitive(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount){
	return DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT APIENTRY MJHook_DrawPrimitive(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount){
	return DrawPrimitive_orig(pDevice, PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT APIENTRY MJHook_SetTexture(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, IDirect3DBaseTexture9* pTexture){
	return SetTexture_orig(pDevice, Sampler, pTexture);
}

HRESULT APIENTRY MJHook_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters){
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT ResetReturn = Reset_orig(pDevice, pPresentationParameters);
	ImGui_ImplDX9_CreateDeviceObjects();
	return ResetReturn;
}

HRESULT APIENTRY MJHook_SetStreamSource(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride){
	if (StreamNumber == 0)
		Stride = sStride;

	return SetStreamSource_orig(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

HRESULT APIENTRY MJHook_SetVertexDeclaration(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9* pdecl){
	if (pdecl != NULL) {
		pdecl->GetDeclaration(decl, &numElements);
	}
	return SetVertexDeclaration_orig(pDevice, pdecl);
}

HRESULT APIENTRY MJHook_SetVertexShaderConstantF(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float* pConstantData, UINT Vector4fCount){
	if (pConstantData != NULL) {
		mStartregister = StartRegister;
		mVectorCount = Vector4fCount;
	}
	return SetVertexShaderConstantF_orig(pDevice, StartRegister, pConstantData, Vector4fCount);
}

HRESULT APIENTRY MJHook_SetVertexShader(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9* veShader){
	if (veShader != NULL) {
		vShader = veShader;
		vShader->GetFunction(NULL, &vSize);
	}
	return SetVertexShader_orig(pDevice, veShader);
}

HRESULT APIENTRY MJHook_SetPixelShader(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9* piShader){
	if (piShader != NULL) {
		pShader = piShader;
		pShader->GetFunction(NULL, &pSize);
	}
	return SetPixelShader_orig(pDevice, piShader);
}

DWORD WINAPI Thread(LPVOID lpParameter) {
	while (!GetModuleHandleA("d3d9.dll")) {
		Sleep(200);
	}
	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;
	HWND tmpWnd = CreateWindowA("BUTTON", "DX", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, Module, NULL);
	if (tmpWnd == NULL) {
		return 0;
	}
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL) {
		DestroyWindow(tmpWnd);
		return 0;
	}
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = tmpWnd;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tmpWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
	if (result != D3D_OK) {
		d3d->Release();
		DestroyWindow(tmpWnd);
		return 0;
	}
    #if defined _M_X64
	DWORD64* dVtable = (DWORD64*)d3ddev;
	dVtable = (DWORD64*)dVtable[0];
    #elif defined _M_IX86
	DWORD* dVtable = (DWORD*)d3ddev;
	dVtable = (DWORD*)dVtable[0];
    #endif
	SetStreamSource_orig = (SetStreamSource)dVtable[100];
	SetVertexDeclaration_orig = (SetVertexDeclaration)dVtable[87];
	SetVertexShaderConstantF_orig = (SetVertexShaderConstantF)dVtable[94];
	SetVertexShader_orig = (SetVertexShader)dVtable[92];
	SetPixelShader_orig = (SetPixelShader)dVtable[107];
	DrawIndexedPrimitive_orig = (DrawIndexedPrimitive)dVtable[82];
	DrawPrimitive_orig = (DrawPrimitive)dVtable[81];
	SetTexture_orig = (SetTexture)dVtable[65];
	Present_orig = (Present)dVtable[17];
	EndScene_orig = (EndScene)dVtable[42];
	Reset_orig = (Reset)dVtable[16];
	Sleep(2000);
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(LPVOID&)SetStreamSource_orig, (PBYTE)MJHook_SetStreamSource);
	DetourAttach(&(LPVOID&)SetVertexDeclaration_orig, (PBYTE)MJHook_SetVertexDeclaration);
	DetourAttach(&(LPVOID&)SetVertexShaderConstantF_orig, (PBYTE)MJHook_SetVertexShaderConstantF);
	DetourAttach(&(LPVOID&)SetVertexShader_orig, (PBYTE)MJHook_SetVertexShader);
	DetourAttach(&(LPVOID&)SetPixelShader_orig, (PBYTE)MJHook_SetPixelShader);
	DetourAttach(&(LPVOID&)DrawIndexedPrimitive_orig, (PBYTE)MJHook_DrawIndexedPrimitive);
	DetourAttach(&(LPVOID&)DrawPrimitive_orig, (PBYTE)MJHook_DrawPrimitive);
	DetourAttach(&(LPVOID&)SetTexture_orig, (PBYTE)MJHook_SetTexture);
	DetourAttach(&(LPVOID&)Present_orig, (PBYTE)MJHook_Present);
	DetourAttach(&(LPVOID&)EndScene_orig, (PBYTE)MJHook_EndScene);
	DetourAttach(&(LPVOID&)Reset_orig, (PBYTE)MJHook_Reset);
	DetourTransactionCommit();
	d3ddev->Release();
	d3d->Release();
	DestroyWindow(tmpWnd);
	return 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved){
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		Module = hModule;
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, Thread, 0, 0, 0);
		break;
	}
	return TRUE;
}


