#pragma once

#include <istream>
#include <map>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Data = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node : private std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict> {
public:
    using variant::variant;
    using Value = variant;

    Node(Value value)
        : variant(std::move(value)) {
    }

    bool IsNull() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
    bool IsBool() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    double AsDouble() const;
    double AsPureDouble() const;
    const std::string& AsString() const;
    bool AsBool() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    Value& GetValue() {
        return *this;
    }
};

bool operator==(const Node&, const Node&);
bool operator!=(const Node&, const Node&);

class Document {
public:
    explicit Document(Node root);
    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document&, const Document&);
bool operator!=(const Document&, const Document&);

Document Load(std::istream& input);
void Print(const Document& doc, std::ostream& output);

} // namespace json