//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "ClientSystemInfo.h"

namespace ZeroConfig_CPP_CX_Test
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		void InitializeZeroConfig();
		Platform::Collections::Vector<ZeroConfig::ZeroConfRecord^>^ mDNSRecords;
		Platform::Collections::Vector<ClientSystemInfo^>^ Directory;
		ZeroConfig::ZeroConfigResolver^ mDNSResolver;
		void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnVectorChanged(Windows::Foundation::Collections::IObservableVector<ZeroConfig::ZeroConfRecord ^> ^sender, Windows::Foundation::Collections::IVectorChangedEventArgs ^event);
	};
}
