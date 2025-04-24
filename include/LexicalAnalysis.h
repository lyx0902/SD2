#ifndef SD2_LEXICALANALYSIS_H
#define SD2_LEXICALANALYSIS_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

// Token类型枚举
enum TokenType {
    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    LIMITER,
    OPERATOR,
    INVALID
};

// Token结构体
struct Token {
    TokenType type;
    std::string value;
    int line_number;
};

// NFA状态节点
struct NFAState {
    int id;
    bool is_final;
    TokenType token_type;
    std::map<char, std::vector<std::shared_ptr<NFAState>>> transitions;
    std::vector<std::shared_ptr<NFAState>> epsilon_transitions;
};

// DFA状态节点
struct DFAState {
    int id;
    bool is_final;
    TokenType token_type;
    std::map<char, std::shared_ptr<DFAState>> transitions;
};

class LexicalAnalysis {
private:
    // NFA相关
    std::shared_ptr<NFAState> nfa_start;
    std::vector<std::shared_ptr<NFAState>> nfa_states;

    // DFA相关
    std::shared_ptr<DFAState> dfa_start;
    std::vector<std::shared_ptr<DFAState>> dfa_states;

    // 文法规则
    struct Rule {
        std::string pattern;
        TokenType type;
    };
    std::vector<Rule> grammar_rules;

    // 私有方法
    void buildNFA(const std::string& grammar_file);
    void convertNFAtoDFA();
    std::shared_ptr<NFAState> createNFAForPattern(const std::string& pattern, TokenType type);
    std::set<std::shared_ptr<NFAState>> getEpsilonClosure(const std::set<std::shared_ptr<NFAState>>& states);
    std::set<std::shared_ptr<NFAState>> move(const std::set<std::shared_ptr<NFAState>>& states, char c);

public:
    LexicalAnalysis();
    ~LexicalAnalysis();

    // 加载文法文件并构建自动机
    bool loadGrammar(const std::string& grammar_file);

    // 分析源代码
    std::vector<Token> analyze(const std::string& source_code);

    // 从文件读取源代码
    static std::string readSourceFile(const std::string& filename);

    // 输出Token序列
    static void printTokens(const std::vector<Token>& tokens);
};

#endif //SD2_LEXICALANALYSIS_H