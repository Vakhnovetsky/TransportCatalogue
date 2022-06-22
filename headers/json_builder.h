#pragma once

#include "json.h"

#include <deque>
#include <string>
#include <variant>
#include <vector>

namespace json {

    enum class MethodTypes {
        BEFORE_START,
        BUILD,
        END_ARRAY,
        END_DICT,
        KEY,
        START_ARRAY,
        START_DICT,
        VALUE
    };
    class Builder;
    class DictItemContext;
    class ArrayItemContext;

    class KeyItemContext {
    public:
        KeyItemContext(Builder& builder);
        DictItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();

    private:
        Builder& builder_;
    };

    class DictItemContext {
    public:
        DictItemContext(Builder& builder);
        KeyItemContext Key(std::string key);
        Builder& EndDict();

    private:
        Builder& builder_;
    };

    class ArrayItemContext {
    public:
        ArrayItemContext(Builder& builder);
        ArrayItemContext Value(Node::Value value);

        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndArray();

    private:
        Builder& builder_;
    };

    class Builder final {
    public:

        Builder() = default;

        KeyItemContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();
        Node Build();

    private:
        MethodTypes current_method_ = MethodTypes::BEFORE_START;
        std::vector<std::string> key_;
        std::deque<Node> nodes_;
        std::vector<Node*> nodes_stack_;
        Node root_;
        
        Node GetNode(Node::Value value);
        void CheckStart(std::string container);
    };
}