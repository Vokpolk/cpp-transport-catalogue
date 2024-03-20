#include "json_builder.h"
#include <exception>
#include <variant>
#include <utility>

using namespace std::literals;

namespace json {
    //ValueAfterKey
    ValueAfterKey::ValueAfterKey(Builder& builder) : builder_(builder) {}

    KeyItemContext ValueAfterKey::Key(std::string key) {
        return builder_.Key(key);
    }
    Builder& ValueAfterKey::EndDict() {
        return builder_.EndDict();
    }
    //ValueAfterKey

    //ValueAfterArray
    ValueAfterArray::ValueAfterArray(Builder& builder) : builder_(builder) {}

    ValueAfterArray ValueAfterArray::Value(Node::Value value) {
        return builder_.Value(value);
    }
    DictItemContext ValueAfterArray::StartDict() {
        return builder_.StartDict();
    }
    ArrayItemContext ValueAfterArray::StartArray() {
        return builder_.StartArray();
    }
    Builder& ValueAfterArray::EndArray() {
        return builder_.EndArray();
    }
    //ValueAfterArray

    //ValueItemContext
    ValueItemContext::ValueItemContext(Builder& builder) : builder_(builder) {}

    KeyItemContext ValueItemContext::Key(std::string key) {
        return builder_.Key(key);
    }

    ValueAfterKey ValueItemContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    DictItemContext ValueItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder& ValueItemContext::EndDict() {
        return builder_.EndDict();
    }

    Builder& ValueItemContext::EndArray() {
        return builder_.EndArray();
    }
    //ValueItemContext

    //KeyItemContext
    KeyItemContext::KeyItemContext(Builder& builder) : builder_(builder) {}

    ValueAfterKey KeyItemContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    DictItemContext KeyItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext KeyItemContext::StartArray() {
        return builder_.StartArray();
    }
    //KeyItemContext

    //ArrayItemContext
    ArrayItemContext::ArrayItemContext(Builder& builder) : builder_(builder) {}

    ValueAfterArray ArrayItemContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    DictItemContext ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }
    //ArrayItemContext

    //DictItemContext
    DictItemContext::DictItemContext(Builder& builder) : builder_(builder) {}

    KeyItemContext DictItemContext::Key(std::string key) {
        return builder_.Key(key);
    }

    Builder& DictItemContext::EndDict() {
        return builder_.EndDict();
    }
    //DictItemContext


    Builder::Builder()
        : root_()
        , nodes_stack_{ &root_ }
    {}

    Node Builder::Build() {
        if (!nodes_stack_.empty()) {
            throw std::logic_error("Attempt to build JSON which isn't finalized"s);
        }
        return std::move(root_);
    }

    KeyItemContext Builder::Key(std::string key) {
        Node::Value& host_value = GetCurrentValue();

        if (!std::holds_alternative<Dict>(host_value)) {
            throw std::logic_error("Key() outside a dict"s);
        }

        nodes_stack_.push_back(
            &std::get<Dict>(host_value)[std::move(key)]
        );
        return KeyItemContext{ *this };
    }

    Builder& Builder::Value(Node::Value value) {
        AddObject(value, /* one_shot */ true);
        return *this;
    }

    DictItemContext Builder::StartDict() {
        AddObject(Dict{}, /* one_shot */ false);
        return DictItemContext{*this};
    }

    ArrayItemContext Builder::StartArray() {
        AddObject(Array{}, /* one_shot */ false);
        return ArrayItemContext{*this};
    }

    Builder& Builder::EndDict() {
        if (!std::holds_alternative<Dict>(GetCurrentValue())) {
            throw std::logic_error("EndDict() outside a dict"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        if (!std::holds_alternative<Array>(GetCurrentValue())) {
            throw std::logic_error("EndDict() outside an array"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    // Current value can be:
    // * Dict, when .Key().Value() or EndDict() is expected
    // * Array, when .Value() or EndArray() is expected
    // * nullptr (default), when first call or dict Value() is expected

    Node::Value& Builder::GetCurrentValue() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("Attempt to change finalized JSON"s);
        }
        return nodes_stack_.back()->GetValue();
    }

    // Tell about this trick
    const Node::Value& Builder::GetCurrentValue() const {
        return const_cast<Builder*>(this)->GetCurrentValue();
    }

    void Builder::AssertNewObjectContext() const {
        if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
            throw std::logic_error("New object in wrong context"s);
        }
    }

    void Builder::AddObject(Node::Value value, bool one_shot) {
        Node::Value& host_value = GetCurrentValue();
        if (std::holds_alternative<Array>(host_value)) {
            // Tell about emplace_back
            Node& node
                = std::get<Array>(host_value).emplace_back(std::move(value));
            if (!one_shot) {
                nodes_stack_.push_back(&node);
            }
        }
        else {
            AssertNewObjectContext();
            host_value = std::move(value);
            if (one_shot) {
                nodes_stack_.pop_back();
            }
        }
    }

}  // namespace json