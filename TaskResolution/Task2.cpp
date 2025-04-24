#include "SyntaxAnalyzer.h"
#include "LexicalAnalyzer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>

int main() {
    SetConsoleOutputCP(65001);
    // 1. 创建并初始化语法分析器
    SyntaxAnalyzer analyzer;

    // 2. 加载文法规则
    if (!analyzer.loadGrammar("../TestCase/Task2Case/grammar_4.txt")) {
        std::cerr << "Failed to load grammar.txt file!" << std::endl;
        return 1;
    }

    // 3. 读取源代码文件并进行词法分析
    std::ifstream source_file("../TestCase/Task2Case/input_4_2.txt");
    if (!source_file.is_open()) {
        std::cerr << "Error: Could not open source code file!" << std::endl;
        return 1;
    }
    std::string source_code((std::istreambuf_iterator<char>(source_file)),
                           std::istreambuf_iterator<char>());

    // 执行词法分析
    std::vector<Token> lexical_tokens = LexicalAnalyzer::analyze(source_code);
    LexicalAnalyzer::processTokens(lexical_tokens);

    // 将词法分析的结果转换为语法分析器需要的格式
    std::vector<TokenInfo> syntax_tokens;
    for (const auto& token : lexical_tokens) {
        TokenInfo info;
        info.type = token.type;
        info.value = token.value;  // 直接使用token的值
        info.lineNumber = token.line_number;
        syntax_tokens.push_back(info);
    }

    // 4. 执行语法分析
    try {
        bool success = analyzer.analyze(syntax_tokens);
        if (success) {
            std::cout << "Syntax analysis completed successfully!" << std::endl;
            analyzer.outputResult("..\\TestCase\\analysis1_result.txt");
        } else {
            std::cout <<"Syntax analysis failed due to invalid input or grammar violations!" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during syntax analysis: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}