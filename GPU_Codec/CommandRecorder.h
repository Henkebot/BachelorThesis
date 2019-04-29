#pragma once
#include "GPUPch.h"

// Simple class that is a command list and command allocator

class CommandRecorder
{
public:
	CommandRecorder() = default;

	void Init(ID3D12Device* _pDevice, D3D12_COMMAND_LIST_TYPE _type, bool _closed = true);

	void Reset(ID3D12PipelineState* _pPipelineState = nullptr);

	ID3D12GraphicsCommandList* operator->(void);
	ID3D12GraphicsCommandList* Get(void);

private:
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
};
