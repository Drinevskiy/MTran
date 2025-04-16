#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include <string>
#include <sstream>
#include <set>
#include <map>
#include <vector>
#include "utils.hpp"
// class ASTNode;  // Forward declaration

class CodeGenerator {
private:
    std::stringstream code;
    std::set<std::string> includes;
    std::string indentation;
    int indentLevel;
    std::map<std::string, std::string> typeMap;

    void increaseIndent();
    void decreaseIndent();
    std::string mapType(const std::string& javaType);
    void initTypeMap();
    void generateCode(ASTNode* node);
    
    // Code generation methods
    void generateProgram(ASTNode* node);
    void generateClassDeclaration(ASTNode* node);
    void generateMethodDeclaration(ASTNode* node);
    void generateBlock(ASTNode* node);
    void generateVariableDeclaration(ASTNode* node);
    void generateArrayInitialization(ASTNode* node);
    void generateIfStatement(ASTNode* node);
    void generateWhileLoop(ASTNode* node);
    void generateDoWhileLoop(ASTNode* node);
    void generateForLoop(ASTNode* node);
    void generateSwitchStatement(ASTNode* node);
    void generateCase(ASTNode* node);
    void generateDefault(ASTNode* node);
    void generateReturnStatement(ASTNode* node);
    void generateMethodCall(ASTNode* node);
    void generateExpressionStatement(ASTNode* node);
    void generateBinaryExpression(ASTNode* node);
    void generateUnaryExpression(ASTNode* node);
    void generateLiteral(ASTNode* node);
    void generateVariable(ASTNode* node);
    void generateArrayAccess(ASTNode* node);
    void generateFieldAccess(ASTNode* node);
    void generateAssignment(ASTNode* node);

public:
    CodeGenerator();
    std::string generate(ASTNode* root);
};

#endif // CODEGENERATOR_HPP