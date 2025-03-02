#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

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

struct ASTNode {
    std::string type;
    std::string value;
    std::vector<ASTNode*> children;

    ASTNode(std::string t, std::string v = "") : type(t), value(v) {}

    void addChild(ASTNode* child) {
        children.push_back(child);
    }

    void print(const std::string& prefix = "", bool isLast = true) {
        std::cout << prefix;

        std::cout << (isLast ? "└── " : "├── ");
        std::cout << type << (value.empty() ? "" : ": " + value) << "\n";

        for (size_t i = 0; i < children.size(); ++i) {
            children[i]->print(prefix + (isLast ? "    " : "│   "), i == children.size() - 1);
        }
    }

    ~ASTNode() {
        for (ASTNode* child : children) {
            delete child;
        }
    }
};

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
            ASTNode* root = new ASTNode("Program");
            while(tokens[current].lexeme != "public"){
                current++;
            }
            while (current < tokens.size()) {
                root->addChild(parseClassDeclaration());
            }
            return root;
        }
    
        ASTNode* parseClassDeclaration() {
            match(KEYWORD, "public");
            match(KEYWORD, "class");
            Token className = consume(); // IDENTIFIER
            match(OPERATOR, "{");
    
            ASTNode* classNode = new ASTNode("ClassDeclaration", className.lexeme);
            while (!match(OPERATOR, "}")) {
                classNode->addChild(parseClassMember());
            }
    
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
            ASTNode* paramList = new ASTNode("ParameterList");
            do {
                if(peek().lexeme == "ArrayList"){
                    ASTNode* array = parseArrayList();
                    ASTNode* paramNode = new ASTNode("Parameter", array->value);
                    paramNode->addChild(new ASTNode("Type", "ArrayList<" + array->children[0]->value + ">"));
                    paramList->addChild(paramNode);
                }
                else if(peek().lexeme == "HashMap"){

                } else {
                    std::string type = consume().lexeme;
                    Token paramName = consume();
                    ASTNode* paramNode = new ASTNode("Parameter", paramName.lexeme);
                    paramNode->addChild(new ASTNode("Type", type));
                    paramList->addChild(paramNode);
                }
            } while (match(OPERATOR, ","));
            return paramList;
        }

        ASTNode* parseMethodDeclaration() {
            std::string returnType = consume().lexeme;
            Token methodName = consume(); // IDENTIFIER
            match(OPERATOR, "(");
            
            ASTNode* methodNode = new ASTNode("MethodDeclaration", methodName.lexeme);
            if (!match(OPERATOR, ")")) {
                methodNode->addChild(parseParameterList());
                match(OPERATOR, ")");
            }
            match(OPERATOR, "{");
    
            while (!match(OPERATOR, "}")) {
                methodNode->addChild(parseStatement());
            }
    
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
                ASTNode* varNode = new ASTNode("VariableDeclaration", varName.lexeme);
                varNode->addChild(new ASTNode("Type", type));
        
                if (match(OPERATOR, "=")) {
                    varNode->addChild(parseExpression());
                }
                match(OPERATOR, ";");
            // }
            return varNode;
        }
    
        ASTNode* parseArrayList() {
            match(KEYWORD, "ArrayList");
            match(OPERATOR, "<");
            Token type = consume();
            match(OPERATOR, ">");
            Token varName = consume();
    
            ASTNode* arrayListNode = new ASTNode("ArrayList", varName.lexeme);
            arrayListNode->addChild(new ASTNode("Type", type.lexeme));
    
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
    
            ASTNode* hashMapNode = new ASTNode("HashMap", varName.lexeme);
            hashMapNode->addChild(new ASTNode("KeyType", type1.lexeme));
            hashMapNode->addChild(new ASTNode("ValueType", type2.lexeme));
    
            return hashMapNode;
        }
    
        ASTNode* parseVariableArrayList(){
            ASTNode* array = parseArrayList();
            ASTNode* varNode = new ASTNode("VariableDeclaration", array->value);
            varNode->addChild(new ASTNode("Type", "ArrayList<" + array->children[0]->value + ">"));
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
            return varNode;
        }

        ASTNode* parseVariableHashMap(){
            ASTNode* array = parseHashMap();
            ASTNode* varNode = new ASTNode("VariableDeclaration", array->value);
            varNode->addChild(new ASTNode("Type", "HashMap<" + array->children[0]->value + ", " + array->children[1]->value + ">"));
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
            return varNode;
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
            ASTNode* condition = new ASTNode("Condition", "");
            condition->addChild(parseCondition());
            match(OPERATOR, ")");
            match(OPERATOR, "{");
            ASTNode* ifBlock = new ASTNode("IfStatement");
            ifBlock->addChild(condition);
            while (!match(OPERATOR, "}")) {
                ifBlock->addChild(parseStatement());
            }
    
            if (match(KEYWORD, "else")) {
                match(OPERATOR, "{");
                ASTNode* elseBlock = new ASTNode("ElseBlock");
                while (!match(OPERATOR, "}")) {
                    elseBlock->addChild(parseStatement());
                }
                ifBlock->addChild(elseBlock);
            }
    
            return ifBlock;
        }
    
        ASTNode* parseWhileLoop() {
            match(KEYWORD, "while");
            match(OPERATOR, "(");
            ASTNode* condition = new ASTNode("Condition", "");
            condition->addChild(parseCondition());
            match(OPERATOR, ")");
            match(OPERATOR, "{");
    
            ASTNode* whileNode = new ASTNode("WhileLoop");
            whileNode->addChild(condition);
            while (!match(OPERATOR, "}")) {
                whileNode->addChild(parseStatement());
            }
    
            return whileNode;
        }
    
        ASTNode* parseDoWhileLoop() {
            match(KEYWORD, "do");
            match(OPERATOR, "{");
    
            ASTNode* doWhileNode = new ASTNode("DoWhileLoop");
            while (!match(OPERATOR, "}")) {
                doWhileNode->addChild(parseStatement());
            }
    
            match(KEYWORD, "while");
            match(OPERATOR, "(");
            ASTNode* condition = new ASTNode("Condition", "");
            condition->addChild(parseCondition());
            doWhileNode->addChild(condition);
            match(OPERATOR, ")");
            match(OPERATOR, ";");
    
            return doWhileNode;
        }

        ASTNode* parseForLoop() {
            match(KEYWORD, "for");
            match(OPERATOR, "(");
            
            ASTNode* init = nullptr;
            if(tokens[current + 2].lexeme == ":"){
                Token type = consume();
                Token name = consume();
                match(OPERATOR, ":");
                // Token collection = consume();
                ASTNode* forNode = new ASTNode("ForLoop");
                ASTNode* iterationCollection = new ASTNode("IterCollection");
                iterationCollection->addChild(new ASTNode("Type", type.lexeme));
                iterationCollection->addChild(new ASTNode("Item", name.lexeme));
                ASTNode* expr = new ASTNode("Expression");
                int countBrackets = 1;
                while (true){
                    Token message = consume();
                    if(message.lexeme == "(") ++countBrackets;
                    if(message.lexeme == ")") --countBrackets;
                    if(countBrackets == 0) break;
                    // if(peek().lexeme == ";" || peek().lexeme == ")") break;
                    expr->addChild(new ASTNode(message.toString(), message.lexeme));
                    // expr->addChild(parseTerm());
                }
                ASTNode* col = new ASTNode("Collection");
                col->addChild(expr);
                iterationCollection->addChild(col);
                forNode->addChild(iterationCollection);

                match(OPERATOR, ")");
                match(OPERATOR, "{");

                while (!match(OPERATOR, "}")) {
                    forNode->addChild(parseStatement());
                }
                return forNode;
            } else{
                if (!match(OPERATOR, ";")) {
                    init = parseVariableDeclaration();
                }

                ASTNode* condition = new ASTNode("Condition", "");
                condition->addChild(parseCondition());
                match(OPERATOR, ";");

                ASTNode* iteration = new ASTNode("IterationChange", "");
                iteration->addChild(parseExpression());
                match(OPERATOR, ")");
                match(OPERATOR, "{");

                ASTNode* forNode = new ASTNode("ForLoop");
                if (init) forNode->addChild(init);
                forNode->addChild(condition);
                forNode->addChild(iteration);

                while (!match(OPERATOR, "}")) {
                    forNode->addChild(parseStatement());
                }
                return forNode;
            }
        }
    
        ASTNode* parseSwitchCaseStatement() {
            match(KEYWORD, "switch");
            match(OPERATOR, "(");
            ASTNode* condition = new ASTNode("Condition", "");
            condition->addChild(parseCondition());
            // ASTNode* expression = parseExpression();
            match(OPERATOR, ")");
            match(OPERATOR, "{");
    
            ASTNode* switchNode = new ASTNode("SwitchStatement");
            switchNode->addChild(condition);
    
            while (!match(OPERATOR, "}")) {
                if (match(KEYWORD, "case")) {
                    Token caseValue = consume();
                    match(OPERATOR, ":");
                    ASTNode* caseNode = new ASTNode("Case", caseValue.lexeme);
                    while (tokens[current].lexeme != "case" && tokens[current].lexeme != "default" && tokens[current].lexeme != "}") {
                        caseNode->addChild(parseStatement());
                    }
                    switchNode->addChild(caseNode);
                }
                else if(match(KEYWORD, "default")){
                    match(OPERATOR, ":");
                    ASTNode* caseNode = new ASTNode("Default");
                    while (tokens[current].lexeme != "case" && tokens[current].lexeme != "default" && tokens[current].lexeme != "}") {
                        caseNode->addChild(parseStatement());
                    }
                    switchNode->addChild(caseNode);
                }
            }
    
            return switchNode;
        }

        ASTNode* parseFunctionCall(){
            ASTNode* statement = new ASTNode("Statement");
            while(!match(OPERATOR, ";")){
                Token token = consume();
                statement->addChild(new ASTNode(token.toString(), token.lexeme));
            }
            return statement;
        }

        ASTNode* parseStatement() {
            if (peek().type == KEYWORD) {
                if (peek().lexeme == "if") return parseIfStatement();
                else if (peek().lexeme == "while") return parseWhileLoop();
                else if (peek().lexeme == "for") return parseForLoop();
                else if (peek().lexeme == "do") return parseDoWhileLoop();
                else if (peek().lexeme == "switch") return parseSwitchCaseStatement();
                else if (peek().lexeme.find("[]") != std::string::npos ) return parseArrayDeclaration();
                else if (peek().lexeme == "break" || peek().lexeme == "continue") return parseTransitionOperator();
                else if (peek().lexeme == "ArrayList") return parseVariableArrayList();
                else if (peek().lexeme == "HashMap") return parseVariableHashMap();
                else return parseVariableDeclaration();
            }
    
            if (peek().type == IDENTIFIER) {
                if(peek().lexeme == "System"){
                    return parseSystemPrint();
                }
                if (tokens[current + 1].lexeme == "=" || tokens[current + 1].lexeme == ";") {
                    return parseVariableDeclaration();
                }
                else{
                    return parseFunctionCall();
                }
            }
    
            return parseExpressionStatement();
        }
        
        ASTNode* parseTransitionOperator(){
            Token operatorTo = consume();
            match(OPERATOR, ";");
            return new ASTNode(operatorTo.toString(), operatorTo.lexeme);
        }

        ASTNode* parseSystemPrint(){
            match(IDENTIFIER, "System");
            match(OPERATOR, ".");
            match(IDENTIFIER, "out");
            match(OPERATOR, ".");
            Token print = consume();
            match(OPERATOR, "(");
            ASTNode* printNode = new ASTNode("System", print.lexeme);
            int countBrackets = 1;
            while(true){
                Token message = consume();
                if(message.lexeme == "(") ++countBrackets;
                if(message.lexeme == ")") --countBrackets;
                if(countBrackets == 0) break;
                // if(message.lexeme == ")") break;
                printNode->addChild(new ASTNode(message.toString(), message.lexeme));
            }
            match(OPERATOR, ";");
            return printNode;
        }

        ASTNode* parseArrayDeclaration(){
            Token type = consume();
            Token varName = consume();
            ASTNode* array = new ASTNode("ArrayDeclaration", varName.lexeme);
            array->addChild(new ASTNode("Type", type.lexeme));
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
            if (tokens[current + 1].lexeme == "<" || tokens[current + 1].lexeme == ">" || tokens[current + 1].lexeme == ">=" || tokens[current + 1].lexeme == ">=" || tokens[current + 1].lexeme == "==" || tokens[current + 1].lexeme == "!=") {
                ASTNode* left = parseTerm();
                Token op = consume();
                ASTNode* right = new ASTNode("Expression");
                while (true){
                    if(peek().lexeme == ";" || peek().lexeme == ")") break;
                    right->addChild(parseTerm());
                }
                ASTNode* expr = new ASTNode("BinaryOp", op.lexeme);
                expr->addChild(left);
                expr->addChild(right);
                left = expr;
                return left;
            // }
            
            } else if((tokens[current].type == IDENTIFIER || tokens[current].lexeme == "true" || tokens[current].lexeme == "false") && tokens[current + 1].type != IDENTIFIER){
                ASTNode* right = new ASTNode("Expression");
                while (true){
                    if(peek().lexeme == ";" || peek().lexeme == ")") break;
                    right->addChild(parseTerm());
                }
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
                ASTNode* expr = new ASTNode("BinaryOp", op.lexeme);
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
                ASTNode* term = new ASTNode("UnaryOp", op.lexeme);
                term->addChild(right);
                return term;
            }
            else {
                ASTNode* left = parseFactor();

                while (peek().lexeme == "*" || peek().lexeme == "/") {
                    Token op = consume();
                    ASTNode* right = parseFactor();
                    ASTNode* term = new ASTNode("BinaryOp", op.lexeme);
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
            if (t.type == IDENTIFIER) return new ASTNode("Identifier", t.lexeme);
            else if (t.type == NUMBER) return new ASTNode("Number", t.lexeme);
            else if (t.type == FLOAT_NUMBER) return new ASTNode("Float_number", t.lexeme);
            else if (t.type == CHAR_LITERAL) return new ASTNode("Char_literal", t.lexeme);
            else if (t.type == STRING_LITERAL) return new ASTNode("String_literal", t.lexeme);
            else if (t.lexeme == ".") return new ASTNode("Operator", t.lexeme);
            else if (t.lexeme == "true" || t.lexeme == "false") return new ASTNode("Boolean_literal", t.lexeme);
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
    delete ast;
    return 0;
}