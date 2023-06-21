#include "json.h"


namespace ctlg{
namespace json {

namespace {

void DeleteSpace(std::istream& input){
    char c;
    input >> c;
    if(c != ' '){
        input.putback(c);
    }
    else{
        while(c == ' '){
            input >> c;
        }
    }
}

std::string reserved_words = " {}[],";

Node LoadNode(std::istream& input);

Node LoadArray(std::istream& input) {
    Array result;
    DeleteSpace(input);

    bool correct = false;
    char c;

    for ( ; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    
    if(c == ']'){
        correct = true;
    }
    if(!correct){
        throw json::ParsingError("AshiPka"); 
    }

    Node tempNode(std::move(result));
    return tempNode;
}

Node LoadIntDouble(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadMap(std::istream& input) {
    Map result;

    DeleteSpace(input);
    bool correct = false;
    char c;

    for (; input >> c;) {
        if(c == '}'){
            break;
        }
        if (c == ',' || c == ' ') {
            input >> c;
        }
        std::string key = LoadString(input).AsString();
        input >> c;
        result.insert({std::move(key), LoadNode(input)});
    }

    if(c == '}'){
        correct = true;
    }
    if(!correct){
        throw json::ParsingError("AshiPka"); 
    }
    return Node(std::move(result));
}

Node LoadNull(std::istream& input){
    Node temp;

    std::string temp_str;
    char c;
    while(input >> c){
        temp_str.push_back(c);
        if(reserved_words.find(c) != std::string::npos){
            break;
        }
        if(temp_str.size() == 4){ // 4 - длина null
            break;
        }
    }

    if(temp_str != "null"){
        throw json::ParsingError( std::to_string(temp_str.size()) + temp_str);
    }
    return temp;
}

Node LoadBool(std::istream& input){
    std::string temp_str;

    char c;
    while(true){
        if(temp_str == "true" || temp_str == "false"){
            break;
        }
        input >> c;
        temp_str.push_back(c);
        if(reserved_words.find(c) != std::string::npos){
            break;
        }
        if(temp_str.size() == 5){
            break;
        }
    }
    if(temp_str == "true"){
        return Node(true);
    }
    else if(temp_str == "false"){
        return Node(false);
    }

    throw json::ParsingError("ошибка " + temp_str );
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;
    while(c == ' '){
        input >> c;
    }

    if (c == '[') {             // array
        return LoadArray(input);    
    } else if (c == '{') {          // Map
        return LoadMap(input);
    } else if (c == '"') {          // string
        return LoadString(input);
    } else if(c == 'n'){            // null
        input.putback(c);
        return LoadNull(input);
    } else if(c == 't' || c == 'f') {// bool
        input.putback(c);
        return LoadBool(input);
    }
    else {                          // int or double 
        input.putback(c);
        return LoadIntDouble(input);
    }
}

}  // namespace




const Array& Node::AsArray() const {
    if(!std::holds_alternative<Array>(data_)){
        throw std::logic_error("что-то идет не по плану");
    }
    return std::get<Array>(data_);
}

const Map& Node::AsMap() const {
    if(!std::holds_alternative<Map>(data_)){
        throw std::logic_error("что-то идет не по плану");
    }
    return std::get<Map>(data_);
}

Value Node::GetValue() const
{
    return data_;
}

bool Node::operator==(const Node &n) const
{
    return data_ == n.data_;
}

bool Node::operator!=(const Node &n) const
{
    return !(*this == n);
}

int Node::AsInt() const {
    if(!std::holds_alternative<int>(data_)){
        throw std::logic_error("что-то идет не по плану");
    }
    return std::get<int>(data_);
}

bool Node::AsBool() const
{
    if(!std::holds_alternative<bool>(data_)){
        throw std::logic_error("что-то идет не по плану");
    }

return std::get<bool>(data_);
}

double Node::AsDouble() const
{
    if(std::holds_alternative<double>(data_)){
        return std::get<double>(data_);
    }
    if(std::holds_alternative<int>(data_)){
        return std::get<int>(data_);
    }
    throw std::logic_error("что-то идет не по плану");
}

const std::string& Node::AsString() const {
    if(!std::holds_alternative<std::string>(data_)){
        throw std::logic_error("что-то идет не по плану");
    }
    return std::get<std::string>(data_);
}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void PrintValue(bool value,  std::ostream& out){
    if(value){
        out << "true";
    }
    else{
        out << "false";
    }
}

void PrintValue(const Map& value, std::ostream& out);

void PrintValue(int value,  std::ostream& out){
    out << value;
}

void PrintValue(double value,  std::ostream& out){
    out << value;
}

void PrintValue(const std::string& value,  std::ostream& out){
    out << "\"";  
    
    std::string copy = value;

    for(char i  : value){
        if(i == '\"'){
            out << "\\\"";
        }
        else if(i == '\r'){
            out << "\\r";
        }  else if( i == '\n'){
            out << "\\n";
        } else if( i == '\\'){
            out << "\\\\";
        } else if( i == '\t'){
            out << "\t";
        }
    
        else{out << i;}
    }
    
    out << "\"";
}

void PrintValue(std::nullptr_t,  std::ostream& out){
    out << "null";
}

void PrintValue(const Array& value,  std::ostream& out){
    out << "[ ";
    for(size_t i = 0; i < value.size(); i++){
        std::visit([&out](const auto& value){
        PrintValue(value, out);
        }, value[i].GetValue());

        if(i != value.size() - 1){
            out << ", ";
        }
    }
    out << " ]";
}


void PrintValue(const Map& value, std::ostream& out){
    out << "{ ";
    for(auto i = value.begin(); i != value.end(); i++){

        if(i != value.begin()){
            out << ", ";
        }

        out << "\"" << i->first << "\": ";
        std::visit([&out](const auto& value){
        PrintValue(value, out);
        }, i->second.GetValue());

    }
    out << " }";
}


void Print(const Document& doc, std::ostream& output) {
    auto node = doc.GetRoot();
    std::visit([&output](const auto& value){
        PrintValue(value, output);
    }, node.GetValue());
}



bool Node::IsInt() const
{
    return std::holds_alternative<int>(data_);
}

bool Node::IsDouble() const
{
    return IsPureDouble() || IsInt();
}

bool Node::IsPureDouble() const
{
    return std::holds_alternative<double>(data_);
}

bool Node::IsBool() const
{
    return std::holds_alternative<bool>(data_);
}

bool Node::IsString() const
{
    return std::holds_alternative<std::string>(data_);
}

bool Node::IsNull() const
{
    return std::holds_alternative<std::nullptr_t>(data_);
}

bool Node::IsArray() const
{
    return std::holds_alternative<Array>(data_);
}

bool Node::IsMap() const
{
    return std::holds_alternative<Map>(data_);
}

} 

}