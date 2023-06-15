#include "json_builder.h"
#include <iostream>

using namespace json;

KeyItemContext json::Builder::Key(std::string_view name)
{
    if( !CheckDict() || Key_.has_value()){
        throw std::logic_error("ОШИБКА С КЛЮЧОМ");
    }

    Key_ = std::string(name);

    // flag = true;


    return KeyItemContext(*this);

}

BaseItemContext json::Builder::Value(Node::Value value)
{   
    if(CheckArray()){
        std::get<Array>(nodes_stack_.back()->GetValue()).push_back(value);
    }
    else if(CheckDict()){
        if(!Key_.has_value()){
            throw std::logic_error("ашипка со словарем");
        }
        std::get<Dict>(nodes_stack_.back()->GetValue())[Key_.value()] = value;
    }
    else if(CheckNull()) {

        nodes_stack_.back()->GetValue() = std::move(value);
    }
    else{
        throw std::logic_error("ашипка с валью");
    }
    flag = true;

    Key_.reset();

    return BaseItemContext(*this);
}

DictItemContext json::Builder::StartDict()
{
    Dict temp;
    if(CheckArray()){
        std::get<Array>(nodes_stack_.back()->GetValue()).push_back(temp);

        nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->GetValue()).back());
    }
    else if(CheckDict()){
        if(!Key_.has_value()){
            throw std::logic_error("ошибка со словарем");
        }   
        nodes_stack_.back()->GetValue() = std::move(temp);
        nodes_stack_.push_back(nodes_stack_.back());
    }
    else if(CheckNull()){
        nodes_stack_.back()->GetValue() = std::move(temp);
        nodes_stack_.push_back(nodes_stack_.back());
    }
    else{
        throw std::logic_error("ошибка со словарем");
    }
    flag = true;
     Key_.reset();

    return DictItemContext(*this);
}

ArrayItemContext json::Builder::StartArray(){
    Array temp;
    if(Key_.has_value() && CheckDict()){
        std::get<Dict>(nodes_stack_.back()->GetValue())[Key_.value()] = temp;
        nodes_stack_.push_back(&std::get<Dict>(nodes_stack_.back()->GetValue()).at(Key_.value()));
    }
    else if(CheckArray()){
        
        std::get<Array>(nodes_stack_.back()->GetValue()).push_back(temp);
        nodes_stack_.push_back(nodes_stack_.back());
    }
    else if(CheckNull()){
        nodes_stack_.back()->GetValue() = std::move(temp);
        nodes_stack_.push_back(nodes_stack_.back());
    }
    else{
        throw std::logic_error("ошибка с массивом");
    }
    
    flag = true;
    Key_.reset();

    return ArrayItemContext(*this);
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
    if(nodes_stack_.size() != 1 || !flag ) {
        throw std::logic_error("JSON недостроен");
    }

    nodes_stack_.pop_back();

    return root;
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

