#include "json_builder.h"

namespace json {

Builder::Builder() {
    nodes_stack_.push_back(&root_);
}

Builder::KeyItemContext Builder::Key(std::string key) {
    if (std::holds_alternative<Dict>(nodes_stack_.back()->GetValue())) {
        Dict& dict = std::get<Dict>(nodes_stack_.back()->GetValue());
        dict.emplace(key, Node{});
        nodes_stack_.push_back(&dict.at(key));
        key_added_ = true;
    }
    else {
        throw std::logic_error("You try to add Key, but there is no opened Dict");
    }
    return BaseContext(*this);
}

Builder::BaseContext Builder::Value(Node::Value value) {
    if (std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue()) && !key_added_ && !value_assigned_) {
        nodes_stack_.back()->GetValue() = value;
        value_assigned_ = true;
        return *this;
    }
    else if (std::holds_alternative<Array>(nodes_stack_.back()->GetValue())) {
        Array& array = std::get<Array>(nodes_stack_.back()->GetValue());
        array.emplace_back(value);
        return BaseContext(*this);
    }
    else if (key_added_) {
        nodes_stack_.back()->GetValue() = value;
        key_added_ = false;
        nodes_stack_.pop_back();
        return BaseContext(*this);
    }
    else {
        throw std::logic_error("You try to add value, but this is not a root, Array, or Node towards Key in Dict");
    }
}

Builder::DictItemContext Builder::StartDict() {
    if (std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue()) && !key_added_) {
        *nodes_stack_.back() = Dict{};
    }
    else if (std::holds_alternative<Array>(nodes_stack_.back()->GetValue())) {
        Array& array = std::get<Array>(nodes_stack_.back()->GetValue());
        array.push_back(Dict{});
        nodes_stack_.push_back(&array.back());
    }
    else if (key_added_) {
        *nodes_stack_.back() = Dict{};
        key_added_ = false;
    }
    else {
        throw std::logic_error("You try to start Dict, but there is no root, Array, or Node towards Key in Dict");
    }
    return BaseContext(*this);
}

Builder::ArrayItemContext Builder::StartArray() {
    if (std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue()) && !key_added_) {
        *nodes_stack_.back() = Array{};
    }
    else if (std::holds_alternative<Array>(nodes_stack_.back()->GetValue())) {
        Array& array = std::get<Array>(nodes_stack_.back()->GetValue());
        array.push_back(Array{});
        nodes_stack_.push_back(&array.back());
    }
    else if (key_added_) {
        *nodes_stack_.back() = Array{};
        key_added_ = false;
    }
    else {
        throw std::logic_error("You try to start Array, but there is no root, another Array, or Node towards Key in Dict");
    }
    return BaseContext(*this);
}

Builder::BaseContext Builder::EndDict() {
    if (std::holds_alternative<Dict>(nodes_stack_.back()->GetValue())) {
        nodes_stack_.pop_back();
    }
    else {
        throw std::logic_error("You try to close Dict, but there is not a Dict.");
    }
    return *this;
}

Builder::BaseContext Builder::EndArray() {
    if (std::holds_alternative<Array>(nodes_stack_.back()->GetValue())) {
        nodes_stack_.pop_back();
    }
    else {
        throw std::logic_error("You try to close Array, but there is not an Array.");
    }
    return *this;
}

Node Builder::Build() {
    if (value_assigned_) {
        return root_;
    }
    if (nodes_stack_.empty()) {
        return root_;
    }
    if ((std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue()) && !key_added_ && !value_assigned_) ||
        (std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue()) && key_added_) ||
        std::holds_alternative<Array>(nodes_stack_.back()->GetValue()) ||
        std::holds_alternative<Dict>(nodes_stack_.back()->GetValue())) {
        throw std::logic_error("You try to build object, but there is empty root or not completed Array or Dict");
    }
    return root_;
}

} // namespace json