#include "main.hxx"

GLOBAL App;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_DESTROY) { PostQuitMessage(0); }
	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

int main()
{
	printf("[INFO]: %s\n", "Initialising DirectX...");

	App.hr = D3D12GetDebugInterface(IID_PPV_ARGS(&App.pDebug));
	if (SUCCEEDED(App.hr)) { App.pDebug->EnableDebugLayer(); }
	else { printf("[WARN]: %s Failed with Status 0x%08lX\n", "D3D12GetDebugInterface()", App.hr); }

	App.hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&App.pFactory);
	if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateDXGIFactory()", App.hr); goto end; }

	for (App.i = 0; App.pFactory->EnumAdapters(App.i, &App.pAdapter) != DXGI_ERROR_NOT_FOUND; ++App.i)
	{
		App.bFound = TRUE;
		App.mAdapters[App.i] = App.pAdapter;
		App.hr = App.pAdapter->GetDesc(&App.descriptor);
		if (FAILED(App.hr)) { continue; }
		
		printf("[INFO]: Device %u:\n", App.i);
		printf("\tDescription: %ls\n", App.descriptor.Description);
		printf("\tVendorId: %u\n", App.descriptor.VendorId);
		printf("\tDeviceId: %u\n", App.descriptor.DeviceId);
		printf("\tSubSysId: %u\n", App.descriptor.SubSysId);
		printf("\tRevision: %u\n", App.descriptor.Revision);
		printf("\tDedicatedVideoMemory: 0x%08lX\n", App.descriptor.DedicatedVideoMemory);
		printf("\tDedicatedSystemMemory: 0x%08lX\n", App.descriptor.DedicatedSystemMemory);
		printf("\tSharedSystemMemory: 0x%08lX\n", App.descriptor.SharedSystemMemory);
		printf("\tLUID: 0x%08lX%08lX\n", App.descriptor.AdapterLuid.HighPart, App.descriptor.AdapterLuid.LowPart);
	}
	if (!App.bFound) { printf("[WARN]: %s\n", "No Available DirectX Adapters"); goto end; }

	App.Width = 640;
	App.Height = 480;
	App.Aspect = (float)App.Width / (float)App.Height;

	App.viewport.Width = App.Width;
	App.viewport.Height = App.Height;
	App.scissorRect.right = App.Width;
	App.scissorRect.bottom = App.Height; //A real kicker if you miss this out...I hate cached variables...

	App.wndClassExA.cbSize = sizeof(WNDCLASSEXA);
	App.wndClassExA.lpszClassName = "Default";
	App.wndClassExA.lpfnWndProc = WndProc;
	App.wndClassExA.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	RegisterClassExA(&App.wndClassExA);
	App.hWnd = CreateWindowExA(WS_EX_APPWINDOW, App.wndClassExA.lpszClassName, "DirectX", WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, App.Width, App.Height, NULL, NULL, NULL, NULL);
	if (!App.hWnd) { printf("[WARN]: %s Failed\n", "CreateWindowExA()"); goto end; }

	App.hr = D3D12CreateDevice(App.mAdapters[0], D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&App.pDevice));
	if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "D3D12CreateDevice()", App.hr); goto end; }

	App.qConfig.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	App.qConfig.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	App.hr = App.pDevice->CreateCommandQueue(&App.qConfig, IID_PPV_ARGS(&App.pCmdQueue));
	if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateCommandQueue()", App.hr); goto end; }

	App.scConfig.OutputWindow = App.hWnd;                         //In this window,
	App.scConfig.Windowed = TRUE;								  //I would like
	App.scConfig.SampleDesc.Count = 1;							  //1 display
	App.scConfig.BufferCount = 2;                                 //with double buffering
	App.scConfig.BufferDesc.Width = App.Width;                    //which is 640 pixels wide
	App.scConfig.BufferDesc.Height = App.Height;                  //and 480 pixels tall
	App.scConfig.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //with Red, Green, Blue and Alpha bytes
	App.scConfig.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;   //for rendering, please.
	App.scConfig.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;      //Oh, and clear the contents after each frame.
	App.hr = App.pFactory->CreateSwapChain(App.pCmdQueue, &App.scConfig, &App.pSwapchain);
	if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateSwapChain()", App.hr); goto end; }

	//App.pSwapchain->GetCurrentBackBufferIndex();

	App.rtvConfig.NumDescriptors = 2;
	App.rtvConfig.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	App.rtvConfig.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	App.hr = App.pDevice->CreateDescriptorHeap(&App.rtvConfig, IID_PPV_ARGS(&App.pHeap));
	if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateDescriptorHeap()", App.hr); goto end; }

	App.szRTV = App.pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	App.hRTV.ptr = App.pHeap->GetCPUDescriptorHandleForHeapStart().ptr;

	for (UINT i = 0; i < 2; ++i)
	{
		App.hr = App.pSwapchain->GetBuffer(i, IID_PPV_ARGS(&App.pRenderTargets[i]));
		App.pDevice->CreateRenderTargetView(App.pRenderTargets[i], nullptr, App.hRTV);
		App.hRTV.ptr += App.szRTV; //App.hRTV.Offset(1, pRtvDescriptorSize);
	}

	App.hr = App.pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&App.pAllocator));
	if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateCommandAllocator()", App.hr); goto end; }

	App.rsConfig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	App.hr = D3D12SerializeRootSignature(&App.rsConfig, D3D_ROOT_SIGNATURE_VERSION_1, &App.pSignature, &App.pError);
	if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "D3D12SerializeRootSignature()", App.hr); goto end; }

	App.hr = App.pDevice->CreateRootSignature(0, App.pSignature->GetBufferPointer(), App.pSignature->GetBufferSize(), IID_PPV_ARGS(&(App.pRootSignature)));
	if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateRootSignature()", App.hr); goto end; }

	app(); //(!)
	while (GetMessageA(&App.msg, NULL, 0, 0)) //Cannot be threaded
	{
		DispatchMessageA(&App.msg); 
		if (App.msg.message == WM_QUIT) { break; }
	}

end:
	if (App.pFactory) { App.pFactory->Release(); }
	printf("Press Any Key to Continue...\n");
	App.any = getc(stdin);
	return 0;
}