#pragma once
#include "../pch.h"


class IPAddress
{
public:

	/*
		Default constructor

		This constructor creates an empty (invalid) address
	*/
	IPAddress();

	/*
		Construct the address from a string

		Params:
		address		IP address or network name
	*/
	IPAddress(const std::string& _address);

	IPAddress(UINT8 _a, UINT8 _b, UINT8 _c, UINT8 _d);

	UINT32 ToInteger() const;
	std::string ToStr() const;

	static const IPAddress Any;
	static const IPAddress Localhost;

private:

	void resolve(const std::string& _address);
private:
	std::string m_addressStr;
	UINT32	m_address;
	bool	m_valid;
};
