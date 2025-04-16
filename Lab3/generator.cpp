#include "generator.hpp"

void CodeGenerator::increaseIndent() {
    indentLevel++;
    indentation = std::string(indentLevel * 4, ' ');
}

void CodeGenerator::decreaseIndent() {
    if (indentLevel > 0) {
        indentLevel--;
        indentation = std::string(indentLevel * 4, ' ');
    }
}

std::string CodeGenerator::mapType(const std::string& javaType) {
    // Обработка массивов
    if (javaType.find("[]") != std::string::npos) {
        std::string baseType = javaType.substr(0, javaType.find("[]"));
        includes.insert("#include <vector>");
        return "std::vector<" + mapType(baseType) + ">";
    }

    // Обработка базовых типов
    if (typeMap.find(javaType) != typeMap.end()) {
        return typeMap[javaType];
    }

    // Обработка ArrayList
    if (javaType.find("ArrayList") != std::string::npos) {
        includes.insert("#include <vector>");
        size_t start = javaType.find('<');
        size_t end = javaType.find('>');
        if (start != std::string::npos && end != std::string::npos) {
            std::string typeParam = javaType.substr(start + 1, end - start - 1);
            return "std::vector<" + mapType(typeParam) + ">";
        }
        return "std::vector<void*>";
    }

    // Обработка HashMap
    if (javaType.find("HashMap") != std::string::npos) {
        includes.insert("#include <unordered_map>");
        size_t start = javaType.find('<');
        size_t end = javaType.find('>');
        if (start != std::string::npos && end != std::string::npos) {
            std::string params = javaType.substr(start + 1, end - start - 1);
            size_t comma = params.find(',');
            if (comma != std::string::npos) {
                std::string keyType = params.substr(0, comma);
                std::string valueType = params.substr(comma + 1);
                
                // Удаление лишних пробелов
                size_t keyStart = keyType.find_first_not_of(" \t");
                size_t keyEnd = keyType.find_last_not_of(" \t");
                keyType = keyType.substr(keyStart, keyEnd - keyStart + 1);
                
                size_t valueStart = valueType.find_first_not_of(" \t");
                size_t valueEnd = valueType.find_last_not_of(" \t");
                valueType = valueType.substr(valueStart, valueEnd - valueStart + 1);
                
                return "std::unordered_map<" + mapType(keyType) + ", " + mapType(valueType) + ">";
            }
        }
        return "std::unordered_map<std::string, int>";
    }

    // Для пользовательских типов возвращаем как есть
    return javaType;
}

void CodeGenerator::initTypeMap() {
    typeMap["int"] = "int";
    typeMap["float"] = "float";
    typeMap["double"] = "double";
    typeMap["char"] = "char";
    typeMap["boolean"] = "bool";
    typeMap["String"] = "std::string";
    typeMap["Integer"] = "int";
    typeMap["void"] = "void";
}

CodeGenerator::CodeGenerator() : indentLevel(0), indentation("") {
    initTypeMap();
}

std::string CodeGenerator::generate(ASTNode* root) {
    // Сбрасываем состояние
    code.str("");
    code.clear();
    includes.clear();
    indentLevel = 0;
    indentation = "";

    // Добавляем стандартные заголовки
    includes.insert("#include <iostream>");
    includes.insert("#include <string>");

    // Генерируем код
    generateCode(root);

    // Составляем итоговый код с заголовками
    std::stringstream result;
    for (const auto& include : includes) {
        result << include << std::endl;
    }
    result << std::endl << code.str();
    return result.str();
}

void CodeGenerator::generateCode(ASTNode* node) {
    if (!node) return;

    switch (node->getType()) {
        case ASTNode::PROGRAM:
            generateProgram(node);
            break;
        case ASTNode::CLASS_DECL:
            generateClassDeclaration(node);
            break;
        case ASTNode::METHOD_DECL:
            generateMethodDeclaration(node);
            break;
        case ASTNode::BLOCK:
            generateBlock(node);
            break;
        case ASTNode::VARIABLE_DECL:
            generateVariableDeclaration(node);
            break;
        case ASTNode::ARRAY_INIT:
            generateArrayInitialization(node);
            break;
        case ASTNode::IF_STMT:
            generateIfStatement(node);
            break;
        case ASTNode::WHILE_STMT:
            generateWhileLoop(node);
            break;
        case ASTNode::DO_WHILE_STMT:
            generateDoWhileLoop(node);
            break;
        case ASTNode::FOR_STMT:
            generateForLoop(node);
            break;
        case ASTNode::SWITCH_STMT:
            generateSwitchStatement(node);
            break;
        case ASTNode::CASE:
            generateCase(node);
            break;
        case ASTNode::DEFAULT:
            generateDefault(node);
            break;
        case ASTNode::RETURN_STMT:
            generateReturnStatement(node);
            break;
        case ASTNode::METHOD_CALL:
            generateMethodCall(node);
            break;
        case ASTNode::EXPRESSION_STMT:
            generateExpressionStatement(node);
            break;
        case ASTNode::BINARY_EXPR:
            generateBinaryExpression(node);
            break;
        case ASTNode::UNARY_EXPR:
            generateUnaryExpression(node);
            break;
        case ASTNode::LITERAL:
            generateLiteral(node);
            break;
        case ASTNode::VARIABLE:
            generateVariable(node);
            break;
        case ASTNode::ARRAY_ACCESS:
            generateArrayAccess(node);
            break;
        case ASTNode::FIELD_ACCESS:
            generateFieldAccess(node);
            break;
        case ASTNode::ASSIGNMENT:
            generateAssignment(node);
            break;
        case ASTNode::BREAK_STMT:
            code << indentation << "break;" << std::endl;
            break;
        case ASTNode::CONTINUE_STMT:
            code << indentation << "continue;" << std::endl;
            break;
        default:
            std::cerr << "Неизвестный тип узла в генерации кода: " << node->getType() << std::endl;
            break;
    }
}

void CodeGenerator::generateProgram(ASTNode* node) {
    for (size_t i = 0; i < node->getChildCount(); ++i) {
        generateCode(node->getChild(i));
    }
}

void CodeGenerator::generateClassDeclaration(ASTNode* node) {
    // В C++ мы не будем оборачивать все в класс
    // Просто генерируем содержимое класса
    for (size_t i = 0; i < node->getChildCount(); ++i) {
        ASTNode* child = node->getChild(i);
        if (child->getType() == ASTNode::BLOCK) {
            // Обрабатываем детей блока, пропуская сам блок
            for (size_t j = 0; j < child->getChildCount(); ++j) {
                generateCode(child->getChild(j));
            }
        } else {
            generateCode(child);
        }
    }
}

void CodeGenerator::generateMethodDeclaration(ASTNode* node) {
    std::string methodName = node->getAttribute("name");
    std::string returnType = node->getAttribute("returnType");
    
    // Преобразуем возвращаемый тип из Java в C++
    std::string cppReturnType = mapType(returnType);
    
    // Особая обработка для main
    if (methodName == "main") {
        code << "int main(int argc, char* argv[])" << std::endl;
    } else {
        code << cppReturnType << " " << methodName << "(";
        
        // Генерация параметров
        for (size_t i = 0; i < node->getChildCount(); ++i) {
            if (node->getChild(i)->getType() == ASTNode::PARAMETER_LIST) {
                ASTNode* paramList = node->getChild(i);
                for (size_t j = 0; j < paramList->getChildCount(); ++j) {
                    if (j > 0) code << ", ";
                    
                    ASTNode* param = paramList->getChild(j);
                    std::string paramName = param->getAttribute("name");
                    std::string paramType = param->getAttribute("type");
                    code << mapType(paramType) << " " << paramName;
                }
            }
        }
        
        code << ")" << std::endl;
    }
    
    // Генерация тела метода
    for (size_t i = 0; i < node->getChildCount(); ++i) {
        ASTNode* child = node->getChild(i);
        if (child->getType() == ASTNode::BLOCK) {
            generateCode(child);
        }
    }
    
    // Для main добавляем return 0
    if (methodName == "main") {
        bool hasReturn = false;
        
        // Проверяем, есть ли уже return
        if (node->getChildCount() > 0) {
            ASTNode* block = nullptr;
            for (size_t i = 0; i < node->getChildCount(); ++i) {
                if (node->getChild(i)->getType() == ASTNode::BLOCK) {
                    block = node->getChild(i);
                    break;
                }
            }
            
            if (block && block->getChildCount() > 0) {
                ASTNode* lastStmt = block->getChild(block->getChildCount() - 1);
                if (lastStmt->getType() == ASTNode::RETURN_STMT) {
                    hasReturn = true;
                }
            }
        }
        
        // if (!hasReturn) {
        //     code << indentation << "return 0;" << std::endl;
        // }
    }
}

void CodeGenerator::generateBlock(ASTNode* node) {
    code << "{" << std::endl;
    increaseIndent();
    
    for (size_t i = 0; i < node->getChildCount(); ++i) {
        generateCode(node->getChild(i));
    }
    
    decreaseIndent();
    code << indentation << "}" << std::endl;
}

void CodeGenerator::generateVariableDeclaration(ASTNode* node) {
    std::string varName = node->getAttribute("name");
    std::string varType = node->getAttribute("type");
    
    // Специальная обработка для ArrayList и HashMap
    if (varType.find("ArrayList") != std::string::npos) {
        includes.insert("#include <vector>");
        size_t start = varType.find('<');
        size_t end = varType.find('>');
        std::string elemType = "void*";
        if (start != std::string::npos && end != std::string::npos) {
            elemType = varType.substr(start + 1, end - start - 1);
        }
        code << "std::vector<" << mapType(elemType) << "> " << varName;
        
        // По умолчанию инициализируем, если нет инициализатора
        if (node->getChildCount() == 0) {
            // code << "()";
        } else {
            code << " = ";
            generateCode(node->getChild(0));
        }
    } else if (varType.find("HashMap") != std::string::npos) {
        includes.insert("#include <unordered_map>");
        size_t start = varType.find('<');
        size_t end = varType.find('>');
        std::string keyType = "std::string";
        std::string valueType = "int";
        if (start != std::string::npos && end != std::string::npos) {
            std::string params = varType.substr(start + 1, end - start - 1);
            size_t comma = params.find(',');
            if (comma != std::string::npos) {
                keyType = params.substr(0, comma);
                valueType = params.substr(comma + 1);
                // Удаляем лишние пробелы
                size_t keyStart = keyType.find_first_not_of(" \t");
                size_t keyEnd = keyType.find_last_not_of(" \t");
                keyType = keyType.substr(keyStart, keyEnd - keyStart + 1);
                
                size_t valueStart = valueType.find_first_not_of(" \t");
                size_t valueEnd = valueType.find_last_not_of(" \t");
                valueType = valueType.substr(valueStart, valueEnd - valueStart + 1);
            }
        }
        code << "std::unordered_map<" << mapType(keyType) << ", " << mapType(valueType) << "> " << varName;
        
        // По умолчанию инициализируем, если нет инициализатора
        if (node->getChildCount() == 0) {
            // code << "()";
        } else {
            code << " = ";
            generateCode(node->getChild(0));
        }
    } else {
        // Обычное объявление переменной
        // code << indentation << mapType(varType) << " " << varName;
        code << mapType(varType) << " " << varName;
        
        // Если есть инициализатор
        if (node->getChildCount() > 0) {
            if (node->getChild(0)->getType() == ASTNode::ARRAY_INIT) {
                // Инициализация массива
                code << " = {";
                ASTNode* arrayInit = node->getChild(0);
                for (size_t i = 0; i < arrayInit->getChildCount(); ++i) {
                    if (i > 0) code << ", ";
                    generateCode(arrayInit->getChild(i));
                }
                code << "}";
            } else {
                // Обычная инициализация
                code << " = ";
                generateCode(node->getChild(0));
            }
        }
    }
    
    // code << ";" << std::endl;
}

void CodeGenerator::generateArrayInitialization(ASTNode* node) {
    code << "{";
    for (size_t i = 0; i < node->getChildCount(); ++i) {
        if (i > 0) code << ", ";
        generateCode(node->getChild(i));
    }
    code << "}";
}

void CodeGenerator::generateIfStatement(ASTNode* node) {
    code << indentation << "if (";
    generateCode(node->getChild(0)); // Условие
    code << ") ";
    
    // Ветка then
    generateCode(node->getChild(1));
    
    // Ветка else (если есть)
    if (node->getChildCount() > 2) {
        code << indentation << "else ";
        generateCode(node->getChild(2));
    }
}

void CodeGenerator::generateWhileLoop(ASTNode* node) {
    code << indentation << "while (";
    generateCode(node->getChild(0)); // Условие
    code << ") ";
    
    // Тело
    generateCode(node->getChild(1));
}

void CodeGenerator::generateDoWhileLoop(ASTNode* node) {
    code << indentation << "do ";
    
    // Тело
    generateCode(node->getChild(0));
    
    code << indentation << "while (";
    generateCode(node->getChild(1)); // Условие
    code << ");" << std::endl;
}

void CodeGenerator::generateForLoop(ASTNode* node) {
    code << indentation << "for (";
    
    // Инициализация
    if (node->getChild(0)->getType() == ASTNode::VARIABLE_DECL) {
        std::string varName = node->getChild(0)->getAttribute("name");
        std::string varType = node->getChild(0)->getAttribute("type");
        
        code << mapType(varType) << " " << varName;
        
        if (node->getChild(0)->getChildCount() > 0) {
            code << " = ";
            generateCode(node->getChild(0)->getChild(0));
        }
    } else {
        generateCode(node->getChild(0));
    }
    
    code << "; ";
    
    // Условие
    generateCode(node->getChild(1));
    
    code << "; ";
    
    // Обновление
    generateCode(node->getChild(2));
    
    code << ") ";
    
    // Тело
    generateCode(node->getChild(3));
}

void CodeGenerator::generateSwitchStatement(ASTNode* node) {
    code << indentation << "switch (";
    generateCode(node->getChild(0)); // Выражение switch
    code << ") {" << std::endl;
    
    // Case операторы обрабатываются дочерними узлами
    for (size_t i = 1; i < node->getChildCount(); ++i) {
        generateCode(node->getChild(i));
    }
    
    code << indentation << "}" << std::endl;
}

void CodeGenerator::generateCase(ASTNode* node) {
    code << indentation << "case ";
    generateCode(node->getChild(0)); // Значение case
    code << ":" << std::endl;
    
    increaseIndent();
    
    // Тело case
    for (size_t i = 1; i < node->getChildCount(); ++i) {
        generateCode(node->getChild(i));
    }
    
    decreaseIndent();
}

void CodeGenerator::generateDefault(ASTNode* node) {
    code << indentation << "default:" << std::endl;
    
    increaseIndent();
    
    // Тело default
    for (size_t i = 0; i < node->getChildCount(); ++i) {
        generateCode(node->getChild(i));
    }
    
    decreaseIndent();
}

void CodeGenerator::generateReturnStatement(ASTNode* node) {
    code << "return";
    
    if (node->getChildCount() > 0) {
        code << " ";
        generateCode(node->getChild(0));
    }
    
    // code << ";" << std::endl;
}

void CodeGenerator::generateMethodCall(ASTNode* node) {
    std::string methodName = node->getAttribute("name");
    
    // Специальная обработка для System.out.println
    if (methodName == "System.out.println") {
        code << "std::cout";
        for (size_t i = 0; i < node->getChildCount(); ++i) {
            code << " << ";
            generateCode(node->getChild(i));
        }
        code << " << std::endl";
        // code << indentation << "std::cout << ";
        
        // if (node->getChildCount() > 0) {
        //     generateCode(node->getChild(0));
        //     code << " << ";
        // }
        
        // code << "std::endl;" << std::endl;
    } else if (methodName.empty() && node->getChildCount() > 0 && node->getChild(0)->getType() == ASTNode::FIELD_ACCESS) {
        // Вызов метода на объекте через field access
        ASTNode* fieldAccess = node->getChild(0);
        
        std::string objField = fieldAccess->getAttribute("field");
        
        // Специальные методы для контейнеров
        if (objField == "add" || objField == "push") {
            generateCode(fieldAccess->getChild(0));
            code << ".push_back(";
            
            if (node->getChildCount() > 1) {
                generateCode(node->getChild(1));
            }
            
            code << ")";
        } else if (objField == "get") {
            generateCode(fieldAccess->getChild(0));
            code << "[";
            
            if (node->getChildCount() > 1) {
                generateCode(node->getChild(1));
            }
            
            code << "]";
        } else if (objField == "put") {
            generateCode(fieldAccess->getChild(0));
            code << "[";
            
            if (node->getChildCount() > 1) {
                generateCode(node->getChild(1));
            }
            
            code << "] = ";
            
            if (node->getChildCount() > 2) {
                generateCode(node->getChild(2));
            }
            // code << ";\n";
        } else if (objField == "size") {
            generateCode(fieldAccess->getChild(0));
            code << ".size()";
        } else {
            // Стандартный вызов метода
            generateCode(fieldAccess->getChild(0));
            code << "." << objField << "(";
            
            for (size_t i = 1; i < node->getChildCount(); ++i) {
                if (i > 1) code << ", ";
                generateCode(node->getChild(i));
            }
            
            code << ")";
        }
    } else {
        // Обычный вызов метода
        if (!methodName.empty()) {
            code << methodName << "(";
        } else {
            code << "(";
        }
        
        // Аргументы
        for (size_t i = 0; i < node->getChildCount(); ++i) {
            if (i > 0) code << ", ";
            generateCode(node->getChild(i));
        }
        
        code << ")";
    }
}

void CodeGenerator::generateExpressionStatement(ASTNode* node) {
    code << indentation;
    generateCode(node->getChild(0));
    code << ";" << std::endl;
}

void CodeGenerator::generateBinaryExpression(ASTNode* node) {
    std::string op = node->getAttribute("operator");
    
    if (op == "+") {
        bool leftIsString = node->getChild(0)->getAttribute("literalType") == "string";
        bool rightIsString = node->getChild(1)->getAttribute("literalType") == "string";
        
        // Если хотя бы один операнд строка - генерируем через операторы потока
        if (leftIsString || rightIsString) {
            generateCode(node->getChild(0));
            code << " << ";
            generateCode(node->getChild(1));
            return;
        }
    }

    // Обработка специальных случаев для операторов типа Java +=
    if (op == "+=" || op == "-=" || op == "*=" || op == "/=") {
        generateCode(node->getChild(0));
        code << " " << op << " ";
        generateCode(node->getChild(1));
        return;
    }
    
    code << "(";
    generateCode(node->getChild(0)); // Левый операнд
    
    code << " " << op << " ";
    
    generateCode(node->getChild(1)); // Правый операнд
    code << ")";
}

void CodeGenerator::generateUnaryExpression(ASTNode* node) {
    std::string op = node->getAttribute("operator");
    
    if (op == "++" || op == "--") {
        // По умолчанию считаем префиксным
        code << op;
        generateCode(node->getChild(0));
    } else {
        code << op;
        generateCode(node->getChild(0));
    }
}

void CodeGenerator::generateLiteral(ASTNode* node) {
    std::string literalType = node->getAttribute("literalType");
    std::string value = node->getAttribute("value");
    
    if (literalType == "string") {
        code << value;  // Теперь просто выводим строку без преобразований
    // }
    // if (literalType == "string") {
    //     // Удаляем кавычки, если они есть
    //     if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
    //         code << value;
    //     } else {
    //         code << "\"" << value << "\"";
    //     }
    } else if (literalType == "char") {
        code << value;
    } else if (literalType == "boolean") {
        // В C++ литералы булевого типа в нижнем регистре
        if (value == "true" || value == "True") {
            code << "true";
        } else if (value == "false" || value == "False") {
            code << "false";
        } else {
            code << value;
        }
    } else {
        code << value;
    }
}

void CodeGenerator::generateVariable(ASTNode* node) {
    code << node->getAttribute("name");
}

void CodeGenerator::generateArrayAccess(ASTNode* node) {
    generateCode(node->getChild(0)); // Массив
    code << "[";
    generateCode(node->getChild(1)); // Индекс
    code << "]";
}

void CodeGenerator::generateFieldAccess(ASTNode* node) {
    // Генерируем объект только если это реальный доступ к объекту
    if (node->getChildCount() > 0) {
        generateCode(node->getChild(0)); // Объект
        code << ".";
    }
    code << node->getAttribute("field");
}

void CodeGenerator::generateAssignment(ASTNode* node) {
    generateCode(node->getChild(0)); // Цель
    code << " = ";
    generateCode(node->getChild(1)); // Значение
}