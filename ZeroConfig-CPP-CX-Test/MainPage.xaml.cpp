//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace ZeroConfig_CPP_CX_Test;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
	InitializeZeroConfig();
}


void ZeroConfig_CPP_CX_Test::MainPage::InitializeZeroConfig()
{
	mDNSRecords = ref new Platform::Collections::Vector<ZeroConfig::ZeroConfRecord^>();
	mDNSResolver = ref new ZeroConfig::ZeroConfigResolver();
	Directory = ref new Platform::Collections::Vector<ClientSystemInfo^>();
	mDNSRecords->VectorChanged += ref new Windows::Foundation::Collections::VectorChangedEventHandler<ZeroConfig::ZeroConfRecord ^>(this, &ZeroConfig_CPP_CX_Test::MainPage::OnVectorChanged);
}

void ZeroConfig_CPP_CX_Test::MainPage::Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	lbHosts->ItemsSource = Directory;
	mDNSResolver->Resolve("_apple-midi._udp.local.", mDNSRecords);
}


void ZeroConfig_CPP_CX_Test::MainPage::OnVectorChanged(Windows::Foundation::Collections::IObservableVector<ZeroConfig::ZeroConfRecord ^> ^sender, Windows::Foundation::Collections::IVectorChangedEventArgs ^event)
{
	if (event->CollectionChange == CollectionChange::ItemInserted)
	{
		bool found = false;
		auto zc = mDNSRecords->GetAt(event->Index);
		auto client = ref new ClientSystemInfo(zc->IPAddress, zc->Name, zc->Port);

		for (ClientSystemInfo^ c : Directory)
		{
			if (c->Equals(client))
				found = true;
		}

		if (!found)
		{
			this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this, client]()
			{
				this->Directory->Append(client);
			}));
		}
	}
}
