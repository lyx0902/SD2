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
    int id;//状态的唯一标识符
    bool is_final;//是否为终止状态
    TokenType token_type;//终止状态对应的Token类型
    std::map<char, std::vector<std::shared_ptr<NFAState>>> transitions;//字符到状态的映射,存储从当前状态出发，经过某个字符到达的状态集合
    std::vector<std::shared_ptr<NFAState>> epsilon_transitions;//ε-闭包,存储从当前状态出发，经过ε到达的状态集合
};

// DFA状态节点
struct DFAState {
    int id;
    bool is_final;
    TokenType token_type;//终止状态对应的Token类型
    std::map<char, std::shared_ptr<DFAState>> transitions;//字符到状态的映射,存储从当前状态出发，经过某个字符到达的状态集合
};

class LexicalAnalysis {
private:
    // NFA相关
    std::shared_ptr<NFAState> nfa_start;
    std::vector<std::shared_ptr<NFAState>> nfa_states;
    //存储NFA的起始状态和所有状态
    // DFA相关
    std::shared_ptr<DFAState> dfa_start;
    std::vector<std::shared_ptr<DFAState>> dfa_states;
    //存储NFA转换成DFA的起始状态和所有状态
    // 文法规则
    struct Rule {//定义语法产生式的结构体
        std::string pattern;
        TokenType type;
    };
    std::vector<Rule> grammar_rules;//存储输入进来的文法规则

    // 私有方法
    void buildNFA(const std::string& grammar_file);
    void convertNFAtoDFA();//使用子集法
    std::shared_ptr<NFAState> createNFAForPattern(const std::string& pattern, TokenType type);
    std::set<std::shared_ptr<NFAState>> getEpsilonClosure(const std::set<std::shared_ptr<NFAState>>& states);
    std::set<std::shared_ptr<NFAState>> move(const std::set<std::shared_ptr<NFAState>>& states, char c);
    //shared_ptr是C++11引入的智能指针，用于自动管理内存，避免内存泄漏，特性是共享对象，对于自动机的状态节点使用shared_ptr可以方便地管理状态之间的引用关系，避免手动管理内存带来的复杂性和错误
    //引用计数，只有当引用计数为0时才会释放内存
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