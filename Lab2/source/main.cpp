#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cctype>
#include <sstream>

// Типы токенов
enum TokenType {
    KEYWORD,
    IDENTIFIER,
    NUMBER,
    FLOAT_NUMBER,
    STRING_LITERAL,
    CHAR_LITERAL,
    OPERATOR,
    UNKNOWN,
    ERROR
};

// Структура токена
struct Token {
    TokenType type;
    std::string value;
    int line;
    int id;

    std::string typeToString() const {
        switch (type) {
            case KEYWORD: return "KEYWORD";
            case IDENTIFIER: return "IDENTIFIER";
            case NUMBER: return "NUMBER";
            case FLOAT_NUMBER: return "FLOAT_NUMBER";
            case STRING_LITERAL: return "STRING_LITERAL";
            case CHAR_LITERAL: return "CHAR_LITERAL";
            case OPERATOR: return "OPERATOR";
            case ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};

// Список ключевых слов Java
const std::unordered_set<std::string> javaKeywords = {
    "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char", "class", "const",
    "continue", "default", "do", "double", "double[]", "else", "enum", "extends", "final", "finally", "float",
    "float[]", "for", "goto", "if", "implements", "import", "instanceof", "int", "int[]", "interface", "long", 
    "long[]", "native", "new", "null", "package", "private", "protected", "public", "return", "short", "short[]", 
    "static", "strictfp", "super", "switch", "synchronized", "this", "throw", "throws", "transient", "try",
    "void", "volatile", "while", "true", "false", "String", "String[]", "ArrayList", "HashMap", "HashSet"
};

// Список операторов Java
const std::unordered_set<std::string> javaOperators = {
    "+", "-", "*", "/", "%", "++", "--", "==", "!=", ">", "<", ">=", "<=", "&&", "||", "!", "=",
    "+=", "-=", "*=", "/=", "%=", "&", "|", "^", "~", "<<", ">>", ">>>", "?", ":", "::", ".", ",",
    ";", "(", ")", "{", "}", "[", "]"
};

// Класс лексического анализатора
class Lexer {
public:
    explicit Lexer(const std::string& sourceCode) 
        : source(sourceCode), pos(0), line(1), nextTokenId(1) {}

    // Основная функция для токенизации
    std::vector<Token> tokenize() {
        tokens.clear();
        while (pos < source.length()) {
            char currentChar = source[pos];

            if (std::isspace(currentChar)) {
                if (currentChar == '\n') {
                    line++; // Увеличиваем номер строки
                }
                pos++; // Пропускаем пробелы
            } else if (std::isalpha(currentChar) || currentChar == '_') {
                tokens.push_back(consumeIdentifierOrKeyword());
            } else if (std::isdigit(currentChar) || 
                        (currentChar == '.' && pos + 1 < source.length() && std::isdigit(source[pos + 1]))) {
                tokens.push_back(consumeNumber());
            } else if (currentChar == '"') {
                tokens.push_back(consumeStringLiteral());
            } else if (currentChar == '\'') {
                tokens.push_back(consumeCharLiteral());
            } else if (currentChar == '/' && (pos + 1 < source.length() && (source[pos + 1] == '/' || source[pos + 1] == '*'))) {
                consumeComment(); // Игнорируем комментарии
            } else if (isOperator(std::string(1, currentChar))) {
                tokens.push_back(consumeOperator());
            } else {
                tokens.push_back(createToken(ERROR, std::string(1, currentChar)));
                pos++;
            }
        }
        return tokens;
    }

private:
    std::string source;
    size_t pos;
    int line;
    int nextTokenId;
    std::vector<Token> tokens;
    std::unordered_map<std::string, int> tokenIds; // Хранение ID токенов

    bool isOperator(const std::string& op) {
        return javaOperators.find(op) != javaOperators.end();
    }

    Token createToken(TokenType type, const std::string& value) {
        if (tokenIds.find(value) == tokenIds.end()) {
            tokenIds[value] = nextTokenId++;
        }
        return {type, value, line, tokenIds[value]};
    }

    Token consumeIdentifierOrKeyword() {
        size_t start = pos;
        while (pos < source.length() && (std::isalnum(source[pos]) || source[pos] == '_' || source[pos] == '[' || source[pos] == ']')) {
            pos++;
        }
        std::string word = source.substr(start, pos - start);
        return createToken(javaKeywords.find(word) != javaKeywords.end() ? KEYWORD : IDENTIFIER, word);
    }

    Token consumeNumber() {
        size_t start = pos;
        bool isFloat = false;

        if (pos > 0 && (source[pos - 1] == '-' || source[pos - 1] == '+') && 
            (pos - 1 == 0 || std::isspace(source[pos - 2]) || source[pos - 2] == '=')) {
            tokens.pop_back();
            start = pos - 1;
        }

        while (pos < source.length() && (std::isdigit(source[pos]) || source[pos] == '.')) {
            if (source[pos] == '.') {
                if (isFloat) {
                    return createToken(ERROR, source.substr(start, pos - start));
                }
                isFloat = true;
            }
            pos++;
        }

        if (pos < source.length() && (source[pos] == 'e' || source[pos] == 'E')) {
            isFloat = true;
            pos++;
            if (pos < source.length() && (source[pos] == '+' || source[pos] == '-')) {
                pos++;
            }
            if (pos >= source.length() || !std::isdigit(source[pos])) {
                return createToken(ERROR, source.substr(start, pos - start));
            }
            while (pos < source.length() && std::isdigit(source[pos])) {
                pos++;
            }
        }

        if (pos < source.length() && (source[pos] == 'f' || source[pos] == 'F' || source[pos] == 'd' || source[pos] == 'D')) {
            isFloat = true;
            pos++;
        }

        return createToken(isFloat ? FLOAT_NUMBER : NUMBER, source.substr(start, pos - start));
    }

    Token consumeStringLiteral() {
        size_t start = pos;
        pos++;
        while (pos < source.length() && source[pos] != '"') {
            if (source[pos] == '\\' && pos + 1 < source.length()) {
                pos += 2;
            } else {
                pos++;
            }
        }
        if (pos < source.length() && source[pos] == '"') {
            pos++;
            return createToken(STRING_LITERAL, source.substr(start, pos - start));
        }
        return createToken(ERROR, source.substr(start, pos - start));
    }

    Token consumeCharLiteral() {
        size_t start = pos;
        pos++;
        if (pos < source.length() && source[pos] == '\\') {
            pos += 2;
        } else {
            pos++;
        }
        if (pos < source.length() && source[pos] == '\'') {
            pos++;
            return createToken(CHAR_LITERAL, source.substr(start, pos - start));
        }
        return createToken(ERROR, source.substr(start, pos - start));
    }

    void consumeComment() {
        if (source[pos + 1] == '/') {
            pos += 2;
            while (pos < source.length() && source[pos] != '\n') {
                pos++;
            }
        } else if (source[pos + 1] == '*') {
            pos += 2;
            while (pos + 1 < source.length() && !(source[pos] == '*' && source[pos + 1] == '/')) {
                pos++;
            }
            if (pos + 1 < source.length()) {
                pos += 2;
            }
        }
    }

    Token consumeOperator() {
        size_t start = pos;
        while (pos < source.length() && isOperator(source.substr(start, pos - start + 1))) {
            pos++;
        }
        return createToken(OPERATOR, source.substr(start, pos - start));
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>\n";
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile) {
        std::cerr << "Error: Could not open file " << argv[1] << "\n";
        return 1;
    }

    std::string sourceCode((std::istreambuf_iterator<char>(inputFile)),
                           std::istreambuf_iterator<char>());

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    std::string output_file_name = "D:\\Study\\6_semestr\\MTran\\output.txt";
    std::ofstream outputFile(output_file_name);
    if (!outputFile) {
        std::cerr << "Error: Could not open output file\n";
        return 1;
    }

    for (const Token& token : tokens) {
        outputFile << "Token: " << token.typeToString()
                   << " Lexem: @" << token.value << "@"
                   << " Line: " << token.line
                   << " Id: " << token.id << "\n";
    }

    std::cout << "Tokens written to output.txt\n";
    return 0;
}