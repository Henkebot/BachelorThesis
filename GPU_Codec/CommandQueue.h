#pragma once
#include "GPUPch.h"

class CommandQueue
{
public:
	CommandQueue();
	void Init(ID3D12Device* _pDevice, D3D12_COMMAND_LIST_TYPE _type);

	ID3D12CommandQueue* operator->(void);
	ID3D12CommandQueue* Get(void);
	ID3D12CommandQueue** GetAddressOf(void); 

	void Submit(ID3D12GraphicsCommandList* _pList);
	void Flush();

	void WaitForAnotherQueue(CommandQueue* _pOther);
	void WaitForComplete(void);

private:
	std::vector<ID3D12CommandList*> m_pListsToExecute; 

	ComPtr<ID3D12CommandQueue> m_pCommandQueue;

	ComPtr<ID3D12Fence> m_pFence;
	UINT m_FenceValue;
	HANDLE m_EventHandle;
};
