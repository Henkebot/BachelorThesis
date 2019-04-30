#include "DX12Wrap.h"
#include "GPUPch.h"

#include "CommandQueue.h"
#include "CommandRecorder.h"

#include "Utillity/d3dx12.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Utillity/stb_image.h"

namespace DX12Var
{
	// DX12 objects ---------------------------------
	ComPtr<ID3D12Device4> gDevice;
	ComPtr<IDXGIFactory4> gFactory;

	// Queues ---------------------------------------
	CommandQueue gDirectQueue;
	CommandRecorder gDirectRecorder;

	CommandQueue gComputeQueue;
	CommandRecorder gComputeRecorder;

	// RTV -----------------------------------------
	ComPtr<ID3D12DescriptorHeap> gRTVHeap;
	UINT gRTVDescriptorSize;
	ComPtr<IDXGISwapChain4> gSwapChain;
	UINT gFrameIndex;
	ComPtr<ID3D12Resource> gRTResource[NUM_BACKBUFFERS];

	// SRV and UAV ----------------------------------
	enum SRV_HEAP_LOCATIONS : UINT
	{
		UAV_TEXTURE0 = 0,
		SRV_TEXTURE0,
		SRV_DCT_MATRIX,
		SRV_DCT_MATRIX_TRANSPOSE,
		NUM_OF_SRV_CBV_UAV

	};
	ComPtr<ID3D12DescriptorHeap> gSRVUAVHeap;
	UINT gSRVUAVCBVDescriptorSize;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHeapHandleAt(unsigned int index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = gSRVUAVHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += index * gSRVUAVCBVDescriptorSize;
		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHeapHandleAt(unsigned int index)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = gSRVUAVHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += index * gSRVUAVCBVDescriptorSize;
		return handle;
	}
	// Image SRV
	ComPtr<ID3D12Resource> gImageResource;
	ComPtr<ID3D12Resource> gImageUpload;

	// UAV output
	ComPtr<ID3D12Resource> gOutputImage;

	// Compute --------------------------------------
	enum ROOT_COMPUTE_SLOTS : UINT
	{
		ROOT_UAV0 = 0,
		ROOT_SRV0,
		ROOT_CONSTANTS0,
		NUM_OF_ROOT_SLOTS

	};
	enum GAZE_PIPELINES : UINT
	{
		GAZE_LINEAR = 0,
		GAZE_LOG,
		GAZE_EXP,
		NUM_OF_GAZE_FUNCTIONS
	};
	UINT gCurrentGazeFunction;
	ComPtr<ID3D12RootSignature> gComputeRoot;
	ComPtr<ID3D12PipelineState> gComputeGazePipeline[NUM_OF_GAZE_FUNCTIONS];
	ComPtr<ID3D12PipelineState> gComputeRAWPipeline;

	// DCT
	ComPtr<ID3D12Resource> gDCTMatrix;
	ComPtr<ID3D12Resource> gDCTMatrixUpload;

	ComPtr<ID3D12Resource> gDCTMatrixTranspose;
	ComPtr<ID3D12Resource> gDCTMatrixTransposeUpload;

} // namespace DX12Var

namespace DX11Var
{
	UINT d11Flags;
	ComPtr<ID3D11DeviceContext> gDevice11Context;
	ComPtr<ID3D11On12Device> gDevice11On12;
	ComPtr<ID3D11Resource> gWrappedResource[NUM_BACKBUFFERS];
	ComPtr<ID2D1Bitmap1> gD2DRenderTarget[NUM_BACKBUFFERS];

	ComPtr<ID2D1SolidColorBrush> gTextBrush;
	ComPtr<IDWriteTextFormat> gDTextFormat;

	D2D1_FACTORY_OPTIONS d2dFactoryOptions;

	ComPtr<ID2D1Factory3> gD2DFactory;
	ComPtr<ID2D1Device2> gD2DDevice;
	ComPtr<ID2D1DeviceContext> gD2DDeviceContext;
	ComPtr<IDWriteFactory> gDWriteFactory;
} // namespace DX11Var

namespace GazePoint
{
	int lastX, lastY;
	int currentX, currentY;

	bool NewPoint()
	{
		if(lastX == currentX && lastY == currentY)
			return false;

		lastX = currentX;
		lastY = currentY;
		return true;
	}

} // namespace GazePoint

namespace Screen
{
	int width, height;
	int width8, height8;
} // namespace Screen

namespace Debug
{
	const WCHAR* GetCurrentRadialFunctionWStr()
	{
		switch (DX12Var::gCurrentGazeFunction)
		{
		case DX12Var::GAZE_LINEAR:
			return L"LIN";
		case DX12Var::GAZE_LOG:
			return L"LOG";
		case DX12Var::GAZE_EXP:
			return L"EXP";
		}
		return L"NUL";
	}
} // namespace Debug

// Returns a image in 32 bit format
BYTE* LoadPPM(const char* _pTexturePath, int* _pWidth, int* _pHeight)
{
	char buf[16];
	FILE* pFile;
	int width, height, colorComp;
	BYTE* pData24Bit;
	BYTE* pData32bit;

	fopen_s(&pFile, _pTexturePath, "rb");
	if(nullptr == pFile)
		return nullptr;

	if(false == fgets(buf, sizeof(buf), pFile))
		return nullptr;

	if(buf[0] != 'P' || buf[1] != '6')
		return nullptr;

	//check for comments
	int c = getc(pFile);
	while(c == '#')
	{
		while(getc(pFile) != '\n')
			;
		c = getc(pFile);
	}

	ungetc(c, pFile);

	if(fscanf_s(pFile, "%d %d %d", &width, &height, &colorComp) != 3)
		return nullptr;

	if(colorComp != 255 && colorComp != ((256 * 256) - 1))
		return nullptr;

	colorComp = (colorComp > 255) ? 2 : 1;

	getc(pFile);

	pData24Bit = new BYTE[width * height * (3 * colorComp)];

	pData32bit = new BYTE[width * height * (4 * colorComp)];

	if(fread(reinterpret_cast<void*>(pData24Bit), 3 * width * height, 1, pFile) != 1)
		return nullptr;

	fclose(pFile);

	// Convert to 32 bit
	for(int i = 0; i < width * height; i++)
	{
		int index24 = i * 3;
		int index32 = i * 4;

		int j = 0;
		for(; j < (3 * colorComp); j++)
		{
			pData32bit[index32 + j] = pData24Bit[index24 + j];
		}

		for(int k = 0; k < colorComp; k++)
		{
			pData32bit[index32 + j + k] = 255;
		}
	}

	delete[] pData24Bit;

	*_pWidth  = width;
	*_pHeight = height;
	return pData32bit;
}

namespace Impl
{
	using namespace DX12Var;
	using namespace DX11Var;

	void CreateDevice(void)
	{

		UINT dxgiFactoryFlags = 0;
		d11Flags			  = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if _DEBUG
		ComPtr<ID3D12Debug> debugController;
		if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			d11Flags |= D3D11_CREATE_DEVICE_DEBUG;
			d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

			/*ComPtr<ID3D12Debug1> debugcontroller1;
			debugController->QueryInterface(IID_PPV_ARGS(&debugcontroller1));
			debugcontroller1->SetEnableGPUBasedValidation(true);*/
		}

#endif

		TIF(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&gFactory)));

		// Create the DX12 device
		{
			ComPtr<IDXGIAdapter1> pAdapter;
			for(UINT adapterIdx = 0;
				DXGI_ERROR_NOT_FOUND != gFactory->EnumAdapters1(adapterIdx, &pAdapter);
				adapterIdx++)
			{

				DXGI_ADAPTER_DESC1 desc;
				pAdapter->GetDesc1(&desc);

				// We dont want a software adapter
				if(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
					continue;

				if(SUCCEEDED(D3D12CreateDevice(
					   pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&gDevice))))
				{
					printf_s("Seleceted Adapter: %ls\n", desc.Description);
					break;
				}
			}
		}
	}

	void CreateD3D11(void)
	{

		// Create D11Device
		{
			ComPtr<ID3D11Device> d3d11Device;

			TIF(D3D11On12CreateDevice(gDevice.Get(),
									  d11Flags,
									  nullptr,
									  0,
									  reinterpret_cast<IUnknown**>(gDirectQueue.GetAddressOf()),
									  1,
									  0,
									  &d3d11Device,
									  &gDevice11Context,
									  nullptr));

			TIF(d3d11Device.As(&gDevice11On12));
		}

		// D2D / DWrite Components
		{
			TIF(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
								  __uuidof(ID2D1Factory3),
								  &d2dFactoryOptions,
								  &gD2DFactory));

			ComPtr<IDXGIDevice> dxgiDevice;
			TIF(gDevice11On12.As(&dxgiDevice));
			TIF(gD2DFactory->CreateDevice(dxgiDevice.Get(), &gD2DDevice));
			TIF(gD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
												&gD2DDeviceContext));
			TIF(DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &gDWriteFactory));
		}

		{
			TIF(gD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::AntiqueWhite),
														 &gTextBrush));

			TIF(gDWriteFactory->CreateTextFormat(L"Verdana",
												 nullptr,
												 DWRITE_FONT_WEIGHT_NORMAL,
												 DWRITE_FONT_STYLE_NORMAL,
												 DWRITE_FONT_STRETCH_NORMAL,
												 45,
												 L"en-us",
												 &gDTextFormat));
			TIF(gDTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
			TIF(gDTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
		}
	}

	void CreateQueuesAndLists(void)
	{

		gDirectQueue.Init(gDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
		gDirectRecorder.Init(gDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);

		gComputeQueue.Init(gDevice.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
		gComputeRecorder.Init(gDevice.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
	}

	void CreateSwapChain(const HWND& _WindowHandle, unsigned int _Width, unsigned int _Height)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type						= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NumDescriptors				= NUM_BACKBUFFERS;
		desc.Flags						= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask					= 1;

		TIF(gDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&gRTVHeap)));

		gRTVDescriptorSize =
			gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.BufferCount			 = NUM_BACKBUFFERS;
		sd.Width				 = _Width;
		sd.Height				 = _Height;
		sd.Format				 = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.Flags				 = 0; //NOTE(Henrik): Check for tearing support
		sd.BufferUsage			 = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.SampleDesc.Count		 = 1;
		sd.SampleDesc.Quality	= 0;
		sd.SwapEffect			 = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.AlphaMode			 = DXGI_ALPHA_MODE_UNSPECIFIED;
		sd.Scaling				 = DXGI_SCALING_STRETCH;
		sd.Stereo				 = FALSE;

		ComPtr<IDXGISwapChain1> swapChain1;
		TIF(gFactory->CreateSwapChainForHwnd(
			gDirectQueue.Get(), _WindowHandle, &sd, nullptr, nullptr, &swapChain1));
		TIF(swapChain1->QueryInterface(IID_PPV_ARGS(&gSwapChain)));

		gFrameIndex = gSwapChain->GetCurrentBackBufferIndex();

		D3D12_CPU_DESCRIPTOR_HANDLE handle = gRTVHeap->GetCPUDescriptorHandleForHeapStart();

		UINT dpi = GetDpiForWindow(GetActiveWindow());

		D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			static_cast<FLOAT>(dpi),
			static_cast<FLOAT>(dpi));

		for(UINT i = 0; i < NUM_BACKBUFFERS; i++)
		{
			TIF(gSwapChain->GetBuffer(i, IID_PPV_ARGS(&gRTResource[i])));
			gDevice->CreateRenderTargetView(gRTResource[i].Get(), nullptr, handle);
			handle.ptr += gRTVDescriptorSize;

			/* Create a wrapped 11on12 resource of this back buffer. Since we are
			 rendering all D3D12 content first and then all D2D content, we specify
			 the In resource state as RENDER_TARGET - because D3D12 will have last
			 used it in this state - and the Out resource state as PRESENT. When
			 ReleaseWrappedResources() is called on the 11on12 device, the resource 
			 will be transitioned to the PRESENT state.*/

			D3D11_RESOURCE_FLAGS d3d11Flags = {D3D11_BIND_RENDER_TARGET};
			TIF(gDevice11On12->CreateWrappedResource(gRTResource[i].Get(),
													 &d3d11Flags,
													 D3D12_RESOURCE_STATE_RENDER_TARGET,
													 D3D12_RESOURCE_STATE_PRESENT,
													 IID_PPV_ARGS(&gWrappedResource[i])));

			// Create a render target for the D2D to draw directly to this back buffer.
			ComPtr<IDXGISurface> surface;
			TIF(gWrappedResource[i].As(&surface));
			TIF(gD2DDeviceContext->CreateBitmapFromDxgiSurface(
				surface.Get(), &bitmapProperties, &gD2DRenderTarget[i]));
		}
	}

	void
	CreateSRVBuffer(ID3D12Resource* _Destination, ID3D12Resource* _Heap, float* _pData, int SRVSlot)
	{
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels			= 1;
		textureDesc.Format				= DXGI_FORMAT_UNKNOWN;
		textureDesc.Width				= sizeof(float) * 64;
		textureDesc.Height				= 1;
		textureDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize	= 1;
		textureDesc.SampleDesc.Count	= 1;
		textureDesc.SampleDesc.Quality  = 0;
		textureDesc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
		textureDesc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		D3D12_HEAP_PROPERTIES heapProp = {};
		heapProp.CPUPageProperty	   = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.CreationNodeMask	  = 1;
		heapProp.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.Type				   = D3D12_HEAP_TYPE_DEFAULT;
		heapProp.VisibleNodeMask	   = 1;

		TIF(gDevice->CreateCommittedResource(&heapProp,
											 D3D12_HEAP_FLAG_NONE,
											 &textureDesc,
											 D3D12_RESOURCE_STATE_COPY_DEST,
											 nullptr,
											 IID_PPV_ARGS(&_Destination)));

		// Get the size needed for this texture buffer
		UINT64 uploadBufferSize;
		gDevice->GetCopyableFootprints(
			&_Destination->GetDesc(), 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

		// This is the GPU upload buffer.
		D3D12_HEAP_PROPERTIES heapProp2 = {};
		heapProp2.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp2.CreationNodeMask		= 1;
		heapProp2.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp2.Type					= D3D12_HEAP_TYPE_UPLOAD;
		heapProp2.VisibleNodeMask		= 1;

		{

			D3D12_RESOURCE_DESC resDesc = {};
			resDesc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
			resDesc.Alignment			= 0;
			resDesc.Format				= DXGI_FORMAT_UNKNOWN;
			resDesc.DepthOrArraySize	= 1;
			resDesc.Height				= 1;
			resDesc.Width				= uploadBufferSize;
			resDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;
			resDesc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resDesc.MipLevels			= 1;
			resDesc.SampleDesc.Count	= 1;

			TIF(gDevice->CreateCommittedResource(&heapProp2,
												 D3D12_HEAP_FLAG_NONE,
												 &resDesc,
												 D3D12_RESOURCE_STATE_GENERIC_READ,
												 nullptr,
												 IID_PPV_ARGS(&_Heap)));
		}

		UINT64 requiredSize = 0;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts;
		UINT NumRows;
		UINT64 RowSizeInBytes;

		D3D12_RESOURCE_DESC resDesc = _Destination->GetDesc();
		gDevice->GetCopyableFootprints(
			&resDesc, 0, 1, 0, &layouts, &NumRows, &RowSizeInBytes, &requiredSize);

		BYTE* pData;
		TIF(_Heap->Map(0, nullptr, reinterpret_cast<LPVOID*>(&pData)));

		memcpy(pData, _pData, sizeof(float) * 64);

		_Heap->Unmap(0, nullptr);

		gDirectRecorder->CopyBufferRegion(
			_Destination, 0, _Heap, layouts.Offset, layouts.Footprint.RowPitch);

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource   = _Destination;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = 0;
		barrier.Type				   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags				   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		gDirectRecorder->ResourceBarrier(1, &barrier);

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping			= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format							= DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension					= D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement				= 0;
		srvDesc.Buffer.NumElements				= 64;
		srvDesc.Buffer.StructureByteStride		= sizeof(float);

		gDevice->CreateShaderResourceView(
			_Destination, &srvDesc, DX12Var::GetCPUHeapHandleAt(SRVSlot));
	}

	void CreateSRVCBVUAVResources(void)
	{
		// Create Descriptor Heap
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors				= NUM_OF_SRV_CBV_UAV;
		heapDesc.Type						= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags						= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		TIF(gDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&gSRVUAVHeap)));

		gSRVUAVCBVDescriptorSize =
			gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Create Output resource
		{
			DXGI_SWAP_CHAIN_DESC swapDesc;
			TIF(gSwapChain->GetDesc(&swapDesc));

			//The dimensions and format should match the swap-chain
			D3D12_RESOURCE_DESC resDesc = {};
			resDesc.DepthOrArraySize	= 1;
			resDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resDesc.Format				= swapDesc.BufferDesc.Format;
			resDesc.Flags				= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			resDesc.Width				= swapDesc.BufferDesc.Width;
			resDesc.Height				= swapDesc.BufferDesc.Height;
			resDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resDesc.MipLevels			= 1;
			resDesc.SampleDesc.Count	= 1;

			D3D12_HEAP_PROPERTIES resHeap = {};
			resHeap.CPUPageProperty		  = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			resHeap.CreationNodeMask	  = 1;
			resHeap.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
			resHeap.Type				  = D3D12_HEAP_TYPE_DEFAULT;
			resHeap.VisibleNodeMask		  = 1;

			TIF(gDevice->CreateCommittedResource(&resHeap,
												 D3D12_HEAP_FLAG_NONE,
												 &resDesc,
												 D3D12_RESOURCE_STATE_COPY_SOURCE,
												 nullptr,
												 IID_PPV_ARGS(&gOutputImage)));
			NAME_D3D12_OBJECT(gOutputImage);

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format							 = resDesc.Format;
			uavDesc.ViewDimension					 = D3D12_UAV_DIMENSION_TEXTURE2D;

			gDevice->CreateUnorderedAccessView(
				gOutputImage.Get(), nullptr, &uavDesc, DX12Var::GetCPUHeapHandleAt(UAV_TEXTURE0));
		}
		gDirectRecorder.Reset();
		// Create DCT Matrix resource
		{

			float DCT_MATRIX[64];
			float DCT_MATRIX_TRANSPOSE[64];
			//compute dct matrix
			for(int y = 0; y < 8; y++)
			{
				for(int x = 0; x < 8; x++)
				{

					if(0 == y)
					{
						DCT_MATRIX[y * 8 + x] = float(1.0 / sqrt(8.0));
					}
					else
					{

						DCT_MATRIX[y * 8 + x] =
							float(sqrt(2.0 / 8.0) *
								  cos(((2 * x + 1) * DirectX::XM_PI * y) / (2.0 * 8.0)));
					}
				}
			}
			CreateSRVBuffer(gDCTMatrix.Get(), gDCTMatrixUpload.Get(), DCT_MATRIX, SRV_DCT_MATRIX);

			//compute dct transpose matrix
			for(int y = 0; y < 8; y++)
			{
				for(int x = 0; x < 8; x++)
				{
					DCT_MATRIX_TRANSPOSE[y * 8 + x] = DCT_MATRIX[x * 8 + y];
				}
			}

			CreateSRVBuffer(gDCTMatrixTranspose.Get(),
							gDCTMatrixTransposeUpload.Get(),
							DCT_MATRIX_TRANSPOSE,
							SRV_DCT_MATRIX_TRANSPOSE);
		}
		gDirectRecorder->Close();
		gDirectQueue.Submit(gDirectRecorder.Get());
		gDirectQueue.Flush();
		gDirectQueue.WaitForComplete();
	}

	void CreateTexture(BYTE* _pTexData, int _Width, int _Height)
	{

		// Create resource
		{
			D3D12_RESOURCE_DESC texDesc = {};
			texDesc.MipLevels			= 1;
			texDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
			texDesc.Width				= _Width;
			texDesc.Height				= _Height;
			texDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;
			texDesc.DepthOrArraySize	= 1;
			texDesc.SampleDesc.Count	= 1;
			texDesc.SampleDesc.Quality  = 0;
			texDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;

			D3D12_HEAP_PROPERTIES heapProp = {};
			heapProp.CPUPageProperty	   = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProp.CreationNodeMask	  = 1;
			heapProp.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
			heapProp.Type				   = D3D12_HEAP_TYPE_DEFAULT;
			heapProp.VisibleNodeMask	   = 1;

			TIF(gDevice->CreateCommittedResource(&heapProp,
												 D3D12_HEAP_FLAG_NONE,
												 &texDesc,
												 D3D12_RESOURCE_STATE_COPY_DEST,
												 nullptr,
												 IID_PPV_ARGS(&gImageResource)));
			NAME_D3D12_OBJECT(gImageResource);
		}

		// Create Upload heap
		{
			UINT64 uploadBufferSize;

			D3D12_HEAP_PROPERTIES heapProp2 = {};
			heapProp2.CPUPageProperty		= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProp2.CreationNodeMask		= 1;
			heapProp2.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
			heapProp2.Type					= D3D12_HEAP_TYPE_UPLOAD;
			heapProp2.VisibleNodeMask		= 1;

			gDevice->GetCopyableFootprints(
				&gImageResource->GetDesc(), 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

			D3D12_RESOURCE_DESC resDesc = {};
			resDesc.Dimension			= D3D12_RESOURCE_DIMENSION_BUFFER;
			resDesc.Alignment			= 0;
			resDesc.Format				= DXGI_FORMAT_UNKNOWN;
			resDesc.DepthOrArraySize	= 1;
			resDesc.Height				= 1;
			resDesc.Width				= uploadBufferSize;
			resDesc.Flags				= D3D12_RESOURCE_FLAG_NONE;
			resDesc.Layout				= D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resDesc.MipLevels			= 1;
			resDesc.SampleDesc.Count	= 1;

			TIF(gDevice->CreateCommittedResource(&heapProp2,
												 D3D12_HEAP_FLAG_NONE,
												 &resDesc,
												 D3D12_RESOURCE_STATE_GENERIC_READ,
												 nullptr,
												 IID_PPV_ARGS(&gImageUpload)));
			NAME_D3D12_OBJECT(gImageUpload);
		}

		// Upload the texture to the resource
		{

			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData				   = _pTexData;
			textureData.RowPitch			   = _Width * 4;
			textureData.SlicePitch			   = textureData.RowPitch * _Height;

			gDirectRecorder.Reset();

			UpdateSubresources(gDirectRecorder.Get(),
							   gImageResource.Get(),
							   gImageUpload.Get(),
							   0,
							   0,
							   1,
							   &textureData);

			D3D12_RESOURCE_BARRIER barrier;
			barrier.Transition.pResource   = gImageResource.Get();
			barrier.Transition.Subresource = 0;
			barrier.Type				   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags				   = D3D12_RESOURCE_BARRIER_FLAG_NONE;

			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

			gDirectRecorder->ResourceBarrier(1, &barrier);

			gDirectRecorder->Close();

			gDirectQueue.Submit(gDirectRecorder.Get());
			gDirectQueue.Flush();
			gDirectQueue.WaitForComplete();
		}

		// Create SRV for the texture
		{
			D3D12_RESOURCE_DESC imageDesc = gImageResource->GetDesc();

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping			= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format							= imageDesc.Format;
			srvDesc.ViewDimension					= D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels				= 1;

			gDevice->CreateShaderResourceView(
				gImageResource.Get(), &srvDesc, DX12Var::GetCPUHeapHandleAt(SRV_TEXTURE0));
		}
	}

	void CreateRootSignatures(void)
	{
		// Compute Root signature
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			featureData.HighestVersion					  = D3D_ROOT_SIGNATURE_VERSION_1_1;
			if(FAILED(gDevice->CheckFeatureSupport(
				   D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			{
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}

			D3D12_DESCRIPTOR_RANGE1 ranges[2];

			ranges[0].RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			ranges[0].NumDescriptors					= 1; // For output resource
			ranges[0].BaseShaderRegister				= 0;
			ranges[0].RegisterSpace						= 0;
			ranges[0].Flags								= D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
			ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			ranges[1].RangeType			 = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			ranges[1].NumDescriptors	 = 3; // NOTE(Henrik): 1 for texture, 2 for dct
			ranges[1].BaseShaderRegister = 0;
			ranges[1].RegisterSpace		 = 0;
			ranges[1].Flags				 = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
			ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER1 rootParameters[NUM_OF_ROOT_SLOTS];

			rootParameters[ROOT_UAV0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameters[ROOT_UAV0].DescriptorTable.NumDescriptorRanges = 1;
			rootParameters[ROOT_UAV0].DescriptorTable.pDescriptorRanges   = &ranges[0];
			rootParameters[ROOT_UAV0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			rootParameters[ROOT_SRV0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameters[ROOT_SRV0].DescriptorTable.NumDescriptorRanges = 1;
			rootParameters[ROOT_SRV0].DescriptorTable.pDescriptorRanges   = &ranges[1];
			rootParameters[ROOT_SRV0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			rootParameters[ROOT_CONSTANTS0].ParameterType =
				D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			rootParameters[ROOT_CONSTANTS0].Constants.Num32BitValues = 2; // two for gaze point
			rootParameters[ROOT_CONSTANTS0].Constants.RegisterSpace  = 0;
			rootParameters[ROOT_CONSTANTS0].Constants.ShaderRegister = 0;
			rootParameters[ROOT_CONSTANTS0].ShaderVisibility		 = D3D12_SHADER_VISIBILITY_ALL;

			D3D12_STATIC_SAMPLER_DESC sampler = {};
			sampler.Filter					  = D3D12_FILTER_ANISOTROPIC;
			sampler.AddressU				  = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.AddressV				  = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.AddressW				  = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			sampler.MipLODBias				  = 0;
			sampler.MaxAnisotropy			  = 0;
			sampler.ComparisonFunc			  = D3D12_COMPARISON_FUNC_NEVER;
			sampler.BorderColor				  = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			sampler.MinLOD					  = 0.0f;
			sampler.MaxLOD					  = D3D12_FLOAT32_MAX;
			sampler.ShaderRegister			  = 0;
			sampler.RegisterSpace			  = 0;
			sampler.ShaderVisibility		  = D3D12_SHADER_VISIBILITY_ALL;

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSig;
			rootSig.Version					   = D3D_ROOT_SIGNATURE_VERSION_1_1;
			rootSig.Desc_1_1.NumParameters	 = _countof(rootParameters);
			rootSig.Desc_1_1.pParameters	   = rootParameters;
			rootSig.Desc_1_1.NumStaticSamplers = 1;
			rootSig.Desc_1_1.pStaticSamplers   = &sampler;
			rootSig.Desc_1_1.Flags			   = D3D12_ROOT_SIGNATURE_FLAG_NONE;

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;

			TIF(D3D12SerializeVersionedRootSignature(&rootSig, &signature, &error));

			TIF(gDevice->CreateRootSignature(0,
											 signature->GetBufferPointer(),
											 signature->GetBufferSize(),
											 IID_PPV_ARGS(&gComputeRoot)));

			NAME_D3D12_OBJECT(gComputeRoot);
		}
	}

	void CreateComputeShader(const wchar_t* _pPath,
							 ID3D12PipelineState** _pPipelineState,
							 D3D_SHADER_MACRO* pDefines = nullptr)
	{
		ComPtr<ID3DBlob> computeBlob;
		ComPtr<ID3DBlob> errorPtr;
		TIF(D3DCompileFromFile(
			_pPath, pDefines, nullptr, "main", "cs_5_1", 0, 0, &computeBlob, &errorPtr));

		if(errorPtr)
		{
			printf_s("Shader Error: %s\n", (char*)errorPtr->GetBufferPointer());
		}
		D3D12_COMPUTE_PIPELINE_STATE_DESC cpsd = {};
		cpsd.pRootSignature					   = gComputeRoot.Get();
		cpsd.CS.pShaderBytecode				   = computeBlob->GetBufferPointer();
		cpsd.CS.BytecodeLength				   = computeBlob->GetBufferSize();
		cpsd.NodeMask						   = 0;

		TIF(gDevice->CreateComputePipelineState(&cpsd, IID_PPV_ARGS(_pPipelineState)));
		(*_pPipelineState)->SetName(L"ComputePipelineState");
	}

	void CreatePipleineStates(void)
	{
		D3D_SHADER_MACRO macro[2];
		macro[1].Name		= nullptr;
		macro[1].Definition = nullptr;

		// Gaze Shaders
		macro[0].Name		= "LINEAR";
		macro[0].Definition = "1";

		CreateComputeShader(L"../GPU_Codec/Shaders/ComputeGAZE.hlsl",
							gComputeGazePipeline[GAZE_LINEAR].GetAddressOf(),
							macro);

		macro[0].Name		= "LOG";
		macro[0].Definition = "1";
		CreateComputeShader(L"../GPU_Codec/Shaders/ComputeGAZE.hlsl",
							gComputeGazePipeline[GAZE_LOG].GetAddressOf(),
							macro);

		macro[0].Name		= "EXP";
		macro[0].Definition = "1";
		CreateComputeShader(L"../GPU_Codec/Shaders/ComputeGAZE.hlsl",
							gComputeGazePipeline[GAZE_EXP].GetAddressOf(),
							macro);

		// RAW shader
		CreateComputeShader(L"../GPU_Codec/Shaders/ComputeRAW.hlsl",
							gComputeRAWPipeline.GetAddressOf());
	}

	void RunGazeCompute(void)
	{
		gComputeRecorder.Reset(gComputeGazePipeline[DX12Var::gCurrentGazeFunction].Get());
		gComputeRecorder->SetComputeRootSignature(gComputeRoot.Get());

		ID3D12DescriptorHeap* heaps[] = {gSRVUAVHeap.Get()};
		gComputeRecorder->SetDescriptorHeaps(1, heaps);

		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type				   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource   = gOutputImage.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
			barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

			gComputeRecorder->ResourceBarrier(1, &barrier);
		}

		gComputeRecorder->SetComputeRootDescriptorTable(ROOT_UAV0,
														DX12Var::GetGPUHeapHandleAt(UAV_TEXTURE0));

		gComputeRecorder->SetComputeRootDescriptorTable(ROOT_SRV0,
														DX12Var::GetGPUHeapHandleAt(SRV_TEXTURE0));

		int data[2] = {GazePoint::currentX, GazePoint::currentY};

		gComputeRecorder->SetComputeRoot32BitConstants(
			ROOT_CONSTANTS0, ARRAYSIZE(data), reinterpret_cast<const void*>(&data), 0);

		gComputeRecorder->Dispatch(Screen::width8, Screen::height8, 1);

		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type				   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource   = gOutputImage.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;
			gComputeRecorder->ResourceBarrier(1, &barrier);
		}

		gComputeRecorder->Close();

		gComputeQueue.Submit(gComputeRecorder.Get());
		gComputeQueue.Flush();
	}

	void RunRAWCompute(void)
	{
		gComputeRecorder.Reset(gComputeRAWPipeline.Get());

		gComputeRecorder->SetComputeRootSignature(gComputeRoot.Get());

		ID3D12DescriptorHeap* heaps[] = {gSRVUAVHeap.Get()};
		gComputeRecorder->SetDescriptorHeaps(1, heaps);

		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type				   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource   = gOutputImage.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
			barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

			gComputeRecorder->ResourceBarrier(1, &barrier);
		}

		gComputeRecorder->SetComputeRootDescriptorTable(ROOT_UAV0,
														DX12Var::GetGPUHeapHandleAt(UAV_TEXTURE0));

		gComputeRecorder->SetComputeRootDescriptorTable(ROOT_SRV0,
														DX12Var::GetGPUHeapHandleAt(SRV_TEXTURE0));

		gComputeRecorder->Dispatch(Screen::width, Screen::height, 1);

		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type				   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource   = gOutputImage.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;
			gComputeRecorder->ResourceBarrier(1, &barrier);
		}

		gComputeRecorder->Close();

		gComputeQueue.Submit(gComputeRecorder.Get());
		gComputeQueue.Flush();
	}

	void RunDirect()
	{
		gDirectQueue.WaitForAnotherQueue(&gComputeQueue);

		gDirectRecorder.Reset();

		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type				   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource   = gRTResource[gFrameIndex].Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
			gDirectRecorder->ResourceBarrier(1, &barrier);
		}

		gDirectRecorder->CopyResource(gRTResource[gFrameIndex].Get(), gOutputImage.Get());

		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type				   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource   = gRTResource[gFrameIndex].Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
			gDirectRecorder->ResourceBarrier(1, &barrier);
		}

		gDirectRecorder->Close();

		gDirectQueue.Submit(gDirectRecorder.Get());
		gDirectQueue.Flush();

		D2D1_SIZE_F rtSize   = gD2DRenderTarget[gFrameIndex]->GetSize();
		D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);

		// Accquire our wrapped render target resource for he current back buffer.
		gDevice11On12->AcquireWrappedResources(gWrappedResource[gFrameIndex].GetAddressOf(), 1);

		// Render text directly to the backbuffer.
		gD2DDeviceContext->SetTarget(gD2DRenderTarget[gFrameIndex].Get());
		gD2DDeviceContext->BeginDraw();
		gD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Translation(-100, 0));
		gD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);

		const WCHAR* text = Debug::GetCurrentRadialFunctionWStr();

		gD2DDeviceContext->DrawTextW(
			text, 3, gDTextFormat.Get(), &textRect, gTextBrush.Get());
		gD2DDeviceContext->EndDraw();

		// Release our wrapped render target resource. Releasing
		// transitions the back buffer resource to the state
		// specified as the OutState when the wrapped resource was
		// created.
		gDevice11On12->ReleaseWrappedResources(gWrappedResource[gFrameIndex].GetAddressOf(), 1);
		// Flush to submit the 11 command list to the shared command queue
		gDevice11Context->Flush();

		gDirectQueue.WaitForComplete();
	}

} // namespace Impl

namespace DX12Wrap
{
	using namespace DX12Var;

	void InitD3D(void)
	{
		GazePoint::lastX = GazePoint::lastY = 0;
		GazePoint::currentX = GazePoint::currentY = 0;
		DX12Var::gCurrentGazeFunction			  = 0;

		Impl::CreateDevice();

		Impl::CreateQueuesAndLists();

		Impl::CreateD3D11();

		Impl::CreateRootSignatures();

		Impl::CreatePipleineStates();
	}

	void InitSwapChain(HWND _WindowHandle, unsigned int _Width, unsigned int _Height)
	{
		Screen::width   = _Width;
		Screen::height  = _Height;
		Screen::width8  = _Width >> 3;
		Screen::height8 = _Height >> 3;

		Impl::CreateSwapChain(_WindowHandle, _Width, _Height);

		Impl::CreateSRVCBVUAVResources();
	}

	void Fullscreen(HWND _WindowHandle)
	{
		HWND windowHandle = _WindowHandle;
		SetWindowLongW(windowHandle,
					   GWL_STYLE,
					   WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX |
											   WS_SYSMENU | WS_THICKFRAME));

		ComPtr<IDXGIOutput> pOutput;
		TIF(gSwapChain->GetContainingOutput(&pOutput));
		DXGI_OUTPUT_DESC outDesc;
		pOutput->GetDesc(&outDesc);

		RECT fullscreenWindowRect = outDesc.DesktopCoordinates;

		SetWindowPos(windowHandle,
					 HWND_TOPMOST,
					 fullscreenWindowRect.left,
					 fullscreenWindowRect.top,
					 fullscreenWindowRect.right,
					 fullscreenWindowRect.bottom,
					 SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(windowHandle, SW_MAXIMIZE);
	}

	void SetGazePoint(DirectX::XMINT2 _Coord)
	{
		// Clamp
		if(_Coord.x < 0)
			_Coord.x = 0;
		else if(_Coord.x > Screen::width)
			_Coord.x = Screen::width;

		if(_Coord.y < 0)
			_Coord.y = 0;
		else if(_Coord.y > Screen::height)
			_Coord.y = Screen::height;

		GazePoint::currentX = _Coord.x;
		GazePoint::currentY = _Coord.y;

		//printf("Gaze Point: (%d,%d)\n", GazePoint::lastX, GazePoint::lastY);
	}

	void SetRadialFunction(UINT _Function)
	{
		//printf("Radial function %i\n", _Function);
		DX12Var::gCurrentGazeFunction = _Function;
	}

	void UseTexture(const char* _pTexturePath)
	{
		int texWidth, texHeight, comp;
		BYTE* texData;
		texData = stbi_load(_pTexturePath, &texWidth, &texHeight, &comp, 4);
		//texData = LoadPPM(_pTexturePath, &texWidth, &texHeight);
		if(nullptr == texData)
			return;

		Impl::CreateTexture(texData, texWidth, texHeight);

		delete[] texData;
	}

	void Render(void)
	{
		// Only run the compute if there is a new gaze point
		//if(GazePoint::NewPoint())
		{
			Impl::RunGazeCompute();
		}

		Impl::RunDirect();

		gSwapChain->Present(1, 0);
		gFrameIndex = gSwapChain->GetCurrentBackBufferIndex();
	}

	void CleanUp(void)
	{
		gComputeQueue.WaitForComplete();
		gDirectQueue.WaitForComplete();
	}

}; // namespace DX12Wrap
