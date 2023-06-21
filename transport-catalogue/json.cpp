#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace ctlg{

namespace json {

class Node;
// Сохраните объявления Map и Array без изменения
using Map = std::map<std::string, Node>;
using Dict = Map;
using Array = std::vector<Node>;
using Value =  std::variant<std::nullptr_t, int, double, std::string, bool, Array, Map>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
   /* Реализуйте Node, используя std::variant */

   Node(){
    std::nullptr_t null;
    data_ = null;
   }
    
    // Node(Value value):data_(value){}

    template<typename T>
    Node(T value): data_(value){}
    
    
    
    bool IsInt() const;
    bool IsDouble() const; 
    bool IsPureDouble() const; 
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    bool IsDict() const{
        return IsMap();
    }
    
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Map& AsMap() const;

    Value GetValue() const;
    const Map& AsDict() const{
        return AsMap();
    }


    int size() const{
        return std::get<Array>(data_).size();
    }

    bool operator==(const Node& n) const ;

    bool operator!=(const Node& n) const;

private:

    Value data_;
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

}