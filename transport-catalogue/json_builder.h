#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {

    class Builder;
    class DictItemContext;
    class ArrayItemContext;
    class KeyItemContext;
    class ValueItemContext;

    class ValueAfterKey;
    class ValueAfterArray;

    class ValueAfterKey {
    private:
        Builder& builder_;
    public:
        ValueAfterKey(Builder& builder);

        KeyItemContext Key(std::string key);
        Builder& EndDict();
    };

    class ValueAfterArray {
    private:
        Builder& builder_;
    public:
        ValueAfterArray(Builder& builder);

        ValueAfterArray Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
    };
    
    class ValueItemContext {
    private:
        Builder& builder_;
    public:
        ValueItemContext(Builder& builder);

        KeyItemContext Key(std::string key);
        ValueAfterKey Value(Node::Value value);
        DictItemContext StartDict();
        Builder& EndDict();
        Builder& EndArray();
    };

    class KeyItemContext {
    private:
        Builder& builder_;
    public:
        KeyItemContext(Builder& builder);

        ValueAfterKey Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    };

    class ArrayItemContext {
    private:
        Builder& builder_;
    public:
        ArrayItemContext(Builder& builder);

        ValueAfterArray Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
    };

    class DictItemContext {
    private:
        Builder& builder_;
    public:
        DictItemContext(Builder& builder);

        KeyItemContext Key(std::string key);
        Builder& EndDict();
    };
    

    class Builder {
    public:
        Builder();
        Node Build();

        KeyItemContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();


    private:
        Node root_;
        std::vector<Node*> nodes_stack_;

        Node::Value& GetCurrentValue();
        const Node::Value& GetCurrentValue() const;
    
        void AssertNewObjectContext() const;
        void AddObject(Node::Value value, bool one_shot);
    };

}  // namespace json