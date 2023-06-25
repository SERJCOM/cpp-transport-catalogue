#pragma once

#include "json.h"
#include <optional>
#include <deque>



namespace json{

class Builder{
private:

    class BaseItemContext;
    class DictItemContext;
    class ArrayItemContext;
    class KeyItemContext;

public:

    Builder(){
        nodes_stack_.push_back(&root_);
    }

    KeyItemContext Key(std::string_view name);

    BaseItemContext Value(Node::Value value);

    DictItemContext StartDict() ;

    ArrayItemContext StartArray();

    BaseItemContext EndDict() ;

    BaseItemContext EndArray();

    Node Build(); 


private:
    
    bool CheckDict(){
        return nodes_stack_.back()->IsDict();
    }  
    
    bool CheckArray(){
        return nodes_stack_.back()->IsArray();
    }
    
    bool CheckNull(){
        return nodes_stack_.back()->IsNull();
    }

    template<typename T>
    Builder* BuildDictArray(T element, bool to_stack);


    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> key_;

    

};


template<typename T>
Builder* Builder::BuildDictArray(T element, bool write_stack){

    if(CheckArray()){
        std::get<Array>(nodes_stack_.back()->GetValue()).push_back(std::move(element));

        if(write_stack)
            nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->GetValue()).back());
    }
    else if(CheckDict()){
        if(!key_.has_value()){
            throw std::logic_error("ошибка со словарем");
        }   

        if(write_stack){
            nodes_stack_.back()->GetValue() = std::move(element);

            nodes_stack_.push_back(nodes_stack_.back());
        }
        else{
            std::get<Dict>(nodes_stack_.back()->GetValue())[key_.value()] = std::move(element);
        }
    }
    else if(CheckNull()){
        nodes_stack_.back()->GetValue() = std::move(element);
        if(write_stack)
            nodes_stack_.push_back(nodes_stack_.back());
    }
    else{
        throw std::logic_error("ошибка со словарем");
    }

    key_.reset();

    return this;
}



class Builder::BaseItemContext{
public:
    BaseItemContext(Builder& temp): build(temp){}

    BaseItemContext EndDict();

    KeyItemContext Key(std::string_view name);

    ArrayItemContext StartArray();

    DictItemContext StartDict();

    BaseItemContext EndArray();

    BaseItemContext Value(Node::Value value);

    Node Build();

    Builder& build;
};

class Builder::DictItemContext final: protected Builder::BaseItemContext{
public:
    DictItemContext(Builder& temp):BaseItemContext(temp){}


    using BaseItemContext::Key;
    using BaseItemContext::EndDict;
};



class Builder::ArrayItemContext final: protected Builder::BaseItemContext{
public:

    ArrayItemContext(Builder& temp): BaseItemContext(temp){}

    using BaseItemContext::StartArray;
    using BaseItemContext::StartDict;
    using BaseItemContext::EndArray;
    

    ArrayItemContext Value(Node::Value value){
        build.Value(value);

        return ArrayItemContext(build);
    }
};


class Builder::KeyItemContext final: protected Builder::BaseItemContext{
public:
    KeyItemContext(Builder& temp): BaseItemContext(temp){}   

    using BaseItemContext::StartArray;
    using BaseItemContext::StartDict;

    DictItemContext Value(Node::Value value){
        build.Value(std::move(value));
        return DictItemContext(build);
    }  
};

}
