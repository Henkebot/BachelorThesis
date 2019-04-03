#include <pch.h>
#include <Include/Packet.h>

////////////////////////////////////////////////////
Packet::Packet() :
	m_isValid(true),
	m_readPos(0),
	m_sendPos(0)
{

}

////////////////////////////////////////////////////
void Packet::Append(const void * data, std::size_t sizeInBytes)
{
	if (data && (sizeInBytes > 0))
	{
		unsigned int size = m_data.size();
		// TODO(Henrik): Use insert instead
		m_data.resize(size + sizeInBytes);
		std::memcpy(&m_data[size], data, sizeInBytes);

	}
}

////////////////////////////////////////////////////
const void * Packet::onSend(unsigned int & _size)
{
	_size = GetDataSize();
	return getData();
}

////////////////////////////////////////////////////
void Packet::onReceive(const void * data, std::size_t size)
{
	Append(data, size);
}

////////////////////////////////////////////////////
const void * Packet::getData() const
{
	return (m_data.empty() == false) ? &m_data[0] : nullptr;
}

////////////////////////////////////////////////////
unsigned int Packet::GetDataSize() const
{
	return m_data.size();
}

////////////////////////////////////////////////////
void Packet::Clear()
{
	m_data.clear();
	m_readPos = 0;
	m_isValid = true;
}

////////////////////////////////////////////////////
bool Packet::CheckSize(std::size_t size)
{
	m_isValid = m_isValid && (m_readPos + size <= m_data.size());
	return m_isValid;
}