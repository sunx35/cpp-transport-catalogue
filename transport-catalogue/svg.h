#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

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

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& slc);

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& slj);

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
    * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
    * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
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
        return { out, indent_step, indent + indent_step };
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
    * ����������� ������� ����� Object ������ ��� ���������������� ��������
    * ���������� ����� SVG-���������
    * ��������� ������� "��������� �����" ��� ������ ����������� ����
    */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

struct Rgb {
    Rgb() = default;

    Rgb(uint8_t red, uint8_t green, uint8_t blue)
        : red(red)
        , green(green)
        , blue(blue) {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    Rgba() = default;

    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
        : red(red)
        , green(green)
        , blue(blue)
        , opacity(opacity) {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

// ������� � ������������ ����� ��������� �� �������������� inline,
// �� ������� ���, ��� ��� ����� ����� �� ��� ������� ����������,
// ������� ���������� ���� ���������.
// � ��������� ������ ������ ������� ���������� ����� ������������ ���� ����� ���� ���������
inline const Color NoneColor{};

std::ostream& operator<<(std::ostream& out, const Color color);

bool operator==(const Color color1, const Color color2);

bool operator!=(const Color color1, const Color color2);

template <typename Owner>
class PathProps { // template class ����� ������������� ������ � .h �����.
public:
    Owner& SetFillColor(Color color) {
        fill_ = color;
        return static_cast<Owner&>(*this);
    }

    Owner& SetStrokeColor(Color color) {
        stroke_ = color;
        return static_cast<Owner&>(*this);
    }

    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return static_cast<Owner&>(*this);
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_linecap_ = line_cap;
        return static_cast<Owner&>(*this);
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_linejoin_ = line_join;
        return static_cast<Owner&>(*this);
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        if (fill_) {
            out << " fill=\"" << *fill_ << "\"";
        }
        if (stroke_) {
            out << " stroke=\"" << *stroke_ << "\"";
        }
        if (stroke_width_) {
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        }
        if (stroke_linecap_) {
            out << " stroke-linecap=\"" << *stroke_linecap_ << "\"";
        }
        if (stroke_linejoin_) {
            out << " stroke-linejoin=\"" << *stroke_linejoin_ << "\"";
        }
    }

    std::optional<Color> fill_;
    std::optional<Color> stroke_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_linecap_;
    std::optional<StrokeLineJoin> stroke_linejoin_;
};

/*
    * ����� Circle ���������� ������� <circle> ��� ����������� �����
    * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
    */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle() = default;

    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_ = { 0.0, 0.0 };
    double radius_ = 1.0;
};

/*
    * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
    * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
    */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline() = default;

    // ��������� ��������� ������� � ������� �����
    Polyline& AddPoint(Point point);

    /*
        * ������ ������ � ������, ����������� ��� ���������� �������� <polyline>
        */
private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/*
    * ����� Text ���������� ������� <text> ��� ����������� ������
    * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
    */
class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;

    // ����� ���������� ������� ����� (�������� x � y)
    Text& SetPosition(Point pos);

    // ����� �������� ������������ ������� ����� (�������� dx, dy)
    Text& SetOffset(Point offset);

    // ����� ������� ������ (������� font-size)
    Text& SetFontSize(uint32_t size);

    // ����� �������� ������ (������� font-family)
    Text& SetFontFamily(std::string font_family);

    // ����� ������� ������ (������� font-weight)
    Text& SetFontWeight(std::string font_weight);

    // ����� ��������� ���������� ������� (������������ ������ ���� text)
    Text& SetData(std::string data);

    // ������ ������ � ������, ����������� ��� ���������� �������� <text>

    std::string ShieldDataString(std::string data) const;

private:
    void RenderObject(const RenderContext& context) const override;

    Point pos_ = { 0.0, 0.0 };
    Point offset_ = { 0.0, 0.0 };
    uint32_t font_size_ = 1;
    std::string font_family_ = "";
    std::string font_weight_ = "";
    std::string data_ = "";
};

class ObjectContainer {
public:
    template <typename T>
    void Add(T obj);

    // ��������� � svg-�������� ������-��������� svg::Object
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
};

template <typename T>
void ObjectContainer::Add(T obj) {
    AddPtr(std::make_unique<T>(obj));
}

class Drawable {
public:
    virtual ~Drawable() = default;

    virtual void Draw(ObjectContainer& container) const = 0;
};

class Document : public ObjectContainer {
public:
    Document() = default;

    // Add in ObjectContainer

    // ��������� � svg-�������� ������-��������� svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // ������� � ostream svg-������������� ���������
    void Render(std::ostream& out) const;

    // ������ ������ � ������, ����������� ��� ���������� ������ Document
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg