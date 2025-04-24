#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <string>
#include <cctype>
#include <LexicalAnalysis.h>
int main() {
    LexicalAnalysis analyzer;

    // 加载文法
    if (!analyzer.loadGrammar("../TestCase/Task1Case/grammar.txt")) {
        std::cerr << "Failed to load grammar" << std::endl;
        return 1;
    }

    // 读取源代码
    std::string source = analyzer.readSourceFile("../TestCase/Task1Case/source_1.txt");

    // 分析源代码
    auto tokens = analyzer.analyze(source);

    // 输出结果
    analyzer.printTokens(tokens);

    return 0;
}