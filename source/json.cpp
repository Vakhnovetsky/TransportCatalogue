#include "json.h"

using namespace std;

namespace json {

    namespace {

        string GetString(std::istream& input) {
            string result;
            while (isalpha(input.peek())) {
                result.push_back(static_cast<char>(input.get()));
            }
            return result;
        }

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input) {
                throw ParsingError("LoadArray error"s);
            }
            return Node(move(result));
        }

        Node LoadString(istream& input) {
            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            string result;
            while (true) {

                if (it == end) {
                    throw ParsingError("String parsing error");
                }

                const char ch = *it;

                if (ch == '"') {
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    ++it;
                    if (it == end) {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char) {
                    case 'n':
                        result.push_back('\n');
                        break;
                    case 't':
                        result.push_back('\t');
                        break;
                    case 'r':
                        result.push_back('\r');
                        break;
                    case '"':
                        result.push_back('"');
                        break;
                    case '\\':
                        result.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    result.push_back(ch);
                }
                ++it;
            }
            return Node(move(result));
        }


        Node LoadBool(istream& input) {
            string result = GetString(input);

            if (result == "true"sv) {
                return Node(true);
            }
            else if (result == "false"sv) {
                return Node(false);
            }
            else {
                throw ParsingError("Failed to parse '"s + result + "' as bool"s);
            }
        }

        Node LoadNull(istream& input) {
            string result = GetString(input);;

            if (result == "null"s) {
                return Node(nullptr);
            }
            else {
                throw ParsingError("Failed to parse '"s + result + "' as null"s);
            }
        }

        Node LoadDict(std::istream& input) {
            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == '"') {
                    std::string key = LoadString(input).AsString();
                    if (input >> c && c == ':') {
                        if (result.find(key) != result.end()) {
                            throw ParsingError("Duplicate key '"s + key + "' have been found");
                        }
                        result.emplace(move(key), LoadNode(input));
                    }
                    else {
                        throw ParsingError(": is expected but '"s + c + "' has been found"s);
                    }
                }
                else if (c != ',') {
                    throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
                }
            }
            if (!input) {
                throw ParsingError("Dictionary parsing error"s);
            }

            return Node(move(result));
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node{ std::stoi(parsed_num) };
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node{ std::stod(parsed_num) };
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            switch (c) {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
                [[fallthrough]];
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace*******************************************************************************************************************

    bool Node::IsNull() const {
        return holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return holds_alternative<Array>(*this);
    }

    bool Node::IsDict() const {
        return holds_alternative<Dict>(*this);
    }

    bool Node::IsBool() const {
        return holds_alternative<bool>(*this);
    }

    bool Node::IsInt() const {
        return holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return holds_alternative<double>(*this) || holds_alternative<int>(*this);
    }

    bool Node::IsPureDouble() const {
        return holds_alternative<double>(*this);
    }

    bool Node::IsString() const {
        return holds_alternative<std::string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("Not an array"s);
        }
        return *get_if<Array>(this);
    }

    const Dict& Node::AsDict() const {
        if (!IsDict()) {
            throw std::logic_error("Not an map"s);
        }
        return *get_if<Dict>(this);
    }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("Not an int"s);
        }
        return *get_if<int>(this);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("Not an bool"s);
        }
        return *get_if<bool>(this);
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("Not an double"s);
        }

        if (IsInt()) {
            return *get_if<int>(this);
        }
        else {
            return *get_if<double>(this);
        }
    }

    double Node::AsPureDouble() const {
        if (!IsPureDouble()) {
            throw std::logic_error("Not an PureDouble"s);
        }
        return *get_if<double>(this);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("Not an string"s);
        }
        return *get_if<string>(this);
    }

    Node::variant Node::GetValue() const {
        return *this;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintString(const std::string& value, std::ostream& out) {
        out.put('"');
        for (const char c : value) {
            switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '"':
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
            }
        }
        out.put('"');
    }

    void PrintValue(const double value, const RenderContext& ctx) {
        ctx.out << value;
    }

    void PrintValue(const int value, const RenderContext& ctx) {
        ctx.out << value;
    }

    void PrintValue(const std::string& value, const RenderContext& ctx) {
        PrintString(value, ctx.out);
    }

    void PrintValue(const std::nullptr_t&, const RenderContext& ctx) {
        ctx.out << "null"sv;
    }

    void PrintValue(const bool& value, const RenderContext& ctx) {
        ctx.out << (value ? "true"sv : "false"sv);
    }

    void PrintValue(const Array& nodes, const RenderContext& ctx) {
        std::ostream& out = ctx.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();

        for (const Node& node : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.RenderIndent();
            PrintNode(node, inner_ctx);
        }

        out.put('\n');
        ctx.RenderIndent();
        out.put(']');
    }

    void PrintValue(const Dict& nodes, const RenderContext& ctx) {
        std::ostream& out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();

        for (const auto& [key, node] : nodes) {
            if (first) {
                first = false;
            }
            else {
                out << ",\n"sv;
            }
            inner_ctx.RenderIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, inner_ctx);
        }

        out.put('\n');
        ctx.RenderIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const RenderContext& ctx) {
        std::visit(
            [&ctx](const auto& value) {
                PrintValue(value, ctx);
            },
            node.GetValue());
    }

    bool Node::operator==(const Node& rhs) const {
        return *this == rhs.GetValue();
    }

    bool Node::operator!=(const Node& rhs) const {
        return *this != rhs.GetValue();
    }

    bool Document::operator==(const Document& rhs) const {
        return GetRoot() == rhs.GetRoot();
    }

    bool Document::operator!=(const Document& rhs) const {
        return GetRoot() != rhs.GetRoot();
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), RenderContext{ output });
    }

}  // namespace json