#pragma once
#include "svg.h"
#include "domain.h"
#include <vector>
#include <optional>
#include <ostream>
#include <algorithm>

namespace ctlg{

    inline const double EPSILON = 1e-6;
bool IsZero(double value) ;


struct MapRenderData{
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    double bus_label_font_size;
    std::pair<double, double> bus_label_offset;
    double stop_label_font_size;
    std::pair<double, double> stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};

class SphereProjector {
public:
// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates

SphereProjector() = default;

template <typename PointInputIt>
SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
    : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}

// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer{
public:
    MapRenderer(){

    }


    MapRenderData data_;


    void DrawMap(std::ostream& out, const std::vector<const ctlg::BusRoute*>& routes);


    private:

    SphereProjector projector;

    svg::Text CreateText(const BusRoute* route , int pos);

    svg::Text CreatePodlozhka(const svg::Text& text);


    std::vector<const BusStop*> CreateLines(const BusRoute* route, svg::Document& doc, int pos);

// void DrawRoute(std::ostream& out, const BusRoute& route);

};

}