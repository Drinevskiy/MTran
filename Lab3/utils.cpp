#include "utils.hpp"

// Type
Type::Type(Kind kind, PrimitiveKind primitiveKind, const std::string& className)
    : kind(kind), primitiveKind(primitiveKind), className(className), arrayDimension(0) {}

Type Type::voidType() { return Type(VOID); }
Type Type::booleanType() { return Type(PRIMITIVE, BOOLEAN); }
Type Type::charType() { return Type(PRIMITIVE, CHAR); }
Type Type::intType() { return Type(PRIMITIVE, INT); }
Type Type::floatType() { return Type(PRIMITIVE, FLOAT); }
Type Type::doubleType() { return Type(PRIMITIVE, DOUBLE); }
Type Type::stringType() { return Type(PRIMITIVE, STRING); }
Type Type::arrayType(const Type& baseType, int dimension) {
    Type type = baseType;
    type.kind = ARRAY;
    type.arrayDimension = dimension;
    return type;
}
Type Type::genericParamType(const std::string& paramName) {
    Type type(GENERIC_PARAM);
    type.genericParamName = paramName;
    return type;
}
Type Type::genericType(const Type& baseType, const std::vector<Type>& typeArgs) {
    Type type(GENERIC_INSTANCE);
    type.genericBaseType = std::make_shared<Type>(baseType);
    type.genericTypeArguments = typeArgs;
    return type;
}
Type Type::classType(const std::string& name) {
    return Type(CLASS, BOOLEAN, name);
}

bool Type::isVoid() const { return kind == VOID; }
bool Type::isPrimitive() const { return kind == PRIMITIVE; }
bool Type::isArray() const { return kind == ARRAY; }
bool Type::isClass() const { return kind == CLASS || kind == GENERIC_INSTANCE; }
bool Type::isBoolean() const { return kind == PRIMITIVE && primitiveKind == BOOLEAN; }
bool Type::isNumeric() const {
    return kind == PRIMITIVE && (primitiveKind == INT || primitiveKind == FLOAT || primitiveKind == DOUBLE);
}
bool Type::isInt() const {
    return kind == PRIMITIVE && primitiveKind == INT;
}
bool Type::isChar() const {
    return kind == PRIMITIVE && primitiveKind == CHAR;
}
bool Type::isString() const { return kind == PRIMITIVE && primitiveKind == STRING; }
bool Type::isGenericParam() const { return kind == GENERIC_PARAM; }
bool Type::isGenericInstance() const { return kind == GENERIC_INSTANCE; }

Type Type::getElementType() const {
    if (!isArray()) return *this;
    Type result = *this;
    result.arrayDimension--;
    if (result.arrayDimension == 0) {
        result.kind = (result.className.empty()) ? PRIMITIVE : CLASS;
    }
    return result;
}

Type Type::getGenericBaseType() const {
    if (!isGenericInstance()) throw std::runtime_error("Not a generic instance");
    return *genericBaseType;
}

std::vector<Type> Type::getGenericArguments() const {
    if (!isGenericInstance()) throw std::runtime_error("Not a generic instance");
    return genericTypeArguments;
}

std::string Type::getGenericParamName() const {
    if (!isGenericParam()) throw std::runtime_error("Not a generic parameter");
    return genericParamName;
}

bool Type::isAssignableTo(const Type& other) const {
    if (*this == other) return true;

    if (isNumeric() && other.isNumeric()) {
        if (primitiveKind == INT && (other.primitiveKind == FLOAT || other.primitiveKind == DOUBLE))
            return true;
        if (primitiveKind == FLOAT && other.primitiveKind == DOUBLE)
            return true;
    }

    if (isGenericInstance() && other.isGenericInstance()) {
        return genericBaseType->isAssignableTo(other.getGenericBaseType()) &&
               genericTypeArguments == other.genericTypeArguments;
    }

    if (isArray() && other.isArray()) {
        return getElementType().isAssignableTo(other.getElementType());
    }

    if (other.isString()) {
        return true;
    }

    if (isClass() && other.isClass()) {
        return className == other.className;
    }

    return false;
}

bool Type::operator==(const Type& other) const {
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

bool Type::operator!=(const Type& other) const {
    return !(*this == other);
}

std::string Type::toString() const {
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
        case GENERIC_PARAM: return genericParamName;
        case GENERIC_INSTANCE: {
            std::string result = genericBaseType->toString() + "<";
            for (size_t i = 0; i < genericTypeArguments.size(); ++i) {
                if (i > 0) result += ", ";
                result += genericTypeArguments[i].toString();
            }
            result += ">";
            std::size_t index = result.find("<>");
            if(index != std::string::npos) return result.erase(index,2);
            return result;
        }
        default: return "unknown";
    }
}


// Symbol

Symbol::Symbol(const std::string& name, const Type& type, Kind kind)
    : name(name), type(type), kind(kind) {}

Symbol::~Symbol() {}

const std::string& Symbol::getName() const { return name; }
const Type& Symbol::getType() const { return type; }
Symbol::Kind Symbol::getKind() const { return kind; }

bool Symbol::isVariable() const { return kind == VARIABLE; }
bool Symbol::isFunction() const { return kind == FUNCTION; }
bool Symbol::isClass() const { return kind == CLASS; }


// FunctionSymbol

FunctionSymbol::FunctionSymbol(const std::string& name, const Type& returnType)
    : Symbol(name, returnType, FUNCTION) {}

void FunctionSymbol::addParameter(const std::string& name, const Type& type) {
    paramNames.push_back(name);
    paramTypes.push_back(type);
}

size_t FunctionSymbol::getParameterCount() const { return paramTypes.size(); }
const std::vector<std::string>& FunctionSymbol::getParameterNames() const { return paramNames; }
const std::vector<Type>& FunctionSymbol::getParameterTypes() const { return paramTypes; }

const Type& FunctionSymbol::getParameterType(size_t index) const {
    if (index >= paramTypes.size()) {
        throw std::out_of_range("Parameter index out of range");
    }
    return paramTypes[index];
}


// ClassSymbol

ClassSymbol::ClassSymbol(const std::string& name)
    : Symbol(name, Type::classType(name), CLASS), symbolTable(new SymbolTable(nullptr)) {}

SymbolTable* ClassSymbol::getSymbolTable() const { return symbolTable.get(); }

void ClassSymbol::setGeneric(bool flag) { isGeneric = flag; }
void ClassSymbol::addGenericParam(const std::string& param) { genericParams.push_back(param); }
bool ClassSymbol::isGenericClass() const { return isGeneric; }
std::vector<std::string> ClassSymbol::getGenericParams() const { return genericParams; }

SymbolTable::SymbolTable(SymbolTable* parent) : parent(parent) {}

void SymbolTable::define(Symbol* symbol) {
    symbols[symbol->getName()] = std::unique_ptr<Symbol>(symbol);
}

Symbol* SymbolTable::resolve(const std::string& name) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return it->second.get();
    }
    
    if (parent) {
        return parent->resolve(name);
    }
    
    return nullptr;
}

Symbol* SymbolTable::resolveLocally(const std::string& name) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return it->second.get();
    }
    return nullptr;
}

SymbolTable* SymbolTable::getParent() const { return parent; }


// SemanticError

SemanticError::SemanticError(const std::string& message, int line)
    : std::runtime_error(buildMessage(message, line)),
      line(line), message(message) {}

std::string SemanticError::buildMessage(const std::string& message, int line) {
    return "Semantic error at " + std::to_string(line) + " - " + message;
}

int SemanticError::getLine() const { return line; }
const std::string& SemanticError::getErrorMessage() const { return message; }


// ASTNode

ASTNode::ASTNode(NodeType type, int line)
    : type(type), line(line) {}

ASTNode::~ASTNode() {
    for (auto child : children) {
        delete child;
    }
}

ASTNode::NodeType ASTNode::getType() const { return type; }
int ASTNode::getLine() const { return line; }

void ASTNode::addChild(ASTNode* child) {
    children.push_back(child);
}

ASTNode* ASTNode::getChild(size_t index) const {
    if (index < children.size()) {
        return children[index];
    }
    return nullptr;
}

size_t ASTNode::getChildCount() const {
    return children.size();
}

void ASTNode::print(const std::string& prefix, bool isLast) {
    std::cout << prefix;

    std::cout << (isLast ? "└── " : "├── ");
    std::string attrs = "";
    for(auto attr: attributes){
        attrs += attr.second + ", ";
    }
    std::cout << toString() << (": " + attrs) << "\n";

    for (size_t i = 0; i < children.size(); ++i) {
        children[i]->print(prefix + (isLast ? "    " : "│   "), i == children.size() - 1);
    }
}

void ASTNode::setAttribute(const std::string& key, const std::string& value) {
    attributes[key] = value;
}

std::string ASTNode::getAttribute(const std::string& key) const {
    auto it = attributes.find(key);
    if (it != attributes.end()) {
        return it->second;
    }
    return "";
}

std::string ASTNode::toString(){
    switch (type) {
        case PROGRAM: return "PROGRAM";
        case CLASS_DECL: return "CLASS_DECL";
        case METHOD_DECL: return "METHOD_DECL";
        case PARAMETER_LIST: return "PARAMETER_LIST";
        case PARAMETER: return "PARAMETER";
        case FIELD_DECL: return "FIELD_DECL";
        case VARIABLE_DECL: return "VARIABLE_DECL";
        case ARRAY_INIT: return "ARRAY_INIT";
        case BLOCK: return "BLOCK";
        case IF_STMT: return "IF_STMT";
        case WHILE_STMT: return "WHILE_STMT";
        case DO_WHILE_STMT: return "DO_WHILE_STMT";
        case FOR_STMT: return "FOR_STMT";
        case SWITCH_STMT: return "SWITCH_STMT";
        case CASE: return "CASE";
        case DEFAULT: return "DEFAULT";
        case RETURN_STMT: return "RETURN_STMT";
        case EXPRESSION_STMT: return "EXPRESSION_STMT";
        case BINARY_EXPR: return "BINARY_EXPR";
        case UNARY_EXPR: return "UNARY_EXPR";
        case BREAK_STMT: return "BREAK_STMT";
        case CONTINUE_STMT: return "CONTINUE_STMT";
        case LITERAL: return "LITERAL";
        case VARIABLE: return "VARIABLE";
        case METHOD_CALL: return "METHOD_CALL";
        case ARRAY_ACCESS: return "ARRAY_ACCESS";
        case FIELD_ACCESS: return "FIELD_ACCESS";
        case NEW_EXPR: return "NEW_EXPR";
        case ASSIGNMENT: return "ASSIGNMENT";
        default: return "UNKNOWN";
    }
}


// SemanticAnalyzer

SemanticAnalyzer::SemanticAnalyzer() {
    globalScope = std::make_unique<SymbolTable>();
    currentScope = globalScope.get();
    initializeBuiltins();
}

SemanticAnalyzer::~SemanticAnalyzer() {
    while (!scopes.empty()) {
        scopes.pop_back();
    }
}

void SemanticAnalyzer::analyze(ASTNode* ast) {
    try {
        visitNode(ast);
        std::cout << "Semantic analysis completed successfully." << std::endl;
    } catch (const SemanticError& error) {
        errors.push_back(error);
        std::cerr << error.what() << std::endl;
    }
}

bool SemanticAnalyzer::hasErrors() const {
    return !errors.empty();
}

const std::vector<SemanticError>& SemanticAnalyzer::getErrors() const {
    return errors;
}

void SemanticAnalyzer::initializeBuiltins() {
    currentScope->define(new Symbol("boolean", Type::booleanType(), Symbol::CLASS));
    currentScope->define(new Symbol("char", Type::charType(), Symbol::CLASS));
    currentScope->define(new Symbol("int", Type::intType(), Symbol::CLASS));
    currentScope->define(new Symbol("float", Type::floatType(), Symbol::CLASS));
    currentScope->define(new Symbol("double", Type::doubleType(), Symbol::CLASS));
    currentScope->define(new Symbol("void", Type::voidType(), Symbol::CLASS));
    currentScope->define(new Symbol("String", Type::stringType(), Symbol::CLASS));

    auto printlnMethod = new FunctionSymbol("println", Type::voidType());
    printlnMethod->addParameter("value", Type::stringType());
    
    auto systemClass = new ClassSymbol("System");
    systemClass->getSymbolTable()->define(new Symbol("out", Type::classType("PrintStream"), Symbol::VARIABLE));
    
    auto printStreamClass = new ClassSymbol("PrintStream");
    printStreamClass->getSymbolTable()->define(printlnMethod);
    


    ClassSymbol* arrayListClass = new ClassSymbol("ArrayList");

    arrayListClass->setGeneric(true);
    arrayListClass->addGenericParam("T");

    arrayListClass->getSymbolTable()->define(
        new FunctionSymbol("size", Type::intType())
    );

    auto getMethod = new FunctionSymbol("get", Type::genericParamType("T"));
    getMethod->addParameter("index", Type::intType());
    arrayListClass->getSymbolTable()->define(getMethod);

    auto addMethod = new FunctionSymbol("add", Type::booleanType());
    addMethod->addParameter("e", Type::genericParamType("T"));
    arrayListClass->getSymbolTable()->define(addMethod);

    // ClassSymbol* hashMapClass = new ClassSymbol("HashMap");
    // hashMapClass->getSymbolTable()->define(
    //     new FunctionSymbol("size", Type::intType())
    // );

    // auto getMethod = new FunctionSymbol("get", Type::classType("T"));
    // getMethod->addParameter("index", Type::intType());
    // arrayListClass->getSymbolTable()->define(getMethod);

    globalScope->define(arrayListClass);
    // globalScope->define(arrayListClass);
    // currentScope->define(hashMapClass);
    currentScope->define(systemClass);
    currentScope->define(printStreamClass);
}

void SemanticAnalyzer::enterScope() {
    scopes.push_back(std::make_unique<SymbolTable>(currentScope));
    currentScope = scopes.back().get();
}

void SemanticAnalyzer::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
        currentScope = scopes.empty() ? globalScope.get() : scopes.back().get();
    }
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

Type SemanticAnalyzer::resolveType(const std::string& typeName, int line) {
    size_t arrayBracketPos = typeName.find("[]");
    if (arrayBracketPos != std::string::npos) {
        std::string baseTypeName = typeName.substr(0, arrayBracketPos);
        Type baseType = resolveType(baseTypeName, line);
        
        int dimensions = 0;
        std::string arrayPart = typeName.substr(arrayBracketPos);
        for (size_t i = 0; i < arrayPart.length(); i += 2) {
            if (i + 1 < arrayPart.length() && arrayPart[i] == '[' && arrayPart[i + 1] == ']') {
                dimensions++;
            }
        }
        
        return Type::arrayType(baseType, dimensions);
    }

    size_t anglePos = typeName.find('<');
    if (anglePos != std::string::npos) {
        std::string baseName = typeName.substr(0, anglePos);
        Type baseType = resolveType(baseName, line);
        
        // Извлекаем аргументы типа
        std::vector<Type> typeArgs;
        std::string argsStr = typeName.substr(anglePos+1, typeName.rfind('>')-anglePos-1);
        
        // Парсим аргументы через запятую
        std::vector<std::string> argStrings = split(argsStr, ',');
        
        for (const auto& arg : argStrings) {
            typeArgs.push_back(resolveType(arg, line));
        }
        
        return Type::genericType(baseType, typeArgs);
    }

    if (typeName == "boolean") return Type::booleanType();
    if (typeName == "char") return Type::charType();
    if (typeName == "int") return Type::intType();
    if (typeName == "float") return Type::floatType();
    if (typeName == "double") return Type::doubleType();
    if (typeName == "void") return Type::voidType();
    if (typeName == "String") return Type::stringType();
    
    // Symbol* symbol = currentScope->resolve(typeName);
    Symbol* symbol = globalScope->resolve(typeName);
    if (!symbol) {
        throw SemanticError("Unknown type: " + typeName, line);
    }
    
    if (symbol->isClass()) {
        ClassSymbol* classSymbol = dynamic_cast<ClassSymbol*>(symbol);
        if (classSymbol && classSymbol->isGenericClass()) {
            // Логика для обработки generic-классов
            return Type::genericType(Type::classType(classSymbol->getName()), 
                {} // Заполнить реальными параметрами при конкретизации
            );
        }
    }

    // Symbol* symbol1 = globalScope->resolve(typeName);
    // if (!symbol1 || !symbol1->isClass()) {
    //     throw SemanticError("Class not found: " + typeName, line);
    // }
    // if (!symbol->isClass()) {
    //     throw SemanticError(typeName + " is not a valid type", line);
    // }
    
    return Type::classType(typeName);
}

void SemanticAnalyzer::visitNode(ASTNode* ASTNode) {
    if (!ASTNode) return;

    switch (ASTNode->getType()) {
        case ASTNode::PROGRAM:
            visitProgram(ASTNode);
            break;
        case ASTNode::CLASS_DECL:
            visitClassDeclaration(ASTNode);
            break;
        case ASTNode::METHOD_DECL:
            visitMethodDeclaration(ASTNode);
            break;
        case ASTNode::FIELD_DECL:
            visitFieldDeclaration(ASTNode);
            break;
        case ASTNode::VARIABLE_DECL:
            visitVariableDeclaration(ASTNode);
            break;
        case ASTNode::BLOCK:
            visitBlock(ASTNode);
            break;
        case ASTNode::IF_STMT:
            visitIfStatement(ASTNode);
            break;
        case ASTNode::WHILE_STMT:
            visitWhileStatement(ASTNode);
            break;
        case ASTNode::FOR_STMT:
            visitForStatement(ASTNode);
            break;
        case ASTNode::SWITCH_STMT: 
            visitSwitchStatement(ASTNode); 
            break;
        case ASTNode::BREAK_STMT:
            checkBreakValidity(ASTNode);
            break;
        case ASTNode::CONTINUE_STMT:
            checkContinueValidity(ASTNode);
            break;
        case ASTNode::CASE: 
            visitCase(ASTNode); 
            break;
        case ASTNode::DEFAULT: 
            visitDefault(ASTNode); 
            break;
        case ASTNode::RETURN_STMT:
            visitReturnStatement(ASTNode);
            break;
        case ASTNode::EXPRESSION_STMT:
            visitExpressionStatement(ASTNode);
            break;
        case ASTNode::ASSIGNMENT:
            visitAssignment(ASTNode);
            break;
        default:
            if (ASTNode->getType() >= ASTNode::BINARY_EXPR) {
                checkExpression(ASTNode);
            }
            break;
    }
}

void SemanticAnalyzer::visitProgram(ASTNode* ASTNode) {
    for (size_t i = 0; i < ASTNode->getChildCount(); i++) {
        visitNode(ASTNode->getChild(i));
    }
}

void SemanticAnalyzer::visitClassDeclaration(ASTNode* ASTNode) {
    std::string className = ASTNode->getAttribute("name");
    
    if (currentScope->resolveLocally(className)) {
        throw SemanticError("Class " + className + " is already defined", ASTNode->getLine());
    }
    
    ClassSymbol* classSymbol = new ClassSymbol(className);
    currentScope->define(classSymbol);
    
    ClassSymbol* outerClass = currentClass;
    currentClass = classSymbol;
    
    currentScope = classSymbol->getSymbolTable();
    
    for (size_t i = 0; i < ASTNode->getChildCount(); i++) {
        visitNode(ASTNode->getChild(i));
    }
    
    currentClass = outerClass;
    currentScope = currentClass ? currentClass->getSymbolTable() : globalScope.get();
}



void SemanticAnalyzer::visitMethodDeclaration(ASTNode* Node) {
    if (Node->getAttribute("genericParams") != "") {
        std::string paramsStr = Node->getAttribute("genericParams");
        // Парсим параметры типа через запятую
        std::vector<std::string> genericParams = split(paramsStr, ',');
        
        for (const auto& param : genericParams) {
            currentScope->define(new Symbol(param, Type::genericParamType(param), Symbol::TYPE_PARAM));
        }
    }
    std::string methodName = Node->getAttribute("name");
    std::string returnTypeName = Node->getAttribute("returnType");
    
    Type returnType = resolveType(returnTypeName, Node->getLine());
    
    FunctionSymbol* methodSymbol = new FunctionSymbol(methodName, returnType);
    
    ASTNode* paramsNode = nullptr;
    ASTNode* bodyNode = nullptr;
    
    for (size_t i = 0; i < Node->getChildCount(); i++) {
        ASTNode* child = Node->getChild(i);
        if (child->getType() == ASTNode::BLOCK) {
            bodyNode = child;
        } else if (child->getAttribute("type") == "parameters") {
            paramsNode = child;
        }
    }
    
    if (paramsNode) {
        for (size_t i = 0; i < paramsNode->getChildCount(); i++) {
            ASTNode* paramNode = paramsNode->getChild(i);
            std::string paramName = paramNode->getAttribute("name");
            std::string paramTypeName = paramNode->getAttribute("type");
            
            Type paramType = resolveType(paramTypeName, paramNode->getLine());
            methodSymbol->addParameter(paramName, paramType);
        }
    }
    
    
    FunctionSymbol* outerMethod = currentMethod;
    currentMethod = methodSymbol;
    
    enterScope();
    currentScope->define(methodSymbol);
    
    if (paramsNode) {
        for (size_t i = 0; i < paramsNode->getChildCount(); i++) {
            ASTNode* paramNode = paramsNode->getChild(i);
            std::string paramName = paramNode->getAttribute("name");
            std::string paramTypeName = paramNode->getAttribute("type");
            
            Type paramType = resolveType(paramTypeName, paramNode->getLine());
            currentScope->define(new Symbol(paramName, paramType, Symbol::VARIABLE));
        }
    }
    
    if (bodyNode) {
        visitNode(bodyNode);
    }
    
    if (!returnType.isVoid() && !hasReturnStatement(bodyNode)) {
        throw SemanticError("Missing return statement in method " + methodName, Node->getLine());
    }
    
    exitScope();
    currentMethod = outerMethod;
}

void SemanticAnalyzer::visitFieldDeclaration(ASTNode* Node) {
    std::string fieldName = Node->getAttribute("name");
    std::string typeName = Node->getAttribute("type");
    
    Type fieldType = resolveType(typeName, Node->getLine());
    
    if (currentScope->resolveLocally(fieldName)) {
        throw SemanticError("Field " + fieldName + " is already defined in this class", Node->getLine());
    }
    
    currentScope->define(new Symbol(fieldName, fieldType, Symbol::VARIABLE));
    
    if (Node->getChildCount() > 0) {
        ASTNode* initASTNode = Node->getChild(0);
        Type initType = checkExpression(initASTNode);
        
        if (!initType.isAssignableTo(fieldType)) {
            throw SemanticError("Cannot assign " + initType.toString() + 
                             " to field of type " + fieldType.toString(), 
                             initASTNode->getLine());
        }
    }
}

void SemanticAnalyzer::visitVariableDeclaration(ASTNode* Node) {
    std::string varName = Node->getAttribute("name");
    std::string typeName = Node->getAttribute("type");
    
    Type varType = resolveType(typeName, Node->getLine());
    
    if (currentScope->resolveLocally(varName)) {
        throw SemanticError("Variable " + varName + " is already defined in this scope", Node->getLine());
    }
    
    currentScope->define(new Symbol(varName, varType, Symbol::VARIABLE));
    
    if (Node->getChildCount() > 0) {
        if (varType.isArray()) {
            visitArrayInitialization(Node, varType);
        } 
        else { // Обычная переменная
            ASTNode* initNode = Node->getChild(0);
            Type initType = checkExpression(initNode);
            
            // Для массивов учитываем ковариантность
            if (varType.isArray() && initType.isArray()) {
                if (!initType.getElementType().isAssignableTo(varType.getElementType())) {
                    // throwTypeMismatchError(varType, initType, initNode);
                    throw SemanticError("Cannot assign " + initType.toString() + 
                             " to variable of type " + varType.toString(), 
                             initNode->getLine());
                }
            }
            if (!initType.isAssignableTo(varType)) {
                throw SemanticError("Cannot assign " + initType.toString() + 
                             " to variable of type " + varType.toString(), 
                             initNode->getLine());
            }
        }
    }
}

void SemanticAnalyzer::visitArrayInitialization(ASTNode* varNode, const Type& arrayType) {
    Type elementType = arrayType.getElementType();
    
    // Проверяем все элементы инициализации
    for (size_t i = 0; i < varNode->getChildCount(); i++) {
        ASTNode* elementNode = varNode->getChild(i);
        Type elementExprType = checkExpression(elementNode);
        // elementExprType.primitiveKind
        if (!elementExprType.getElementType().isAssignableTo(elementType)) {
            throw SemanticError("Array element type mismatch. Expected " +
                              elementType.toString() + ", got " +
                              elementExprType.toString(),
                              elementNode->getLine());
        }
    }
    
    // Дополнительная проверка для явного создания массива
    // if (varNode->getChildCount() == 1) {
    //     ASTNode* initNode = varNode->getChild(0);
    //     if (initNode->getType() == ASTNode::NEW_EXPR) {
    //         // checkNewArrayExpression(initNode, arrayType);
    //         Type createdType = checkExpression(initNode);
    
    //         if (createdType != arrayType) {
    //             throw SemanticError("Array type mismatch. Expected " +
    //                             arrayType.toString() + ", got " +
    //                             createdType.toString(),
    //                             initNode->getLine());
    //         }
    //     }
    // }
}

void SemanticAnalyzer::visitBlock(ASTNode* Node) {
    enterScope();
    
    for (size_t i = 0; i < Node->getChildCount(); i++) {
        visitNode(Node->getChild(i));
    }
    
    exitScope();
}

void SemanticAnalyzer::visitIfStatement(ASTNode* Node) {
    ASTNode* conditionASTNode = Node->getChild(0);
    Type condType = checkExpression(conditionASTNode);
    
    if (!condType.isBoolean()) {
        throw SemanticError("If condition must be boolean, found " + condType.toString(), 
                         conditionASTNode->getLine());
    }
    
    visitNode(Node->getChild(1));
    
    if (Node->getChildCount() > 2) {
        visitNode(Node->getChild(2));
    }
}

void SemanticAnalyzer::visitWhileStatement(ASTNode* Node) {
    ASTNode* conditionNode = Node->getChild(0);
    Type condType = checkExpression(conditionNode);
    contextStack.push(LOOP_CONTEXT);
    if (!condType.isBoolean()) {
        throw SemanticError("While condition must be boolean, found " + condType.toString(), 
                         conditionNode->getLine());
    }
    
    visitNode(Node->getChild(1));
    contextStack.pop();
}

void SemanticAnalyzer::visitDoWhileStatement(ASTNode* Node) {
    ASTNode* conditionNode = Node->getChild(1);
    Type condType = checkExpression(conditionNode);
    contextStack.push(LOOP_CONTEXT);
    
    if (!condType.isBoolean()) {
        throw SemanticError("While condition must be boolean, found " + condType.toString(), 
                         conditionNode->getLine());
    }
    
    visitNode(Node->getChild(0));
    contextStack.pop();
}

void SemanticAnalyzer::visitForStatement(ASTNode* Node) {
    enterScope();
    
    if (Node->getChildCount() > 0) {
        visitNode(Node->getChild(0));
    }
    
    if (Node->getChildCount() > 1) {
        ASTNode* conditionASTNode = Node->getChild(1);
        Type condType = checkExpression(conditionASTNode);
        
        if (!condType.isBoolean()) {
            throw SemanticError("For condition must be boolean, found " + condType.toString(), 
                             conditionASTNode->getLine());
        }
    }
    
    if (Node->getChildCount() > 2) {
        checkExpression(Node->getChild(2));
    }
    contextStack.push(LOOP_CONTEXT);
    
    if (Node->getChildCount() > 3) {
        visitNode(Node->getChild(3));
    }
    contextStack.pop();
    exitScope();
}

void SemanticAnalyzer::visitSwitchStatement(ASTNode* node) {
    // Проверка условия switch
    ASTNode* condition = node->getChild(0);
    Type condType = checkExpression(condition);
    
    // Условие должно быть целочисленным или enum
    if (!condType.isInt() && !condType.isChar()) {
        throw SemanticError("Switch condition must be integer or char", node->getLine());
    }
    
    // Проверка case-блоков
    bool hasDefault = false;
    std::set<std::string> caseValues;
    switchConditionStack.push_back(condType);
    contextStack.push(SWITCH_CONTEXT);

    for (size_t i = 1; i < node->getChildCount(); i++) {
        ASTNode* child = node->getChild(i);
        if (child->getType() == ASTNode::CASE) {
            visitCase(child);
            
            // Проверка уникальности значений
            std::string value = child->getChild(0)->getAttribute("value");
            if (caseValues.count(value)) {
                throw SemanticError("Duplicate case value: " + value, child->getLine());
            }
            caseValues.insert(value);
        }
        else if (child->getType() == ASTNode::DEFAULT) {
            if (hasDefault) {
                throw SemanticError("Multiple default cases", child->getLine());
            }
            hasDefault = true;
            visitDefault(child);
        }
    }
    switchConditionStack.pop_back();
    contextStack.pop();
}

void SemanticAnalyzer::visitCase(ASTNode* node) {
    if (switchConditionStack.empty()) {
        throw SemanticError("Case outside switch statement", node->getLine());
    }
    Type switchType = switchConditionStack.back();

    ASTNode* valueNode = node->getChild(0);
    Type caseType = checkExpression(valueNode);
    
    if (!caseType.isAssignableTo(switchType)) {
        throw SemanticError(
            "Case type " + caseType.toString() + 
            " is incompatible with switch type " + switchType.toString(),
            node->getLine()
        );
    }
    enterScope();
    for (size_t i = 1; i < node->getChildCount(); i++) {
        visitNode(node->getChild(i));
    }
    exitScope();
}

void SemanticAnalyzer::checkBreakValidity(ASTNode* node) {
    bool valid = false;
    std::stack<ContextType> temp = contextStack;
    
    while (!temp.empty()) {
        if (temp.top() == LOOP_CONTEXT || temp.top() == SWITCH_CONTEXT) {
            valid = true;
            break;
        }
        temp.pop();
    }
    
    if (!valid) {
        throw SemanticError("Break outside loop or switch", node->getLine());
    }
}

void SemanticAnalyzer::checkContinueValidity(ASTNode* node) {
    bool valid = false;
    std::stack<ContextType> temp = contextStack;
    
    while (!temp.empty()) {
        if (temp.top() == LOOP_CONTEXT) {
            valid = true;
            break;
        }
        temp.pop();
    }
    
    if (!valid) {
        throw SemanticError("Continue outside loop", node->getLine());
    }
}

void SemanticAnalyzer::visitDefault(ASTNode* node) {
    if (switchConditionStack.empty()) {
        throw SemanticError("Default outside switch statement", node->getLine());
    }
    enterScope();
    for (size_t i = 0; i < node->getChildCount(); i++) {
        visitNode(node->getChild(i));
    }
    exitScope();
}

void SemanticAnalyzer::visitReturnStatement(ASTNode* Node) {
    if (!currentMethod) {
        throw SemanticError("Return statement outside of method", Node->getLine());
    }
    
    Type methodReturnType = currentMethod->getType();
    
    if (Node->getChildCount() > 0) {
        ASTNode* exprASTNode = Node->getChild(0);
        Type exprType = checkExpression(exprASTNode);
        
        if (methodReturnType.isVoid()) {
            throw SemanticError("Cannot return a value from a void method", 
                             exprASTNode->getLine());
        }
        
        if (!exprType.isAssignableTo(methodReturnType)) {
            throw SemanticError("Cannot return " + exprType.toString() + 
                             " from method with return type " + methodReturnType.toString(), 
                             exprASTNode->getLine());
        }
    } else {
        if (!methodReturnType.isVoid()) {
            throw SemanticError("Missing return value in method with return type " + 
                             methodReturnType.toString(), 
                             Node->getLine());
        }
    }
}

void SemanticAnalyzer::visitExpressionStatement(ASTNode* Node) {
    if (Node->getChildCount() > 0) {
        checkExpression(Node->getChild(0));
    }
}

void SemanticAnalyzer::visitAssignment(ASTNode* Node) {
    ASTNode* lhsASTNode = Node->getChild(0);
    ASTNode* rhsASTNode = Node->getChild(1);
    
    Type lhsType = checkAssignmentTarget(lhsASTNode);
    Type rhsType = checkExpression(rhsASTNode);
    
    if (!rhsType.isAssignableTo(lhsType)) {
        throw SemanticError("Cannot assign " + rhsType.toString() + 
                         " to variable of type " + lhsType.toString(), 
                         Node->getLine());
    }
}

Type SemanticAnalyzer::checkAssignmentTarget(ASTNode* Node) {
    ASTNode::NodeType type = Node->getType();
    
    if (type == ASTNode::VARIABLE) {
        std::string varName = Node->getAttribute("name");
        Symbol* symbol = currentScope->resolve(varName);
        
        if (!symbol) {
            throw SemanticError("Undefined variable: " + varName, 
                             Node->getLine());
        }
        
        if (!symbol->isVariable()) {
            throw SemanticError(varName + " is not a variable", 
                             Node->getLine());
        }
        
        return symbol->getType();
    } else if (type == ASTNode::ARRAY_ACCESS) {
        ASTNode* arrayASTNode = Node->getChild(0);
        ASTNode* indexASTNode = Node->getChild(1);
        
        Type arrayType = checkExpression(arrayASTNode);
        Type indexType = checkExpression(indexASTNode);
        
        if (!arrayType.isArray()) {
            throw SemanticError("Array access on non-array type: " + arrayType.toString(), 
                             arrayASTNode->getLine());
        }
        
        if (!indexType.isNumeric()) {
            throw SemanticError("Array index must be numeric, found: " + indexType.toString(), 
                             indexASTNode->getLine());
        }
        
        return arrayType.getElementType();
    } else if (type == ASTNode::FIELD_ACCESS) {
        ASTNode* objectASTNode = Node->getChild(0);
        std::string fieldName = Node->getAttribute("field");
        
        Type objectType = checkExpression(objectASTNode);
        
        if (!objectType.isClass()) {
            throw SemanticError("Cannot access field on non-class type: " + objectType.toString(), 
                             objectASTNode->getLine());
        }
        
        std::string className = objectType.toString();
        Symbol* classSymbol = currentScope->resolve(className);
        
        if (!classSymbol || !classSymbol->isClass()) {
            throw SemanticError("Class not found: " + className, 
                             Node->getLine());
        }
        
        ClassSymbol* cls = static_cast<ClassSymbol*>(classSymbol);
        Symbol* fieldSymbol = cls->getSymbolTable()->resolve(fieldName);
        
        if (!fieldSymbol) {
            throw SemanticError("Field " + fieldName + " not found in class " + className, 
                             Node->getLine());
        }
        
        return fieldSymbol->getType();
    }
    
    throw SemanticError("Invalid assignment target", Node->getLine());
}

Type SemanticAnalyzer::checkExpression(ASTNode* Node) {
    switch (Node->getType()) {
        case ASTNode::LITERAL: return checkLiteral(Node);
        case ASTNode::VARIABLE: return checkVariable(Node);
        case ASTNode::ARRAY_INIT: return checkArrayInitializer(Node);
        case ASTNode::BINARY_EXPR: return checkBinaryExpression(Node);
        case ASTNode::UNARY_EXPR: return checkUnaryExpression(Node);
        case ASTNode::METHOD_CALL: return checkMethodCall(Node);
        case ASTNode::ARRAY_ACCESS: return checkArrayAccess(Node);
        case ASTNode::FIELD_ACCESS: return checkFieldAccess(Node);
        case ASTNode::NEW_EXPR: return checkNewExpression(Node);
        default: 
            throw SemanticError("Unknown expression type", Node->getLine());
    }
}

Type SemanticAnalyzer::checkLiteral(ASTNode* Node) {
    std::string literalType = Node->getAttribute("literalType");
    
    if (literalType == "int") return Type::intType();
    if (literalType == "float") return Type::floatType();
    if (literalType == "double") return Type::doubleType();
    if (literalType == "boolean") return Type::booleanType();
    if (literalType == "char") return Type::charType();
    if (literalType == "string") return Type::stringType();
    if (literalType == "null") return Type::classType("null");
    
    throw SemanticError("Unknown literal type: " + literalType, Node->getLine());
}

Type SemanticAnalyzer::checkVariable(ASTNode* Node) {
    std::string varName = Node->getAttribute("name");
    Symbol* symbol = currentScope->resolve(varName);
    
    if (!symbol) {
        throw SemanticError("Undefined variable: " + varName, Node->getLine());
    }
    
    if (!symbol->isVariable()) {
        throw SemanticError(varName + " is not a variable", Node->getLine());
    }
    
    return symbol->getType();
}

Type SemanticAnalyzer::checkBinaryExpression(ASTNode* Node) {
    std::string op = Node->getAttribute("operator");
    ASTNode* leftASTNode = Node->getChild(0);
    ASTNode* rightASTNode = Node->getChild(1);
    
    Type leftType = checkExpression(leftASTNode);
    Type rightType = checkExpression(rightASTNode);
    
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (op == "+" && (leftType.isString() || rightType.isString())) {
            return Type::stringType();
        }
        
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
                         Node->getLine());
    }
    
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
        if ((op == "==" || op == "!=") && 
            (leftType.isAssignableTo(rightType) || rightType.isAssignableTo(leftType))) {
            return Type::booleanType();
        }
        
        if (leftType.isNumeric() && rightType.isNumeric()) {
            return Type::booleanType();
        }
        
        throw SemanticError("Operator " + op + " cannot be applied to types " + 
                         leftType.toString() + " and " + rightType.toString(), 
                         Node->getLine());
    }
    
    if (op == "&&" || op == "||") {
        if (leftType.isBoolean() && rightType.isBoolean()) {
            return Type::booleanType();
        }
        
        throw SemanticError("Operator " + op + " cannot be applied to types " + 
                         leftType.toString() + " and " + rightType.toString(), 
                         Node->getLine());
    }
    if (op == "+=" || op == "-=" || op == "*=" || op == "/=") {
        // Проверяем, что левая часть - изменяемая переменная
        if (!isLValue(leftASTNode)) {
            throw SemanticError("Left operand must be assignable", Node->getLine());
        }
        
        // Вычисляем тип результата операции
        Type operationType = checkOperationType(op[0], leftType, rightType, Node->getLine());
        
        // Проверяем совместимость типов
        if (!operationType.isAssignableTo(leftType)) {
            throw SemanticError("Cannot apply '" + op + "' to " + 
                leftType.toString() + " and " + rightType.toString(),
                Node->getLine());
        }
        
        return leftType; // Тип выражения совпадает с типом левого операнда
    }
    throw SemanticError("Unknown binary operator: " + op, 
                     Node->getLine());
}

bool SemanticAnalyzer::isLValue(ASTNode* node) {
    return node->getType() == ASTNode::VARIABLE ||
           node->getType() == ASTNode::ARRAY_ACCESS ||
           node->getType() == ASTNode::FIELD_ACCESS;
}

Type SemanticAnalyzer::checkOperationType(char op, Type left, Type right, int line) {
    switch(op) {
        case '+':
            if (left.isString() || right.isString()) {
                return Type::stringType();
            }
            if (left.isNumeric() && right.isNumeric()) {
                return getNumericResultType(left, right);
            }
            break;
        case '-':
        case '*':
        case '/':
            if (left.isNumeric() && right.isNumeric()) {
                return getNumericResultType(left, right);
            }
            break;
    }
    
    throw SemanticError("Invalid operation for types", line);
}

Type SemanticAnalyzer::getNumericResultType(Type t1, Type t2) {
    const std::map<std::string, int> typeRanks = {
        {"double", 4}, 
        {"float", 3}, 
        {"int", 2}, 
        {"char", 1}
    };

    std::string type1 = t1.toString();
    std::string type2 = t2.toString();
    
    if (typeRanks.at(type1) > typeRanks.at(type2)) {
        return t1;
    }
    return t2;
}

Type SemanticAnalyzer::checkUnaryExpression(ASTNode* Node) {
    std::string op = Node->getAttribute("operator");
    ASTNode* exprASTNode = Node->getChild(0);
    Type exprType = checkExpression(exprASTNode);
    
    if (op == "-") {
        if (exprType.isNumeric()) {
            return exprType;
        }
        
        throw SemanticError("Operator - cannot be applied to type " + exprType.toString(), 
                         Node->getLine());
    }
    
    if (op == "--") {
        if (exprType.isNumeric()) {
            return exprType;
        }
        
        throw SemanticError("Operator -- cannot be applied to type " + exprType.toString(), 
                         Node->getLine());
    }

    if (op == "++") {
        if (exprType.isNumeric()) {
            return exprType;
        }
        
        throw SemanticError("Operator ++ cannot be applied to type " + exprType.toString(), 
                         Node->getLine());
    }

    if (op == "!") {
        if (exprType.isBoolean()) {
            return Type::booleanType();
        }

        throw SemanticError("Operator ! cannot be applied to type " + exprType.toString(), 
                         Node->getLine());
    }
    
    throw SemanticError("Unknown unary operator: " + op, 
                     Node->getLine());
}

Type SemanticAnalyzer::checkMethodCall(ASTNode* Node) {
    std::string methodName = Node->getAttribute("name");
    
    if (Node->getChildCount() > 0 && Node->getChild(0)->getType() == ASTNode::FIELD_ACCESS) {
        ASTNode* objectNode = Node->getChild(0);
        Type objectType = checkExpression(objectNode);
        methodName = objectNode->getAttribute("field");

        // Получаем класс объекта
        Symbol* symb = currentScope->resolve(objectNode->getChild(0)->getAttribute("name"));
        std::string s = symb->getType().toString();
        s = s.erase(s.find("<"), s.find(">") - s.find("<") + 1);
        ClassSymbol* classSymbol = dynamic_cast<ClassSymbol*>(globalScope->resolve(s));
        if (!classSymbol) {
            if(!classSymbol){
                throw SemanticError("Class '" + objectType.toString() + "' not found", Node->getLine());
            }
        }
        
        std::map<std::string, Type> genericMap;
        if (symb->getType().isGenericInstance()) {
            auto params = classSymbol->getGenericParams();
            auto args = symb->getType().getGenericArguments();
            for (size_t i = 0; i < params.size() && i < args.size(); ++i) {
                genericMap[params[i]] = args[i];
            }
        }

        // Ищем метод в классе
        Symbol* methodSymbol = classSymbol->getSymbolTable()->resolve(methodName);
        if (!methodSymbol || !methodSymbol->isFunction()) {
            throw SemanticError("Method '" + methodName + "' not found in class " + objectType.toString(), Node->getLine());
        }
        
        // Проверяем параметры
        FunctionSymbol* method = static_cast<FunctionSymbol*>(methodSymbol);
        checkMethodParameters(Node, method, genericMap);
        return method->getType();
    }

    std::vector<Type> argTypes;
    for (size_t i = 0; i < Node->getChildCount(); i++) {
        argTypes.push_back(checkExpression(Node->getChild(i)));
    }
    
    if (methodName == "System.out.println") {
        if (argTypes.size() > 1) {
            throw SemanticError("System.out.println requires exactly less than two arguments", 
                             Node->getLine());
        }
        return Type::voidType();
    }
    
    Symbol* symbol = currentScope->resolve(methodName);
    
    if (!symbol) {
        throw SemanticError("Undefined method: " + methodName, Node->getLine());
    }
    
    if (!symbol->isFunction()) {
        throw SemanticError(methodName + " is not a method", Node->getLine());
    }
    
    FunctionSymbol* method = static_cast<FunctionSymbol*>(symbol);
    
    if (method->getParameterCount() != argTypes.size()) {
        throw SemanticError("Method " + methodName + " expects " + 
                         std::to_string(method->getParameterCount()) + 
                         " arguments, but got " + std::to_string(argTypes.size()), 
                         Node->getLine());
    }
    
    for (size_t i = 0; i < argTypes.size(); i++) {
        if (!argTypes[i].isAssignableTo(method->getParameterType(i))) {
            throw SemanticError("Argument type mismatch for parameter " + 
                             std::to_string(i+1) + " of method " + methodName, 
                             Node->getChild(i)->getLine());
        }
    }
    
    return method->getType();
}

void SemanticAnalyzer::checkMethodParameters(ASTNode* callNode, FunctionSymbol* method, const std::map<std::string, Type>& genericMap) {
    size_t expectedCount = method->getParameterCount();
    size_t actualCount = callNode->getChildCount() - 1; // Первый child - объект
    
    if (expectedCount != actualCount) {
        throw SemanticError("Method expects " + std::to_string(expectedCount) + 
                          " parameters, got " + std::to_string(actualCount),
                          callNode->getLine());
    }

    for (size_t i = 0; i < actualCount; ++i) {
        Type paramType = method->getParameterType(i);
        // Заменяем generic-параметры в типе параметра
        Type resolvedParam = resolveTypeWithSubstitution(paramType, genericMap);
        Type argType = checkExpression(callNode->getChild(i + 1));

        if (!argType.isAssignableTo(resolvedParam)) {
            throw SemanticError("Parameter type mismatch: expected " + resolvedParam.toString() + 
                              ", got " + argType.toString(), callNode->getChild(i + 1)->getLine());
        }
    }

    // for (size_t i = 0; i < actualCount; i++) {
    //     Type paramType = method->getParameterType(i);
    //     Type argType = checkExpression(callNode->getChild(i+1));
        
    //     if (!argType.isAssignableTo(paramType)) {
    //         throw SemanticError("Parameter type mismatch in method call", 
    //                           callNode->getChild(i+1)->getLine());
    //     }
    // }
}

Type SemanticAnalyzer::resolveTypeWithSubstitution(const Type& type, const std::map<std::string, Type>& genericMap) {
    if (type.isGenericParam()) {
        auto it = genericMap.find(type.getGenericParamName());
        return (it != genericMap.end()) ? it->second : type;
    }
    if (type.isGenericInstance()) {
        Type base = resolveTypeWithSubstitution(type.getGenericBaseType(), genericMap);
        std::vector<Type> args;
        for (const auto& arg : type.getGenericArguments()) {
            args.push_back(resolveTypeWithSubstitution(arg, genericMap));
        }
        return Type::genericType(base, args);
    }
    return type;
}

Type SemanticAnalyzer::checkArrayAccess(ASTNode* Node) {
    ASTNode* arrayNode = Node->getChild(0);
    Type arrayType = checkExpression(arrayNode);
    
    if (!arrayType.isArray()) {
        throw SemanticError("Array access on non-array type", Node->getLine());
    }
    
    ASTNode* indexNode = Node->getChild(1);
    Type indexType = checkExpression(indexNode);
    
    if (!indexType.isInt()) {
        throw SemanticError("Array index must be numeric", indexNode->getLine());
    }
    
    return arrayType.getElementType();
}

Type SemanticAnalyzer::checkArrayInitializer(ASTNode* Node) {
    if (Node->getChildCount() == 0) {
        return Type::arrayType(Type::voidType(), 1); // Нельзя определить тип пустого массива
    }

    // Определяем тип элементов по первому элементу
    Type elementType = checkExpression(Node->getChild(0));
    
    // Проверяем совместимость всех элементов
    for (size_t i = 1; i < Node->getChildCount(); i++) {
        Type currentType = checkExpression(Node->getChild(i));
        if (!currentType.isAssignableTo(elementType)) {
            throw SemanticError("Inconsistent array element types", Node->getLine());
        }
    }

    return Type::arrayType(elementType, 1);
}

Type SemanticAnalyzer::checkFieldAccess(ASTNode* Node) {
    ASTNode* objectASTNode = Node->getChild(0);
    std::string fieldName = Node->getAttribute("field");
    
    Type objectType = checkExpression(objectASTNode);
    
    if (objectType.toString() == "System" && fieldName == "out") {
        return Type::classType("PrintStream");
    }
    
    Type baseType = objectType;
    if (objectType.isGenericInstance()) {
        baseType = objectType.getGenericBaseType();
    }
    
    if (!baseType.isClass()) {
        throw SemanticError("Cannot access field on non-class type: " + objectType.toString(),
                          objectASTNode->getLine());
    }
    
    std::string className = baseType.toString();
    Symbol* classSymbol = currentScope->resolve(className);
    
    if (!classSymbol) {
        // if(!classSymbol->isClass())
        classSymbol = globalScope->resolve(className);
        if(!classSymbol){
            throw SemanticError("Class not found: " + className, Node->getLine());
        }
    }
    ClassSymbol* cls = static_cast<ClassSymbol*>(classSymbol);
    Symbol* fieldSymbol = cls->getSymbolTable()->resolve(fieldName);
    
    if (!fieldSymbol) {
        throw SemanticError("Field " + fieldName + " not found in class " + objectType.toString(),
                          Node->getLine());
    }
    
    return fieldSymbol->getType();
}

Type SemanticAnalyzer::checkNewExpression(ASTNode* Node) {
    std::string typeName = Node->getAttribute("type");
    
    if (Node->getAttribute("isArray") == "true") {
        ASTNode* sizeASTNode = Node->getChild(0);
        Type sizeType = checkExpression(sizeASTNode);
        
        if (!sizeType.isInt()) {
            throw SemanticError("Array size must be int, found: " + sizeType.toString(), 
                             sizeASTNode->getLine());
        }
        
        Type elementType = resolveType(typeName, Node->getLine());
        return Type::arrayType(elementType);
    }
    
    Type classType = resolveType(typeName, Node->getLine());
    
    if (!classType.isClass()) {
        throw SemanticError("Cannot create an instance of non-class type: " + typeName, 
                         Node->getLine());
    }
    
    Symbol* classSymbol = currentScope->resolve(typeName);
    
    if (!classSymbol || !classSymbol->isClass()) {
        throw SemanticError("Class not found: " + typeName, 
                         Node->getLine());
    }
    
    return classType;
}

bool SemanticAnalyzer::hasReturnStatement(ASTNode* Node) {
    if (!Node) return false;
    
    for (size_t i = 0; i < Node->getChildCount(); i++) {
        ASTNode* child = Node->getChild(i);
        if (child->getType() == ASTNode::RETURN_STMT) return true;
        if (child->getType() == ASTNode::BLOCK && hasReturnStatement(child)) return true;
        if (child->getType() == ASTNode::IF_STMT) {
            bool thenHasReturn = hasReturnStatement(child->getChild(1));
            bool elseHasReturn = child->getChildCount() > 2 && 
                                hasReturnStatement(child->getChild(2));
            if (thenHasReturn && elseHasReturn) return true;
        }
    }
    
    return false;
}