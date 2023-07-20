#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>



namespace json {

class Node;
// Сохраните объявления Map и Array без изменения

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;


// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final
    : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:

   using variant::variant;
    using Value = variant;
    

    Node(Value value) : variant(std::move(value)) {}
    
    
    
    bool IsInt() const;
    bool IsDouble() const; 
    bool IsPureDouble() const; 
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsDict() const;
    
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsDict() const;

    const Value& GetValue() const{
        return *this;
    }

    Value& GetValue(){
        return *this;
    }

    

    int size() const{
        return std::get<Array>(*this).size();
    }

    bool operator==(const Node& n) const ;

    bool operator!=(const Node& n) const;


};

class Document {
public:

    Document() = default;
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& doc) const{
        return root_ == doc.root_;
    }

    bool operator!=(const Document& doc) const{
        return !(root_ == doc.root_);
    }

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json

