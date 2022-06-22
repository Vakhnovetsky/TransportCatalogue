#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using AllType = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private AllType {
    public:
        using variant::variant;
        using Value = variant;

        bool IsNull() const;

        bool IsArray() const;
        const Array& AsArray() const;

        bool IsDict() const;
        const Dict& AsDict() const;

        bool IsBool() const;
        bool AsBool() const;

        bool IsInt() const;
        int AsInt() const;

        bool IsDouble() const;
        double AsDouble() const;

        bool IsPureDouble() const;
        double AsPureDouble() const;

        bool IsString() const;
        const std::string& AsString() const;

        bool operator==(const Node& rhs) const;
        bool operator!=(const Node& rhs) const;

        variant GetValue() const;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& rhs) const;
        bool operator!=(const Document& rhs) const;

    private:
        Node root_;
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
        int indent_step = 4;
        int indent = 0;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);
    void PrintNode(const Node& node, const RenderContext& ctx);

}  // namespace json