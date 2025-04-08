#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "utils.hpp"

enum TokenType {
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    FLOAT_NUMBER,
    STRING_LITERAL,
    CHAR_LITERAL,
    OPERATOR,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;

    std::string toString(){
        if (type == KEYWORD) return "KEYWORD";
            else if (type == IDENTIFIER) return "IDENTIFIER";
            else if (type == NUMBER) return "NUMBER";
            else if (type == FLOAT_NUMBER) return "FLOAT_NUMBER";
            else if (type == STRING_LITERAL) return "STRING_LITERAL";
            else if (type == CHAR_LITERAL) return "CHAR_LITERAL";
            else if (type == OPERATOR) return "OPERATOR";
            else return "ERROR";
    }
};

// struct ASTNode {
//     std::string type;
//     std::string value;
//     std::vector<ASTNode*> children;

//     ASTNode(std::string t, std::string v = "") : type(t), value(v) {}

//     void addChild(ASTNode* child) {
//         children.push_back(child);
//     }

//     void print(const std::string& prefix = "", bool isLast = true) {
//         std::cout << prefix;

//         std::cout << (isLast ? "└── " : "├── ");
//         std::cout << type << (value.empty() ? "" : ": " + value) << "\n";

//         for (size_t i = 0; i < children.size(); ++i) {
//             children[i]->print(prefix + (isLast ? "    " : "│   "), i == children.size() - 1);
//         }
//     }

//     ~ASTNode() {
//         for (ASTNode* child : children) {
//             delete child;
//         }
//     }
// };

// Функция для чтения токенов из файла
std::vector<Token> readTokensFromFile(const std::string& filename) {
    std::vector<Token> tokens;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Ошибка открытия файла: " << filename << std::endl;
        return tokens;
    }

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue; // Пропускаем пустые строки
        }

        std::istringstream iss(line);
        std::string tokenTypeStr, lexeme, lineStr, idStr;

        std::getline(iss, tokenTypeStr, ' '); 
        std::getline(iss, tokenTypeStr, ' '); 
        std::getline(iss, lexeme, '@'); 
        std::getline(iss, lexeme, '@'); 

        std::getline(iss, lineStr, ' ');
        std::getline(iss, lineStr, ' ');
        std::getline(iss, lineStr, ' ');
        // Извлечение номера строки
        int lineNum = 0;
        try {
            lineNum = std::stoi(lineStr);
        } catch (const std::invalid_argument&) {
            std::cerr << "Ошибка преобразования номера строки: " << lineStr << std::endl;
            continue; // Пропускаем токен при ошибке
        } catch (const std::out_of_range&) {
            std::cerr << "Номер строки вне допустимого диапазона: " << lineStr << std::endl;
            continue; // Пропускаем токен при ошибке
        }
        // Определяем тип токена
        TokenType type;
        if (tokenTypeStr == "KEYWORD") type = KEYWORD;
        else if (tokenTypeStr == "IDENTIFIER") type = IDENTIFIER;
        else if (tokenTypeStr == "NUMBER") type = NUMBER;
        else if (tokenTypeStr == "FLOAT_NUMBER") type = FLOAT_NUMBER;
        else if (tokenTypeStr == "STRING_LITERAL") type = STRING_LITERAL;
        else if (tokenTypeStr == "CHAR_LITERAL") type = CHAR_LITERAL;
        else if (tokenTypeStr == "OPERATOR") type = OPERATOR;
        else type = ERROR;

        tokens.push_back({type, lexeme, lineNum});
    }

    file.close();
    return tokens;
}

class ParseException : public std::exception {
    public:
        ParseException(const std::string& message, int line)
            : msg_(message), line_(line) {}
    
        virtual const char* what() const noexcept override {
            return msg_.c_str();
        }
    
        int getLine() const {
            return line_;
        }
    
    private:
        std::string msg_;
        int line_;
    };

class Parser {
    private:
        std::vector<Token> tokens;
        int current;
    
        Token peek() {
            return tokens[current];
        }
    
        Token consume() {
            return tokens[current++];
        }
    
        bool match(TokenType expected, std::string lexeme = "") {
            if (current < tokens.size() && tokens[current].type == expected) {
                if (lexeme.empty() || tokens[current].lexeme == lexeme) {
                    consume();
                    return true;
                }
            }
            return false;
        }
    
    public:
        Parser(std::vector<Token> t) : tokens(t), current(0) {}
    
        ASTNode* parseProgram() {
            ASTNode* root = new ASTNode(ASTNode::PROGRAM, tokens[current].line);
            // ASTNode* root = new ASTNode("Program");
            while(tokens[current].lexeme != "public"){
                current++;
            }
            while (current < tokens.size()) {
                root->addChild(parseClassDeclaration());
            }
            return root;
        }
        
        // ASTNode* parseBlock(int line) {
        //     ASTNode* block = new ASTNode(ASTNode::BLOCK, line);
        //     match(OPERATOR, "{");
            
        //     while (!match(OPERATOR, "}")) {
        //         block->addChild(parseStatement());
        //     }
            
        //     return block;
        // }

        ASTNode* parseClassDeclaration() {
            match(KEYWORD, "public");
            match(KEYWORD, "class");
            Token className = consume(); // IDENTIFIER
            match(OPERATOR, "{");
    
            ASTNode* classNode = new ASTNode(ASTNode::CLASS_DECL, className.line);
            classNode->setAttribute("name", className.lexeme);
            ASTNode* block = new ASTNode(ASTNode::BLOCK, tokens[current - 1].line);

            // ASTNode* classNode = new ASTNode("ClassDeclaration", className.lexeme);
            // classNode->addChild(new ASTNode("BlockStart", "{")); // Добавляем открывающую скобку

            while (!match(OPERATOR, "}")) {
                block->addChild(parseClassMember());
                // classNode->addChild(parseClassMember());
            }
            classNode->addChild(block); 
            
            return classNode;
        }
    
        ASTNode* parseClassMember() {
            if (match(KEYWORD, "public")) {
                if (match(KEYWORD, "static")) {
                    return parseMethodDeclaration();
                }
            }
            return parseVariableDeclaration();
        }
    
        ASTNode* parseParameterList() {
            ASTNode* paramList = new ASTNode(ASTNode::PARAMETER_LIST, tokens[current].line);
            paramList->setAttribute("type", "parameters");
            // ASTNode* paramList = new ASTNode("ParameterList");
            // paramList->addChild(new ASTNode("ParenthesisStart", "(")); // Добавляем открывающую скобку
            
            do {
                if(peek().lexeme == "ArrayList"){
                    ASTNode* paramNode = parseArrayList();
                    // ASTNode* paramNode = new ASTNode(ASTNode::PARAMETER, array->getLine());
                    // paramNode->setAttribute("type", type);
                    // paramNode->setAttribute("name", paramName.lexeme);
                    // ASTNode* paramNode = new ASTNode("Parameter", array->value);
                    // paramNode->addChild(new ASTNode("Type", "ArrayList<" + array->children[0]->value + ">"));
                    paramList->addChild(paramNode);
                }
                else if(peek().lexeme == "HashMap"){
                    ASTNode* map = parseHashMap();
                    paramList->addChild(map);
                } else {
                    std::string type = consume().lexeme;
                    Token paramName = consume();
                    ASTNode* paramNode = new ASTNode(ASTNode::PARAMETER, paramName.line);
                    paramNode->setAttribute("type", type);
                    paramNode->setAttribute("name", paramName.lexeme);
                    // ASTNode* paramNode = new ASTNode("Parameter", paramName.lexeme);
                    // paramNode->addChild(new ASTNode("Type", type));
                    paramList->addChild(paramNode);
                }
            } while (match(OPERATOR, ","));
            // paramList->addChild(new ASTNode("ParenthesisEnd", ")")); // Добавляем открывающую скобку

            return paramList;
        }

        ASTNode* parseMethodDeclaration() {
            Token returnType = consume();
            Token methodName = consume(); // IDENTIFIER
            match(OPERATOR, "(");
            
            ASTNode* methodNode = new ASTNode(ASTNode::METHOD_DECL, methodName.line);
            methodNode->setAttribute("returnType", returnType.lexeme);
            methodNode->setAttribute("name", methodName.lexeme);
            // ASTNode* block = new ASTNode(ASTNode::BLOCK, tokens[current - 1].line);
            // ASTNode* methodNode = new ASTNode("MethodDeclaration", methodName.lexeme);
            if (!match(OPERATOR, ")")) {
                methodNode->addChild(parseParameterList());
                match(OPERATOR, ")");
            }
            match(OPERATOR, "{");
            ASTNode* block = new ASTNode(ASTNode::BLOCK, tokens[current - 1].line);
            // methodNode->addChild(new ASTNode("BlockStart", "{")); // Добавляем открывающую скобку
            // match(OPERATOR, "{");
    
            while (!match(OPERATOR, "}")) {
                block->addChild(parseStatement());
                // methodNode->addChild(parseStatement());
            }
            methodNode->addChild(block);
            return methodNode;
        }
    

        ASTNode* parseVariableDeclaration() {
            // ASTNode* varNode = nullptr;
            // if(peek().lexeme == "ArrayList"){
            //     ASTNode* array = parseArrayList();
            //     varNode = new ASTNode("VariableDeclaration", array->value);
            //     varNode->addChild(new ASTNode("Type", "ArrayList<" + array->children[0]->value + ">"));
            //     if (match(OPERATOR, "=")) {
            //         varNode->addChild(parseExpression());
            //     }
            //     match(OPERATOR, ";");
            // } else if(peek().lexeme == "HashMap"){

            // } else{
                std::string type = consume().lexeme;
                Token varName = consume(); // IDENTIFIER

                ASTNode* varNode = new ASTNode(ASTNode::VARIABLE_DECL, varName.line);
                varNode->setAttribute("type", type);
                varNode->setAttribute("name", varName.lexeme);
                // ASTNode* varNode = new ASTNode("VariableDeclaration", varName.lexeme);
                // varNode->addChild(new ASTNode("Type", type));
                if (match(OPERATOR, "=")) {
                    if(peek().lexeme == "{"){
                        varNode->addChild(parseArrayInitializer());
                    } else{
                        // Обработать вызов функции
                        if(tokens[current + 1].lexeme == "." && tokens[current + 3].lexeme == "(" && (tokens[current + 4].lexeme == ")" || tokens[current + 5].lexeme == ")")){
                            varNode->addChild(parseMethodCall());
                        } else if(tokens[current + 1].lexeme == "("){
                            varNode->addChild(parseFunctionCall());
                        } else{
                            varNode->addChild(parseExpression());
                        }
                    }
                }
                match(OPERATOR, ";");
                // varNode->addChild(new ASTNode("Semicolon", ";"));
            // }
            return varNode;
        }
        
        ASTNode* parseVariableAssigment() {
                Token varName = consume(); 
                ASTNode* varNode = new ASTNode(ASTNode::ASSIGNMENT, varName.line);

                if(varName.lexeme.find("[") != std::string::npos){
                    ASTNode* node = new ASTNode(ASTNode::ARRAY_ACCESS, varName.line);
                    ASTNode* var = new ASTNode(ASTNode::VARIABLE, varName.line);
                    var->setAttribute("name", varName.lexeme.substr(0, varName.lexeme.find("[")));
                    size_t start = varName.lexeme.find("[") + 1;
                    size_t end = varName.lexeme.find("]");
                    size_t length = end - start;
                    std::string index = varName.lexeme.substr(start, length);
                    node->addChild(var);
                    try {
                        int intIndex = std::stoi(index); 
                        ASTNode* i = new ASTNode(ASTNode::LITERAL, varName.line);
                        i->setAttribute("literalType", "int");
                        i->setAttribute("name", index);
                        node->addChild(i);
                    } catch (const std::invalid_argument& e) {
                        ASTNode* i = new ASTNode(ASTNode::VARIABLE, varName.line);
                        i->setAttribute("name", index);
                        node->addChild(i);
                    } 
                    varNode->addChild(node);
                } else{
                    ASTNode* left = new ASTNode(ASTNode::VARIABLE, varName.line);
                    left->setAttribute("name", varName.lexeme);
                    varNode->addChild(left);
                }
                if (match(OPERATOR, "=")) {
                    if(peek().lexeme == "{"){
                        varNode->addChild(parseArrayInitializer());
                    } else{
                        varNode->addChild(parseExpression());
                    }
                }
                match(OPERATOR, ";");
            return varNode;
        }

        ASTNode* parseArrayInitializer() {
            ASTNode* arrayNode = new ASTNode(ASTNode::ARRAY_INIT, consume().line);
            match(OPERATOR, "{");
            
            while (!match(OPERATOR, "}")) {
                arrayNode->addChild(parseExpression());
                if (peek().lexeme == ",") consume();
            }
            
            return arrayNode;
        }

        ASTNode* parseArrayList() {
            match(KEYWORD, "ArrayList");
            match(OPERATOR, "<");
            Token type = consume();
            match(OPERATOR, ">");
            Token varName = consume();
            
            ASTNode* arrayListNode = new ASTNode(ASTNode::PARAMETER, varName.line);
            arrayListNode->setAttribute("type", "ArrayList<" + type.lexeme + ">");
            arrayListNode->setAttribute("name", varName.lexeme);
            // ASTNode* arrayListNode = new ASTNode("ArrayList", varName.lexeme);
            // arrayListNode->addChild(new ASTNode("Type", type.lexeme));
    
            return arrayListNode;
        }

        ASTNode* parseHashMap(){
            match(KEYWORD, "HashMap");
            match(OPERATOR, "<");
            Token type1 = consume();
            match(OPERATOR, ",");
            Token type2 = consume();
            match(OPERATOR, ">");
            Token varName = consume();
            ASTNode* hashMapNode = new ASTNode(ASTNode::PARAMETER, varName.line);
            hashMapNode->setAttribute("type", "HashMap<" + type1.lexeme + ", " + type2.lexeme + ">");
            hashMapNode->setAttribute("name", varName.lexeme);
            // ASTNode* hashMapNode = new ASTNode("HashMap", varName.lexeme);
            // hashMapNode->addChild(new ASTNode("KeyType", type1.lexeme));
            // hashMapNode->addChild(new ASTNode("ValueType", type2.lexeme));
    
            return hashMapNode;
        }
    
        ASTNode* parseVariableArrayList(){
            // ASTNode* array = parseArrayList();
            match(KEYWORD, "ArrayList");
            match(OPERATOR, "<");
            Token type = consume();
            match(OPERATOR, ">");
            Token varName = consume();
            
            ASTNode* arrayListNode = new ASTNode(ASTNode::VARIABLE_DECL, varName.line);
            arrayListNode->setAttribute("type", "ArrayList<" + type.lexeme + ">");
            arrayListNode->setAttribute("name", varName.lexeme);
           
            // ASTNode* varNode = new ASTNode("VariableDeclaration", array->value);
            // varNode->addChild(new ASTNode("Type", "ArrayList<" + array->children[0]->value + ">"));
            if (match(OPERATOR, "=")) {
                match(KEYWORD, "new");
                match(KEYWORD, "ArrayList");
                match(OPERATOR, "<");
                match(OPERATOR, ">");
                match(OPERATOR, "(");
                match(OPERATOR, ")");
                // varNode->addChild(parseExpression());
            }
            match(OPERATOR, ";");
            // varNode->addChild(new ASTNode("Semicolon", ";"));
            return arrayListNode;
        }

        ASTNode* parseVariableHashMap(){
            // ASTNode* array = parseHashMap();
            match(KEYWORD, "HashMap");
            match(OPERATOR, "<");
            Token type1 = consume();
            match(OPERATOR, ",");
            Token type2 = consume();
            match(OPERATOR, ">");
            Token varName = consume();
            ASTNode* hashMapNode = new ASTNode(ASTNode::VARIABLE_DECL, varName.line);
            hashMapNode->setAttribute("type", "HashMap<" + type1.lexeme + "," + type2.lexeme + ">");
            hashMapNode->setAttribute("name", varName.lexeme);
            // ASTNode* varNode = new ASTNode("VariableDeclaration", array->value);
            // varNode->addChild(new ASTNode("Type", "HashMap<" + array->children[0]->value + ", " + array->children[1]->value + ">"));
            if (match(OPERATOR, "=")) {
                match(KEYWORD, "new");
                match(KEYWORD, "HashMap");
                match(OPERATOR, "<");
                match(OPERATOR, ">");
                match(OPERATOR, "(");
                match(OPERATOR, ")");
                // varNode->addChild(parseExpression());
            }
            match(OPERATOR, ";");
            // varNode->addChild(new ASTNode("Semicolon", ";"));
            return hashMapNode;
        }

        // ASTNode* parseHashMap() {
        //     match(IDENTIFIER, "HashMap");
        //     match(OPERATOR, "<");
        //     Token keyType = consume();
        //     match(OPERATOR, ",");
        //     Token valueType = consume();
        //     match(OPERATOR, ">");
        //     Token varName = consume();
    
        //     ASTNode* hashMapNode = new ASTNode("HashMap", varName.lexeme);
        //     hashMapNode->addChild(new ASTNode("KeyType", keyType.lexeme));
        //     hashMapNode->addChild(new ASTNode("ValueType", valueType.lexeme));
    
        //     return hashMapNode;
        // }

        ASTNode* parseIfStatement() {
            match(KEYWORD, "if");
            match(OPERATOR, "(");
            ASTNode* ifNode = new ASTNode(ASTNode::IF_STMT, tokens[current - 1].line);
            // ASTNode* condition = new ASTNode("Condition", "");
            ifNode->addChild(parseCondition());
            match(OPERATOR, ")");
            
            match(OPERATOR, "{");
            ASTNode* thenBlock = new ASTNode(ASTNode::BLOCK, tokens[current - 1].line);
            // ASTNode* ifBlock = new ASTNode("IfStatement");
            // ifBlock->addChild(new ASTNode("ParenthesisStart", "(")); // Добавляем открывающую скобку
            // ifBlock->addChild(condition);
            // ifBlock->addChild(new ASTNode("ParenthesisEnd", ")")); // Добавляем открывающую скобку
            // ifBlock->addChild(new ASTNode("BlockStart", "{"));
            while (!match(OPERATOR, "}")) {
                thenBlock->addChild(parseStatement());
            }
            ifNode->addChild(thenBlock);
            // ifBlock->addChild(new ASTNode("BlockEnd", "}"));
            if (match(KEYWORD, "else")) {
                match(OPERATOR, "{");
                ASTNode* elseBlock = new ASTNode(ASTNode::BLOCK, tokens[current - 1].line);
                // elseBlock->addChild(new ASTNode("BlockStart", "{"));
                while (!match(OPERATOR, "}")) {
                    elseBlock->addChild(parseStatement());
                }
                // elseBlock->addChild(new ASTNode("BlockEnd", "}"));
                ifNode->addChild(elseBlock);
            }
    
            return ifNode;
        }
    
        ASTNode* parseWhileLoop() {
            match(KEYWORD, "while");
            match(OPERATOR, "(");
            ASTNode* whileNode = new ASTNode(ASTNode::WHILE_STMT, tokens[current - 1].line);
            // ASTNode* condition = new ASTNode("Condition", "");
            whileNode->addChild(parseCondition());
            // ASTNode* condition = new ASTNode("Condition", "");
            // condition->addChild(parseCondition());
            match(OPERATOR, ")");
            
            match(OPERATOR, "{");
            ASTNode* block = new ASTNode(ASTNode::BLOCK, tokens[current - 1].line);
            
            // ASTNode* whileNode = new ASTNode("WhileLoop");
            // whileNode->addChild(new ASTNode("ParenthesisStart", "(")); // Добавляем открывающую скобку
            // whileNode->addChild(condition);
            // whileNode->addChild(new ASTNode("ParenthesisEnd", ")")); // Добавляем открывающую скобку

            // whileNode->addChild(new ASTNode("BlockStart", "{"));
            while (!match(OPERATOR, "}")) {
                block->addChild(parseStatement());
            }
            whileNode->addChild(block);
            
            return whileNode;
        }
    
        ASTNode* parseDoWhileLoop() {
            match(KEYWORD, "do");
            match(OPERATOR, "{");
    
            ASTNode* doWhileNode = new ASTNode(ASTNode::DO_WHILE_STMT, tokens[current - 1].line);
            // ASTNode* doWhileNode = new ASTNode("DoWhileLoop");
            // doWhileNode->addChild(new ASTNode("BlockStart", "{"));
            ASTNode* block = new ASTNode(ASTNode::BLOCK, tokens[current - 1].line);

            while (!match(OPERATOR, "}")) {
                block->addChild(parseStatement());
            }
            doWhileNode->addChild(block);
            
            match(KEYWORD, "while");
            match(OPERATOR, "(");
            // doWhileNode->addChild(new ASTNode("ParenthesisStart", "(")); // Добавляем открывающую скобку
            doWhileNode->addChild(parseCondition());
            // ASTNode* condition = new ASTNode("Condition", "");
            // condition->addChild(parseCondition());
            // doWhileNode->addChild(condition);
            match(OPERATOR, ")");
            // doWhileNode->addChild(new ASTNode("ParenthesisEnd", ")")); // Добавляем открывающую скобку

            match(OPERATOR, ";");
    
            return doWhileNode;
        }

        ASTNode* parseForLoop() {
            match(KEYWORD, "for");
            match(OPERATOR, "(");
            
            ASTNode* init = nullptr;
            // if(tokens[current + 2].lexeme == ":"){
                // Token type = consume();
                // Token name = consume();
                // match(OPERATOR, ":");
                // // Token collection = consume();
                // ASTNode* forNode = new ASTNode("ForLoop");
                // forNode->addChild(new ASTNode("ParenthesisStart", "(")); // Добавляем открывающую скобку
                // ASTNode* iterationCollection = new ASTNode("IterCollection");
                // iterationCollection->addChild(new ASTNode("Type", type.lexeme));
                // iterationCollection->addChild(new ASTNode("Item", name.lexeme));
                // ASTNode* expr = new ASTNode("Expression");
                // int countBrackets = 1;
                // while (true){
                //     Token message = consume();
                //     if(message.lexeme == "(") ++countBrackets;
                //     if(message.lexeme == ")") --countBrackets;
                //     if(countBrackets == 0) break;
                //     // if(peek().lexeme == ";" || peek().lexeme == ")") break;
                //     expr->addChild(new ASTNode(message.toString(), message.lexeme));
                //     // expr->addChild(parseTerm());
                // }
                // ASTNode* col = new ASTNode("Collection");
                // col->addChild(expr);
                // iterationCollection->addChild(col);
                // forNode->addChild(iterationCollection);

                // match(OPERATOR, ")");
                // forNode->addChild(new ASTNode("ParenthesisEnd", ")")); // Добавляем открывающую скобку

                // match(OPERATOR, "{");
                // forNode->addChild(new ASTNode("BlockStart", "{"));

                // while (!match(OPERATOR, "}")) {
                //     forNode->addChild(parseStatement());
                // }
                // forNode->addChild(new ASTNode("BlockEnd", "}"));

                // return forNode;
            // } else{
            ASTNode* forNode = new ASTNode(ASTNode::FOR_STMT, tokens[current-1].line);
                if (!match(OPERATOR, ";")) {
                    init = parseVariableDeclaration();
                    forNode->addChild(init);
                }

                // ASTNode* condition = new ASTNode("Condition", "");
                forNode->addChild(parseCondition());
                match(OPERATOR, ";");

                // ASTNode* iteration = new ASTNode("IterationChange", "");
                forNode->addChild(parseExpression());
                match(OPERATOR, ")");
                match(OPERATOR, "{");

                // ASTNode* forNode = new ASTNode("ForLoop");
                // forNode->addChild(new ASTNode("ParenthesisStart", "(")); // Добавляем открывающую скобку

                // if (init) forNode->addChild(init);
                // forNode->addChild(condition);
                // forNode->addChild(iteration);
                // forNode->addChild(new ASTNode("ParenthesisEnd", ")")); // Добавляем открывающую скобку
                // forNode->addChild(new ASTNode("BlockStart", "{"));
                ASTNode* block = new ASTNode(ASTNode::BLOCK, tokens[current - 1].line);
                while (!match(OPERATOR, "}")) {
                    block->addChild(parseStatement());
                }
                forNode->addChild(block);

                return forNode;
            // }
        }
    
        ASTNode* parseSwitchCaseStatement() {
            ASTNode* switchNode = new ASTNode(ASTNode::SWITCH_STMT, consume().line);
            
            match(OPERATOR, "(");
            switchNode->addChild(parseExpression());
            match(OPERATOR, ")");
            match(OPERATOR, "{");
            
            while (!match(OPERATOR, "}")) {
                if (match(KEYWORD, "case")) {
                    ASTNode* caseNode = new ASTNode(ASTNode::CASE, tokens[current - 1].line);
                    caseNode->addChild(parseExpression());
                    match(OPERATOR, ":");
                    while (!match(KEYWORD, "case") && !match(KEYWORD, "default") && !match(OPERATOR, "}")) {
                        caseNode->addChild(parseStatement());
                    }
                    --current;
                    switchNode->addChild(caseNode);
                }
                else if (match(KEYWORD, "default")) {
                    ASTNode* defaultNode = new ASTNode(ASTNode::DEFAULT, tokens[current - 1].line);
                    match(OPERATOR, ":");
                    while (!match(KEYWORD, "case") && !match(KEYWORD, "default") && !match(OPERATOR, "}")) {
                        defaultNode->addChild(parseStatement());
                    }
                    --current;
                    switchNode->addChild(defaultNode);
                }
            }
            
            return switchNode;
        }
        // ASTNode* parseSwitchCaseStatement() {
        //     match(KEYWORD, "switch");
        //     match(OPERATOR, "(");
        //     ASTNode* condition = new ASTNode("Condition", "");
        //     condition->addChild(parseCondition());
        //     // ASTNode* expression = parseExpression();
        //     match(OPERATOR, ")");
        //     match(OPERATOR, "{");
    
        //     ASTNode* switchNode = new ASTNode("SwitchStatement");
        //     switchNode->addChild(new ASTNode("ParenthesisStart", "(")); // Добавляем открывающую скобку
        //     switchNode->addChild(condition);
        //     switchNode->addChild(new ASTNode("ParenthesisEnd", ")")); // Добавляем открывающую скобку
        //     switchNode->addChild(new ASTNode("BlockStart", "{"));
            
        //     while (!match(OPERATOR, "}")) {
        //         if (match(KEYWORD, "case")) {
        //             Token caseValue = consume();
        //             match(OPERATOR, ":");
        //             ASTNode* caseNode = new ASTNode("Case", caseValue.lexeme);
        //             while (tokens[current].lexeme != "case" && tokens[current].lexeme != "default" && tokens[current].lexeme != "}") {
        //                 caseNode->addChild(parseStatement());
        //             }
        //             switchNode->addChild(caseNode);
        //         }
        //         else if(match(KEYWORD, "default")){
        //             match(OPERATOR, ":");
        //             ASTNode* caseNode = new ASTNode("Default");
        //             while (tokens[current].lexeme != "case" && tokens[current].lexeme != "default" && tokens[current].lexeme != "}") {
        //                 caseNode->addChild(parseStatement());
        //             }
        //             switchNode->addChild(caseNode);
        //         }
        //     }
        //     switchNode->addChild(new ASTNode("BlockEnd", "}"));
        //     return switchNode;
        // }

        ASTNode* parseFunctionCall(){
            ASTNode* statement = new ASTNode(ASTNode::METHOD_CALL, tokens[current].line);
            Token token = consume();
            statement->setAttribute("name", token.lexeme);
            match(OPERATOR, "(");
            
            while(!match(OPERATOR, ")")){
                statement->addChild(parseExpression());
                match(OPERATOR, ",");
            } 
            match(OPERATOR, ";");
            return statement;
        }

        ASTNode* parseMethodCall(){
            Token t = consume();
            ASTNode* methodNode = new ASTNode(ASTNode::METHOD_CALL, t.line);
            ASTNode* node = new ASTNode(ASTNode::FIELD_ACCESS, t.line);
            ASTNode* object = new ASTNode(ASTNode::VARIABLE, t.line);
            match(OPERATOR, ".");
            object->setAttribute("name", t.lexeme);
            node->addChild(object);
            node->setAttribute("field", consume().lexeme);
            methodNode->addChild(node);
            match(OPERATOR, "(");
            while(!match(OPERATOR, ")")) {
                methodNode->addChild(parseExpression());
                match(OPERATOR, ",");
            }
            match(OPERATOR, ";");
            return methodNode;
        }



        ASTNode* parseStatement() {
            if (peek().type == KEYWORD) {
                if (peek().lexeme == "if") return parseIfStatement();
                else if (peek().lexeme == "while") return parseWhileLoop();
                else if (peek().lexeme == "for") return parseForLoop();
                else if (peek().lexeme == "do") return parseDoWhileLoop();
                else if (peek().lexeme == "switch") return parseSwitchCaseStatement();
                // else if (peek().lexeme.find("[]") != std::string::npos ) return parseArrayDeclaration();
                else if (peek().lexeme == "break" || peek().lexeme == "continue") return parseTransitionOperator();
                else if (peek().lexeme == "ArrayList") return parseVariableArrayList();
                else if (peek().lexeme == "HashMap") return parseVariableHashMap();
                else if (peek().lexeme == "return") return parseReturnStatement();
                else return parseVariableDeclaration();
            }
    
            if (peek().type == IDENTIFIER) {
                if(peek().lexeme == "System"){
                    return parseSystemPrint();
                }
                if (tokens[current + 1].lexeme == "=" || tokens[current + 1].lexeme == ";") {
                    return parseVariableAssigment();
                }
                if (tokens[current + 1].lexeme == "+=" || tokens[current + 1].lexeme == "-=") {
                    return parseExpressionStatement();
                }
                if(tokens[current + 1].lexeme == "."){
                    return parseMethodCall();
                }
                // else{
                    return parseFunctionCall();
                // }
            }
    
            return parseExpressionStatement();
        }
        
        ASTNode* parseReturnStatement(){
            ASTNode* returnNode = new ASTNode(ASTNode::RETURN_STMT, consume().line);
            if (!match(OPERATOR, ";")) {
                returnNode->addChild(parseExpression());
            }
            match(OPERATOR, ";");
            return returnNode;
        }

        ASTNode* parseTransitionOperator() {
            Token keyword = consume();
            ASTNode* node = nullptr;
        
            if (keyword.lexeme == "break") {
                node = new ASTNode(ASTNode::BREAK_STMT, keyword.line);
            } 
            else if (keyword.lexeme == "continue") {
                node = new ASTNode(ASTNode::CONTINUE_STMT, keyword.line);
            }
        
            if (!match(OPERATOR, ";")) {
                throw ParseException("Missing ';' after " + keyword.lexeme, keyword.line);
            }
        
            return node;
        }
        // ASTNode* parseTransitionOperator(){
        //     Token operatorTo = consume();
        //     match(OPERATOR, ";");
        //     return new ASTNode(operatorTo.toString(), operatorTo.lexeme);
        // }

        ASTNode* parseSystemPrint(){
            match(IDENTIFIER, "System");
            match(OPERATOR, ".");
            match(IDENTIFIER, "out");
            match(OPERATOR, ".");
            Token print = consume();
            match(OPERATOR, "(");
            ASTNode* printNode = new ASTNode(ASTNode::METHOD_CALL, print.line);
            printNode->setAttribute("name", "System.out.println");
            match(OPERATOR, "(");
            if(peek().lexeme != ")"){
                printNode->addChild(parseExpression());
            }
            match(OPERATOR, ")");
            match(OPERATOR, ";");
            return printNode;
        }

        ASTNode* parseArrayDeclaration(){
            Token type = consume();
            Token varName = consume();
            ASTNode* array = new ASTNode(ASTNode::VARIABLE_DECL, varName.line);
            array->setAttribute("name", varName.lexeme);
            array->setAttribute("type", type.lexeme);
            // ASTNode* array = new ASTNode("ArrayDeclaration", varName.lexeme);
            // array->addChild(new ASTNode("Type", type.lexeme));
            match(OPERATOR, "=");
            match(OPERATOR, "{");
            while(true){
                array->addChild(parseFactor());
                if (consume().lexeme == "}") break;
            }
            match(OPERATOR, ";");
            return array;
        }

        ASTNode* parseCondition(){
            if (tokens[current + 1].lexeme == "<" || tokens[current + 1].lexeme == ">" || tokens[current + 1].lexeme == ">=" || tokens[current + 1].lexeme == "<=" || tokens[current + 1].lexeme == "==" || tokens[current + 1].lexeme == "!=") {
                ASTNode* left = parseTerm();
                Token op = consume();
                ASTNode* right = parseTerm();
                // ASTNode* right = new ASTNode("Expression");
                // while (true){
                //     if(peek().lexeme == ";" || peek().lexeme == ")") break;
                //     right->addChild(parseTerm());
                // }
                ASTNode* expr = new ASTNode(ASTNode::BINARY_EXPR, op.line);
                expr->setAttribute("operator", op.lexeme);
                // ASTNode* expr = new ASTNode("BinaryOp", op.lexeme);
                expr->addChild(left);
                expr->addChild(right);
                // left = expr;
                return expr;
            // }
            
            } else if(tokens[current].type == IDENTIFIER){
                Token t = consume();
                ASTNode* right = new ASTNode(ASTNode::VARIABLE, t.line);
                right->setAttribute("name", t.lexeme);
                return right;
            } else if(tokens[current].lexeme == "true" || tokens[current].lexeme == "false"){
                ASTNode* right = new ASTNode(ASTNode::LITERAL, tokens[current].line);
                Token t = consume();
                right->setAttribute("literalType", "boolean");
                right->setAttribute("value", t.lexeme);
                return right;
            } else{
                Token t = consume();
                throw ParseException("Обнаружена ошибка токена: " + t.lexeme, t.line);
                return nullptr;
            }
        }

        ASTNode* parseExpressionStatement() {
            ASTNode* exprNode = parseExpression();
            match(OPERATOR, ";");
            return exprNode;
        }
    
        ASTNode* parseExpression() {
            ASTNode* left = parseTerm();
            while (peek().lexeme == "+" || peek().lexeme == "-" || peek().lexeme == "+=") {
                Token op = consume();
                ASTNode* right = parseTerm();
                ASTNode* expr = new ASTNode(ASTNode::BINARY_EXPR, op.line);
                expr->setAttribute("operator", op.lexeme);
                // ASTNode* expr = new ASTNode("BinaryOp", op.lexeme);
                expr->addChild(left);
                expr->addChild(right);
                left = expr;
            }
            return left;
        }
    
        ASTNode* parseTerm() {

            if(peek().lexeme == "++" || peek().lexeme == "--"){
                Token op = consume();
                ASTNode* right = parseFactor();
                ASTNode* term = new ASTNode(ASTNode::UNARY_EXPR, op.line);
                term->setAttribute("operator", op.lexeme);
                // ASTNode* term = new ASTNode("UnaryOp", op.lexeme);
                term->addChild(right);
                return term;
            }
            else {
                ASTNode* left = parseFactor();

                while (peek().lexeme == "*" || peek().lexeme == "/") {
                    Token op = consume();
                    ASTNode* right = parseFactor();
                    ASTNode* term = new ASTNode(ASTNode::BINARY_EXPR, op.line);
                    term->setAttribute("operator", op.lexeme);
                    // ASTNode* term = new ASTNode("BinaryOp", op.lexeme);
                    term->addChild(left);
                    term->addChild(right);
                    left = term;
                }
                return left;
            }
        }
    
        ASTNode* parseFactor() {
            if (match(OPERATOR, "(")) {
                ASTNode* expr = parseExpression();
                match(OPERATOR, ")");
                return expr;
            }
            Token t = consume();
            if (t.type == IDENTIFIER) { 
                if(t.lexeme.find("[") != std::string::npos){
                    ASTNode* node = new ASTNode(ASTNode::ARRAY_ACCESS, t.line);
                    ASTNode* var = new ASTNode(ASTNode::VARIABLE, t.line);
                    var->setAttribute("name", t.lexeme.substr(0, t.lexeme.find("[")));
                    size_t start = t.lexeme.find("[") + 1;
                    size_t end = t.lexeme.find("]");
                    size_t length = end - start;
                    std::string index = t.lexeme.substr(start, length);
                    ASTNode* i = new ASTNode(ASTNode::VARIABLE, t.line);
                    i->setAttribute("name", index);
                    node->addChild(var);
                    node->addChild(i);
                    return node;
                } else if(match(OPERATOR, ".")){
                    ASTNode* node = new ASTNode(ASTNode::FIELD_ACCESS, t.line);
                    ASTNode* object = new ASTNode(ASTNode::VARIABLE, t.line);
                    object->setAttribute("name", t.lexeme);
                    node->addChild(object);
                    node->setAttribute("field", consume().lexeme);
                    return node;
                } else{
                    ASTNode* node = new ASTNode(ASTNode::VARIABLE, t.line);
                    node->setAttribute("name", t.lexeme);
                    return node;
                }
            }
            else if (t.type == NUMBER) {
                ASTNode* node = new ASTNode(ASTNode::LITERAL, t.line);
                node->setAttribute("literalType", "int");
                node->setAttribute("value", t.lexeme);
                return node;
            }
            else if (t.type == FLOAT_NUMBER) {
                ASTNode* node = new ASTNode(ASTNode::LITERAL, t.line);
                node->setAttribute("literalType", "float");
                node->setAttribute("value", t.lexeme);
                return node;
            }
            else if (t.type == CHAR_LITERAL) {
                ASTNode* node = new ASTNode(ASTNode::LITERAL, t.line);
                node->setAttribute("literalType", "char");
                node->setAttribute("value", t.lexeme);
                return node;
            }
            else if (t.type == STRING_LITERAL) {
                ASTNode* node = new ASTNode(ASTNode::LITERAL, t.line);
                node->setAttribute("literalType", "string");
                node->setAttribute("value", t.lexeme);
                return node;
                // return new ASTNode("String_literal", t.line);
            }
            // else if (t.lexeme == ".") return new ASTNode("Operator", t.line);
            else if (t.lexeme == "true" || t.lexeme == "false") {
                ASTNode* node = new ASTNode(ASTNode::LITERAL, t.line);
                node->setAttribute("literalType", "boolean");
                node->setAttribute("value", t.lexeme);
                return node;
                // return new ASTNode("Boolean_literal", t.line);
            }
            throw ParseException("Обнаружена ошибка токена: " + t.lexeme, t.line);
            return nullptr;
        }
    };

int main() {
    std::vector<Token> tokens = readTokensFromFile("D:\\Study\\6_semestr\\MTran\\output.txt");

    Parser parser(tokens);
    ASTNode* ast = nullptr;
    try {
        ast = parser.parseProgram();
        ast->print();
    } catch (const ParseException& e) {
        std::cout << "Ошибка: " << e.what() << " в строке " << e.getLine() << std::endl;
    } 
    SemanticAnalyzer sm_analyzer = SemanticAnalyzer();
    sm_analyzer.analyze(ast);
    // for(auto error: sm_analyzer.getErrors()){
    //     std::cout << error.getErrorMessage() << std::endl;
    // }
    delete ast;
    return 0;
}