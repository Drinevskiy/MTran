#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <cassert>
#include <sstream>   

class Type;
class Symbol;
class SymbolTable;
class FunctionSymbol;
class ClassSymbol;
class Node;

class Type {
public:
    enum Kind {
        VOID,
        PRIMITIVE,
        ARRAY,
        CLASS,
        GENERIC_PARAM,   
        GENERIC_INSTANCE
    };

    enum PrimitiveKind {
        BOOLEAN,
        CHAR,
        INT,
        FLOAT,
        DOUBLE,
        STRING
    };

    Type(Kind kind = VOID, PrimitiveKind primitiveKind = BOOLEAN, const std::string& className = "");
    
    static Type voidType();
    static Type booleanType();
    static Type charType();
    static Type intType();
    static Type floatType();
    static Type doubleType();
    static Type stringType();
    static Type arrayType(const Type& baseType, int dimension = 1);
    static Type genericParamType(const std::string& paramName);
    static Type genericType(const Type& baseType, const std::vector<Type>& typeArgs);
    static Type classType(const std::string& name);

    bool isVoid() const;
    bool isPrimitive() const;
    bool isArray() const;
    bool isClass() const;
    bool isBoolean() const;
    bool isNumeric() const;
    bool isInt() const;
    bool isChar() const;
    bool isString() const;
    bool isGenericParam() const;
    bool isGenericInstance() const;

    Type getElementType() const;
    Type getGenericBaseType() const;
    std::vector<Type> getGenericArguments() const;
    std::string getGenericParamName() const;

    bool isAssignableTo(const Type& other) const;
    
    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const;
    
    std::string toString() const;

private:
    Kind kind;
    PrimitiveKind primitiveKind;
    std::string className;
    int arrayDimension;
    std::string genericParamName;              
    std::shared_ptr<Type> genericBaseType;     
    std::vector<Type> genericTypeArguments;
};

class Symbol {
public:
    enum Kind {
        VARIABLE,
        FUNCTION,
        CLASS, 
        TYPE_PARAM
    };

    Symbol(const std::string& name, const Type& type, Kind kind);
    virtual ~Symbol();

    const std::string& getName() const;
    const Type& getType() const;
    Kind getKind() const;

    bool isVariable() const;
    bool isFunction() const;
    bool isClass() const;

protected:
    std::string name;
    Type type;
    Kind kind;
};

class FunctionSymbol : public Symbol {
public:
    FunctionSymbol(const std::string& name, const Type& returnType);
    
    void addParameter(const std::string& name, const Type& type);
    size_t getParameterCount() const;
    const std::vector<std::string>& getParameterNames() const;
    const std::vector<Type>& getParameterTypes() const;
    const Type& getParameterType(size_t index) const;

private:
    std::vector<std::string> paramNames;
    std::vector<Type> paramTypes;
};

class ClassSymbol : public Symbol {
public:
    ClassSymbol(const std::string& name);
    SymbolTable* getSymbolTable() const;
    void setGeneric(bool flag) ;
    void addGenericParam(const std::string& param) ;
    bool isGenericClass() const;
    std::vector<std::string> getGenericParams() const;
private:
    std::unique_ptr<SymbolTable> symbolTable;
    bool isGeneric = false;
    std::vector<std::string> genericParams;
};

class SymbolTable {
public:
    SymbolTable(SymbolTable* parent = nullptr);
    
    void define(Symbol* symbol);
    Symbol* resolve(const std::string& name) const;
    Symbol* resolveLocally(const std::string& name) const;
    SymbolTable* getParent() const;

private:
    SymbolTable* parent;
    std::map<std::string, std::unique_ptr<Symbol>> symbols;
};

class SemanticError : public std::runtime_error {
public:
    SemanticError(const std::string& message, int line);
    
    int getLine() const;
    const std::string& getErrorMessage() const;

private:
    static std::string buildMessage(const std::string& message, int line);
    
    int line;
    std::string message;
};

class ASTNode {
public:
    enum NodeType {
        PROGRAM,
        CLASS_DECL,
        METHOD_DECL,
        PARAMETER_LIST,
        PARAMETER,
        FIELD_DECL,
        VARIABLE_DECL,
        ARRAY_INIT,
        BLOCK,
        IF_STMT,
        WHILE_STMT,
        DO_WHILE_STMT,
        FOR_STMT,
        SWITCH_STMT,
        CASE,
        DEFAULT,
        RETURN_STMT,
        EXPRESSION_STMT,
        BINARY_EXPR,
        UNARY_EXPR,
        BREAK_STMT,
        CONTINUE_STMT,
        LITERAL,
        VARIABLE,
        METHOD_CALL,
        ARRAY_ACCESS,
        FIELD_ACCESS,
        NEW_EXPR,
        ASSIGNMENT
    };

    ASTNode(NodeType type, int line);
    virtual ~ASTNode();

    NodeType getType() const;
    int getLine() const;

    void addChild(ASTNode* child);
    ASTNode* getChild(size_t index) const;
    size_t getChildCount() const;

    void print(const std::string& prefix = "", bool isLast = true);
    
    void setAttribute(const std::string& key, const std::string& value);
    std::string getAttribute(const std::string& key) const;

private:
    std::string toString();
    NodeType type;
    int line;
    std::vector<ASTNode*> children;
    std::map<std::string, std::string> attributes;
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    void analyze(ASTNode* ast);
    bool hasErrors() const;
    const std::vector<SemanticError>& getErrors() const;

private:
    void initializeBuiltins();
    void enterScope();
    void exitScope();
    Type resolveType(const std::string& typeName, int line);
    
    void visitNode(ASTNode* node);
    void visitProgram(ASTNode* node);
    void visitClassDeclaration(ASTNode* node);
    void visitMethodDeclaration(ASTNode* node);
    void visitFieldDeclaration(ASTNode* node);
    void visitVariableDeclaration(ASTNode* node);
    void visitBlock(ASTNode* node);
    void visitIfStatement(ASTNode* node);
    void visitWhileStatement(ASTNode* node);
    void visitDoWhileStatement(ASTNode* node);
    void visitForStatement(ASTNode* node);
    void visitSwitchStatement(ASTNode* node);
    void visitCase(ASTNode* node);
    void visitArrayInitialization(ASTNode* varNode, const Type& arrayType);
    void checkBreakValidity(ASTNode* node);
    void checkContinueValidity(ASTNode* node);
    void visitDefault(ASTNode* node);
    void visitReturnStatement(ASTNode* node);
    void visitExpressionStatement(ASTNode* node);
    void visitAssignment(ASTNode* node);
    
    Type checkAssignmentTarget(ASTNode* node);
    Type checkExpression(ASTNode* node);
    Type checkLiteral(ASTNode* node);
    Type checkVariable(ASTNode* node);
    Type checkBinaryExpression(ASTNode* node);
    bool isLValue(ASTNode* node);
    Type checkOperationType(char op, Type left, Type right, int line);
    Type getNumericResultType(Type t1, Type t2);
    Type checkUnaryExpression(ASTNode* node);
    Type checkMethodCall(ASTNode* node);
    void checkMethodParameters(ASTNode* callNode, FunctionSymbol* method, const std::map<std::string, Type>& genericMap);
    Type resolveTypeWithSubstitution(const Type& type, const std::map<std::string, Type>& genericMap);
    Type checkArrayAccess(ASTNode* node);
    Type checkArrayInitializer(ASTNode* node);
    Type checkFieldAccess(ASTNode* node);
    Type checkNewExpression(ASTNode* node);
    
    bool hasReturnStatement(ASTNode* node);

    std::unique_ptr<SymbolTable> globalScope;
    SymbolTable* currentScope;
    std::vector<std::unique_ptr<SymbolTable>> scopes;
    ClassSymbol* currentClass;
    FunctionSymbol* currentMethod;
    std::vector<SemanticError> errors;
    enum ContextType { LOOP_CONTEXT, SWITCH_CONTEXT };
    std::stack<ContextType> contextStack;
    std::vector<Type> switchConditionStack;
};

#endif // UTILS_HPP