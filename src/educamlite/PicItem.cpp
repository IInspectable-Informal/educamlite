#include "pch.h"
#include "PicItem.h"
#if __has_include("PicItem.g.cpp")
#include "PicItem.g.cpp"
#endif

using ns winrt;
using ns Windows::Storage::Streams;
using ns Windows::UI;
using ns Windows::UI::Xaml;
using ns Windows::UI::Xaml::Controls;
using ns Windows::UI::Xaml::Media;
using ns Windows::UI::Xaml::Media::Imaging;

namespace winrt::educamlite::implementation
{
    PicItem::PicItem(IRandomAccessStream const& picStream, uint32_t const& order) : _Order(order), _StreamSource(picStream)
    {
        Grid grid; ContentPresenter presenter; Image imageView;
        ColumnDefinition colDef1, colDef2;
        colDef1.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));
        colDef2.Width(GridLengthHelper::Auto());
        grid.ColumnDefinitions().ReplaceAll({ colDef1, colDef2 });
        grid.MinWidth(320); grid.ColumnSpacing(4); grid.Margin(ThicknessHelper::FromUniformLength(4));
        _OrderView.VerticalAlignment(VerticalAlignment::Center); _OrderView.Text(to_hstring(order));
        presenter.Width(268); presenter.Height(150);
        SolidColorBrush brush; brush.Color(Colors::Black()); presenter.Background(brush);
        _PicSource.SetSource(picStream);
        imageView.Source(_PicSource); presenter.Content(imageView);
        grid.Children().ReplaceAll({ _OrderView, presenter }); Grid::SetColumn(presenter, 1);
        Content(grid);
    }

    uint32_t PicItem::Order()
    { return _Order; }

    void PicItem::Order(uint32_t const& order)
    { _Order = order; _OrderView.Text(to_hstring(order)); }

    BitmapImage PicItem::PicSource()
    { return _PicSource; }

    void PicItem::SetImageFrom(IRandomAccessStream const& picStream)
    { _PicSource.SetSource(picStream); }

    PicItem::~PicItem()
    {
        _PicSource.SetSource(nullptr);
        _StreamSource.Close();
    }
}
