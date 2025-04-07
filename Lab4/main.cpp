#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <cassert>

class Type;
class Symbol;
class SymbolTable;
class FunctionSymbol;
class ClassSymbol;
class Node;

// Типы данных в Java
class Type {
public:
    enum Kind {
        VOID,
        PRIMITIVE,
        ARRAY,
        CLASS
    };

    enum PrimitiveKind {
        BOOLEAN,
        CHAR,
        INT,
        FLOAT,
        DOUBLE,
        STRING
    };

    Type(Kind kind = VOID, PrimitiveKind primitiveKind = BOOLEAN, const std::string& className = "")
        : kind(kind), primitiveKind(primitiveKind), className(className), arrayDimension(0) {}

    static Type voidType() { return Type(VOID); }
    static Type booleanType() { return Type(PRIMITIVE, BOOLEAN); }
    static Type charType() { return Type(PRIMITIVE, CHAR); }
    static Type intType() { return Type(PRIMITIVE, INT); }
    static Type floatType() { return Type(PRIMITIVE, FLOAT); }
    static Type doubleType() { return Type(PRIMITIVE, DOUBLE); }
    static Type stringType() { return Type(PRIMITIVE, STRING); }
    static Type arrayType(const Type& baseType, int dimension = 1) {
        Type type = baseType;
        type.kind = ARRAY;
        type.arrayDimension = dimension;
        return type;
    }
    static Type classType(const std::string& name) {
        return Type(CLASS, BOOLEAN, name);
    }

    // Проверка типа
    bool isVoid() const { return kind == VOID; }
    bool isPrimitive() const { return kind == PRIMITIVE; }
    bool isArray() const { return kind == ARRAY; }
    bool isClass() const { return kind == CLASS; }
    bool isBoolean() const { return kind == PRIMITIVE && primitiveKind == BOOLEAN; }
    bool isNumeric() const {
        return kind == PRIMITIVE && (primitiveKind == INT || primitiveKind == FLOAT || primitiveKind == DOUBLE);
    }
    bool isString() const { return kind == PRIMITIVE && primitiveKind == STRING; }

    // Получение типа элемента массива
    Type getElementType() const {
        if (!isArray()) return *this;
        Type result = *this;
        result.arrayDimension--;
        if (result.arrayDimension == 0) {
            result.kind = (result.className.empty()) ? PRIMITIVE : CLASS;
        }
        return result;
    }

    // Проверка совместимости типов
    bool isAssignableTo(const Type& other) const {
        // Идентичные типы
        if (*this == other) return true;

        // Числовые преобразования
        if (isNumeric() && other.isNumeric()) {
            if (primitiveKind == INT && (other.primitiveKind == FLOAT || other.primitiveKind == DOUBLE))
                return true;
            if (primitiveKind == FLOAT && other.primitiveKind == DOUBLE)
                return true;
        }

        // Массивы
        if (isArray() && other.isArray()) {
            return getElementType().isAssignableTo(other.getElementType());
        }

        // Любой тип может быть преобразован в String через конкатенацию
        if (other.isString()) {
            return true;
        }

        // Классы (в простой реализации просто проверяем имена)
        if (isClass() && other.isClass()) {
            return className == other.className;
        }

        return false;
    }

    bool operator==(const Type& other) const {
        if (kind != other.kind) return false;
        
        switch (kind) {
            case VOID: return true;
            case PRIMITIVE: return primitiveKind == other.primitiveKind;
            case ARRAY: 
                return arrayDimension == other.arrayDimension && 
                       getElementType() == other.getElementType();
            case CLASS: return className == other.className;
        }
        
        return false;
    }

    bool operator!=(const Type& other) const {
        return !(*this == other);
    }

    std::string toString() const {
        switch (kind) {
            case VOID: return "void";
            case PRIMITIVE:
                switch (primitiveKind) {
                    case BOOLEAN: return "boolean";
                    case CHAR: return "char";
                    case INT: return "int";
                    case FLOAT: return "float";
                    case DOUBLE: return "double";
                    case STRING: return "String";
                    default: return "unknown";
                }
            case ARRAY: {
                std::string result = getElementType().toString();
                for (int i = 0; i < arrayDimension; i++) {
                    result += "[]";
                }
                return result;
            }
            case CLASS: return className;
            default: return "unknown";
        }
    }

private:
    Kind kind;
    PrimitiveKind primitiveKind;
    std::string className;
    int arrayDimension;
};

// Класс для представления символов (переменных, функций, классов)
class Symbol {
public:
    enum Kind {
        VARIABLE,
        FUNCTION,
        CLASS
    };

    Symbol(const std::string& name, const Type& type, Kind kind)
        : name(name), type(type), kind(kind) {}

    virtual ~Symbol() {}

    const std::string& getName() const { return name; }
    const Type& getType() const { return type; }
    Kind getKind() const { return kind; }

    bool isVariable() const { return kind == VARIABLE; }
    bool isFunction() const { return kind == FUNCTION; }
    bool isClass() const { return kind == CLASS; }

protected:
    std::string name;
    Type type;
    Kind kind;
};

// Расширение Symbol для функций
class FunctionSymbol : public Symbol {
public:
    FunctionSymbol(const std::string& name, const Type& returnType)
        : Symbol(name, returnType, FUNCTION) {}

    void addParameter(const std::string& name, const Type& type) {
        paramNames.push_back(name);
        paramTypes.push_back(type);
    }

    size_t getParameterCount() const { return paramTypes.size(); }
    const std::vector<std::string>& getParameterNames() const { return paramNames; }
    const std::vector<Type>& getParameterTypes() const { return paramTypes; }
    
    const Type& getParameterType(size_t index) const {
        if (index >= paramTypes.size()) {
            throw std::out_of_range("Parameter index out of range");
        }
        return paramTypes[index];
    }

private:
    std::vector<std::string> paramNames;
    std::vector<Type> paramTypes;
};

// Расширение Symbol для классов
class ClassSymbol : public Symbol {
public:
    ClassSymbol(const std::string& name): Symbol(name, Type::classType(name), CLASS), symbolTable(new SymbolTable(nullptr)) {}

    SymbolTable* getSymbolTable() const { return symbolTable.get(); }

private:
    std::unique_ptr<SymbolTable> symbolTable;
};

// Таблица символов для хранения переменных, функций и классов
class SymbolTable {
public:
    SymbolTable(SymbolTable* parent = nullptr) : parent(parent) {}

    // Добавление символа в текущую область видимости
    void define(Symbol* symbol) {
        symbols[symbol->getName()] = std::unique_ptr<Symbol>(symbol);
    }

    // Поиск символа в текущей и родительских областях видимости
    Symbol* resolve(const std::string& name) const {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second.get();
        }
        
        if (parent) {
            return parent->resolve(name);
        }
        
        return nullptr;
    }

    // Поиск символа только в текущей области видимости
    Symbol* resolveLocally(const std::string& name) const {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    
    SymbolTable* getParent() const { return parent; }

private:
    SymbolTable* parent;
    std::map<std::string, std::unique_ptr<Symbol>> symbols;
};

// Класс для ошибок семантического анализа
class SemanticError : public std::runtime_error {
public:
    SemanticError(const std::string& message, int line, int column)
        : std::runtime_error(buildMessage(message, line, column)),
          line(line), column(column), message(message) {}

    int getLine() const { return line; }
    int getColumn() const { return column; }
    const std::string& getErrorMessage() const { return message; }

private:
    static std::string buildMessage(const std::string& message, int line, int column) {
        return "Semantic error at " + std::to_string(line) + ":" + 
               std::to_string(column) + " - " + message;
    }

    int line;
    int column;
    std::string message;
};

// Базовый класс для узлов AST
class Node {
public:
    enum NodeType {
        PROGRAM,
        CLASS_DECL,
        METHOD_DECL,
        FIELD_DECL,
        VARIABLE_DECL,
        BLOCK,
        IF_STMT,
        WHILE_STMT,
        FOR_STMT,
        RETURN_STMT,
        EXPRESSION_STMT,
        BINARY_EXPR,
        UNARY_EXPR,
        LITERAL,
        VARIABLE,
        METHOD_CALL,
        ARRAY_ACCESS,
        FIELD_ACCESS,
        NEW_EXPR,
        ASSIGNMENT
    };

    Node(NodeType type, int line, int column)
        : type(type), line(line), column(column) {}
    
    virtual ~Node() {
        for (auto child : children) {
            delete child;
        }
    }

    NodeType getType() const { return type; }
    int getLine() const { return line; }
    int getColumn() const { return column; }

    void addChild(Node* child) {
        children.push_back(child);
    }

    Node* getChild(size_t index) const {
        if (index < children.size()) {
            return children[index];
        }
        return nullptr;
    }

    size_t getChildCount() const {
        return children.size();
    }

    void setAttribute(const std::string& key, const std::string& value) {
        attributes[key] = value;
    }

    std::string getAttribute(const std::string& key) const {
        auto it = attributes.find(key);
        if (it != attributes.end()) {
            return it->second;
        }
        return "";
    }

private:
    NodeType type;
    int line;
    int column;
    std::vector<Node*> children;
    std::map<std::string, std::string> attributes;
};

// Семантический анализатор
class SemanticAnalyzer {
public:
    SemanticAnalyzer() {
        // Инициализация глобальной области видимости
        globalScope = std::make_unique<SymbolTable>();
        currentScope = globalScope.get();
        
        // Добавление встроенных типов и функций
        initializeBuiltins();
    }

    ~SemanticAnalyzer() {
        while (!scopes.empty()) {
            scopes.pop_back();
        }
    }

    // Анализ AST
    void analyze(Node* ast) {
        try {
            visitNode(ast);
            std::cout << "Semantic analysis completed successfully." << std::endl;
        } catch (const SemanticError& error) {
            errors.push_back(error);
            std::cerr << error.what() << std::endl;
        }
    }

    bool hasErrors() const {
        return !errors.empty();
    }

    const std::vector<SemanticError>& getErrors() const {
        return errors;
    }

private:
    // Инициализация встроенных типов и функций
    void initializeBuiltins() {
        // Примитивные типы
        currentScope->define(new Symbol("boolean", Type::booleanType(), Symbol::CLASS));
        currentScope->define(new Symbol("char", Type::charType(), Symbol::CLASS));
        currentScope->define(new Symbol("int", Type::intType(), Symbol::CLASS));
        currentScope->define(new Symbol("float", Type::floatType(), Symbol::CLASS));
        currentScope->define(new Symbol("double", Type::doubleType(), Symbol::CLASS));
        currentScope->define(new Symbol("void", Type::voidType(), Symbol::CLASS));
        currentScope->define(new Symbol("String", Type::stringType(), Symbol::CLASS));

        // System.out.println
        auto printlnMethod = new FunctionSymbol("println", Type::voidType());
        printlnMethod->addParameter("value", Type::stringType());
        
        // System class with out field
        auto systemClass = new ClassSymbol("System");
        systemClass->getSymbolTable()->define(new Symbol("out", Type::classType("PrintStream"), Symbol::VARIABLE));
        
        // PrintStream class with println method
        auto printStreamClass = new ClassSymbol("PrintStream");
        printStreamClass->getSymbolTable()->define(printlnMethod);
        
        currentScope->define(systemClass);
        currentScope->define(printStreamClass);
    }

    // Входим в новую область видимости
    void enterScope() {
        auto newScope = std::make_unique<SymbolTable>(currentScope);
        scopes.push_back(std::move(newScope));
        currentScope = scopes.back().get();
    }

    // Выходим из текущей области видимости
    void exitScope() {
        if (!scopes.empty()) {
            scopes.pop_back();
            currentScope = scopes.empty() ? globalScope.get() : scopes.back().get();
        }
    }

    // Распознавание типа из строки
    Type resolveType(const std::string& typeName, int line, int column) {
        // Массивы
        size_t arrayBracketPos = typeName.find("[]");
        if (arrayBracketPos != std::string::npos) {
            std::string baseTypeName = typeName.substr(0, arrayBracketPos);
            Type baseType = resolveType(baseTypeName, line, column);
            
            // Подсчитываем количество пар []
            int dimensions = 0;
            std::string arrayPart = typeName.substr(arrayBracketPos);
            for (size_t i = 0; i < arrayPart.length(); i += 2) {
                if (i + 1 < arrayPart.length() && arrayPart[i] == '[' && arrayPart[i + 1] == ']') {
                    dimensions++;
                }
            }
            
            return Type::arrayType(baseType, dimensions);
        }

        // Примитивные типы
        if (typeName == "boolean") return Type::booleanType();
        if (typeName == "char") return Type::charType();
        if (typeName == "int") return Type::intType();
        if (typeName == "float") return Type::floatType();
        if (typeName == "double") return Type::doubleType();
        if (typeName == "void") return Type::voidType();
        if (typeName == "String") return Type::stringType();
        
        // Классы
        Symbol* symbol = currentScope->resolve(typeName);
        if (!symbol) {
            throw SemanticError("Unknown type: " + typeName, line, column);
        }
        
        if (!symbol->isClass()) {
            throw SemanticError(typeName + " is not a valid type", line, column);
        }
        
        return Type::classType(typeName);
    }

    // Обход узлов AST
    void visitNode(Node* node) {
        if (!node) return;

        switch (node->getType()) {
            case Node::PROGRAM:
                visitProgram(node);
                break;
            case Node::CLASS_DECL:
                visitClassDeclaration(node);
                break;
            case Node::METHOD_DECL:
                visitMethodDeclaration(node);
                break;
            case Node::FIELD_DECL:
                visitFieldDeclaration(node);
                break;
            case Node::VARIABLE_DECL:
                visitVariableDeclaration(node);
                break;
            case Node::BLOCK:
                visitBlock(node);
                break;
            case Node::IF_STMT:
                visitIfStatement(node);
                break;
            case Node::WHILE_STMT:
                visitWhileStatement(node);
                break;
            case Node::FOR_STMT:
                visitForStatement(node);
                break;
            case Node::RETURN_STMT:
                visitReturnStatement(node);
                break;
            case Node::EXPRESSION_STMT:
                visitExpressionStatement(node);
                break;
            case Node::ASSIGNMENT:
                visitAssignment(node);
                break;
            default:
                // Для выражений используем checkExpression
                if (node->getType() >= Node::BINARY_EXPR) {
                    checkExpression(node);
                }
                break;
        }
    }

    void visitProgram(Node* node) {
        // Обходим все объявления классов
        for (size_t i = 0; i < node->getChildCount(); i++) {
            visitNode(node->getChild(i));
        }
    }

    void visitClassDeclaration(Node* node) {
        std::string className = node->getAttribute("name");
        
        // Проверяем, что класс еще не объявлен
        if (currentScope->resolveLocally(className)) {
            throw SemanticError("Class " + className + " is already defined", 
                             node->getLine(), node->getColumn());
        }
        
        // Создаем символ класса
        ClassSymbol* classSymbol = new ClassSymbol(className);
        currentScope->define(classSymbol);
        
        // Сохраняем текущий класс
        ClassSymbol* outerClass = currentClass;
        currentClass = classSymbol;
        
        // Входим в область видимости класса
        currentScope = classSymbol->getSymbolTable();
        
        // Обрабатываем поля и методы класса
        for (size_t i = 0; i < node->getChildCount(); i++) {
            visitNode(node->getChild(i));
        }
        
        // Восстанавливаем контекст
        currentClass = outerClass;
        currentScope = currentClass ? currentClass->getSymbolTable() : globalScope.get();
    }

    void visitMethodDeclaration(Node* node) {
        std::string methodName = node->getAttribute("name");
        std::string returnTypeName = node->getAttribute("returnType");
        
        // Разрешаем тип возвращаемого значения
        Type returnType = resolveType(returnTypeName, node->getLine(), node->getColumn());
        
        // Создаем символ метода
        FunctionSymbol* methodSymbol = new FunctionSymbol(methodName, returnType);
        
        // Находим параметры и тело метода
        Node* paramsNode = nullptr;
        Node* bodyNode = nullptr;
        
        for (size_t i = 0; i < node->getChildCount(); i++) {
            Node* child = node->getChild(i);
            if (child->getType() == Node::BLOCK) {
                bodyNode = child;
            } else if (child->getAttribute("type") == "parameters") {
                paramsNode = child;
            }
        }
        
        // Обрабатываем параметры
        if (paramsNode) {
            for (size_t i = 0; i < paramsNode->getChildCount(); i++) {
                Node* paramNode = paramsNode->getChild(i);
                std::string paramName = paramNode->getAttribute("name");
                std::string paramTypeName = paramNode->getAttribute("type");
                
                Type paramType = resolveType(paramTypeName, paramNode->getLine(), paramNode->getColumn());
                methodSymbol->addParameter(paramName, paramType);
            }
        }
        
        // Добавляем метод в текущий класс
        currentScope->define(methodSymbol);
        
        // Сохраняем текущий метод
        FunctionSymbol* outerMethod = currentMethod;
        currentMethod = methodSymbol;
        
        // Входим в область видимости метода
        enterScope();
        
        // Добавляем параметры в область видимости метода
        if (paramsNode) {
            for (size_t i = 0; i < paramsNode->getChildCount(); i++) {
                Node* paramNode = paramsNode->getChild(i);
                std::string paramName = paramNode->getAttribute("name");
                std::string paramTypeName = paramNode->getAttribute("type");
                
                Type paramType = resolveType(paramTypeName, paramNode->getLine(), paramNode->getColumn());
                currentScope->define(new Symbol(paramName, paramType, Symbol::VARIABLE));
            }
        }
        
        // Обрабатываем тело метода
        if (bodyNode) {
            visitNode(bodyNode);
        }
        
        // Если метод не void, проверяем наличие return
        if (!returnType.isVoid() && !hasReturnStatement(bodyNode)) {
            throw SemanticError("Missing return statement in method " + methodName, 
                             node->getLine(), node->getColumn());
        }
        
        // Восстанавливаем контекст
        exitScope();
        currentMethod = outerMethod;
    }

    void visitFieldDeclaration(Node* node) {
        std::string fieldName = node->getAttribute("name");
        std::string typeName = node->getAttribute("type");
        
        // Разрешаем тип поля
        Type fieldType = resolveType(typeName, node->getLine(), node->getColumn());
        
        // Проверяем, что поле еще не объявлено
        if (currentScope->resolveLocally(fieldName)) {
            throw SemanticError("Field " + fieldName + " is already defined in this class", 
                             node->getLine(), node->getColumn());
        }
        
        // Добавляем поле в текущий класс
        currentScope->define(new Symbol(fieldName, fieldType, Symbol::VARIABLE));
        
        // Если есть инициализатор, проверяем совместимость типов
        if (node->getChildCount() > 0) {
            Node* initNode = node->getChild(0);
            Type initType = checkExpression(initNode);
            
            if (!initType.isAssignableTo(fieldType)) {
                throw SemanticError("Cannot assign " + initType.toString() + 
                                 " to field of type " + fieldType.toString(), 
                                 initNode->getLine(), initNode->getColumn());
            }
        }
    }

    void visitVariableDeclaration(Node* node) {
        std::string varName = node->getAttribute("name");
        std::string typeName = node->getAttribute("type");
        
        // Разрешаем тип переменной
        Type varType = resolveType(typeName, node->getLine(), node->getColumn());
        
        // Проверяем, что переменная еще не объявлена в текущей области видимости
        if (currentScope->resolveLocally(varName)) {
            throw SemanticError("Variable " + varName + " is already defined in this scope", 
                             node->getLine(), node->getColumn());
        }
        
        // Добавляем переменную в текущую область видимости
        currentScope->define(new Symbol(varName, varType, Symbol::VARIABLE));
        
        // Если есть инициализатор, проверяем совместимость типов
        if (node->getChildCount() > 0) {
            Node* initNode = node->getChild(0);
            Type initType = checkExpression(initNode);
            
            if (!initType.isAssignableTo(varType)) {
                throw SemanticError("Cannot assign " + initType.toString() + 
                                 " to variable of type " + varType.toString(), 
                                 initNode->getLine(), initNode->getColumn());
            }
        }
    }

    void visitBlock(Node* node) {
        // Входим в новую область видимости для блока
        enterScope();
        
        // Обрабатываем все выражения в блоке
        for (size_t i = 0; i < node->getChildCount(); i++) {
            visitNode(node->getChild(i));
        }
        
        // Выходим из области видимости блока
        exitScope();
    }

    void visitIfStatement(Node* node) {
        // Проверяем условие - должно быть boolean
        Node* conditionNode = node->getChild(0);
        Type condType = checkExpression(conditionNode);
        
        if (!condType.isBoolean()) {
            throw SemanticError("If condition must be boolean, found " + condType.toString(), 
                             conditionNode->getLine(), conditionNode->getColumn());
        }
        
        // Обрабатываем then-блок
        visitNode(node->getChild(1));
        
        // Обрабатываем else-блок, если он есть
        if (node->getChildCount() > 2) {
            visitNode(node->getChild(2));
        }
    }

    void visitWhileStatement(Node* node) {
        // Проверяем условие - должно быть boolean
        Node* conditionNode = node->getChild(0);
        Type condType = checkExpression(conditionNode);
        
        if (!condType.isBoolean()) {
            throw SemanticError("While condition must be boolean, found " + condType.toString(), 
                             conditionNode->getLine(), conditionNode->getColumn());
        }
        
        // Обрабатываем тело цикла
        visitNode(node->getChild(1));
    }

    void visitForStatement(Node* node) {
        // Входим в область видимости for-цикла
        enterScope();
        
        // Обрабатываем инициализатор
        if (node->getChildCount() > 0) {
            visitNode(node->getChild(0));
        }
        
        // Проверяем условие - должно быть boolean
        if (node->getChildCount() > 1) {
            Node* conditionNode = node->getChild(1);
            Type condType = checkExpression(conditionNode);
            
            if (!condType.isBoolean()) {
                throw SemanticError("For condition must be boolean, found " + condType.toString(), 
                                 conditionNode->getLine(), conditionNode->getColumn());
            }
        }
        
        // Обрабатываем инкремент
        if (node->getChildCount() > 2) {
            checkExpression(node->getChild(2));
        }
        
        // Обрабатываем тело цикла
        if (node->getChildCount() > 3) {
            visitNode(node->getChild(3));
        }
        
        // Выходим из области видимости for-цикла
        exitScope();
    }

    void visitReturnStatement(Node* node) {
        // Убеждаемся, что мы внутри метода
        if (!currentMethod) {
            throw SemanticError("Return statement outside of method", 
                             node->getLine(), node->getColumn());
        }
        
        Type methodReturnType = currentMethod->getType();
        
        // Проверяем возвращаемое выражение
        if (node->getChildCount() > 0) {
            Node* exprNode = node->getChild(0);
            Type exprType = checkExpression(exprNode);
            
            // Void методы не могут возвращать значения
            if (methodReturnType.isVoid()) {
                throw SemanticError("Cannot return a value from a void method", 
                                 exprNode->getLine(), exprNode->getColumn());
            }
            
            // Проверяем совместимость типов
            if (!exprType.isAssignableTo(methodReturnType)) {
                throw SemanticError("Cannot return " + exprType.toString() + 
                                 " from method with return type " + methodReturnType.toString(), 
                                 exprNode->getLine(), exprNode->getColumn());
            }
        } else {
            // Пустой return - только для void методов
            if (!methodReturnType.isVoid()) {
                throw SemanticError("Missing return value in method with return type " + 
                                 methodReturnType.toString(), 
                                 node->getLine(), node->getColumn());
            }
        }
    }

    void visitExpressionStatement(Node* node) {
        // Просто проверяем выражение
        if (node->getChildCount() > 0) {
            checkExpression(node->getChild(0));
        }
    }

    void visitAssignment(Node* node) {
        // Проверяем левую и правую части присваивания
        Node* lhsNode = node->getChild(0);
        Node* rhsNode = node->getChild(1);
        
        Type lhsType = checkAssignmentTarget(lhsNode);
        Type rhsType = checkExpression(rhsNode);
        
        // Проверяем совместимость типов
        if (!rhsType.isAssignableTo(lhsType)) {
            throw SemanticError("Cannot assign " + rhsType.toString() + 
                             " to variable of type " + lhsType.toString(), 
                             node->getLine(), node->getColumn());
        }
    }

    Type checkAssignmentTarget(Node* node) {
        // Проверяем, что левая часть присваивания корректна
        Node::NodeType type = node->getType();
        
        if (type == Node::VARIABLE) {
            std::string varName = node->getAttribute("name");
            Symbol* symbol = currentScope->resolve(varName);
            
            if (!symbol) {
                throw SemanticError("Undefined variable: " + varName, 
                                 node->getLine(), node->getColumn());
            }
            
            if (!symbol->isVariable()) {
                throw SemanticError(varName + " is not a variable", 
                                 node->getLine(), node->getColumn());
            }
            
            return symbol->getType();
        } else if (type == Node::ARRAY_ACCESS) {
            Node* arrayNode = node->getChild(0);
            Node* indexNode = node->getChild(1);
            
            Type arrayType = checkExpression(arrayNode);
            Type indexType = checkExpression(indexNode);
            
            if (!arrayType.isArray()) {
                throw SemanticError("Array access on non-array type: " + arrayType.toString(), 
                                 arrayNode->getLine(), arrayNode->getColumn());
            }
            
            if (!indexType.isNumeric()) {
                throw SemanticError("Array index must be numeric, found: " + indexType.toString(), 
                                 indexNode->getLine(), indexNode->getColumn());
            }
            
            return arrayType.getElementType();
        } else if (type == Node::FIELD_ACCESS) {
            Node* objectNode = node->getChild(0);
            std::string fieldName = node->getAttribute("field");
            
            Type objectType = checkExpression(objectNode);
            
            if (!objectType.isClass()) {
                throw SemanticError("Cannot access field on non-class type: " + objectType.toString(), 
                                 objectNode->getLine(), objectNode->getColumn());
            }
            
            // Ищем класс
            std::string className = objectType.toString();
            Symbol* classSymbol = currentScope->resolve(className);
            
            if (!classSymbol || !classSymbol->isClass()) {
                throw SemanticError("Class not found: " + className, 
                                 node->getLine(), node->getColumn());
            }
            
            // Ищем поле в классе
            ClassSymbol* cls = static_cast<ClassSymbol*>(classSymbol);
            Symbol* fieldSymbol = cls->getSymbolTable()->resolve(fieldName);
            
            if (!fieldSymbol) {
                throw SemanticError("Field " + fieldName + " not found in class " + className, 
                                 node->getLine(), node->getColumn());
            }
            
            return fieldSymbol->getType();
        }
        
        throw SemanticError("Invalid assignment target", 
                         node->getLine(), node->getColumn());
    }

    Type checkExpression(Node* node) {
        if (!node) {
            throw SemanticError("Null expression", 0, 0);
        }

        switch (node->getType()) {
            case Node::LITERAL:
                return checkLiteral(node);
            case Node::VARIABLE:
                return checkVariable(node);
            case Node::BINARY_EXPR:
                return checkBinaryExpression(node);
            case Node::UNARY_EXPR:
                return checkUnaryExpression(node);
            case Node::METHOD_CALL:
                return checkMethodCall(node);
            case Node::ARRAY_ACCESS:
                return checkArrayAccess(node);
            case Node::FIELD_ACCESS:
                return checkFieldAccess(node);
            case Node::NEW_EXPR:
                return checkNewExpression(node);
            default:
                throw SemanticError("Unknown expression type", 
                                 node->getLine(), node->getColumn());
        }
    }

    Type checkLiteral(Node* node) {
        std::string literalType = node->getAttribute("literalType");
        
        if (literalType == "int") return Type::intType();
        if (literalType == "float") return Type::floatType();
        if (literalType == "double") return Type::doubleType();
        if (literalType == "boolean") return Type::booleanType();
        if (literalType == "char") return Type::charType();
        if (literalType == "string") return Type::stringType();
        if (literalType == "null") return Type::classType("null"); // Специальный тип для null
        
        throw SemanticError("Unknown literal type: " + literalType, 
                         node->getLine(), node->getColumn());
    }

    Type checkVariable(Node* node) {
        std::string varName = node->getAttribute("name");
        Symbol* symbol = currentScope->resolve(varName);
        
        if (!symbol) {
            throw SemanticError("Undefined variable: " + varName, 
                             node->getLine(), node->getColumn());
        }
        
        if (!symbol->isVariable()) {
            throw SemanticError(varName + " is not a variable", 
                             node->getLine(), node->getColumn());
        }
        
        return symbol->getType();
    }

    Type checkBinaryExpression(Node* node) {
        std::string op = node->getAttribute("operator");
        Node* leftNode = node->getChild(0);
        Node* rightNode = node->getChild(1);
        
        Type leftType = checkExpression(leftNode);
        Type rightType = checkExpression(rightNode);
        
        // Арифметические операторы
        if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
            // Строковая конкатенация с +
            if (op == "+" && (leftType.isString() || rightType.isString())) {
                return Type::stringType();
            }
            
            // Числовые операции
            if (leftType.isNumeric() && rightType.isNumeric()) {
                if (leftType.toString() == "double" || rightType.toString() == "double") {
                    return Type::doubleType();
                } else if (leftType.toString() == "float" || rightType.toString() == "float") {
                    return Type::floatType();
                }
                return Type::intType();
            }
            
            throw SemanticError("Operator " + op + " cannot be applied to types " + 
                             leftType.toString() + " and " + rightType.toString(), 
                             node->getLine(), node->getColumn());
        }
        
        // Операторы сравнения
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
            // == и != могут сравнивать любые типы
            if ((op == "==" || op == "!=") && 
                (leftType.isAssignableTo(rightType) || rightType.isAssignableTo(leftType))) {
                return Type::booleanType();
            }
            
            // Другие операторы сравнения требуют числовых операндов
            if (leftType.isNumeric() && rightType.isNumeric()) {
                return Type::booleanType();
            }
            
            throw SemanticError("Operator " + op + " cannot be applied to types " + 
                             leftType.toString() + " and " + rightType.toString(), 
                             node->getLine(), node->getColumn());
        }
        
        // Логические операторы
        if (op == "&&" || op == "||") {
            if (leftType.isBoolean() && rightType.isBoolean()) {
                return Type::booleanType();
            }
            
            throw SemanticError("Operator " + op + " cannot be applied to types " + 
                             leftType.toString() + " and " + rightType.toString(), 
                             node->getLine(), node->getColumn());
        }
        
        throw SemanticError("Unknown binary operator: " + op, 
                         node->getLine(), node->getColumn());
    }

    Type checkUnaryExpression(Node* node) {
        std::string op = node->getAttribute("operator");
        Node* exprNode = node->getChild(0);
        Type exprType = checkExpression(exprNode);
        
        // Унарный минус
        if (op == "-") {
            if (exprType.isNumeric()) {
                return exprType;
            }
            
            throw SemanticError("Operator - cannot be applied to type " + exprType.toString(), 
                             node->getLine(), node->getColumn());
        }
        
        // Логическое отрицание
        if (op == "!") {
            if (exprType.isBoolean()) {
                return Type::booleanType();
            }
            
            throw SemanticError("Operator ! cannot be applied to type " + exprType.toString(), 
                             node->getLine(), node->getColumn());
        }
        
        throw SemanticError("Unknown unary operator: " + op, 
                         node->getLine(), node->getColumn());
    }

    Type checkMethodCall(Node* node) {
        std::string methodName = node->getAttribute("name");
        
        // Проверка аргументов метода
        std::vector<Type> argTypes;
        for (size_t i = 0; i < node->getChildCount(); i++) {
            Type argType = checkExpression(node->getChild(i));
            argTypes.push_back(argType);
        }
        
        // Особый случай для System.out.println
        if (methodName == "System.out.println") {
            if (argTypes.size() != 1) {
                throw SemanticError("System.out.println requires exactly one argument", 
                                 node->getLine(), node->getColumn());
            }
            return Type::voidType();
        }
        
        // Поиск метода в текущей области видимости
        Symbol* symbol = currentScope->resolve(methodName);
        
        if (!symbol) {
            throw SemanticError("Undefined method: " + methodName, 
                             node->getLine(), node->getColumn());
        }
        
        if (!symbol->isFunction()) {
            throw SemanticError(methodName + " is not a method", 
                             node->getLine(), node->getColumn());
        }
        
        FunctionSymbol* method = static_cast<FunctionSymbol*>(symbol);
        
        // Проверяем количество аргументов
        if (method->getParameterCount() != argTypes.size()) {
            throw SemanticError("Method " + methodName + " expects " + 
                             std::to_string(method->getParameterCount()) + 
                             " arguments, but got " + std::to_string(argTypes.size()), 
                             node->getLine(), node->getColumn());
        }
        
        // Проверяем типы аргументов
        for (size_t i = 0; i < argTypes.size(); i++) {
            if (!argTypes[i].isAssignableTo(method->getParameterType(i))) {
                throw SemanticError("Argument type mismatch for parameter " + 
                                 std::to_string(i+1) + " of method " + methodName, 
                                 node->getChild(i)->getLine(), 
                                 node->getChild(i)->getColumn());
            }
        }
        
        return method->getType();
    }

    Type checkArrayAccess(Node* node) {
        Node* arrayNode = node->getChild(0);
        Node* indexNode = node->getChild(1);
        
        Type arrayType = checkExpression(arrayNode);
        Type indexType = checkExpression(indexNode);
        
        if (!arrayType.isArray()) {
            throw SemanticError("Array access on non-array type: " + arrayType.toString(), 
                             arrayNode->getLine(), arrayNode->getColumn());
        }
        
        if (!indexType.isNumeric()) {
            throw SemanticError("Array index must be numeric, found: " + indexType.toString(), 
                             indexNode->getLine(), indexNode->getColumn());
        }
        
        return arrayType.getElementType();
    }

    Type checkFieldAccess(Node* node) {
        Node* objectNode = node->getChild(0);
        std::string fieldName = node->getAttribute("field");
        
        Type objectType = checkExpression(objectNode);
        
        // Особый случай для System.out
        if (objectType.toString() == "System" && fieldName == "out") {
            return Type::classType("PrintStream");
        }
        
        if (!objectType.isClass()) {
            throw SemanticError("Cannot access field on non-class type: " + objectType.toString(), 
                             objectNode->getLine(), objectNode->getColumn());
        }
        
        // Ищем класс
        std::string className = objectType.toString();
        Symbol* classSymbol = currentScope->resolve(className);
        
        if (!classSymbol || !classSymbol->isClass()) {
            throw SemanticError("Class not found: " + className, 
                             node->getLine(), node->getColumn());
        }
        
        // Ищем поле в классе
        ClassSymbol* cls = static_cast<ClassSymbol*>(classSymbol);
        Symbol* fieldSymbol = cls->getSymbolTable()->resolve(fieldName);
        
        if (!fieldSymbol) {
            throw SemanticError("Field " + fieldName + " not found in class " + className, 
                             node->getLine(), node->getColumn());
        }
        
        return fieldSymbol->getType();
    }

    Type checkNewExpression(Node* node) {
        std::string typeName = node->getAttribute("type");
        
        // Создание массива
        if (node->getAttribute("isArray") == "true") {
            Node* sizeNode = node->getChild(0);
            Type sizeType = checkExpression(sizeNode);
            
            if (!sizeType.isNumeric()) {
                throw SemanticError("Array size must be numeric, found: " + sizeType.toString(), 
                                 sizeNode->getLine(), sizeNode->getColumn());
            }
            
            Type elementType = resolveType(typeName, node->getLine(), node->getColumn());
            return Type::arrayType(elementType);
        }
        
        // Создание объекта
        Type classType = resolveType(typeName, node->getLine(), node->getColumn());
        
        if (!classType.isClass()) {
            throw SemanticError("Cannot create an instance of non-class type: " + typeName, 
                             node->getLine(), node->getColumn());
        }
        
        // Проверяем, что класс существует
        Symbol* classSymbol = currentScope->resolve(typeName);
        
        if (!classSymbol || !classSymbol->isClass()) {
            throw SemanticError("Class not found: " + typeName, 
                             node->getLine(), node->getColumn());
        }
        
        return classType;
    }

    // Проверка наличия return в методе
    bool hasReturnStatement(Node* node) {
        if (!node) return false;
        
        // Для простой проверки просто ищем return в текущем блоке
        // В реальном анализаторе нужно проверять все ветви выполнения
        for (size_t i = 0; i < node->getChildCount(); i++) {
            Node* child = node->getChild(i);
            if (child->getType() == Node::RETURN_STMT) {
                return true;
            }
            if (child->getType() == Node::BLOCK && hasReturnStatement(child)) {
                return true;
            }
            if (child->getType() == Node::IF_STMT) {
                // Если у if есть else и оба блока имеют return, считаем что путь имеет return
                bool thenHasReturn = hasReturnStatement(child->getChild(1));
                bool elseHasReturn = child->getChildCount() > 2 && hasReturnStatement(child->getChild(2));
                if (thenHasReturn && elseHasReturn) {
                    return true;
                }
            }
        }
        
        return false;
    }

    std::unique_ptr<SymbolTable> globalScope;
    SymbolTable* currentScope = nullptr;
    std::vector<std::unique_ptr<SymbolTable>> scopes;
    ClassSymbol* currentClass = nullptr;
    FunctionSymbol* currentMethod = nullptr;
    std::vector<SemanticError> errors;
};