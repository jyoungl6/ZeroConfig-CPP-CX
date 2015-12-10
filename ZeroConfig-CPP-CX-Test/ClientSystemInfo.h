#pragma once
namespace ZeroConfig_CPP_CX_Test
{
	[Windows::UI::Xaml::Data::Bindable]
	public ref class ClientSystemInfo sealed
	{
	public:

		ClientSystemInfo() { ManualEntry = false; }

		ClientSystemInfo(Platform::String^ ipAddress, Platform::String^ systemName, Platform::String^ port)
		{
			IPAddress = ipAddress;
			SystemName = systemName;
			Port = port;
		}

		ClientSystemInfo(Platform::String^ ipAddress, Platform::String^ systemName, Platform::String^ port, bool manualEntry)
		{
			IPAddress = ipAddress;
			SystemName = systemName;
			Port = port;
			ManualEntry = true;
		}

		bool Equals(ClientSystemInfo^ r)
		{
			if (r->IPAddress == this->IPAddress &&
				r->Port == this->Port)
				return true;

			return false;
		}

		property Platform::String^ IPAddress;
		property Platform::String^ SystemName;
		property Platform::String^ Port;
		property bool ManualEntry;
	};

}