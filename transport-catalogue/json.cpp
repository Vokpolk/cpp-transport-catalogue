#include "json.h"

namespace json {
    using namespace std::literals;
    namespace {
        Node LoadNode(std::istream& input);

        Node LoadArray(std::istream& input) {
            Array result;
            char c;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (c != ']')
            {
                throw ParsingError("Нет заверщающей скобки ]");
            }
            return Node(std::move(result));
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
                        return Node(std::stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(s);
        }

        Node LoadDict(std::istream& input) {
            Dict result;
            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                std::string key = LoadString(input).AsString();
                input >> c;
                result.insert({ std::move(key), LoadNode(input) });
            }
            if (c != '}' && c != ':') {
                throw ParsingError("нет завершающей скобки'}'");
            }
            return Node(std::move(result));
        }

        Node LoadBool(std::istream& input) {
            std::string str;
            char c;
            /*while ((c = static_cast<char>(input.get())) && std::isalpha(c))
            {
                str += c;
            }*/
            for (; input >> c && std::isalpha(c);) {
                str += c;
            }
            if (!std::isalpha(c)) {
                input.putback(c);
            }
            if (str == "ull"s || str == "ullptr"s) {
                return Node(nullptr);
            }
            if (str == "rue"s || str == "alse"s) {
                return Node(str == "rue"s);
            }
            throw ParsingError("Parsing Error"s);
        }

        Node LoadNode(std::istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {
                return LoadBool(input);
            }
            else if (c == 't') {
                return LoadBool(input);
            }
            else if (c == 'f') {
                return LoadBool(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Document::Document(Node root) : root_(std::move(root)) {}
    const Node& Document::GetRoot() const {
        return root_;
    }
    Document Load(std::istream& input) {
        return Document{ LoadNode(input) };
    }


    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };

    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx) {
        ctx.out << value;
    }

    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, const PrintContext& ctx) {
        ctx.out << "null"sv;
    }
    void PrintValue(Array array, const PrintContext& ctx) {
        ctx.out << '[';
        ctx.out << '\n';
        bool first = false;
        for (const auto& i : array) {
            if (first) {
                ctx.out << ", ";
                ctx.out << '\n';
            }
            first = true;
            ctx.Indented().PrintIndent();
            PrintNode(i, ctx.out);
        }
        ctx.out << '\n';
        ctx.out << ']';
    }
    void PrintValue(Dict map, const PrintContext& ctx) {
        ctx.out << '{';
        ctx.out << '\n';
        bool first = false;
        for (const auto& [key, value] : map) {
            if (first) {
                ctx.out << ',';
                ctx.out << '\n';
            }
            first = true;
            //ctx.out << '\"';
            //ctx.out << key;
            ctx.Indented().PrintIndent();
            PrintNode(key, ctx.out);
            ctx.out << ": ";
            PrintNode(value, ctx.out);
        }
        ctx.out << '\n';
        ctx.out << '}';
    }
    void PrintValue(std::string str, const PrintContext& ctx) {
        ctx.out << "\"";
        for (auto ch : str) {
            if (ch == '\"') {
                ctx.out << "\\\"";
            }
            else if (ch == '\n') {
                ctx.out << "\\n";
            }
            else if (ch == '\r') {
                ctx.out << "\\r";
            }
            else if (ch == '\t') {
                ctx.out << "\\t";
            }
            else if (ch == '\\') {
                ctx.out << "\\\\";
            }
            else {
                ctx.out << ch;
            }
        }
        ctx.out << "\"";
    }
    void PrintValue(bool boolean, const PrintContext& ctx) {
        boolean ? ctx.out << "true"s : ctx.out << "false"s;
    }
    void PrintNode(const Node& node, std::ostream& out) {
        const PrintContext ctx{ out };

        std::visit([&ctx](const auto& value) { PrintValue(value, ctx); }, node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
        //(void)&doc;
        //(void)&output;

        // Реализуйте функцию самостоятельно
    }

    bool operator==(const Node& l, const Node& r) {
        return l.GetValue() == r.GetValue();
    }
    bool operator!=(const Node& l, const Node& r) {
        return !(l == r);
    }

    bool operator==(const Document& l, const Document& r) {
        return l.GetRoot().GetValue() == r.GetRoot().GetValue();
    }
    bool operator!=(const Document& l, const Document& r) {
        return !(l == r);
    }

}  // namespace json
