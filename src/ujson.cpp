#include "uJSON/ujson.h"
#include <sstream>
#include <cctype>
#include <cmath>

namespace uJSON {

// 值的实现

Value::Value() : type_(Type::Null) {}
Value::Value(std::nullptr_t) : type_(Type::Null) {}
Value::Value(bool b) : type_(Type::Boolean), data_(b) {}
Value::Value(int i) : type_(Type::Number), data_(static_cast<double>(i)) {}
Value::Value(double d) : type_(Type::Number), data_(d) {}
Value::Value(const char* s) : type_(Type::String) {
    if (s) {
        data_ = std::string(s);
    } else {
        // std::cerr << "Warning: Value(const char*) initialized with nullptr" << std::endl;
        type_ = Type::Null;
        data_ = std::monostate{};
    }
}
Value::Value(const std::string& s) : type_(Type::String), data_(s) {}

Value Value::object() {
    Value v;
    v.type_ = Type::Object;
    v.data_ = std::make_shared<ObjectType>();
    return v;
}

Value Value::array() {
    Value v;
    v.type_ = Type::Array;
    v.data_ = std::make_shared<ArrayType>();
    return v;
}

Type Value::type() const { return type_; }
bool Value::is_null() const { return type_ == Type::Null; }
bool Value::is_boolean() const { return type_ == Type::Boolean; }
bool Value::is_number() const { return type_ == Type::Number; }
bool Value::is_number_integer() const { 
    if (type_ != Type::Number) return false;
    double d = std::get<double>(data_);
    double intpart;
    return std::modf(d, &intpart) == 0.0;
}
bool Value::is_string() const { return type_ == Type::String; }
bool Value::is_array() const { return type_ == Type::Array; }
bool Value::is_object() const { return type_ == Type::Object; }

bool Value::contains(const std::string& key) const {
    if (!is_object()) return false;
    auto& obj = *std::get<std::shared_ptr<ObjectType>>(data_);
    return obj.find(key) != obj.end();
}

const Value& Value::at(const std::string& key) const {
    if (!is_object()) throw TypeError("Not an object");
    auto& obj = *std::get<std::shared_ptr<ObjectType>>(data_);
    auto it = obj.find(key);
    if (it == obj.end()) throw RuntimeError("Key not found: " + key);
    return it->second;
}

Value& Value::operator[](const std::string& key) {
    if (!is_object()) {
        if (is_null()) {
            *this = Value::object(); // 自动转换为对象
        } else {
            throw TypeError("Not an object");
        }
    }
    auto& obj = *std::get<std::shared_ptr<ObjectType>>(data_);
    return obj[key];
}

Value& Value::operator[](const char* key) {
    if (key == nullptr) throw RuntimeError("Key cannot be null");
    return (*this)[std::string(key)];
}

void Value::push_back(const Value& v) {
    if (!is_array()) {
        if (is_null()) {
            *this = Value::array(); // 自动转换为数组
        } else {
            throw TypeError("Not an array");
        }
    }
    auto& arr = *std::get<std::shared_ptr<ArrayType>>(data_);
    arr.push_back(v);
}

const Value& Value::at(size_t index) const {
    if (!is_array()) throw TypeError("Not an array");
    auto& arr = *std::get<std::shared_ptr<ArrayType>>(data_);
    if (index >= arr.size()) throw RuntimeError("Index out of bounds");
    return arr[index];
}

Value& Value::operator[](size_t index) {
    if (!is_array()) {
        if (is_null()) {
            *this = Value::array();
        } else {
            throw TypeError("Not an array");
        }
    }
    auto& arr = *std::get<std::shared_ptr<ArrayType>>(data_);
    if (index >= arr.size()) {
        if (index >= arr.size()) {
             arr.resize(index + 1);
        }
    }
    return arr[index];
}

Value& Value::operator[](int index) {
    if (index < 0) throw RuntimeError("Index cannot be negative");
    return (*this)[static_cast<size_t>(index)];
}

const Value& Value::operator[](size_t index) const {
    return at(index);
}

const Value& Value::operator[](int index) const {
    if (index < 0) throw RuntimeError("Index cannot be negative");
    return at(static_cast<size_t>(index));
}

size_t Value::size() const {
    if (is_array()) return std::get<std::shared_ptr<ArrayType>>(data_)->size();
    if (is_object()) return std::get<std::shared_ptr<ObjectType>>(data_)->size();
    return 0;
}

template<> bool Value::get<bool>() const {
    if (!is_boolean()) throw TypeError("Not a boolean");
    return std::get<bool>(data_);
}

template<> int Value::get<int>() const {
    if (!is_number()) throw TypeError("Not a number");
    return static_cast<int>(std::get<double>(data_));
}

template<> double Value::get<double>() const {
    if (!is_number()) throw TypeError("Not a number");
    return std::get<double>(data_);
}

template<> std::string Value::get<std::string>() const {
    if (type_ == Type::Null) throw TypeError("Value is null, cannot convert to string");
    // 如果是字符串，则返回
    if (is_string()) return std::get<std::string>(data_);
    // 如果是 null，我们应该抛出 "Not a string"
    // 之前的错误 "basic_string::_M_construct null not valid" 表明有人试图从 nullptr 构造字符串。
    // 让我们验证 data_ 状态。
    
    throw TypeError("Not a string");
}

// 容器访问器实现
Value::ObjectType& Value::get_object() {
    if (!is_object()) throw TypeError("Not an object");
    return *std::get<std::shared_ptr<ObjectType>>(data_);
}

const Value::ObjectType& Value::get_object() const {
    if (!is_object()) throw TypeError("Not an object");
    return *std::get<std::shared_ptr<ObjectType>>(data_);
}

Value::ArrayType& Value::get_array() {
    if (!is_array()) throw TypeError("Not an array");
    return *std::get<std::shared_ptr<ArrayType>>(data_);
}

const Value::ArrayType& Value::get_array() const {
    if (!is_array()) throw TypeError("Not an array");
    return *std::get<std::shared_ptr<ArrayType>>(data_);
}

// 迭代器支持实现 (目前仅支持 Object，类似于 ConfigManager 的用法)
Value::ObjectType::iterator Value::begin() { return get_object().begin(); }
Value::ObjectType::const_iterator Value::begin() const { return get_object().begin(); }
Value::ObjectType::iterator Value::end() { return get_object().end(); }
Value::ObjectType::const_iterator Value::end() const { return get_object().end(); }

// 解析器实现

class Parser {
public:
    Parser(std::istream& is) : is_(is) {}
    
    Value parse() {
        skipWhitespace();
        char c = peek();
        if (is_.eof()) return Value();
        
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return parseString();
        if (c == 't' || c == 'f') return parseBoolean();
        if (c == 'n') return parseNull();
        if (c == '-' || isdigit(c)) return parseNumber();
        
        throw ParseError(std::string("Unexpected character: ") + c);
    }

private:
    std::istream& is_;

    char peek() { return static_cast<char>(is_.peek()); }
    char get() { return static_cast<char>(is_.get()); }
    
    void skipWhitespace() {
        while (isspace(peek())) get();
    }

    Value parseObject() {
        Value v = Value::object();
        get(); // 跳过 '{'
        skipWhitespace();
        if (peek() == '}') {
            get();
            return v;
        }
        
        while (true) {
            skipWhitespace();
            if (peek() != '"') throw ParseError("Expected string key");
            std::string key = parseStringValue();
            
            skipWhitespace();
            if (get() != ':') throw ParseError("Expected ':'");
            
            v[key] = parse();
            
            skipWhitespace();
            char c = get();
            if (c == '}') break;
            if (c != ',') throw ParseError("Expected ',' or '}'");
        }
        return v;
    }

    Value parseArray() {
        Value v = Value::array();
        get(); // 跳过 '['
        skipWhitespace();
        if (peek() == ']') {
            get();
            return v;
        }
        
        while (true) {
            v.push_back(parse());
            
            skipWhitespace();
            char c = get();
            if (c == ']') break;
            if (c != ',') throw ParseError("Expected ',' or ']'");
        }
        return v;
    }

    std::string parseStringValue() {
        get(); // 跳过 '"'
        std::string s;
        while (true) {
            char c = get();
            if (c == '"') break;
            if (c == '\\') {
                c = get();
                switch (c) {
                    case '"': s += '"'; break;
                    case '\\': s += '\\'; break;
                    case '/': s += '/'; break;
                    case 'b': s += '\b'; break;
                    case 'f': s += '\f'; break;
                    case 'n': s += '\n'; break;
                    case 'r': s += '\r'; break;
                    case 't': s += '\t'; break;
                    // TODO: unicode \uXXXX
                    default: s += c; break;
                }
            } else {
                s += c;
            }
        }
        return s;
    }

    Value parseString() {
        return Value(parseStringValue());
    }

    Value parseBoolean() {
        if (peek() == 't') {
            get(); get(); get(); get(); // true
            return Value(true);
        } else {
            get(); get(); get(); get(); get(); // false
            return Value(false);
        }
    }

    Value parseNull() {
        get(); get(); get(); get(); // null
        return Value();
    }

    Value parseNumber() {
        std::string numStr;
        if (peek() == '-') {
        numStr += get();
    }
    
    // Check if the next character is a digit
    if (!isdigit(peek())) {
        throw ParseError("Invalid number format");
    }

    while (isdigit(peek())) numStr += get();
    if (peek() == '.') {
        numStr += get();
        while (isdigit(peek())) numStr += get();
    }
    // TODO: 支持科学计数法 (e/E)
        
        return Value(std::stod(numStr));
    }
};

Value Value::parse(std::istream& is) {
    Parser p(is);
    return p.parse();
}

Value Value::parse(const std::string& s) {
    std::stringstream ss(s);
    return parse(ss);
}

std::istream& operator>>(std::istream& is, Value& v) {
    v = Value::parse(is);
    return is;
}

std::ostream& operator<<(std::ostream& os, const Value& v) {
    // 简单的输出实现
    if (v.is_null()) os << "null";
    else if (v.is_boolean()) os << (v.get<bool>() ? "true" : "false");
    else if (v.is_number()) os << v.get<double>();
    else if (v.is_string()) os << "\"" << v.get<std::string>() << "\"";
    else if (v.is_array()) {
        os << "[";
        size_t size = v.size();
        for (size_t i = 0; i < size; ++i) {
            os << v.at(i);
            if (i < size - 1) os << ",";
        }
        os << "]";
    } else if (v.is_object()) {
        os << "{";
        auto& obj = *std::get<std::shared_ptr<Value::ObjectType>>(v.data_);
        auto it = obj.begin();
        while (it != obj.end()) {
            os << "\"" << it->first << "\":" << it->second;
            if (++it != obj.end()) os << ",";
        }
        os << "}";
    }
    return os;
}

}
