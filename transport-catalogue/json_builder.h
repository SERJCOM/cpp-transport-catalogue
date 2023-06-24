#pragma once

#include "json.h"
#include <optional>
#include <deque>



namespace json{

class BaseItemContext;
class DictItemContext;
class ArrayItemContext;
class KeyItemContext;


class Builder{
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
        return std::holds_alternative<Dict>(nodes_stack_.back()->GetValue());
    }  
    
    bool CheckArray(){
        return std::holds_alternative<Array>(nodes_stack_.back()->GetValue());
    }
    
    bool CheckNull(){
        return std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue());
    }

    template<typename T>
    Builder* BuildDictArray(T element, bool to_stack);


    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> Key_;


};


template<typename T>
Builder* Builder::BuildDictArray(T element, bool to_stack){

    if(Key_.has_value() && CheckDict()){
        std::get<Dict>(nodes_stack_.back()->GetValue())[Key_.value()] = std::move(element);
        
        if(to_stack)
            nodes_stack_.push_back(&std::get<Dict>(nodes_stack_.back()->GetValue()).at(Key_.value()));
    }
    else if(CheckArray()){
        
        std::get<Array>(nodes_stack_.back()->GetValue()).push_back(std::move(element));
        if(to_stack)
            nodes_stack_.push_back(nodes_stack_.back());
    }
    else if(CheckNull()){
        nodes_stack_.back()->GetValue() = std::move(element);
        if(to_stack)
            nodes_stack_.push_back(nodes_stack_.back());
    }
    else{
        throw std::logic_error("ошибка с массивом или словарем");
    }
    
    Key_.reset();

    return this;
}



class BaseItemContext{
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

class DictItemContext final: protected BaseItemContext{
public:
    DictItemContext(Builder& temp):BaseItemContext(temp){}


    using BaseItemContext::Key;
    using BaseItemContext::EndDict;
};



class ArrayItemContext final: protected BaseItemContext{
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


class KeyItemContext final: protected BaseItemContext{
public:
    KeyItemContext(Builder& temp): BaseItemContext(temp){}   

    using BaseItemContext::StartArray;
    using BaseItemContext::StartDict;

    DictItemContext Value(Node::Value value){
        build.Value(value);
        return DictItemContext(build);
    }  
};

}
