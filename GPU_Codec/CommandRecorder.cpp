#include "GPUPch.h"
#include "CommandRecorder.h"

void CommandRecorder::Init(ID3D12Device* _pDevice, D3D12_COMMAND_LIST_TYPE _type, bool _closed)
{
	TIF(_pDevice->CreateCommandAllocator(_type, IID_PPV_ARGS(&m_pCommandAllocator)));

	NAME_D3D12_OBJECT(m_pCommandAllocator);

	TIF(_pDevice->CreateCommandList(
		1, _type, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList)));

	NAME_D3D12_OBJECT(m_pCommandList);

	if(_closed)
		m_pCommandList->Close();
}

void CommandRecorder::Reset(ID3D12PipelineState* _pPipelineState )
{
	m_pCommandAllocator->Reset();
	m_pCommandList->Reset(m_pCommandAllocator.Get(), _pPipelineState);
}

ID3D12GraphicsCommandList* CommandRecorder::operator->(void)
{
	return m_pCommandList.Get();
}

ID3D12GraphicsCommandList* CommandRecorder::Get(void)
{
	return m_pCommandList.Get();
}
