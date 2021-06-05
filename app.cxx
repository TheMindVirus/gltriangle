#include "main.hxx"

///https://docs.microsoft.com/en-gb/windows/win32/direct3d11/overviews-direct3d-11-devices-enum
///https://docs.microsoft.com/en-gb/windows/win32/direct3d12/creating-a-basic-direct3d-12-component

void app()
{
    VERTEX vertices[] =
    {
        { { 0.0f, 0.25f * App.Aspect, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.25f, -0.25f * App.Aspect, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.25f * App.Aspect, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    D3D12_INPUT_ELEMENT_DESC vertexFormat[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOUR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    App.hr = D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "VSMain", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &App.pVertexShader, NULL);
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "Vertex Shader Compilation", App.hr); return; }

    App.hr = D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &App.pFragmentShader, NULL);
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "Fragment Shader Compilation", App.hr); return; }
    
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE, FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    App.psoConfig.InputLayout = { vertexFormat, _countof(vertexFormat) };
    App.psoConfig.pRootSignature = App.pRootSignature;
    App.psoConfig.VS = { reinterpret_cast<PUINT8>(App.pVertexShader->GetBufferPointer()), App.pVertexShader->GetBufferSize() };
    App.psoConfig.PS = { reinterpret_cast<PUINT8>(App.pFragmentShader->GetBufferPointer()), App.pFragmentShader->GetBufferSize() };
    App.psoConfig.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    App.psoConfig.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    App.psoConfig.RasterizerState.FrontCounterClockwise = FALSE;
    App.psoConfig.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    App.psoConfig.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    App.psoConfig.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    App.psoConfig.RasterizerState.DepthClipEnable = TRUE;
    App.psoConfig.RasterizerState.MultisampleEnable = FALSE;
    App.psoConfig.RasterizerState.AntialiasedLineEnable = FALSE;
    App.psoConfig.RasterizerState.ForcedSampleCount = 0;
    App.psoConfig.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    App.psoConfig.BlendState.AlphaToCoverageEnable = FALSE;
    App.psoConfig.BlendState.IndependentBlendEnable = FALSE;
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    {
        App.psoConfig.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;
    }
    App.psoConfig.DepthStencilState.DepthEnable = FALSE;
    App.psoConfig.DepthStencilState.StencilEnable = FALSE;
    App.psoConfig.SampleMask = UINT_MAX;
    App.psoConfig.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    App.psoConfig.NumRenderTargets = 1;
    App.psoConfig.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    App.psoConfig.SampleDesc.Count = 1;
    App.psoConfig.SampleDesc.Quality = 0;
    App.psoConfig.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    App.hr = App.pDevice->CreateGraphicsPipelineState(&App.psoConfig, IID_PPV_ARGS(&(App.pState)));
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateGraphicsPipelineState()", App.hr); return; }
    
    App.hr = App.pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, App.pAllocator, App.pState, IID_PPV_ARGS(&App.pCommandList));
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateCommandList()", App.hr); return; }
    App.pCommandList->Close();
    
    App.hConfig.Type = D3D12_HEAP_TYPE_UPLOAD;
    App.hConfig.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    App.hConfig.CreationNodeMask = 1;
    App.hConfig.VisibleNodeMask = 1;
    App.crConfig.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    App.crConfig.Width = sizeof(vertices);
    App.crConfig.Height = 1;
    App.crConfig.DepthOrArraySize = 1;
    App.crConfig.MipLevels = 1;
    App.crConfig.Format = DXGI_FORMAT_UNKNOWN;
    App.crConfig.SampleDesc.Count = 1;
    App.crConfig.SampleDesc.Quality = 0;
    App.crConfig.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    App.crConfig.Flags = D3D12_RESOURCE_FLAG_NONE;
    App.hr = App.pDevice->CreateCommittedResource(&App.hConfig, D3D12_HEAP_FLAG_NONE, &App.crConfig, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&App.pVertexBuffer));
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateCommittedResource()", App.hr); return; }

    PUINT8 pVertexDataBegin = NULL;
    D3D12_RANGE readRange = { 0 };
    readRange.Begin = 0;
    readRange.End = 0;
    App.hr = App.pVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "Map()", App.hr); return; }

    memcpy(pVertexDataBegin, vertices, sizeof(vertices));
    App.pVertexBuffer->Unmap(0, NULL);

    App.shared.BufferLocation = App.pVertexBuffer->GetGPUVirtualAddress();
    App.shared.StrideInBytes = sizeof(VERTEX);
    App.shared.SizeInBytes = sizeof(vertices);

    App.hr = App.pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&App.pFence));
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateFence()", App.hr); return; }

    App.fenceValue = 1;
    App.hFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (App.hFenceEvent == NULL)
    {
        App.hr = HRESULT_FROM_WIN32(GetLastError());
        if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "CreateEvent()", App.hr); return; }
    }

    App.hr = App.pAllocator->Reset();
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "Allocator Reset", App.hr); return; }

    App.hr = App.pCommandList->Reset(App.pAllocator, App.pState);
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "Command List Reset", App.hr); return; }

    App.pCommandList->SetGraphicsRootSignature(App.pRootSignature);
    App.pCommandList->RSSetViewports(1, &App.viewport);
    App.pCommandList->RSSetScissorRects(1, &App.scissorRect);
    
    App.barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    App.barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    App.barrier.Transition.pResource = App.pRenderTargets[App.frameIndex];
    App.barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    App.barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    App.barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    App.pCommandList->ResourceBarrier(1, &App.barrier);
    
    App.hRTV.ptr = App.pHeap->GetCPUDescriptorHandleForHeapStart().ptr;
    //, m_frameIndex, m_rtvDescriptorSize);
    App.pCommandList->OMSetRenderTargets(1, &App.hRTV, FALSE, NULL);
    
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    App.pCommandList->ClearRenderTargetView(App.hRTV, clearColor, 0, NULL);
    App.pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    App.pCommandList->IASetVertexBuffers(0, 1, &(App.shared));
    App.pCommandList->DrawInstanced(3, 1, 0, 0);

    App.barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    App.barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    App.barrier.Transition.pResource = App.pRenderTargets[App.frameIndex];
    App.barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    App.barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    App.barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    App.pCommandList->ResourceBarrier(1, &App.barrier);
    
    App.hr = App.pCommandList->Close();
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "Command List Close", App.hr); return; }

    ID3D12CommandList* ppCommandLists[] = { App.pCommandList };
    App.pCmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    App.hr = App.pSwapchain->Present(1, 0);
    if (FAILED(App.hr)) { printf("[WARN]: %s Failed with Status 0x%08lX\n", "Present()", App.hr); return; }

    CloseHandle(App.hFenceEvent);
}