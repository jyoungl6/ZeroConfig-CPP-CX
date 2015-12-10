#pragma once
#include <pch.h>
#include <collection.h>
#include <sstream>
#include <vector>
#include "DnsMessage.h"

namespace ws = Windows::Storage::Streams;

namespace ZeroConfig
{
	public ref class ZeroConfRecord sealed
	{
	public:
		property Platform::String^ Name;
		property Platform::String^ IPAddress;
		property Platform::String^ Host;
		property Platform::String^ Port;
		property Platform::String^ Description
		{
			Platform::String^ get()
			{
				wchar_t buffer[1000];

				swprintf(buffer, 1000, L"Name:%s IP:%s Host:%s Port:%s", Name->Data(), IPAddress->Data(), Host->Data(), Port->Data());
				_Description = ref new Platform::String(buffer);

				return _Description;
			}
		}

	private:
		Platform::String^ _Description;
	};

    public ref class ZeroConfigResolver sealed
    {
    public:
        ZeroConfigResolver();

		void test();

		void Resolve(Platform::String^ Protocol, Windows::Foundation::Collections::IVector<ZeroConfRecord^>^ ResolveList);

	private:
		std::vector<std::shared_ptr<DnsRecord>> dnsRecordCache;
		std::vector<ZeroConfRecord^> dnsRecordList;
		Windows::Networking::Sockets::DatagramSocket^ socket;

		void WriteQueryMessage(std::wstring protocol, ws::IDataWriter^ dataWriter);
		void WriteParts(std::vector<std::string> parts, ws::IDataWriter^ dataWriter);
		void DnsToZeroConf(std::shared_ptr<DnsMessage> message);
		void ProcessMessage(ws::DataReader^ reader);
		std::shared_ptr<DnsMessage> ReadDnsMessage(ws::IDataReader^ dataReader);
		void OnMessage(Windows::Networking::Sockets::DatagramSocket^ socket, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^ args);
		Windows::Foundation::Collections::IVector<ZeroConfRecord^>^ _ResolveList;
    };

	class DnsStringReader
	{
	public:
		DnsStringReader(ws::IDataReader^ dataReader, UINT totalBytes);

	public:
		std::wstring ReadString(USHORT resLength = 0);

	private: 
		std::map<UINT, std::wstring> _map;
		ws::IDataReader^ _dataReader;
		UINT _totalBytes;
		std::wstring emptyString;
		Windows::Foundation::Collections::IVector<ZeroConfRecord^>^ resolveList;
		std::wstring getReturnVal(std::vector<UINT> substringPositions);
	};

	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}

	std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, elems);
		return elems;
	}

}

