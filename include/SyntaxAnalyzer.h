#ifndef SD2_SYNTAXANALYZER_H
#define SD2_SYNTAXANALYZER_H

#include "LR1Item.h"
#include <map>
#include <vector>
#include "LexicalAnalyzer.h"

struct TokenInfo {
    TokenType type;
    std::string value;
    int lineNumber;
};

class SyntaxAnalyzer {
public:
    SyntaxAnalyzer();

    bool loadGrammar(const std::string& filename);
    bool analyze(const std::vector<TokenInfo>& tokens);
    void outputResult(const std::string& filename) const;
    void printTokensAndFirstSets() const;  // 打印词法token和First集
    void printLR1Table() const;           // 打印LR(1)分析表
    void printItemSets() const;

private:
    std::vector<Production> productions;
    std::set<Symbol> terminals;
    std::set<Symbol> nonTerminals;
    std::map<int, std::set<LR1Item>> states;
    std::map<std::pair<int, Symbol>, std::string> actionTable;
    std::map<std::pair<int, Symbol>, int> gotoTable;

    // First集合映射表：符号 -> First集合
    std::map<Symbol, std::set<Symbol>> firstSets;

    // First集合相关的辅助函数
    void initializeFirstSets();
    std::set<Symbol> getFirstOfSymbolSequence(const std::vector<Symbol>& symbols);
    bool addToFirstSet(const Symbol& symbol, const Symbol& firstSymbol);

    // Follow集合映射表：符号 -> Follow集合
    std::map<Symbol, std::set<Symbol>> followSets;

    // Follow集合相关的辅助函数
    void initializeFollowSets();
    bool addToFollowSet(const Symbol& symbol, const Symbol& followSymbol);
    bool addToFollowSet(const Symbol& symbol, const std::set<Symbol>& followSymbols);

    void computeFirstSets();
    void computeFollowSets();
    void buildLR1Automaton();
    void constructParsingTables();
    std::set<LR1Item> closure(const std::set<LR1Item>& items);
    std::set<LR1Item> goTo(const std::set<LR1Item>& items, const Symbol& symbol);
};

#endif //SD2_SYNTAXANALYZER_H