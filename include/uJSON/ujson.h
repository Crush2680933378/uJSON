#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <iostream>
#include <memory>
#include <stdexcept>

#if defined(_WIN32) && !defined(UJSON_STATIC)
  #ifdef UJSON_EXPORT
    #define UJSON_API __declspec(dllexport)
  #else
    #define UJSON_API __declspec(dllimport)
  #endif
#else
  #define UJSON_API
#endif

namespace uJSON {

// 异常基类
class UJSON_API Exception : public std::exception {
public:
    explicit Exception(const std::string& msg) : msg_(msg) {}
    const char* what() const noexcept override { return msg_.c_str(); }
protected:
    std::string msg_;
};

// 解析错误
class UJSON_API ParseError : public Exception {
public:
    using Exception::Exception;
};

// 类型错误
class UJSON_API TypeError : public Exception {
public:
    using Exception::Exception;
};

// 其他运行时错误
class UJSON_API RuntimeError : public Exception {
public:
    using Exception::Exception;
};

enum class Type {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

class UJSON_API Value {
public:
    using ObjectType = std::map<std::string, Value>;
    using ArrayType = std::vector<Value>;

    Value();
    Value(std::nullptr_t);
    Value(bool b);
    Value(int i);
    Value(double d);
    Value(const char* s);
    Value(const std::string& s);
    
    // 显式构造函数
    static Value object();
    static Value array();

    Type type() const;

    bool is_null() const;
    bool is_boolean() const;
    bool is_number() const;
    bool is_number_integer() const;
    bool is_string() const;
    bool is_array() const;
    bool is_object() const;

    bool contains(const std::string& key) const;
    const Value& at(const std::string& key) const;
    Value& operator[](const std::string& key);
    Value& operator[](const char* key); 
    
    // 数组操作
    void push_back(const Value& v);
    const Value& at(size_t index) const;
    Value& operator[](size_t index);
    Value& operator[](int index); // 解决字面量 0 的歧义
    const Value& operator[](size_t index) const;
    const Value& operator[](int index) const; // 解决字面量 0 的歧义
    size_t size() const;

    template<typename T>
    T get() const;

    // 迭代器支持 (模仿 STL/nlohmann::json)
    // 简单实现：对于 Object 和 Array 转发到内部容器，对于其他类型抛出异常或返回空迭代器
    // 注意：由于 Value 内部使用 shared_ptr，我们需要自定义迭代器或者直接暴露内部容器的迭代器
    // 为了简单，我们暂时只提供 Object 的迭代器接口，或者使用 Value::ObjectType 的迭代器
    
    // 容器访问器
    ObjectType& get_object();
    const ObjectType& get_object() const;
    ArrayType& get_array();
    const ArrayType& get_array() const;

    // 迭代器支持
    ObjectType::iterator begin();
    ObjectType::const_iterator begin() const;
    ObjectType::iterator end();
    ObjectType::const_iterator end() const;

    static Value parse(std::istream& is);
    static Value parse(const std::string& s);

    friend std::istream& operator>>(std::istream& is, Value& v);
    friend std::ostream& operator<<(std::ostream& os, const Value& v);

private:
    Type type_ = Type::Null;
    // 使用指针处理递归类型以简化 variant 的使用
    using DataVariant = std::variant<std::monostate, bool, double, std::string, std::shared_ptr<ArrayType>, std::shared_ptr<ObjectType>>;
    DataVariant data_;
    
    friend class Parser;
    
    void set_object(ObjectType* obj);
    void set_array(ArrayType* arr);
};

// Specializations
template<> bool Value::get<bool>() const;
template<> int Value::get<int>() const;
template<> double Value::get<double>() const;
template<> std::string Value::get<std::string>() const;

}
