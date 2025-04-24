#include "LexicalAnalysis.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <queue>
#include <algorithm>
#include <stack>

LexicalAnalysis::LexicalAnalysis() : nfa_start(nullptr), dfa_start(nullptr) {}

LexicalAnalysis::~LexicalAnalysis() = default;

bool LexicalAnalysis::loadGrammar(const std::string& grammar_file) {
    std::ifstream file(grammar_file);
    if (!file.is_open()) {
        std::cerr << "无法打开文法文件: " << grammar_file << std::endl;
        return false;
    }

    // 清除旧的状态
    nfa_states.clear();
    dfa_states.clear();
    grammar_rules.clear();

    std::cout << "=== 开始加载文法规则 ===" << std::endl;

    // 读取文法规则
    std::string line;
    int rule_count = 0;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string type, arrow, pattern;
        iss >> type >> arrow >> pattern;

        // 忽略箭头符号，只处理类型和模式
        if (arrow != "->") continue;

        TokenType token_type;
        switch (type[0]) {  // 只取首字母
            case 'K': token_type = KEYWORD; break;
            case 'I': token_type = IDENTIFIER; break;
            case 'C': token_type = CONSTANT; break;
            case 'L': token_type = LIMITER; break;
            case 'O': token_type = OPERATOR; break;
            case 'E': token_type = INVALID; break;
            default: continue;
        }

        grammar_rules.push_back({pattern, token_type});
        ++rule_count;
    }
    std::cout << "总共加载了 " << rule_count << " 条规则" << std::endl;
    std::cout << "=== 文法规则加载完成 ===" << std::endl;

    // 构建NFA
    buildNFA(grammar_file);

    // 输出NFA状态信息
    std::cout << "\n=== NFA状态信息 ===" << std::endl;
    std::cout << "NFA状态总数: " << nfa_states.size() << std::endl;
    std::cout << "起始状态ID: " << nfa_start->id << std::endl;

    // 转换为DFA
    convertNFAtoDFA();

    // 输出DFA状态信息
    std::cout << "\n=== DFA状态信息 ===" << std::endl;
    std::cout << "DFA状态总数: " << dfa_states.size() << std::endl;
    std::cout << "起始状态ID: " << dfa_start->id << std::endl;

    return true;
}

void LexicalAnalysis::buildNFA(const std::string& grammar_file) {
    // 创建NFA的起始状态
    nfa_start = std::make_shared<NFAState>();
    nfa_start->id = 0;
    nfa_start->is_final = false;
    nfa_states.push_back(nfa_start);

    // 为每个规则创建NFA子图并与起始状态连接
    for (const auto& rule : grammar_rules) {
        auto sub_nfa = createNFAForPattern(rule.pattern, rule.type);
        nfa_start->epsilon_transitions.push_back(sub_nfa);
    }
}

std::shared_ptr<NFAState> LexicalAnalysis::createNFAForPattern(
    const std::string& pattern, TokenType type) {
    auto start = std::make_shared<NFAState>();
    auto end = std::make_shared<NFAState>();

    start->id = nfa_states.size();
    end->id = nfa_states.size() + 1;
    end->is_final = true;
    end->token_type = type;  // 设置终态的token类型

    nfa_states.push_back(start);
    nfa_states.push_back(end);

    // 如果是简单的关键字、运算符或限定符
    if (pattern.find('[') == std::string::npos) {
        std::shared_ptr<NFAState> current = start;

        for (size_t i = 0; i < pattern.length(); i++) {
            if (pattern[i] == '\\' && i + 1 < pattern.length()) {
                i++;
                continue;
            }
            auto next = (i == pattern.length() - 1) ? end :
                       std::make_shared<NFAState>();
            if (i != pattern.length() - 1) {
                next->id = nfa_states.size();
                nfa_states.push_back(next);
            }
            current->transitions[pattern[i]].push_back(next);
            current = next;
        }
    }
    // 处理标识符和常量的正则表达式规则
    else {
        // 特殊处理标识符规则 [a-zA-Z_][a-zA-Z0-9_]*
        if (type == IDENTIFIER) {
            auto middle = std::make_shared<NFAState>();
            middle->id = nfa_states.size();
            nfa_states.push_back(middle);
            // 首字符: a-z, A-Z, _
            for (char c = 'a'; c <= 'z'; c++)
                start->transitions[c].push_back(middle);
            for (char c = 'A'; c <= 'Z'; c++)
                start->transitions[c].push_back(middle);
            start->transitions['_'].push_back(middle);

            // 后续字符: a-z, A-Z, 0-9, _
            for (char c = 'a'; c <= 'z'; c++) {
                middle->transitions[c].push_back(middle);
                middle->transitions[c].push_back(end);
            }
            for (char c = 'A'; c <= 'Z'; c++) {
                middle->transitions[c].push_back(middle);
                middle->transitions[c].push_back(end);
            }
            for (char c = '0'; c <= '9'; c++) {
                middle->transitions[c].push_back(middle);
                middle->transitions[c].push_back(end);
            }
            middle->transitions['_'].push_back(middle);
            middle->transitions['_'].push_back(end);
            middle->epsilon_transitions.push_back(end);
        }
        // 处理常量规则
        else if (type == CONSTANT) {
            // 处理整数和浮点数
            auto num_state = std::make_shared<NFAState>();
            num_state->id = nfa_states.size();
            nfa_states.push_back(num_state);

            // 数字转换
            for (char c = '0'; c <= '9'; c++) {
                start->transitions[c].push_back(num_state);
                num_state->transitions[c].push_back(num_state);
            }
            num_state->epsilon_transitions.push_back(end);

            // 处理小数点部分
            if (pattern.find('.') != std::string::npos) {
                auto decimal_state = std::make_shared<NFAState>();
                decimal_state->id = nfa_states.size();
                nfa_states.push_back(decimal_state);

                num_state->transitions['.'].push_back(decimal_state);
                for (char c = '0'; c <= '9'; c++) {
                    decimal_state->transitions[c].push_back(decimal_state);
                }
                decimal_state->epsilon_transitions.push_back(end);
            }
        }
    }
    return start;
}

void LexicalAnalysis::convertNFAtoDFA() {
    // 子集构造法
    std::map<std::set<std::shared_ptr<NFAState>>, std::shared_ptr<DFAState>> dfaStates;
    std::queue<std::set<std::shared_ptr<NFAState>>> workList;

    // 初始状态集合
    auto initial_states = getEpsilonClosure({nfa_start});
    dfa_start = std::make_shared<DFAState>();
    dfa_start->id = 0;
    dfa_start->is_final = false;
    dfa_start->token_type = INVALID;
    dfaStates[initial_states] = dfa_start;
    workList.push(initial_states);
    dfa_states.push_back(dfa_start);

    // 处理工作队列
    while (!workList.empty()) {
        auto current_states = workList.front();
        workList.pop();
        auto current_dfa = dfaStates[current_states];

        // 设置DFA状态的属性
        current_dfa->is_final = false;
        current_dfa->token_type = INVALID;

        // 检查是否包含终态，并设置对应的token类型
        for (const auto& state : current_states) {
            if (state->is_final) {
                current_dfa->is_final = true;
                current_dfa->token_type = state->token_type;
                break;  // 使用第一个找到的终态类型
            }
        }

        // 获取所有可能的输入字符
        std::set<char> inputs;
        for (const auto& state : current_states) {
            for (const auto& trans : state->transitions) {
                inputs.insert(trans.first);
            }
        }

        // 对每个输入字符创建转换
        for (char input : inputs) {
            auto moved_states = move(current_states, input);
            if (moved_states.empty()) continue;

            auto next_states = getEpsilonClosure(moved_states);
            if (next_states.empty()) continue;

            if (dfaStates.find(next_states) == dfaStates.end()) {
                auto new_state = std::make_shared<DFAState>();
                new_state->id = dfa_states.size();
                new_state->is_final = false;
                new_state->token_type = INVALID;

                // 检查新状态是否包含终态
                for (const auto& state : next_states) {
                    if (state->is_final) {
                        new_state->is_final = true;
                        new_state->token_type = state->token_type;
                        break;
                    }
                }

                dfaStates[next_states] = new_state;
                workList.push(next_states);
                dfa_states.push_back(new_state);
            }

            current_dfa->transitions[input] = dfaStates[next_states];
        }
    }
}

std::vector<Token> LexicalAnalysis::analyze(const std::string& source_code) {
    std::vector<Token> tokens;
    int line_number = 1;
    size_t i = 0;

    while (i < source_code.length()) {
        char c = source_code[i];
        // 跳过空白字符
        if (isspace(c)) {
            if (c == '\n') line_number++;
            i++;
            continue;
        }

        // 检查是否是数字（可能是常量的开始）
        if (isdigit(c)) {
            std::string number;
            size_t j = i;
        // 读取整数部分
        while (j < source_code.length() && isdigit(source_code[j])) {
            number += source_code[j++];
        }

        // 检查是否为非法标识符
        if (j < source_code.length() && isalpha(source_code[j])) {
            std::string invalid_token = number;
            while (j < source_code.length() && (isalnum(source_code[j]) || source_code[j] == '_')) {
                invalid_token += source_code[j++];
            }
            tokens.push_back({INVALID, invalid_token, line_number});
            std::cerr << "Error at line " << line_number
                 << ": Identifier cannot start with a number: "
                 << invalid_token << std::endl;
            i = j;
            continue;
        }

        // 检查小数点
        if (j < source_code.length() && source_code[j] == '.') {
            number += source_code[j++];
            while (j < source_code.length() && isdigit(source_code[j])) {
                number += source_code[j++];
            }
        }

        // 检查科学计数法
        if (j < source_code.length() && (source_code[j] == 'e' || source_code[j] == 'E')) {
            number += source_code[j++];
            if (j < source_code.length() && (source_code[j] == '+' || source_code[j] == '-')) {
                number += source_code[j++];
            }
            while (j < source_code.length() && isdigit(source_code[j])) {
                number += source_code[j++];
            }
        }

        // 检查复数
        else if (j < source_code.length() && (source_code[j] == '+' || source_code[j] == '-')) {
            number += source_code[j++];
            while (j < source_code.length() && isdigit(source_code[j])) {
                number += source_code[j++];
            }
            if (j < source_code.length() && source_code[j] == '.') {
                number += source_code[j++];
                while (j < source_code.length() && isdigit(source_code[j])) {
                    number += source_code[j++];
                }
            }
            if (j < source_code.length() && source_code[j] == 'i') {
                number += source_code[j++];
            }
        }

        tokens.push_back({CONSTANT, number, line_number});
        i = j;
        continue;
        }

        // 首先检查是否是限定符或运算符
        std::string special_chars = "[](){};,+-*/<>=!";
        if (special_chars.find(c) != std::string::npos) {
            std::string op;
            op += c;

            // 检查双字符运算符
            if (i + 1 < source_code.length()) {
                char next = source_code[i + 1];
                std::string double_op = op + next;
                if ((c == '=' && next == '=') ||
                    (c == '!' && next == '=') ||
                    (c == '<' && next == '=') ||
                    (c == '>' && next == '=')) {
                    op = double_op;
                    i++;
                }
            }

            // 确定token类型
            TokenType type;
            if (std::string("[](){};,").find(c) != std::string::npos) {
                type = LIMITER;
            } else {
                type = OPERATOR;
            }

            tokens.push_back({type, op, line_number});
            i++;
            continue;
        }

        // 尝试匹配最长的token
        std::string longest_match;
        TokenType best_type = INVALID;
        size_t best_length = 0;
        size_t best_pos = i;

        // 提取可能的token
        std::string possible_token;
        size_t j = i;
        while (j < source_code.length() &&
               !isspace(source_code[j]) &&
               special_chars.find(source_code[j]) == std::string::npos) {
            possible_token += source_code[j];
            j++;
        }

        // 首先检查是否是关键字
        for (const auto& rule : grammar_rules) {
            if (rule.type == KEYWORD && possible_token == rule.pattern) {
                longest_match = possible_token;
                best_type = KEYWORD;
                best_length = possible_token.length();
                best_pos = i + best_length - 1;
                break;
            }
        }

        // 如果不是关键字，再尝试其他模式
        if (best_type == INVALID) {
            auto current_state = dfa_start;
            std::string current_token;

            // 检查第一个字符是否是数字
            bool starts_with_digit = isdigit(source_code[i]);

            for (size_t k = i; k < source_code.length(); k++) {
                char curr_char = source_code[k];
                if (isspace(curr_char) || special_chars.find(curr_char) != std::string::npos) break;

                auto it = current_state->transitions.find(curr_char);
                if (it == current_state->transitions.end()) break;

                current_state = it->second;
                current_token += curr_char;

                if (current_state->is_final) {
                    // 如果是标识符且以数字开头，跳过这种情况，后面再进行处理
                    if (current_state->token_type == IDENTIFIER && starts_with_digit) {
                        continue;
                    }
                    longest_match = current_token;
                    best_type = current_state->token_type;
                    best_length = current_token.length();
                    best_pos = k;
                }
            }
        }

        if (best_type != INVALID) {
            tokens.push_back({best_type, longest_match, line_number});
            i = best_pos + 1;
        } else if (!possible_token.empty()) {
            // 如果是以数字开头的标识符，作为一个整体处理为Invalid类型
            if (isdigit(possible_token[0]) && std::any_of(possible_token.begin() + 1, possible_token.end(), ::isalpha)) {
                tokens.push_back({INVALID, possible_token, line_number});
                std::cerr << "Error at line " << line_number
                         << ": Identifier cannot start with a number: "
                         << possible_token << std::endl;
                i = i + possible_token.length();
            } else {
                std::cerr << "Error: Unrecognized token at line " << line_number
                         << ": " << possible_token << std::endl;
                i++;
            }
        }
    }

    return tokens;
}

std::string LexicalAnalysis::readSourceFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open source file");
    }
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

void LexicalAnalysis::printTokens(const std::vector<Token>& tokens) {
    std::cout<< "\n=== 词法分析结果 ===" << std::endl;
    for (size_t i = 0; i < tokens.size(); i++) {
        const auto& token = tokens[i];

        // 检查非法标识符情况
        if (i > 0 && tokens[i-1].type == KEYWORD &&
            token.type == CONSTANT && i+1 < tokens.size() &&
            tokens[i+1].type == IDENTIFIER) {
            std::cerr << "Error at line " << token.line_number
                     << ": Identifier cannot start with a number" << std::endl;
            }
        std::cout << "(Line: " << token.line_number << ", Type: ";
        switch (token.type) {
            case KEYWORD: std::cout << "Keyword"; break;
            case IDENTIFIER: std::cout << "Identifier"; break;
            case CONSTANT: std::cout << "Constant"; break;
            case LIMITER: std::cout << "Limiter"; break;
            case OPERATOR: std::cout << "Operator"; break;
            default: std::cout << "Invalid"; break;
        }
        std::cout << ", Value: " << token.value << ")" << std::endl;
    }
}
std::set<std::shared_ptr<NFAState>> LexicalAnalysis::getEpsilonClosure(
    const std::set<std::shared_ptr<NFAState>>& states) {
    std::set<std::shared_ptr<NFAState>> closure = states;
    std::stack<std::shared_ptr<NFAState>> stack;

    // 将所有状态压入栈中
    for (const auto& state : states) {
        stack.push(state);
    }

    // 处理所有的ε转换
    while (!stack.empty()) {
        auto current = stack.top();
        stack.pop();

        // 检查所有ε转换
        for (const auto& next : current->epsilon_transitions) {
            // 如果这是一个新状态
            if (closure.find(next) == closure.end()) {
                closure.insert(next);
                stack.push(next);
            }
        }
    }

    return closure;
}

std::set<std::shared_ptr<NFAState>> LexicalAnalysis::move(
    const std::set<std::shared_ptr<NFAState>>& states, char c) {
    std::set<std::shared_ptr<NFAState>> result;
    // 对每个状态检查字符c的转换
    for (const auto& state : states) {
        // 如果存在对应字符的转换
        auto it = state->transitions.find(c);
        if (it != state->transitions.end()) {
            // 将所有目标状态添加到结果集合中
            for (const auto& next : it->second) {
                result.insert(next);
            }
        }
    }
    return result;
}