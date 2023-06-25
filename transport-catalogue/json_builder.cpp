#include "json_builder.h"
#include <iostream>

using namespace json;


Builder::KeyItemContext json::Builder::Key(std::string_view name)
{
    if( !CheckDict() || key_.has_value()){
        throw std::logic_error("ОШИБКА С КЛЮЧОМ");
    }

    key_ = std::string(name);

    return KeyItemContext(*this);
}

Builder::BaseItemContext json::Builder::Value(Node::Value value)
{   
    return BaseItemContext(*BuildDictArray(std::move(value), false));
}

Builder::DictItemContext json::Builder::StartDict()
{
    return DictItemContext(*BuildDictArray(Dict{}, true));
}

Builder::ArrayItemContext json::Builder::StartArray()
{
    return ArrayItemContext(*BuildDictArray(Array{}, true));
}

Builder::BaseItemContext json::Builder::EndDict(){
    if( !CheckDict()){
        throw std::logic_error("ОШИБКА В КОНЦЕ СЛОВАРЯ");
    }
    nodes_stack_.pop_back();
    return BaseItemContext(*this);
}


Builder::BaseItemContext json::Builder::EndArray(){
    if( !CheckArray()){
        throw std::logic_error("ОШИБКА В КОНЦЕ МАССИВА");
    }
    nodes_stack_.pop_back();
    return BaseItemContext(*this);
}

json::Node json::Builder::Build()
{   
    if(nodes_stack_.size() != 1 || root_.IsNull() ) {
        throw std::logic_error("JSON недостроен");
    }

    nodes_stack_.pop_back();

    return root_;
}

Builder::BaseItemContext Builder::BaseItemContext::EndDict(){
    return build.EndDict();
}


Builder::KeyItemContext Builder::BaseItemContext::Key(std::string_view name){
    return build.Key(name);
} 

Builder::ArrayItemContext Builder::BaseItemContext::StartArray(){
    return build.StartArray();
}

Builder::DictItemContext Builder::BaseItemContext::StartDict(){
    return build.StartDict();
}


Builder::BaseItemContext Builder::BaseItemContext::EndArray(){
    return build.EndArray();
}

Builder::BaseItemContext Builder::BaseItemContext::Value(Node::Value value){
    return build.Value(std::move(value));
}

Node json::Builder::BaseItemContext::Build()
{
    return build.Build();
}

