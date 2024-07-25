#include "json.h"

using namespace std;

namespace json {

Node::Node(nullptr_t data)
    : data_(data) {
}

Node::Node(int data)
    : data_(data) {
}

Node::Node(double data)
    : data_(data) {
}

Node::Node(std::string data)
    : data_(move(data)) {
}

Node::Node(bool data)
    : data_(data) {
}

Node::Node(Array data)
    : data_(move(data)) {
}

Node::Node(Dict data)
    : data_(move(data)) {
}

bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(data_);
}

bool Node::IsInt() const {
    return holds_alternative<int>(data_);
}

bool Node::IsDouble() const {
    return holds_alternative<double>(data_) || holds_alternative<int>(data_);
}

bool Node::IsPureDouble() const {
    return holds_alternative<double>(data_);
}

bool Node::IsString() const {
    return holds_alternative<string>(data_);
}

bool Node::IsBool() const {
    return holds_alternative<bool>(data_);
}

bool Node::IsArray() const {
    return holds_alternative<Array>(data_);
}

bool Node::IsMap() const {
    return holds_alternative<Dict>(data_);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("logic error");
    }
    return get<int>(data_);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw logic_error("logic error");
    }
    if (IsInt()) {
        return get<int>(data_);
    }
    return get<double>(data_);
}

double Node::AsPureDouble() const {
    if (!IsPureDouble()) {
        throw logic_error("logic error");
    }
    return get<double>(data_);
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw logic_error("logic error");
    }
    return get<string>(data_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("logic error");
    }
    return get<bool>(data_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("logic error");
    }
    return get<Array>(data_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw logic_error("logic error");
    }
    return get<Dict>(data_);
}

bool operator==(const Node& left, const Node& right) {
    if (left.IsNull() && right.IsNull()) {
        return true;
    }
    else if (left.IsInt() && right.IsInt() && (left.AsInt() == right.AsInt())) {
        return true;
    }
    // мы намеренно не сравниваем IsDouble, потому что 2 разных типа могут быть описаны в этом методе.
    else if (left.IsPureDouble() && right.IsPureDouble() && (left.AsPureDouble() == right.AsPureDouble())) {
        return true;
    }
    else if (left.IsString() && right.IsString() && (left.AsString() == right.AsString())) {
        return true;
    }
    else if (left.IsBool() && right.IsBool() && (left.AsBool() == right.AsBool())) {
        return true;
    }
    else if (left.IsArray() && right.IsArray() && (left.AsArray() == right.AsArray())) {
        return true;
    }
    else if (left.IsMap() && right.IsMap() && (left.AsMap() == right.AsMap())) {
        return true;
    }
    else {
        return false;
    }
}

bool operator!=(const Node& left, const Node& right) {
    return !(left == right);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator==(const Document& left, const Document& right) {
    return left.GetRoot() == right.GetRoot();
}

bool operator!=(const Document& left, const Document& right) {
    return !(left == right);
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    vector<Node> result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (input.eof()) {
        throw ParsingError("Array has no closing symbol");
    }

    return Node(move(result));
}

Node LoadNull(istream& input) {
    string str;
    while (input.peek() != ',' && input.peek() != ']' && input.peek() != '}' && input.peek() != EOF) {
        char c = input.get();
        if (c != ' ' && c != '\n' && c != '\t' && c != '\r') {
            str.push_back(c);
        }
    }
    if (str == "null") {
        return Node{};
    }
    else {
        throw ParsingError("wrong null");
    }
}

Node LoadBool(istream& input) {
    string str;
    while (input.peek() != ',' && input.peek() != ']' && input.peek() != '}' && input.peek() != EOF) {
        char c = input.get();
        if (c != ' ' && c != '\n' && c != '\t' && c != '\r') {
            str.push_back(c);
        }
    }
    if (str == "true") {
        return Node{ true };
    }
    else if (str == "false") {
        return Node{ false };
    }
    else {
        throw ParsingError("wrong bool");
    }
}

// Распознаем int или double
Node LoadNumber(istream& input) {
    /*
    "int": 123,
    "double" : 1.23,
    "double_negative" : -1.23,
    "double_e" : 1.23e3,
    "double_E" : 1.23E3,
    "double_e+" : 1.23e+3,
    "double_E+" : 1.23E+3,
    "double_e-" : 1.23e-3,
    "double_E-" : 1.23E-3
    */

    string str;
    char c;
    c = input.peek();
    while (isdigit(c) || c == '-' || c == '.' || c == 'e' || c == 'E' || c == '+') {
        str.push_back(input.get());
        c = input.peek();
    }
    // int or double
    if (str.find_first_of(".eE+") == string::npos) {
        return Node{ stoi(str) };
    }
    else {
        return Node{ stod(str) };
    }
}

// Deprecated
Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }
    return Node(result);
}

Node LoadString(istream& input) {
    //  "\"Hello, \\\"everybody\\\"\""s
    //  "Hello, \"everybody\""s
    //  "\"\\r\\n\\t\\\"\\\\\""s
    //  "\r\n\t\"\\"s

    // \\r \\n \\t \\\" \\\\ \"

    string line;
    bool flag_stop = false;

    while (flag_stop == false) {
        char c1;
        input.get(c1);
        if (input.eof()) {
            throw ParsingError("String has no closing symbol");
        }
        else if (c1 == '\\') {
            char c2;
            input.get(c2);
            if (c2 == 'r') {
                line.push_back('\r');
            }
            else if (c2 == 'n') {
                line.push_back('\n');
            }
            else if (c2 == 't') {
                line.push_back('\t');
            }
            else if (c2 == '\"') {
                line.push_back('\"');
            }
            else if (c2 == '\\') {
                line.push_back('\\');
            }
        }
        else if (c1 == '\"') {
            flag_stop = true;
        }
        else {
            line.push_back(c1);
        }
    }

    return Node(move(line));
}

Node LoadDict(istream& input) {
    map<string, Node> result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({ move(key), LoadNode(input) });
    }

    if (input.eof()) {
        throw ParsingError("Dict has no closing symbol");
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == ',') {
        throw ParsingError("Comma out of Array or Dict");
    }
    else if (c == ']') {
        throw ParsingError("Closing ']' out of Array");
    }
    else if (c == '}') {
        throw ParsingError("Closing '}' out of Dict");
    }
    else if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else if (c == '\"') {
        return LoadString(input);
    }
    else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    }
    else {
        input.putback(c);
        return LoadNumber(input);
    }
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}

struct Indent {
    int count = 0;
    int size = 4;
};

void DoIndent(Indent indent, ostream& output) {
    for (int i = 0; i < indent.count * indent.size; ++i) {
        output << ' ';
    }
}

void PrintNull(ostream& output) {
    output << "null";
}

void PrintInt(Node node, ostream& output) {
    output << node.AsInt();
}

void PrintDouble(Node node, ostream& output) {
    output << node.AsDouble();
}

// PureDouble не особо нужен, возьмет либо Int либо Double.

void PrintString(Node node, ostream& output) {
    //  "Hello, \"everybody\""s
    //  "\"Hello, \\\"everybody\\\"\""s

    output << "\"";  //  \"

    string str = node.AsString();
    for (const char c : str) {
        if (c == '\"') {
            output << "\\\""; //  \\\"
        }
        else if (c == '\\') {
            output << "\\\\";
        }
        else if (c == '\n') {
            output << "\\n";
        }
        else if (c == '\r') {
            output << "\\r";
        }
        else if (c == '\t') {
            output << "\\t";
        }
        else {
            output << c;
        }
    }

    output << "\"";
}

void PrintBool(Node node, ostream& output) {
    if (node.AsBool() == true) {
        output << "true";
    }
    else {
        output << "false";
    }

}

void PrintNode(Node node, ostream& output, Indent indent);

void PrintArray(Node node, ostream& output, Indent indent) {
    Array array = node.AsArray();

    output << '[' << '\n';
    indent.count += 1;

    bool is_first = true;
    for (const Node& node : array) {
        if (is_first) {
            is_first = false;
            DoIndent(indent, output);
            PrintNode(node, output, indent);
        }
        else {
            output << ',' << '\n';
            DoIndent(indent, output);
            PrintNode(node, output, indent);
        }
    }

    output << '\n';
    indent.count -= 1;
    DoIndent(indent, output);
    output << ']';
}

void PrintMap(Node node, ostream& output, Indent indent) {
    Dict dict = node.AsMap();

    output << '{' << '\n';
    indent.count += 1;

    bool is_first = true;
    for (const auto& [str, node] : dict) {
        if (is_first) {
            is_first = false;
            DoIndent(indent, output);
            output << '\"' << str << '\"' << ": ";
            PrintNode(node, output, indent);
        }
        else {
            output << ',' << '\n';
            DoIndent(indent, output);
            output << '\"' << str << '\"' << ": ";
            PrintNode(node, output, indent);
        }
    }

    output << '\n';
    indent.count -= 1;
    DoIndent(indent, output);
    output << '}';
}

void PrintNode(Node node, ostream& output, Indent indent) {
    if (node.IsNull()) {
        PrintNull(output);
    }
    else if (node.IsInt()) {
        PrintInt(node, output);
    }
    else if (node.IsDouble()) {
        PrintDouble(node, output);
    }
    // PureDouble не особо нужен, возьмет либо Int либо Double.
    else if (node.IsString()) {
        PrintString(node, output);
    }
    else if (node.IsBool()) {
        PrintBool(node, output);
    }
    else if (node.IsArray()) {
        PrintArray(node, output, indent);
    }
    else if (node.IsMap()) {
        PrintMap(node, output, indent);
    }
}

void Print(const Document& doc, ostream& output) {
    Indent indent;

    PrintNode(doc.GetRoot(), output, indent);
}

} // namespace json