#include "json_builder.h"
#include <iostream>

using namespace json;

KeyItemContext json::Builder::Key(std::string_view name)
{
    if( !CheckDict() || Key_.has_value()){
        throw std::logic_error("ОШИБКА С КЛЮЧОМ");
    }

    Key_ = std::string(name);

    return KeyItemContext(*this);
}

BaseItemContext json::Builder::Value(Node::Value value)
{   
    auto temp = BuildDictArray(value, false);

    return BaseItemContext(*temp);
}

DictItemContext json::Builder::StartDict()
{
    Dict temp;
    
    auto temp_dict = BuildDictArray(temp, true);

    return DictItemContext(*temp_dict);
}

ArrayItemContext json::Builder::StartArray(){
    Array temp;

    auto temp_array = BuildDictArray(temp, true);

    return ArrayItemContext(*temp_array);
}

BaseItemContext json::Builder::EndDict(){
    if( !CheckDict()){
        throw std::logic_error("ОШИБКА В КОНЦЕ СЛОВАРЯ");
    }
    nodes_stack_.pop_back();
    return BaseItemContext(*this);
}

BaseItemContext json::Builder::EndArray(){
    if( !CheckArray()){
        throw std::logic_error("ОШИБКА В КОНЦЕ МАССИВА");
    }
    nodes_stack_.pop_back();
    return BaseItemContext(*this);
}

json::Node json::Builder::Build()
{   
    if(nodes_stack_.size() != 1 || std::holds_alternative<std::nullptr_t>(root_.GetValue())) {
        throw std::logic_error("JSON недостроен");
    }

    nodes_stack_.pop_back();

    return root_;
}



BaseItemContext BaseItemContext::EndDict(){
    return build.EndDict();
}


KeyItemContext BaseItemContext::Key(std::string_view name){
    return build.Key(name);
} 

ArrayItemContext BaseItemContext::StartArray(){
    return build.StartArray();
}

DictItemContext BaseItemContext::StartDict(){
    return build.StartDict();
}


BaseItemContext BaseItemContext::EndArray(){
    return build.EndArray();
}

BaseItemContext BaseItemContext::Value(Node::Value value){
    return build.Value(value);
}

Node json::BaseItemContext::Build()
{
    return build.Build();
}

