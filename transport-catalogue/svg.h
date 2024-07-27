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

// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor{};

std::ostream& operator<<(std::ostream& out, const Color color);

bool operator==(const Color color1, const Color color2);

bool operator!=(const Color color1, const Color color2);

template <typename Owner>
class PathProps { // template class можно реализовывать только в .h файле.
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
    * Класс Circle моделирует элемент <circle> для отображения круга
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
    * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
    * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
    */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline() = default;

    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
        * Прочие методы и данные, необходимые для реализации элемента <polyline>
        */
private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/*
    * Класс Text моделирует элемент <text> для отображения текста
    * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
    */
class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>

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

    // Добавляет в svg-документ объект-наследник svg::Object
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

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg