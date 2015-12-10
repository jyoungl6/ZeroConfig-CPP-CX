#pragma once
#include <pch.h>

namespace ZeroConfig
{
	//////////////////////////////////////////////////////////////////////////
	/// DnsMath
	//////////////////////////////////////////////////////////////////////////
	class DnsMath
	{
	public:
		static bool IsResponse(USHORT flags) { return (flags & 0x8000) != 0; }
		
		static bool IsPointer(BYTE value) { return (value & 0xC0) > 0; }
		
		static USHORT TwoBytesToPointer(BYTE first, BYTE second)
		{
			return (USHORT) (((first ^ 0xC0) << 8) | second);
		}
		
	};

	//////////////////////////////////////////////////////////////////////////
	// DnsRecord
	//////////////////////////////////////////////////////////////////////////
	class DnsRecord
	{
		public:

			// ResourceName property
			__declspec(property(get = getResourceName, put = setResourceName)) std::wstring Resourcename;
			std::wstring getResourceName() { return _ResourceName; }
			void setResourceName(std::wstring v) { _ResourceName = v; }


			__declspec(property(get = getClass, put = setClass)) USHORT Class;
			USHORT getClass() { return _Class; }
			void setClass(USHORT v) { _Class = v; }

			virtual ~DnsRecord(){}

		private:
			std::wstring _ResourceName;
			USHORT _Class;
	};

	//////////////////////////////////////////////////////////////////////////
	// DnsMessage
	//////////////////////////////////////////////////////////////////////////
	class DnsMessage
	{
	public:
		DnsMessage()
		{
			_records = new std::vector<std::shared_ptr<DnsRecord>>();
		}
		
		~DnsMessage()
		{
			_records->clear();
			delete _records;
		}

		// QueryIdentifier property
		__declspec ( property ( put = setQueryIdentifier, get = getQueryIdentifier) ) USHORT QueryIdentifier;
		USHORT getQueryIdentifier() { return _QueryIdentifier;}
		void setQueryIdentifier(USHORT v) { _QueryIdentifier = v; }

		// Flags property
		__declspec (property(put = setFlags, get = getFlags)) USHORT Flags;
		USHORT getFlags() { return _Flags; }
		void setFlags(USHORT v) { _Flags = v; }

		// IsResponse property
		__declspec(property(get = getResponse)) bool IsResponse;
		bool getResponse() { return DnsMath::IsResponse(Flags); }

		// Records property
		__declspec(property(get = getRecords)) std::vector<std::shared_ptr<DnsRecord>> *Records;
		std::vector<std::shared_ptr<DnsRecord>> *getRecords() { return _records; }

	protected:
		USHORT _QueryIdentifier;
		USHORT _Flags;

	private:
		std::vector<std::shared_ptr<DnsRecord>> *_records;
	};

	//////////////////////////////////////////////////////////////////////////
	// QuestionRecord
	//////////////////////////////////////////////////////////////////////////
	class QuestionRecord : public DnsRecord
	{
	public:

		// QuestionType property
		__declspec(property(get = getQuestionType, put = setQuestionType)) USHORT QuestionType;
		USHORT getQuestionType() { return _QuestionType; }
		void setQuestionType(USHORT v) { _QuestionType = v; }

		virtual ~QuestionRecord(){}

	private:
		USHORT _QuestionType;


	};

	//////////////////////////////////////////////////////////////////////////
	// ResourceRecord
	//////////////////////////////////////////////////////////////////////////
	class ResourceRecord : public DnsRecord
	{
	public:
		
		// Ttl property
		__declspec(property(get = getTtl, put = setTtl)) unsigned int Ttl;
		unsigned int getTtl() { return _Ttl; }
		void setTtl(unsigned int v) { _Ttl = v; }

		virtual ~ResourceRecord(){}

	private:
		unsigned int _Ttl;

	};

	//////////////////////////////////////////////////////////////////////////
	// HostAddressRecord
	//////////////////////////////////////////////////////////////////////////
	class HostAddressRecord : public  ResourceRecord
	{
	public:

		// IPAddress property
		__declspec(property(get = getIPAddress, put = setIPAddress)) std::wstring IPAddress;
		std::wstring getIPAddress() { return _IPAddress; }
		void setIPAddress(std::wstring v) { _IPAddress = v; }

		virtual ~HostAddressRecord(){}
		
	private:
		std::wstring _IPAddress;
	};

	//////////////////////////////////////////////////////////////////////////
	// PtrRecord
	//////////////////////////////////////////////////////////////////////////
	class PtrRecord : public ResourceRecord
	{
	public:
		
		// DomainNamePointer
		__declspec(property(get = getDomainNamePointer, put = setDomainNamePointer)) std::wstring DomainNamePointer;
		std::wstring getDomainNamePointer() { return _DomainNamePointer; }
		void setDomainNamePointer(std::wstring v) { _DomainNamePointer = v; }

		virtual ~PtrRecord(){}

	private:
		std::wstring _DomainNamePointer;
	};

	//////////////////////////////////////////////////////////////////////////
	// TxtRecord
	//////////////////////////////////////////////////////////////////////////
	class TxtRecord : public ResourceRecord
	{
	public:
		// TextData property
		__declspec(property(get = getTextData, put = setTextData)) std::wstring TextData;
		std::wstring getTextData() { return _TextData; }
		void setTextData(std::wstring v) { _TextData = v; }

		virtual ~TxtRecord(){}

	private:
		std::wstring _TextData;
	};

	//////////////////////////////////////////////////////////////////////////
	// SrvRecord
	//////////////////////////////////////////////////////////////////////////
	class SrvRecord : public ResourceRecord
	{
	public:
		// Priority property
		__declspec(property(get = getPriority, put = setPriority)) USHORT Priority;
		USHORT getPriority() { return _Priority; }
		void setPriority(USHORT v) { _Priority = v; }

		// Weight property
		__declspec(property(get = getWeight, put = setWeight)) USHORT Weight;
		USHORT getWeight() { return _Weight; }
		void setWeight(USHORT v) { _Weight = v; }

		// Port property
		__declspec(property(get = getPort, put = setPort)) USHORT Port;
		USHORT getPort() { return _Port; }
		void setPort(USHORT v) {_Port = v; }

		// PortString property
		__declspec(property(get = getPortString)) std::wstring PortString;
		std::wstring getPortString()
		{
			wchar_t buffer[1000];
			swprintf(buffer, 1000, L"%d", _Port);
			std::wstring rs = buffer;

			return rs;
		}

		// Target property
		__declspec(property(get = getTarget, put = setTarget)) std::wstring Target;
		std::wstring getTarget() { return _Target; }
		void setTarget(std::wstring v) { _Target = v; }

		// Service property
		__declspec(property(get = getService)) std::wstring Service;
		std::wstring getService() { return Resourcename.substr(0, Resourcename.find(std::wstring(L"."), 0)); }
		virtual ~SrvRecord(){}

	private:
		USHORT _Priority;
		USHORT _Weight;
		USHORT _Port;
		std::wstring _Target;
	};

	//////////////////////////////////////////////////////////////////////////
	// UnknownDnsRecord
	//////////////////////////////////////////////////////////////////////////
	class UnknownDnsRecord : public ResourceRecord
	{
	public:
		UnknownDnsRecord()
		{
			_Data = new std::vector<UCHAR>();
		}

		virtual ~UnknownDnsRecord()
		{
			_Data->clear();
		}

		__declspec(property(get = getData)) std::vector<BYTE> *Data;
		std::vector<BYTE>* getData() { return _Data; }

		__declspec(property(get = getResourceType, put = setResourceType)) USHORT ResourceType;
		USHORT getResourceType() { return _ResourceType; }
		void setResourceType(USHORT v) { _ResourceType = v; }

	private:
		USHORT _ResourceType;
		std::vector<BYTE> *_Data;
	};
}