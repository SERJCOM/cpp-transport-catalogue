#include "svg.h"
#include <ostream>

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    //context.Indented();

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;

    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline &Polyline::AddPoint(Point point)
{
    point_.push_back(point);
   return *this;
}

void Polyline::RenderObject(const RenderContext &ctx) const
{
    auto& out = ctx.out;
    out << "<polyline points=\""sv;
    for(size_t i = 0; i < point_.size(); i++){
        out << point_[i].x << "," << point_[i].y;
        if(i != point_.size() - 1)
            out << " ";
    }

    
    
    out << "\"";
    
    RenderAttrs(out);

    out <<  "/>"sv;
}


// ---------- Text ------------------

Text &Text::SetPosition(Point pos)
{
    point_ = pos;
    return *this;
}

Text &Text::SetOffset(Point offset)
{
    offset_ = offset;
    return *this;
}

Text &Text::SetFontSize(uint32_t size)
{
    font_size_ = size;
    return *this;
}

Text &Text::SetFontFamily(std::string_view font_family)
{
    font_family_ = font_family;
    return *this;
}

Text &Text::SetFontWeight(std::string_view font_weight)
{
    font_weight_ = font_weight;
    return *this;
}

Text &Text::SetData(std::string_view data)
{
    data_ = data;
    return *this;
}

// <text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">Hello C++</text>
void Text::RenderObject(const RenderContext &ctx) const
{
    auto& out = ctx.out;

    out << "<text"sv;

    RenderAttrs(out);

    out << " x=\""sv << point_.x << "\"";
    out << " y=\""sv << point_.y << "\"";
    out << " dx=\""sv << offset_.x << "\"";
    out << " dy=\""sv << offset_.y << "\"";
    out << " font-size=\""sv << font_size_ << "\"";
    if(font_family_.size() > 0)
        out << " font-family=\""sv << font_family_ << "\"";
    if(font_weight_.size() > 0)
        out << " font-weight=\""sv << font_weight_ << "\"";

    out << ">";
    std::string copy = data_;

    size_t pos = 0;
    while(true){
         pos = copy.find("&", pos + 1);
        if(pos == copy.npos)   break;
        else{
            copy.replace(pos, 1, "&amp;"s);
        }
    }

    while(true){
        size_t pos = copy.find("\"");
        if(pos == copy.npos)   break;
        else{
            copy.replace(pos, 1, "&quot;"s);
        }
    }

    while(true){
        size_t pos = copy.find("<");
        if(pos == copy.npos)   break;
        else{
            copy.replace(pos, 1, "&lt;"s);
        }
    }

    while(true){
        size_t pos = copy.find(">");
        if(pos == copy.npos)   break;
        else{
            copy.replace(pos, 1, "&gt;"s);
        }
    }

    while(true){
        size_t pos = copy.find("\'");
        if(pos == copy.npos)   break;
        else{
            copy.replace(pos, 1, "&apos;"s);
        }
    }
    
    out << copy;
    out << "</text>"; 
}


void Document::Render(std::ostream &out) const
{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out<< "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for(const auto& i: object_){
        i->Render({out, 2, 2});
    }
    out << "</svg>"sv;
}

void Document::AddPtr(std::unique_ptr<Object> &&obj)
{
    object_.push_back(std::move(obj));
}



} // namespace svg




std::ostream& operator<< (std::ostream& out, const svg::StrokeLineJoin line){
    switch(line){
        case svg::StrokeLineJoin::ARCS :
            out << "arcs";
            break;

        case svg::StrokeLineJoin::BEVEL :
            out << "bevel";
            break;

        case svg::StrokeLineJoin::MITER :
            out << "miter";
            break;

        case svg::StrokeLineJoin::MITER_CLIP :
            out << "miter-clip";
            break;

        case svg::StrokeLineJoin::ROUND :
            out << "round";
            break;
        } 

    return out;
}


void svg::RenderColor(std::ostream& out, std::monostate) {
    out << "none";
}

void svg::RenderColor(std::ostream& out, const std::string& value) {
    out << value;
}

void svg::RenderColor(std::ostream& out, svg::Rgb rgb) {
    out << "rgb(" << static_cast<int>(rgb.red)  //
        << ',' << static_cast<int>(rgb.green)     //
        << ',' << static_cast<int>(rgb.blue) << ')';
}

void svg::RenderColor(std::ostream& out, svg::Rgba rgba) {
    out << "rgba(" << static_cast<int>(rgba.red)  //
        << ',' << static_cast<int>(rgba.green)      //
        << ',' << static_cast<int>(rgba.blue)       //
        << ',' << rgba.opacity << ')';
}



std::ostream &operator<<(std::ostream &out, const svg::Color line)
{

    std::visit([&](auto value){
        svg::RenderColor(out, value);
    }, line);


   return out;
}

std::ostream& operator<< (std::ostream& out, const svg::StrokeLineCap line){
    switch (line)
        {
        case svg::StrokeLineCap::BUTT :
            out << "butt"; 
            break;
        
        case svg::StrokeLineCap::ROUND :
            out << "round";
            break;
        
        case svg::StrokeLineCap::SQUARE :
            out << "square";
            break;
        default:
            break;
    }

    return out;
}


