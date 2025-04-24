#ifndef SD2_LEXICAL_ANALYZER_H
#define SD2_LEXICAL_ANALYZER_H

#include <string>
#include <vector>

// Token类型枚举
enum TokenType {
    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    LIMITER,
    OPERATOR,
    INVALID,
    COMPLEX
};

// Token 结构体
struct Token {
    TokenType type;
    std::string value;
    int line_number;
};

// 词法分析器类
class LexicalAnalyzer {
public:
    // 主要的词法分析函数
    static std::vector<Token> analyze(const std::string& source_code);
    
    // 处理Token序列的函数
    static void processTokens(std::vector<Token>& tokens);

private:
    // 辅助函数
    static TokenType get_token_type(const std::string& str);
};

#endif // SD2_LEXICAL_ANALYZER_H