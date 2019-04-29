#include "GPUPch.h"

#include "CommandQueue.h"

CommandQueue::CommandQueue()
	: m_FenceValue(0)

{
	m_EventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void CommandQueue::Init(ID3D12Device* _pDevice, D3D12_COMMAND_LIST_TYPE _type)
{

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type					   = _type;
	queueDesc.Flags					   = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask				   = 1;

	// Create queue
	TIF(_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));
	NAME_D3D12_OBJECT(m_pCommandQueue);

	// Create fence and event for direct Queue
	TIF(_pDevice->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence)));
	NAME_D3D12_OBJECT(m_pFence);
}

ID3D12CommandQueue* CommandQueue::operator->(void)
{
	return m_pCommandQueue.Get();
}

ID3D12CommandQueue* CommandQueue::Get(void)
{
	return m_pCommandQueue.Get();
}

ID3D12CommandQueue** CommandQueue::GetAddressOf(void)
{
	return m_pCommandQueue.GetAddressOf();
}

void CommandQueue::Submit(ID3D12GraphicsCommandList* _pList)
{
	m_pListsToExecute.push_back(_pList);
}

void CommandQueue::Flush()
{

	m_pCommandQueue->ExecuteCommandLists(m_pListsToExecute.size(), m_pListsToExecute.data());
	m_pListsToExecute.clear();

}

void CommandQueue::WaitForAnotherQueue(CommandQueue* _pOther)
{
	_pOther->m_pCommandQueue->Signal(_pOther->m_pFence.Get(), _pOther->m_FenceValue);
	m_pCommandQueue->Wait(_pOther->m_pFence.Get(), _pOther->m_FenceValue);
	_pOther->m_FenceValue++;
}

void CommandQueue::WaitForComplete(void)
{
	m_pCommandQueue->Signal(m_pFence.Get(), m_FenceValue);
	if(m_FenceValue > m_pFence->GetCompletedValue())
	{
		m_pFence->SetEventOnCompletion(m_FenceValue, m_EventHandle);
		WaitForMultipleObjects(1, &m_EventHandle, TRUE, INFINITE);
	}
	m_FenceValue++;
	
}
