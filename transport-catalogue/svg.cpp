#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& slc) {
    switch (slc) {
    case StrokeLineCap::BUTT:
        return out << "butt";
    case StrokeLineCap::ROUND:
        return out << "round";
    case StrokeLineCap::SQUARE:
        return out << "square";
    default:
        return out << "";
    }
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& slj) {
    switch (slj) {
    case StrokeLineJoin::ARCS:
        return out << "arcs";
    case StrokeLineJoin::BEVEL:
        return out << "bevel";
    case StrokeLineJoin::MITER:
        return out << "miter";
    case StrokeLineJoin::MITER_CLIP:
        return out << "miter-clip";
    case StrokeLineJoin::ROUND:
        return out << "round";
    default:
        return out << "";
    }
}

struct ColorVisitorPrinter {
    std::ostream& out;

    void operator()(std::monostate) {
        out << "none";
    }
    void operator()(std::string str) {
        out << str;
    }
    void operator()(Rgb rgb) {
        out << "rgb(" << unsigned(rgb.red) << "," << unsigned(rgb.green) << "," << unsigned(rgb.blue) << ")";
    }
    void operator()(Rgba rgba) {
        out << "rgba(" << unsigned(rgba.red) << "," << unsigned(rgba.green) << ","
            << unsigned(rgba.blue) << "," << rgba.opacity << ")";
    }
};

struct ColorVisitorConverter {
    std::string& str;

    void operator()(std::monostate) {
        str = "none";
    }
    void operator()(std::string _str) {
        str = _str;
    }
    void operator()(Rgb rgb) {
        str = "rgb(" + std::to_string(rgb.red) + "," + std::to_string(rgb.green) + "," + std::to_string(rgb.blue) + ")";
    }
    void operator()(Rgba rgba) {
        str = "rgba(" + std::to_string(rgba.red) + "," + std::to_string(rgba.green) + "," + std::to_string(rgba.blue)
            + "," + std::to_string(rgba.opacity) + ")";
    }
};

std::ostream& operator<<(std::ostream& out, const Color color) {
    std::visit(ColorVisitorPrinter{ out }, color);
    return out;
}

bool operator==(const Color color1, const Color color2) {
    std::string str1;
    std::string str2;
    std::visit(ColorVisitorConverter{ str1 }, color1);
    std::visit(ColorVisitorConverter{ str2 }, color2);
    return str1 == str2;
}

bool operator!=(const Color color1, const Color color2) {
    return !(color1 == color2);
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
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

// ---------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;

    int size = points_.size();
    for (const auto& p : points_) {
        // if size == 0 - никаких проходов не будет вообще
        if (size == 1) {
            out << p.x << "," << p.y;
        }
        else {
            out << p.x << "," << p.y << " ";
        }
        --size;
    }

    out << "\"";
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text --------------------

// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

std::string Text::ShieldDataString(std::string data) const {
    std::string new_data;

    for (char c : data) {
        if (c == '\"') {
            new_data.append("&quot;");
        }
        else if (c == '\'') {
            new_data.append("&apos;");
        }
        else if (c == '<') {
            new_data.append("&lt;");
        }
        else if (c == '>') {
            new_data.append("&gt;");
        }
        else if (c == '&') {
            new_data.append("&amp;");
        }
        else {
            new_data.append(1, c);
        }
    }

    return new_data;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    // <text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">Hello C++</text>

    out << "<text x=\""sv << pos_.x << "\" y=\"" << pos_.y << "\" ";
    out << "dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" ";
    out << "font-size=\"" << font_size_ << "\"";
    if (font_family_ != ""s) {
        out << " font-family=\"" << font_family_ << "\"";
    }
    if (font_weight_ != ""s) {
        out << " font-weight=\"" << font_weight_ << "\"";
    }
    RenderAttrs(out);
    out << ">" << ShieldDataString(data_) << "</text>";
}

// ---------- Document ----------------

// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    RenderContext ctx(out, 2, 2);
    for (auto& each : objects_) {
        each->Render(ctx);
    }

    out << "</svg>"sv;
}

}  // namespace svg