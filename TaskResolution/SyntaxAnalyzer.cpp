#include "SyntaxAnalyzer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>
#include <iomanip>

SyntaxAnalyzer::SyntaxAnalyzer() {
    // 添加增广文法的开始符号
    Symbol startSymbol("S'", NON_TERMINAL);
    nonTerminals.insert(startSymbol);
}

bool SyntaxAnalyzer::loadGrammar(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    // 清空之前的数据
    productions.clear();
    terminals.clear();
    nonTerminals.clear();
    states.clear();
    actionTable.clear();
    gotoTable.clear();

    // 读取第一行获取原始文法的开始符号
    std::string line;
    std::getline(file, line);
    std::istringstream firstLine(line);
    std::string leftStr, arrow;
    firstLine >> leftStr >> arrow;
    Symbol originalStart(leftStr, NON_TERMINAL);

    // 添加增广文法：S' -> E
    Symbol startSymbol("S'", NON_TERMINAL);
    nonTerminals.insert(startSymbol);
    productions.emplace_back(startSymbol, std::vector<Symbol>{originalStart}, 0);

    // 重置文件指针到开始
    file.clear();
    file.seekg(0);

    int productionIndex = 1;  // 从1开始，因为0已经用于增广产生式

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string leftStr, arrow;
        iss >> leftStr >> arrow;

        if (arrow != "->") continue;

        // 创建左侧非终结符
        Symbol left(leftStr, NON_TERMINAL);
        nonTerminals.insert(left);

        // 读取右侧符号
        std::vector<Symbol> rightSymbols;
        std::string symbol;
        while (iss >> symbol) {
            SymbolType type = isupper(symbol[0]) ? NON_TERMINAL : TERMINAL;
            rightSymbols.emplace_back(symbol, type);
            if (type == TERMINAL) terminals.insert(Symbol(symbol, TERMINAL));
        }

        // 添加产生式
        productions.emplace_back(left, rightSymbols, productionIndex++);
    }

    // 添加终止符号到终结符集合
    terminals.insert(Symbol("#", END_MARKER));

    // 构建First和Follow集
    computeFirstSets();
    computeFollowSets();

    // 构建分析表
    constructParsingTables();

    return true;
}

void SyntaxAnalyzer::initializeFirstSets() {
    // 初始化所有终结符的First集
    for (const auto& terminal : terminals) {
        firstSets[terminal].insert(terminal);
    }

    // 初始化所有非终结符的First集为空集
    for (const auto& nonTerminal : nonTerminals) {
        firstSets[nonTerminal] = std::set<Symbol>();
    }

    // 处理空符号
    Symbol epsilon("ε", EPSILON);
    firstSets[epsilon].insert(epsilon);
}

bool SyntaxAnalyzer::addToFirstSet(const Symbol& symbol, const Symbol& firstSymbol) {
    // 如果First集中已经有这个符号，返回false表示没有变化
    if (firstSets[symbol].count(firstSymbol) > 0) {
        return false;
    }
    // 否则添加到First集中，返回true表示有变化
    firstSets[symbol].insert(firstSymbol);
    return true;
}

void SyntaxAnalyzer::computeFirstSets() {
    // 步骤1: 初始化First集
    initializeFirstSets();

    // 步骤2: 重复计算直到没有变化
    bool changed;
    do {
        changed = false;

        // 遍历所有产生式
        for (const auto& prod : productions) {
            const Symbol& leftSymbol = prod.left;

            // 如果右部为空，将ε加入到左部符号的First集中
            if (prod.right.empty()) {
                Symbol epsilon("ε", EPSILON);
                changed |= addToFirstSet(leftSymbol, epsilon);
                continue;
            }

            // 处理右部符号序列
            bool allCanBeEmpty = true;
            for (size_t i = 0; i < prod.right.size(); ++i) {
                const Symbol& currentSymbol = prod.right[i];
                allCanBeEmpty = false;

                // 如果是终结符，直接添加到左部的First集
                if (currentSymbol.type == TERMINAL) {
                    changed |= addToFirstSet(leftSymbol, currentSymbol);
                    break;
                }

                // 将当前符号的First集（除去ε）添加到左部符号的First集
                for (const auto& firstSymbol : firstSets[currentSymbol]) {
                    if (firstSymbol.type != EPSILON) {
                        changed |= addToFirstSet(leftSymbol, firstSymbol);
                    }
                }

                // 检查当前符号是否可以推导出ε
                bool hasEpsilon = false;
                for (const auto& firstSymbol : firstSets[currentSymbol]) {
                    if (firstSymbol.type == EPSILON) {
                        hasEpsilon = true;
                        allCanBeEmpty = true;
                        break;
                    }
                }

                // 如果当前符号不能推导出ε，停止处理
                if (!hasEpsilon) {
                    break;
                }
            }

            // 如果所有符号都可以推导出ε，将ε加入到左部符号的First集
            if (allCanBeEmpty) {
                Symbol epsilon("ε", EPSILON);
                changed |= addToFirstSet(leftSymbol, epsilon);
            }
        }
    } while (changed); // 当没有变化时停止迭代
}

// 计算符号序列的First集
std::set<Symbol> SyntaxAnalyzer::getFirstOfSymbolSequence(const std::vector<Symbol>& symbols) {
    std::set<Symbol> result;

    if (symbols.empty()) {
        Symbol epsilon("ε", EPSILON);
        result.insert(epsilon);
        return result;
    }

    bool allCanBeEmpty = true;

    for (const auto& symbol : symbols) {
        allCanBeEmpty = false;

        // 添加当前符号的First集（除去ε）
        for (const auto& firstSymbol : firstSets[symbol]) {
            if (firstSymbol.type != EPSILON) {
                result.insert(firstSymbol);
            }
        }

        // 检查是否可以继续处理下一个符号
        bool hasEpsilon = false;
        for (const auto& firstSymbol : firstSets[symbol]) {
            if (firstSymbol.type == EPSILON) {
                hasEpsilon = true;
                allCanBeEmpty = true;
                break;
            }
        }

        if (!hasEpsilon) break;
    }

    // 如果所有符号都可以推导出ε，将ε加入结果集
    if (allCanBeEmpty) {
        Symbol epsilon("ε", EPSILON);
        result.insert(epsilon);
    }

    return result;
}
void SyntaxAnalyzer::initializeFollowSets() {
    // 为所有非终结符初始化空的Follow集
    for (const auto& nonTerminal : nonTerminals) {
        followSets[nonTerminal] = std::set<Symbol>();
    }

    // 为开始符号的Follow集加入结束符号#
    Symbol startSymbol("S'", NON_TERMINAL);
    Symbol endMarker("#", END_MARKER);
    followSets[startSymbol].insert(endMarker);
}

bool SyntaxAnalyzer::addToFollowSet(const Symbol& symbol, const Symbol& followSymbol) {
    // 不将ε加入Follow集
    if (followSymbol.type == EPSILON) {
        return false;
    }

    // 如果Follow集中已经有这个符号，返回false表示没有变化
    if (followSets[symbol].count(followSymbol) > 0) {
        return false;
    }

    // 否则添加到Follow集中，返回true表示有变化
    followSets[symbol].insert(followSymbol);
    return true;
}

bool SyntaxAnalyzer::addToFollowSet(const Symbol& symbol, const std::set<Symbol>& followSymbols) {
    bool changed = false;
    for (const auto& followSymbol : followSymbols) {
        // 不将ε加入Follow集
        if (followSymbol.type != EPSILON) {
            changed |= addToFollowSet(symbol, followSymbol);
        }
    }
    return changed;
}

void SyntaxAnalyzer::computeFollowSets() {
    // 步骤1: 初始化Follow集
    initializeFollowSets();

    // 步骤2: 重复计算直到没有变化
    bool changed;
    do {
        changed = false;

        // 遍历所有产生式
        for (const auto& prod : productions) {
            // 遍历产生式右部的每个符号
            for (size_t i = 0; i < prod.right.size(); i++) {
                const Symbol& currentSymbol = prod.right[i];

                // 只处理非终结符
                if (currentSymbol.type != NON_TERMINAL) {
                    continue;
                }

                // 如果不是最后一个符号
                if (i < prod.right.size() - 1) {
                    // 获取后续符号序列的First集
                    std::vector<Symbol> restSymbols(prod.right.begin() + i + 1, prod.right.end());
                    std::set<Symbol> firstOfRest = getFirstOfSymbolSequence(restSymbols);

                    // 将First集加入到当前非终结符的Follow集
                    changed |= addToFollowSet(currentSymbol, firstOfRest);

                    // 如果后续符号序列可以推导出ε，则将产生式左部的Follow集加入到当前符号的Follow集
                    bool allCanBeEmpty = true;
                    for (const auto& symbol : restSymbols) {
                        bool hasEpsilon = false;
                        for (const auto& firstSymbol : firstSets[symbol]) {
                            if (firstSymbol.type == EPSILON) {
                                hasEpsilon = true;
                                break;
                            }
                        }
                        if (!hasEpsilon) {
                            allCanBeEmpty = false;
                            break;
                        }
                    }
                    if (allCanBeEmpty) {
                        changed |= addToFollowSet(currentSymbol, followSets[prod.left]);
                    }
                }
                // 如果是最后一个符号，或者后面的符号都可以推导出ε
                else {
                    // 将产生式左部的Follow集加入到当前符号的Follow集
                    changed |= addToFollowSet(currentSymbol, followSets[prod.left]);
                }
            }
        }
    } while (changed); // 当没有变化时停止迭代
}

std::set<LR1Item> SyntaxAnalyzer::closure(const std::set<LR1Item>& items) {
    std::map<std::pair<int, size_t>, std::set<Symbol>> mergedItems;

    // 初始化：合并输入项目的向前看符号
    for (const auto& item : items) {
        auto key = std::make_pair(item.prod.index, item.dotPosition);
        mergedItems[key].insert(item.lookahead.begin(), item.lookahead.end());
    }

    bool changed;
    do {
        changed = false;
        std::map<std::pair<int, size_t>, std::set<Symbol>> newItems = mergedItems;

        // 对每个已有项目
        for (const auto& [key, lookAhead] : mergedItems) {
            const Production& prod = productions[key.first];
            size_t dotPos = key.second;

            // 如果点号后面是非终结符
            if (dotPos < prod.right.size() && prod.right[dotPos].type == NON_TERMINAL) {
                // 计算后续符号串的First集
                std::vector<Symbol> beta(prod.right.begin() + dotPos + 1, prod.right.end());
                std::set<Symbol> firstBeta = getFirstOfSymbolSequence(beta);

                // 如果First(β)为空，使用当前的向前看符号
                if (firstBeta.empty()) {
                    firstBeta = lookAhead;
                } else {
                    // 移除epsilon并添加向前看符号
                    firstBeta.erase(Symbol("ε", EPSILON));
                    firstBeta.insert(lookAhead.begin(), lookAhead.end());
                }

                // 为该非终结符的每个产生式添加新项目
                for (const auto& newProd : productions) {
                    if (newProd.left == prod.right[dotPos]) {
                        auto newKey = std::make_pair(newProd.index, 0);
                        size_t oldSize = newItems[newKey].size();
                        newItems[newKey].insert(firstBeta.begin(), firstBeta.end());
                        if (newItems[newKey].size() > oldSize) {
                            changed = true;
                        }
                    }
                }
            }
        }
        mergedItems = newItems;
    } while (changed);

    // 构造结果集
    std::set<LR1Item> result;
    for (const auto& [key, lookAhead] : mergedItems) {
        result.insert(LR1Item(productions[key.first], key.second, lookAhead));
    }

    return result;
}

std::set<LR1Item> SyntaxAnalyzer::goTo(const std::set<LR1Item>& items, const Symbol& symbol) {
    std::map<std::pair<int, size_t>, std::set<Symbol>> mergedItems;

    // 收集所有可以移进的项目
    for (const auto& item : items) {
        if (!item.isComplete() && item.getNextSymbol() == symbol) {
            auto key = std::make_pair(item.prod.index, item.dotPosition + 1);
            mergedItems[key].insert(item.lookahead.begin(), item.lookahead.end());
        }
    }

    // 构造新项目集
    std::set<LR1Item> newItems;
    for (const auto& [key, lookAhead] : mergedItems) {
        newItems.insert(LR1Item(productions[key.first], key.second, lookAhead));
    }

    // 返回新项目集的闭包
    return closure(newItems);
}

void SyntaxAnalyzer::buildLR1Automaton() {
    states.clear();
    actionTable.clear();
    gotoTable.clear();

    // 创建初始项目集
    const Production& augmentedProd = productions[0];
    std::set<Symbol> initialLookahead = {Symbol("#", END_MARKER)};
    LR1Item startItem(augmentedProd, 0, initialLookahead);
    states[0] = closure({startItem});

    std::queue<int> workList;
    workList.push(0);

    while (!workList.empty()) {
        int currentState = workList.front();
        workList.pop();

        // 对当前状态中的每个项目
        for (const auto& item : states[currentState]) {
            if (item.isComplete()) {
                // 特殊处理接受状态
                if (item.prod.index == 0) {
                    for (const auto& la : item.lookahead) {
                        if (la.type == END_MARKER) {
                            actionTable[{currentState, la}] = "acc";
                        }
                    }
                } else {
                    // 规约动作
                    for (const auto& la : item.lookahead) {
                        actionTable[{currentState, la}] = "r" + std::to_string(item.prod.index);
                    }
                }
                continue;
            }

            // 获取下一个符号
            Symbol next = item.getNextSymbol();

            // 计算GOTO结果
            auto nextState = goTo(states[currentState], next);
            if (nextState.empty()) continue;

            // 查找或创建新状态
            int newStateId = -1;
            for (const auto& [id, existing] : states) {
                if (existing == nextState) {
                    newStateId = id;
                    break;
                }
            }
            if (newStateId == -1) {
                newStateId = states.size();
                states[newStateId] = nextState;
                workList.push(newStateId);
            }

            // 更新分析表
            if (next.type == TERMINAL || next.type == END_MARKER) {
                actionTable[{currentState, next}] = "s" + std::to_string(newStateId);
            } else {
                gotoTable[{currentState, next}] = newStateId;
            }
        }
    }
}

void SyntaxAnalyzer::constructParsingTables() {
    std::cout << "=== 启动LR(1)分析器 ===";
    // 首先构建LR(1)自动机，这会同时构建action和goto表
    buildLR1Automaton();
    // 检查并处理冲突
    for (const auto& [stateSymbol, action] : actionTable) {
        int state = stateSymbol.first;
        Symbol symbol = stateSymbol.second;

        // 检查是否存在移入-规约冲突或规约-规约冲突
        if (action[0] == 's') {  // 移入动作
            for (const auto& [otherStateSymbol, otherAction] : actionTable) {
                if (otherStateSymbol.first == state &&
                    otherStateSymbol.second == symbol &&
                    otherAction[0] == 'r') {
                    // 这里可以实现冲突解决策略
                    // 默认采用移入优先策略
                    std::cout << "Shift-reduce conflict in state " << state
                              << " on symbol " << symbol.name << std::endl;
                }
            }
        }
    }
}

bool SyntaxAnalyzer::analyze(const std::vector<TokenInfo>& tokens) {
    // 打印输入字符串的token序列分析
    std::cout << "\n=== Token合法性检查 ===\n";
    std::cout << "Token序列: ";
    for (const auto& token : tokens) {
        std::cout << token.value << " ";
    }
    std::cout << "\n";

    // 检查每个token是否在文法的终结符集合中
    bool allTokensValid = true;
    for (const auto& token : tokens) {
        Symbol symbol(token.value, TERMINAL);
        bool isValid = false;

        // 检查token是否在终结符集合中
        for (const auto& terminal : terminals) {
            if (terminal.name == token.value) {
                isValid = true;
                break;
            }
        }

        std::cout << "Token '" << token.value << "' - ";
        if (isValid) {
            std::cout << "有效\n";
        } else {
            std::cout << "无效 (不在文法的终结符集合中)\n";
            allTokensValid = false;
        }
    }

    if (!allTokensValid) {
        std::cout << "\n错误：输入序列包含不在文法中的终结符\n";
        return false;
    }

    std::cout << "\n结论：输入序列在词法上是合法的\n";

    std::vector<int> stateStack = {0};
    std::vector<Symbol> symbolStack;
    std::vector<Symbol> inputSymbols;

    for (const auto& token : tokens) {
        Symbol symbol(token.value, TERMINAL);
        inputSymbols.push_back(symbol);
    }
    inputSymbols.push_back(Symbol("#", END_MARKER));

    size_t inputPos = 0;
    bool analysisSuccess = false;
    int step = 1;

    // 打印原有的分析信息
    printTokensAndFirstSets();

    // 打印分析过程表头
    std::cout << "\n=== LR(1)字符串输入分析过程 ===\n";
    std::cout << "\n步骤  | 状态栈               | 符号栈               | 输入串               | 动作\n";
    std::cout << "----------------------------------------------------------------------------------------\n";

    while (true) {
        int currentState = stateStack.back();
        Symbol currentSymbol = inputSymbols[inputPos];

        // 构建状态栈字符串
        std::string stateStackStr;
        for (int state : stateStack) {
            stateStackStr += std::to_string(state) + " ";
        }

        // 构建符号栈字符串
        std::string symbolStackStr;
        for (const auto& sym : symbolStack) {
            symbolStackStr += sym.name + " ";
        }

        // 构建剩余输入串
        std::string inputStr;
        for (size_t i = inputPos; i < inputSymbols.size(); i++) {
            inputStr += inputSymbols[i].name;
            if (i < inputSymbols.size() - 1) inputStr += " ";
        }
        auto actionIt = actionTable.find({currentState, currentSymbol});
        if (actionIt == actionTable.end()) {
            std::cout << "Error: No action defined for state " << currentState
                      << " on symbol " << currentSymbol.name << std::endl;
            return false;
        }

        std::string action = actionIt->second;
        std::string actionStr;

        if (action[0] == 's') {
            int nextState = std::stoi(action.substr(1));
            actionStr = "移进s" + std::to_string(nextState);
            stateStack.push_back(nextState);
            symbolStack.push_back(currentSymbol);
            inputPos++;
        }
        else if (action[0] == 'r') {
            int prodIndex = std::stoi(action.substr(1));
            const Production& prod = productions[prodIndex];
            actionStr = "规约r" + std::to_string(prodIndex) + "(" +
                       prod.left.name + "->" +
                       [&prod]() {
                           std::string s;
                           for (const auto& sym : prod.right) s += sym.name;
                           return s;
                       }() + ")";

            for (size_t i = 0; i < prod.right.size(); i++) {
                stateStack.pop_back();
                symbolStack.pop_back();
            }

            symbolStack.push_back(prod.left);
            int previousState = stateStack.back();
            auto gotoIt = gotoTable.find({previousState, prod.left});
            if (gotoIt == gotoTable.end()) {
                std::cout << "Error: No goto defined for state " << previousState
                          << " on symbol " << prod.left.name << std::endl;
                return false;
            }
            stateStack.push_back(gotoIt->second);
        }
        else if (action == "acc") {
            printf("%2d   | %-20s| %-20s| %-20s| 接受\n",
                   step, stateStackStr.c_str(), symbolStackStr.c_str(), inputStr.c_str());
            analysisSuccess = true;
            break;
        }
        else {
            std::cout << "Error: Invalid action " << action << std::endl;
            return false;
        }

        printf("%2d   | %-20s| %-20s| %-20s| %s\n",
               step, stateStackStr.c_str(), symbolStackStr.c_str(), inputStr.c_str(), actionStr.c_str());
        step++;
    }

    if (analysisSuccess) {
        printLR1Table();
        printItemSets();
        return true;
    }
    return false;
}

void SyntaxAnalyzer::outputResult(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open output file " << filename << std::endl;
        return;
    }

    // 1. 输出文法产生式
    file << "Grammar Productions:\n";
    file << "-------------------\n";
    for (const auto& prod : productions) {
        file << prod.index << ": " << prod.toString() << "\n";
    }
    file << "\n";

    // 2. 输出First集
    file << "First Sets:\n";
    file << "-----------\n";
    for (const auto& nonTerminal : nonTerminals) {
        file << "FIRST(" << nonTerminal.name << ") = { ";
        for (const auto& symbol : firstSets.at(nonTerminal)) {
            file << symbol.name << " ";
        }
        file << "}\n";
    }
    file << "\n";

    // 3. 输出Follow集
    file << "Follow Sets:\n";
    file << "-----------\n";
    for (const auto& nonTerminal : nonTerminals) {
        file << "FOLLOW(" << nonTerminal.name << ") = { ";
        for (const auto& symbol : followSets.at(nonTerminal)) {
            file << symbol.name << " ";
        }
        file << "}\n";
    }
    file << "\n";

    // 4. 输出LR(1)项目集族
    file << "LR(1) Item Sets:\n";
    file << "---------------\n";
    for (const auto& [stateId, itemSet] : states) {
        file << "State " << stateId << ":\n";
        for (const auto& item : itemSet) {
            file << "    " << item.toString() << "\n";
        }
        file << "\n";
    }
    file.close();
}

void SyntaxAnalyzer::printTokensAndFirstSets() const {
    // 1. 显示词法分析token令牌表
    std::cout << "\n=== 词法分析Token令牌表 ===\n";
    for (const auto& terminal : terminals) {
        if (terminal.type == END_MARKER) continue;
        std::cout << terminal.name << " ";
    }

    // 2. 显示First集
    std::cout << "\n=== 非终结符First集 ===\n";
    for (const auto& nonTerminal : nonTerminals) {
        std::cout << "FIRST(" << nonTerminal.name << ") = { ";
        for (const auto& symbol : firstSets.at(nonTerminal)) {
            std::cout << symbol.name << " ";
        }
        std::cout << "}\n";
    }
    std::cout << std::flush;
}

void SyntaxAnalyzer::printLR1Table() const {
    std::cout << "\n=== LR(1)分析表 ===\n\n";

    // 获取终结符和非终结符列表
    std::vector<Symbol> termList, nonTermList;
    for (const auto& term : terminals) {
        if (term.type != END_MARKER) {
            termList.push_back(term);
        }
    }
    termList.push_back(Symbol("#", END_MARKER));

    for (const auto& nonTerm : nonTerminals) {
        if (nonTerm.name != "S'") {
            nonTermList.push_back(nonTerm);
        }
    }

    // 打印表头
    std::cout << "State\t";
    std::cout << "ACTION\t";
    for (const auto& term : termList) {
        std::cout << term.name << "\t";
    }
    std::cout << "GOTO\t";
    for (size_t i = 0; i < nonTermList.size(); ++i) {
        std::cout << nonTermList[i].name;
        if (i < nonTermList.size() - 1) {
            std::cout << "\t";
        }
    }
    std::cout << "\n";

    // 打印每个状态的动作和转移
    int maxState = states.size() - 1;
    for (int state = 0; state <= maxState; ++state) {
        std::cout << state << "\t\t|\t\t";

        // 检查该状态下是否有规约动作以及规约产生式
        std::string reductionAction;
        bool hasReduction = false;
        for (const auto& term : termList) {
            if (term.type != END_MARKER) {
                auto it = actionTable.find({state, term});
                if (it != actionTable.end() && it->second[0] == 'r') {
                    reductionAction = it->second;
                    hasReduction = true;
                    break;
                }
            }
        }

        // 打印ACTION部分
        for (const auto& term : termList) {
            auto it = actionTable.find({state, term});
            if (it != actionTable.end()) {
                if (term.type == END_MARKER && it->second[0] == 'r' && hasReduction) {
                    // 如果是#号且是规约动作，检查是否与其他终结符的规约相同
                    if (it->second == reductionAction) {
                        std::cout << "\t";  // 相同则不显示
                    } else {
                        std::cout << it->second << "\t";  // 不同则显示
                    }
                } else {
                    std::cout << it->second << "\t";
                }
            } else {
                std::cout << "\t";
            }
        }

        std::cout << "|\t\t";

        // 打印GOTO部分
        for (const auto& nonTerm : nonTermList) {
            auto it = gotoTable.find({state, nonTerm});
            if (it != gotoTable.end()) {
                std::cout << it->second << "\t";
            } else {
                std::cout << "\t";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    std::cout << std::flush;
}
void SyntaxAnalyzer::printItemSets() const {
    std::cout << "\n=== LR(1)项目集 ===\n";
    for (const auto& [stateId, itemSet] : states) {
        std::cout << "\nI" << stateId << ":\n";
        std::cout << "----------------\n";

        for (const auto& item : itemSet) {
            std::cout << item.prod.left.name << " -> ";

            // 打印点号之前的符号
            for (size_t i = 0; i < item.dotPosition; i++) {
                std::cout << item.prod.right[i].name << " ";
            }

            // 打印点号
            std::cout << "· ";

            // 打印点号之后的符号
            for (size_t i = item.dotPosition; i < item.prod.right.size(); i++) {
                std::cout << item.prod.right[i].name << " ";
            }

            // 处理向前看符号集合
            std::cout << ", { ";
            bool hasOnlyEndMarker = true;
            bool hasEndMarker = false;

            // 首先检查向前看符号集合的内容
            for (const auto& symbol : item.lookahead) {
                if (symbol.type == END_MARKER) {
                    hasEndMarker = true;
                } else {
                    hasOnlyEndMarker = false;
                }
            }

            // 打印向前看符号
            for (const auto& symbol : item.lookahead) {
                if (symbol.type == END_MARKER) {
                    if (hasOnlyEndMarker) {
                        std::cout << symbol.name << " ";
                    }
                } else {
                    std::cout << symbol.name << " ";
                }
            }
            std::cout << "}\n";
        }
    }
    std::cout << std::endl;
}
