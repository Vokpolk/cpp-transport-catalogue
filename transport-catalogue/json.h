#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <variant>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        /* Реализуйте Node, используя std::variant */
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        Node(std::nullptr_t null) : value_(null) {}
        Node(Array arr) : value_(arr) {}
        Node(Dict map) : value_(map) {}
        Node(bool boolean) : value_(boolean) {}
        Node(int num) : value_(num) {}
        Node(double num) : value_(num) {}
        Node(std::string str) : value_(str) {}

        const Value& GetValue() const { return value_; }

        bool IsInt() const {
            if (std::holds_alternative<int>(value_)) {
                return true;
            }
            else {
                return false;
            }
        }
        bool IsDouble() const {
            if (std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_)) {
                return true;
            }
            else {
                return false;
            }
        }
        bool IsPureDouble() const {
            if (std::holds_alternative<double>(value_)) {
                return true;
            }
            else {
                return false;
            }
        }
        bool IsBool() const {
            if (std::holds_alternative<bool>(value_)) {
                return true;
            }
            else {
                return false;
            }
        }
        bool IsString() const {
            if (std::holds_alternative<std::string>(value_)) {
                return true;
            }
            else {
                return false;
            }
        }
        bool IsNull() const {
            if (std::holds_alternative<std::nullptr_t>(value_)) {
                return true;
            }
            else {
                return false;
            }
        }
        bool IsArray() const {
            if (std::holds_alternative<Array>(value_)) {
                return true;
            }
            else {
                return false;
            }
        }
        bool IsMap() const {
            if (std::holds_alternative<Dict>(value_)) {
                return true;
            }
            else {
                return false;
            }
        }

        int AsInt() const {
            if (IsInt()) {
                return std::get<int>(value_);
            }
            throw std::logic_error("Not int");
        }
        bool AsBool() const {
            if (IsBool()) {
                return std::get<bool>(value_);
            }
            throw std::logic_error("Not bool");
        }
        double AsDouble() const {
            if (IsPureDouble()) {
                return std::get<double>(value_);
            }
            else if (IsDouble()) {
                return static_cast<double>(std::get<int>(value_));
            }
            throw std::logic_error("Not double");
        }
        const std::string& AsString() const {
            if (IsString()) {
                return std::get<std::string>(value_);
            }
            throw std::logic_error("Not string");
        }
        const Array& AsArray() const {
            if (IsArray()) {
                return std::get<Array>(value_);
            }
            throw std::logic_error("Not Array");
        }
        const Dict& AsMap() const {
            if (IsMap()) {
                return std::get<Dict>(value_);
            }
            throw std::logic_error("Not Dict");
        }

    private:
        Value value_ = std::nullptr_t{};
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);
    void Print(const Document& doc, std::ostream& output);

    bool operator==(const Node& l, const Node& r);
    bool operator!=(const Node& l, const Node& r);
    bool operator==(const Document& l, const Document& r);
    bool operator!=(const Document& l, const Document& r);
    void PrintNode(const Node& node, std::ostream& out);

}  // namespace json