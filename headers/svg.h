#pragma once

#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

    struct Rgb {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;

        Rgb(uint8_t p_red, uint8_t p_green, uint8_t p_blue)
            :red(p_red)
            , green(p_green)
            , blue(p_blue) {
        }

        Rgb() = default;
    };

    struct Rgba {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;

        Rgba(uint8_t p_red, uint8_t p_green, uint8_t p_blue, double p_opacity)
            :red(p_red)
            , green(p_green)
            , blue(p_blue)
            , opacity(p_opacity) {
        }

        Rgba() = default;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline void ColorPrinter(std::ostream& out, std::monostate) {
        using namespace std::literals;
        out << "none"sv;
    }

    inline void ColorPrinter(std::ostream& out, const std::string& color) {
        out << color;
    }

    inline void ColorPrinter(std::ostream& out, const Rgb& rgb) {
        using namespace std::literals;
        out << "rgb("sv << std::to_string(rgb.red) <<
            ","sv << std::to_string(rgb.green) <<
            ","sv << std::to_string(rgb.blue) <<
            ")"sv;
    }

    inline void ColorPrinter(std::ostream& out, const Rgba& rgba) {
        using namespace std::literals;
        out << "rgba("sv << std::to_string(rgba.red) <<
            ","sv << std::to_string(rgba.green) <<
            ","sv << std::to_string(rgba.blue) <<
            ","sv << rgba.opacity
            << ")"sv;
    }

    inline std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(
            [&out](auto& value) {
                ColorPrinter(out, value);
            }, color);
        return out;
    }

    inline const Color NoneColor{ "none" };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    inline std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap) {
        using namespace std::literals;
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            os << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            os << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            os << "square"sv;
            break;
        }
        return os;
    }

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    inline std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join) {
        using namespace std::literals;
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            os << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            os << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            os << "round"sv;
            break;
        }
        return os;
    }

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

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

    template <typename Owner>
    class PathProps {
    public:
        //цвет заливки
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        //цвет контура
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        //толщина линии
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = std::move(width);
            return AsOwner();
        }

        //тип формы конца линии
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = std::move(line_cap);
            return AsOwner();
        }

        //тип формы соединения линий
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << " fill=\""sv;
                out << *fill_color_;
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                out << *stroke_color_;
                out << "\""sv;
            }

            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }

            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv;
                out << stroke_line_cap_.value();
                out << "\""sv;
            }

            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv;
                out << stroke_line_join_.value();
                out << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
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

    class Circle : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::deque <Point> points_;
    };

    class Text : public Object, public PathProps<Text> {
    public:
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

    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_; // Задаёт координаты опорной точки (атрибуты x и y)
        Point offset_; // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        uint32_t size_ = 1; // Задаёт размеры шрифта (атрибут font-size)
        std::string font_family_; // Задаёт название шрифта (атрибут font-family)
        std::string font_weight_; // Задаёт толщину шрифта (атрибут font-weight)
        std::string data_; // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    };


    //ObjectContainer задаёт интерфейс для доступа к контейнеру SVG-объектов. Через этот интерфейс Drawable-объекты могут визуализировать себя, добавляя в контейнер SVG-примитивы
    //ObjectContainer — это абстрактный класс, а не интерфейс, так как шаблонный метод Add, принимающий наследников Object по значению, не получится сделать виртуальным: в C++ шаблонные методы не могут быть виртуальными. 
    class ObjectContainer {
    public:

        virtual ~ObjectContainer() = default;
        //Метод ObjectContainer::Add реализуйте на основе чисто виртуального метода ObjectContainer::AddPtr, принимающего unique_ptr<Object>&&
        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }

        // Добавляет в svg-документ объект-наследник svg::Object
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    //svg::Document — пока единственный класс библиотеки, реализующий интерфейс ObjectContainer
    class Document : public ObjectContainer {
    public:

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

}  // namespace svg