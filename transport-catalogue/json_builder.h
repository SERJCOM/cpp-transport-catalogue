#pragma once

#include "json.h"
#include <optional>
#include <deque>

namespace json{

class BaseItemContext;
class DictItemContext;
class ArrayItemContext;
class KeyItemContext;
class ValueItemContext;
class ValueKeyItemContext;

class Builder{
public:

    Builder(){
        nodes_stack_.push_back(&root);
    }

    Builder(const Builder& builder){
        flag = builder.flag;
        root = builder.root;
        nodes_stack_ = builder.nodes_stack_;
        Key_ = builder.Key_;
    }

    Builder& operator=(Builder builder){
        flag = builder.flag;
        root = std::move(builder.root);
        nodes_stack_ = std::move(builder.nodes_stack_);
        Key_ = std::move(builder.Key_);

        return *this;
    }

    KeyItemContext Key(std::string_view name);

    BaseItemContext Value(Node::Value value);

    DictItemContext StartDict() ;

    ArrayItemContext StartArray();

    BaseItemContext EndDict() ;

    BaseItemContext EndArray();

    json::Node Build(); 


private:
    bool flag = false;

    bool CheckDict(){
        return std::holds_alternative<Dict>(nodes_stack_.back()->GetValue());
    }  

    bool CheckArray(){
        return std::holds_alternative<Array>(nodes_stack_.back()->GetValue());
    }

    bool CheckNull(){
        return std::holds_alternative<std::nullptr_t>(nodes_stack_.back()->GetValue());
    }

    Node root;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> Key_;

};






class BaseItemContext{
public:
    BaseItemContext(Builder& temp): build(temp){}

    BaseItemContext EndDict();

    KeyItemContext Key(std::string_view name);

    ArrayItemContext StartArray();

    DictItemContext StartDict();

    BaseItemContext EndArray();

    BaseItemContext Value(Node::Value value);

    //ValueKeyItemContext Value(Node::Value value);
    
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