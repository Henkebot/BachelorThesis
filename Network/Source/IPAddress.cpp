#include <pch.h>
#include <Include/IPAddress.h>
const IPAddress IPAddress::Any(0, 0, 0, 0);
const IPAddress IPAddress::Localhost("localhost");

////////////////////////////////////////////////////
IPAddress::IPAddress() :
	m_address(0),
	m_addressStr(),
	m_valid(false)
{
}

////////////////////////////////////////////////////
IPAddress::IPAddress(const std::string & _address) :
	m_address(0),
	m_addressStr(_address),
	m_valid(false)

{
	resolve(_address);
}

////////////////////////////////////////////////////
IPAddress::IPAddress(UINT8 _a, UINT8 _b, UINT8 _c, UINT8 _d) :
	m_address(htonl((_a << 24) | (_b << 16) | (_c << 8) | _d)),
	m_addressStr(std::to_string(_a) + "." + std::to_string(_b) + "." + std::to_string(_c) + "." + std::to_string(_d)),
	m_valid(true)
{

}

////////////////////////////////////////////////////
UINT32 IPAddress::ToInteger() const
{
	return ntohl(m_address);
}

////////////////////////////////////////////////////
std::string IPAddress::ToStr() const
{
	return m_addressStr;
}

////////////////////////////////////////////////////
void IPAddress::resolve(const std::string & _address)
{
	m_address = 0;
	m_valid = false;

	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	addrinfo* result = nullptr;
	if (getaddrinfo(_address.c_str(), nullptr, &hints, &result) == 0)
	{
		if (result)
		{
			UINT32 ip = reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_addr.s_addr;
			freeaddrinfo(result);
			m_address = ip;
			m_valid = true;
		}
	}


}