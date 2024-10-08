#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <string_view>
#include <optional>
namespace svg {
    using namespace std::literals;

    namespace detail {

        template <typename T>
        inline void RenderValue(std::ostream& out, const T& value) {
            out << value;
        }

        void HtmlEncodeString(std::ostream& out, std::string_view sv);

        template <>
        inline void RenderValue<std::string>(std::ostream& out, const std::string& s) {
            HtmlEncodeString(out, s);
        }
        template <typename AttrType>
        inline void RenderAttr(std::ostream& out, std::string_view name, const AttrType& value) {
            out << name << "=\""sv;
            RenderValue(out, value);
            out.put('"');
        }
        template <typename AttrType>
        inline void RenderOptionalAttr(std::ostream& out, std::string_view name, const std::optional<AttrType>& value) {
            if (value) {
                RenderAttr(out, name, *value);
            }
        }

    } //namespace

    using Color = std::string;
    inline const Color NoneColor{ "none" };

    enum class StrokeLineCap { //��� ����� ����� �����.
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin { //��� ����� ���������� �����.
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color fill_color) {
            fill_color_ = std::move(fill_color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color stroke_color) {
            stroke_color_ = std::move(stroke_color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double stroke_width) {
            stroke_width_ = stroke_width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            detail::RenderOptionalAttr(out, " fill"sv, fill_color_);
            detail::RenderOptionalAttr(out, " stroke"sv, stroke_color_);
            detail::RenderOptionalAttr(out, " stroke-width"sv, stroke_width_);
            detail::RenderOptionalAttr(out, " stroke-linecap"sv, line_cap_);
            detail::RenderOptionalAttr(out, " stroke-linejoin"sv, line_join_);
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

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

    /*
     * ����� Circle ���������� ������� <circle> ��� ����������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // ��������� ��������� ������� � ������� �����
        Polyline& AddPoint(Point point);

        /*
         * ������ ������ � ������, ����������� ��� ���������� �������� <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_{};
    };

    /*
     * ����� Text ���������� ������� <text> ��� ����������� ������
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
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
    private:
        void RenderObject(const RenderContext& context) const override;

        Point position_{ 0.0, 0.0 };
        Point offset_{ 0.0, 0.0 };
        uint32_t size_{ 1 };
        std::string font_family_{ ""s };
        std::string font_weight_{ ""s };
        std::string data_{ ""s };
    };

    class ObjectContainer {
    public:
        virtual ~ObjectContainer() = default;

        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
        // ��������� � svg-�������� ������-��������� svg::Object
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    class Drawable {
    public:
        virtual ~Drawable() = default;

        virtual void Draw(ObjectContainer& object_container) const = 0;
    };

    class Document : public ObjectContainer {
    public:
        // ��������� � svg-�������� ������-��������� svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // ������� � ostream svg-������������� ���������
        void Render(std::ostream& out) const;

        // ������ ������ � ������, ����������� ��� ���������� ������ Document
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg