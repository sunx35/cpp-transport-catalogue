#pragma once

#include "json.h"

namespace json {

class Builder {
    class BaseContext;
    class DictItemContext;
    class KeyItemContext;
    class ArrayItemContext;
public:
    Builder();
    KeyItemContext Key(std::string key);
    BaseContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build() const;

private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    bool key_added_ = false;
    bool value_assigned_ = false;

    class BaseContext {
    public:
        BaseContext(Builder& builder)
            : builder_(builder) {}
        KeyItemContext Key(std::string key) {
            return builder_.Key(key);
        }
        BaseContext Value(Node::Value value) {
            return builder_.Value(value);
        }
        DictItemContext StartDict() {
            return builder_.StartDict();
        }
        ArrayItemContext StartArray() {
            return builder_.StartArray();
        }
        BaseContext EndDict() {
            return builder_.EndDict();
        }
        BaseContext EndArray() {
            return builder_.EndArray();
        }
        Node Build() const {
            return builder_.Build();
        }

    private:
        Builder& builder_;
    };

    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext context)
            : BaseContext(context) {}
        BaseContext Value(Node::Value value) = delete;
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;
    };

    class KeyItemContext : public BaseContext {
    public:
        KeyItemContext(BaseContext context)
            : BaseContext(context) {}
        DictItemContext Value(Node::Value value) {
            return BaseContext::Value(value);
        }
        KeyItemContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;
    };

    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext context)
            : BaseContext(context) {}
        ArrayItemContext Value(Node::Value value) {
            return BaseContext::Value(value);
        }
        KeyItemContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        Node Build() = delete;
    };
};

} // namespace json