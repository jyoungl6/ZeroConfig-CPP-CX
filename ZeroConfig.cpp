
#include "pch.h"
#include "ZeroConfig.h"
#include "DnsMessage.h"
#include <string>
#include <locale>
#include <codecvt> 
#include <memory>



using namespace ZeroConfig;
using namespace Platform;
using namespace Windows::Networking;
using namespace Windows::Networking::Connectivity;
using namespace Windows::Networking::Sockets;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Windows::Storage::Streams;
using namespace Concurrency;

#undef  _DEBUG_ZEROCONFIG
//////////////////////////////////////////////////////////////////////////
//
// ZeroConfigResolver	constructor
//
//////////////////////////////////////////////////////////////////////////
ZeroConfigResolver::ZeroConfigResolver()
{
	socket = nullptr;

	//socket->MessageReceived += ref new TypedEventHandler<DatagramSocket^, DatagramSocketMessageReceivedEventArgs^>([this](DatagramSocket^ s, DatagramSocketMessageReceivedEventArgs^ e)->void
	//{
	//	wchar_t buffer[1000];
	//	auto dr = e->GetDataReader();
	//	auto bytecount = dr->UnconsumedBufferLength;
	//	swprintf(buffer, 1000, L"**** In MessageReceived event handler****. Byte count: %d\n", bytecount);
	//	OutputDebugString(buffer);
	//	auto msg = ProcessMessage(dr);
	//});

}

//////////////////////////////////////////////////////////////////////////
//
// ZeroConfigResolver::OnMessage	event handler
//
//////////////////////////////////////////////////////////////////////////
void ZeroConfigResolver::OnMessage(Windows::Networking::Sockets::DatagramSocket^ socket, Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^ args)
{

	auto dr = args->GetDataReader();
	auto bytecount = dr->UnconsumedBufferLength;
#ifdef _DEBUG_ZEROCONFIG
	wchar_t buffer[1000];
	swprintf(buffer, 1000, L"**** In MessageReceived event handler****. Byte count: %d\n", bytecount);
	OutputDebugString(buffer);
#endif // _DEBUG_ZEROCONFIG
	
	/*auto msg =*/ ProcessMessage(dr);

	for (auto m = dnsRecordList.begin(); m != dnsRecordList.end(); m++)
	{
		auto msg = *m;
#ifdef _DEBUG_ZEROCONFIG
		if (msg != nullptr)
		{
			OutputDebugString(L"Zero DNS Host Name: ");
			OutputDebugString(msg->Host->Data()); OutputDebugString(L"\n");

			OutputDebugString(L"Zero DNS IP Address: ");
			OutputDebugString(msg->IPAddress->Data()); OutputDebugString(L"\n");

			OutputDebugString(L"Zero DNS Port: ");
			OutputDebugString(msg->Port->Data()); OutputDebugString(L"\n");
}
#endif

		if (msg != nullptr && msg->Port != nullptr && msg->IPAddress != nullptr)
		{
			bool found = false;
			auto i = _ResolveList->First();

			while (i->HasCurrent)
			{
				auto r = i->Current;
				if (r->Name == msg->Name && r->IPAddress == msg->IPAddress && r->Host == msg->Host)
				{
					found = true;
				}

				i->MoveNext();
			}

			if (!found)
			{
				_ResolveList->Append(msg);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
// ZeroConfigResolver::Resolve
//
//////////////////////////////////////////////////////////////////////////
void ZeroConfigResolver::Resolve(Platform::String^ _protocol, Windows::Foundation::Collections::IVector<ZeroConfRecord^>^ _resolveList)
{
	_ResolveList = _resolveList;

	if (socket != nullptr)
	{
		delete socket;
	}

	socket = ref new DatagramSocket;
	socket->Control->MulticastOnly = true;
	socket->MessageReceived += ref new TypedEventHandler<DatagramSocket^, DatagramSocketMessageReceivedEventArgs^>(this, &ZeroConfigResolver::OnMessage);

	ConnectionProfile^ connectionProfile = NetworkInformation::GetInternetConnectionProfile();

	task<void>(socket->BindServiceNameAsync("5353", connectionProfile->NetworkAdapter)).then([this, _protocol] (task<void> previousTask)
	{
		try 
		{
			previousTask.get();
			socket->JoinMulticastGroup(ref new HostName("224.0.0.251"));

			IAsyncOperation<IOutputStream^> ^op = socket->GetOutputStreamAsync(ref new HostName("224.0.0.251"), "5353");

			op->Completed = ref new AsyncOperationCompletedHandler<IOutputStream^>
				([this, _protocol](IAsyncOperation<IOutputStream^> ^op, Windows::Foundation::AsyncStatus status)
			{

				// If an error is thrown here the we don't have a networking stack to work with. Bail 
				auto ec = op->ErrorCode;
				if (ec.Value != 0x0000)
				{
					return;
				}

				IOutputStream ^ostream = op->GetResults();

				DataWriter	^writer = ref new DataWriter(ostream);

				std::wstring p = _protocol->Data();
				this->WriteQueryMessage(p, writer);
				writer->StoreAsync();
			});

			//task<IOutputStream^>(this->socket->GetOutputStreamAsync(ref new HostName("224.0.0.251"), "5353")).then([this, _protocol] (task<IOutputStream^> previousTask2)
			//{
			//	auto os = previousTask2.get();
			//	auto writer = ref new DataWriter(os);
			//	std::wstring p = _protocol->Data();
			//	this->WriteQueryMessage(p, writer);
			//	writer->StoreAsync();
			//	return;
			//});
		}
		catch(Exception^ ex)
		{
#ifdef _DEBUG_ZEROCONFIG
			std::wstring error = ex->Message->Data();
			OutputDebugString(error.c_str());
#endif // _DEBUG_ZEROCONFIG
		}
	});

	//socket.JoinMulticastGroup(ref new HostName("224.0.0.251"));
	//auto os = socket.GetOutputStreamAsync(new HostName("224.0.0.251"), "5353");
	//auto writer = ref new DataWriter(os);
	//WriteQueryMessage(protocol, writer);
	//writer.StoreAsync();
	//return s;

}

//////////////////////////////////////////////////////////////////////////
//
// ZeroConfigResolver::WriteQueryMessage
//
//////////////////////////////////////////////////////////////////////////
void ZeroConfigResolver::WriteQueryMessage(std::wstring protocol, IDataWriter^ dataWriter)
{

	dataWriter->WriteUInt16(0);
	dataWriter->WriteInt16(0);
	dataWriter->WriteInt16(1);
	dataWriter->WriteInt16(0);
	dataWriter->WriteInt16(0);
	dataWriter->WriteInt16(0);
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	std::string p = converter.to_bytes(protocol);
	WriteParts(split(p,'.'), dataWriter);
	dataWriter->WriteUInt16(255);
	dataWriter->WriteUInt16(255);
}

//////////////////////////////////////////////////////////////////////////
//
// ZeroConfigResolver::WriteParts
//
//////////////////////////////////////////////////////////////////////////
void ZeroConfigResolver::WriteParts(std::vector<std::string> parts, ws::IDataWriter^ dataWriter)
{
	for (auto i = parts.begin(); i != parts.end(); i++)
	{
		std::string part = *i;
		//std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;


		//Platform::String^ ws = ref new String(converter.from_bytes(part).c_str());

		if (part.length() > 0)
		{
			auto x = part.length();
			std::vector<char> writeable(part.size() + 1);
			std::copy(part.begin(), part.end(), writeable.begin());
			x = writeable.size();
			auto theBytes = ref new Platform::Array<UCHAR>((UCHAR*)&writeable[0], writeable.size()-1);
			x = theBytes->Length;
			dataWriter->WriteByte((BYTE)part.length());
			dataWriter->WriteBytes(theBytes);
		}		
	}

	dataWriter->WriteByte(0);
}

//////////////////////////////////////////////////////////////////////////
//	
// ZeroConfigResolver::ReadDnsMessage
//
//////////////////////////////////////////////////////////////////////////
std::shared_ptr<DnsMessage> ZeroConfigResolver::ReadDnsMessage(ws::IDataReader^ dataReader)
{
	DnsStringReader stringReader(dataReader, dataReader->UnconsumedBufferLength);

	auto msg = std::make_shared<DnsMessage>();

	msg->QueryIdentifier = dataReader->ReadUInt16();
	msg->Flags = dataReader->ReadUInt16();

	auto qCount = dataReader->ReadUInt16();
	auto aCount = dataReader->ReadUInt16();
	auto nsCount = dataReader->ReadUInt16();
	auto arCount = dataReader->ReadUInt16();
	auto nonQuestions = aCount + nsCount + arCount;

	for (auto i = 0; i < qCount; i++)
	{
		auto qr = std::make_shared<QuestionRecord>();
		qr->Resourcename = stringReader.ReadString();
		qr->QuestionType = dataReader->ReadUInt16();
		qr->Class = dataReader->ReadUInt16();
		msg->Records->push_back(std::dynamic_pointer_cast<DnsRecord>(qr));
	}

	for (auto i = 0; i < nonQuestions; i++)
	{
		auto resName = stringReader.ReadString();
		auto resType = dataReader->ReadUInt16();
		auto resClass = dataReader->ReadUInt16();
		auto ttl = dataReader->ReadUInt32();
		auto resLength = dataReader->ReadUInt16();
		auto remaining = dataReader->UnconsumedBufferLength;

		std::shared_ptr<ResourceRecord> rec(nullptr);

		switch(resType)
		{
		case 1: // A record
			wchar_t buffer[1000];
			BYTE oct1, oct2, oct3, oct4;
			oct1 = dataReader->ReadByte();
			oct2 = dataReader->ReadByte();
			oct3 = dataReader->ReadByte();
			oct4 = dataReader->ReadByte();
			swprintf(buffer, 1000, L"%d.%d.%d.%d", 
				oct1, oct2, oct3, oct4);
			rec = std::make_shared<HostAddressRecord>();
			std::dynamic_pointer_cast<HostAddressRecord>(rec)->IPAddress = buffer;
			break;

		case 12: // PTR record
			rec = std::make_shared<PtrRecord>();
			std::dynamic_pointer_cast<PtrRecord>(rec)->DomainNamePointer = stringReader.ReadString();
			break;

		case 16: // TXT record
			rec = std::make_shared<TxtRecord>();
			std::dynamic_pointer_cast<TxtRecord>(rec)->TextData = stringReader.ReadString(resLength);
			break;

		case 33: // SRV
			rec = std::make_shared<SrvRecord>();
			std::dynamic_pointer_cast<SrvRecord>(rec)->Priority = dataReader->ReadUInt16();
			std::dynamic_pointer_cast<SrvRecord>(rec)->Weight = dataReader->ReadUInt16();
			std::dynamic_pointer_cast<SrvRecord>(rec)->Port = dataReader->ReadUInt16();
			std::dynamic_pointer_cast<SrvRecord>(rec)->Target = stringReader.ReadString(resLength);
			break;

		default:
			rec = std::make_shared<UnknownDnsRecord>();
			std::dynamic_pointer_cast<UnknownDnsRecord>(rec)->ResourceType = resType;
			break;
		}

		rec->Resourcename = resName;
		rec->Ttl = ttl;
		rec->Class = resClass;
		msg->Records->push_back(std::dynamic_pointer_cast<DnsRecord>(rec));

		int remainingResourceBytes = resLength - (remaining - dataReader->UnconsumedBufferLength);

		if (remainingResourceBytes < 0)
		{
			OutputDebugString(L"Error reading resource - reached into next record\n");
			return msg;
		}

		if (remainingResourceBytes > 0)
		{
			dataReader->ReadBuffer(remainingResourceBytes);
		}

	}
	return msg;
}

//////////////////////////////////////////////////////////////////////////
//
// ZeroConfigResolver::DnsToZeroConf
//
//////////////////////////////////////////////////////////////////////////
void ZeroConfigResolver::DnsToZeroConf(std::shared_ptr<DnsMessage> message)
{
	//dnsRecordList.clear();

	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wsconverter;

	if (!message->IsResponse)
		return; //nullptr;

	dnsRecordCache.clear();
	for (auto rc = message->Records->begin(); rc != message->Records->end(); rc++)
	{
		dnsRecordCache.push_back(*rc);
	}

	for (auto i = message->Records->begin(); i != message->Records->end(); i++)
	{
		auto x = *i;

		//auto ptr = std::dynamic_pointer_cast<PtrRecord>(*i);
		//if (ptr != nullptr && zr->Name->IsEmpty()) 
		//	zr->Name = ref new Platform::String(wsconverter.from_bytes(split(wsconverter.to_bytes(ptr->DomainNamePointer), '.').at(0)).c_str());

		//auto hst = std::dynamic_pointer_cast<HostAddressRecord>(*i);
		//if (hst != nullptr)
		//{
		//	if (zr->Host->IsEmpty())
		//		zr->Host = ref new Platform::String(wsconverter.from_bytes(split(wsconverter.to_bytes(hst->Resourcename), '.').at(0)).c_str());
		//	if (zr->IPAddress->IsEmpty())
		//		zr->IPAddress = ref new Platform::String(hst->IPAddress.c_str());
		//}

		auto srv = std::dynamic_pointer_cast<SrvRecord>(x);
		if (srv != nullptr /*&& zr->Port->IsEmpty()*/)
		{
			auto zr = ref new ZeroConfRecord();

			for (auto rc = dnsRecordCache.begin(); rc != dnsRecordCache.end(); rc++)
			{
				auto hst = std::dynamic_pointer_cast<HostAddressRecord>(*rc);
				if (hst != nullptr && hst->Resourcename == srv->Target)
				{
					zr->IPAddress = ref new Platform::String(hst->IPAddress.c_str());
					zr->Host = ref new Platform::String(wsconverter.from_bytes(split(wsconverter.to_bytes(hst->Resourcename), '.').at(0)).c_str());
				}
			}
			zr->Port = ref new Platform::String(srv->PortString.c_str());
			zr->Name = ref new Platform::String(srv->Service.c_str());
			//zr->Host = ref new Platform::String(srv->Service.c_str());
			dnsRecordList.push_back(zr);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//	
// ZeroConfigResolver::ProcessMessage
//
//////////////////////////////////////////////////////////////////////////
void ZeroConfigResolver::ProcessMessage(ws::DataReader^ dataReader)
{
	auto byteCount = dataReader->UnconsumedBufferLength;
	auto msg = ReadDnsMessage(dataReader);

	DnsToZeroConf(msg);
}

//////////////////////////////////////////////////////////////////////////
//
// DnsStringReader
//
//////////////////////////////////////////////////////////////////////////
DnsStringReader::DnsStringReader(ws::IDataReader^ dataReader, UINT totalBytes)
{
	_dataReader = dataReader;
	_totalBytes = totalBytes;
}

std::wstring DnsStringReader::getReturnVal(std::vector<UINT> substringPositions)
{
	if (substringPositions.size() == 0)
		return L"";

	return _map[substringPositions[0]];
}

//////////////////////////////////////////////////////////////////////////
//
// DnsStringReader::ReadString
//
//////////////////////////////////////////////////////////////////////////
std::wstring DnsStringReader::ReadString(USHORT resLength)
{
	std::vector<UINT> substringPositions;
	auto startingPosition = _totalBytes - _dataReader->UnconsumedBufferLength;

	//auto retVal = [substringPositions, this]()-> std::wstring {return substringPositions.size() == 0 ? L"" : _map[substringPositions[0]]; };

	auto appendToPositions = [&substringPositions, this] (std::wstring str, UINT position) -> void 
	{
		for(auto i = substringPositions.begin(); i != substringPositions.end(); i++)
		{
			auto pos = *i;
			_map[pos] = _map[pos].append(L".").append(str);
		}
		_map[position] = str;
		substringPositions.push_back(position);

	};

	while(true)
	{
		auto position = _totalBytes - _dataReader->UnconsumedBufferLength;

		if (resLength > 0 && position - startingPosition >= resLength)
			return getReturnVal(substringPositions);

		auto strlen = _dataReader->ReadByte();

		if (strlen == 0)
			return getReturnVal(substringPositions);

		std::wstring str;
		if (DnsMath::IsPointer(strlen))
		{
			auto ptr = DnsMath::TwoBytesToPointer(strlen, _dataReader->ReadByte());
			auto i = _map.find(ptr);
			if (i != _map.end())
			{
				str = i->second;
				appendToPositions(str, position);
			}
			std::wstring ret = getReturnVal(substringPositions);
			return ret;
		}

		auto u = _dataReader->UnconsumedBufferLength;
		str = _dataReader->ReadString(strlen)->Data();
		appendToPositions(str, position);

	}
}


void ZeroConfigResolver::test()
{
	//auto subject = rxcpp::CreateSubject<int>();
	//std::shared_ptr<rxcpp::Observable<int>> observable = subject;

	//std::function<void(int)> observer = [](int inp) 
	//{ 
	//	
	//	std::wstring str;

	//	str = std::to_wstring(inp);
	//	
	//	OutputDebugString(str.c_str());
	//};

	//auto disp1 = cpplinq::from(observable).select([](int x) 
	//{ 
	//	std::wstring str;
	//	str = L"obs 1" + std::to_wstring(x*2);
	//	OutputDebugString(str.c_str());

	//}).subscribe(observer); 

	//auto disp2 = cpplinq::from(observable).select([](int x) 
	//{ 
	//	std::wstring str;
	//	str = L"obs 2" + std::to_wstring(x*3);
	//	OutputDebugString(str.c_str());
	//}).subscribe(observer);

	//subject->OnNext(1);
	//subject->OnNext(2);

}
