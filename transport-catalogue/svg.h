#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <sstream>

namespace svg {

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

}


std::ostream& operator<< (std::ostream& out, const svg::StrokeLineCap line);
std::ostream& operator<< (std::ostream& out, const svg::StrokeLineJoin line);

namespace svg {

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

struct Rgb{

    Rgb() = default;

    Rgb(uint8_t red, uint8_t green, uint8_t blue): red(red), green(green), blue(blue){}

    uint8_t red = 0, green = 0, blue = 0;
};

struct Rgba: Rgb{


    Rgba() = default;

    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity): Rgb(red, green, blue), opacity(opacity) {}

    double opacity = 1.0;
};


/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};




/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};


//INTERFACE

class ObjectContainer;

class Drawable{
public:

    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
protected:
};


class ObjectContainer{
public:

    template<typename T>
    void Add(T obj);

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    std::vector<std::unique_ptr<Object>> object_;
    ~ObjectContainer() = default;
};

// ------------------PathProps--------------

//using Color = std::string;

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

inline const Color NoneColor{};

template<typename Owner>
class PathProps{
public:

    Owner& SetFillColor(Color color);

    Owner& SetStrokeColor(Color color);

    Owner& SetStrokeWidth(double width);

    Owner& SetStrokeLineCap(StrokeLineCap line_cap);

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

    

protected:

    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const;

private:
    Owner& AsOwner(){
        return static_cast<Owner&>(*this);
    }

    std::optional<std::string> fill_color_;
    std::optional<std::string> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};



template<typename T>
void ObjectContainer::Add(T obj){
    object_.push_back(std::make_unique<T>(std::move(obj)));
}


/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle>{
public:

    Circle() = default;

    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};


/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final: public Object, public PathProps<Polyline> {
public:

    Polyline() = default;

    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:

    void RenderObject(const RenderContext& ctx) const override;

    std::vector<Point> point_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final: public Object, public PathProps<Text> {
public:

    Text() = default;

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string_view font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string_view font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string_view data);

private:
    void RenderObject(const RenderContext& ctx) const override;

    Point point_;
    Point offset_;
    std::string font_family_;
    uint32_t font_size_ = 1;
    std::string data_;
    std::string font_weight_;

    // Прочие данные и методы, необходимые для реализации элемента <text>
};

class Document: public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

private:
   
};



void RenderColor(std::ostream& out, std::monostate) ;
void RenderColor(std::ostream& out, const std::string& value) ;

void RenderColor(std::ostream& out, svg::Rgb rgb) ;
void RenderColor(std::ostream& out, svg::Rgba rgba) ;



template <typename Owner>
Owner &PathProps<Owner>::SetFillColor(Color color){
    std::ostringstream str;
    std::visit([&](auto value){
        RenderColor(str, value);
    }, color);
    fill_color_ = str.str();
    return AsOwner();
}

template <typename Owner>
Owner &PathProps<Owner>::SetStrokeColor(Color color){
    
    std::ostringstream str;
    std::visit([&](auto value){
        RenderColor(str, value);
    }, color);
    stroke_color_ = str.str();
    return AsOwner();
}

template <typename Owner>
Owner &PathProps<Owner>::SetStrokeWidth(double width){

    stroke_width_ = width;
    return AsOwner();
}

template <typename Owner>
Owner &PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap){

    line_cap_ = line_cap;  
    return AsOwner();
}

template <typename Owner>
Owner &PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join){

     line_join_ = line_join;
    return AsOwner();
}

template <typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream &out) const{

    using namespace std::literals;

        if(fill_color_){
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if(stroke_color_){
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if(stroke_width_){
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if(line_cap_){
            out << " stroke-linecap=\""sv; 
            
            out << *line_cap_;
            
            out << "\""sv;
        }
        if(line_join_){
            out << " stroke-linejoin=\""sv;
            
            out << *line_join_;
            
            out << "\""sv;
        }
}


} // namespace svg



std::ostream& operator<< (std::ostream& out, const svg::Color line) ;