#pragma once
#include "PicItem.g.h"

namespace winrt::educamlite::implementation
{
    struct PicItem : PicItemT<PicItem>
    {
    public:
        PicItem(Windows::Storage::Streams::IRandomAccessStream const&, uint32_t const&);

        uint32_t Order();
        void Order(uint32_t const&);
        Windows::UI::Xaml::Media::Imaging::BitmapImage PicSource();

        void SetImageFrom(Windows::Storage::Streams::IRandomAccessStream const&);

        ~PicItem();
    private:
        uint32_t _Order;

        Windows::Storage::Streams::IRandomAccessStream _StreamSource;
        Windows::UI::Xaml::Controls::TextBlock _OrderView;
        Windows::UI::Xaml::Media::Imaging::BitmapImage _PicSource;
    };
}

namespace winrt::educamlite::factory_implementation
{
    struct PicItem : PicItemT<PicItem, implementation::PicItem>
    {
    };
}
