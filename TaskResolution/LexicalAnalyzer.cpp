#include "LexicalAnalyzer.h"
#include <regex>
#include <cctype>

// 正则表达式定义
const std::regex keyword_regex("\\b(int|float|double|complex|char|string|main|long|if|else|while|return|for|void|break)\\b");
const std::regex identifier_regex("[a-zA-Z_][a-zA-Z0-9_]*");
const std::regex constant_regex("([+-]?\\d*\\.\\d+([eE][+-]?\\d+)?|[+-]?\\d+([eE][+-]?\\d+)?|[+-]?\\d+)");
const std::regex complex_constant_regex("^[+-]?\\d+(?:[+-]\\d+)?i#");
const std::regex limiter_regex("[;,.(){}]");
const std::regex operator_regex("[+\\-*/%&|!<>^=]");

TokenType LexicalAnalyzer::get_token_type(const std::string& str) {
    if (regex_match(str, keyword_regex)) return KEYWORD;
    if (regex_match(str, identifier_regex)) return IDENTIFIER;
    if (regex_match(str, complex_constant_regex)) return COMPLEX;
    if (regex_match(str, constant_regex)) return CONSTANT;
    if (regex_match(str, limiter_regex)) return LIMITER;
    if (regex_match(str, operator_regex)) return OPERATOR;
    return INVALID;
}

std::vector<Token> LexicalAnalyzer::analyze(const std::string& source_code) {
    std::vector<Token> tokens;
    std::string current_token;
    int line_number = 1;

    for (size_t i = 0; i < source_code.size(); ++i) {
        char c = source_code[i];

        if (c == '\n') {
            line_number++;
            continue;
        }

        if ((c == 'E' || c == 'e') && !current_token.empty() &&
            i + 1 < source_code.size() &&
            (isdigit(current_token.back()) || current_token.back() == '.')) {
            current_token += c;
            if (source_code[i + 1] == '+' || source_code[i + 1] == '-') {
                current_token += source_code[i + 1];
                i++;
            }
            continue;
        }

        if (isspace(c)) {
            if (!current_token.empty()) {
                TokenType type = get_token_type(current_token);
                if (type != INVALID) {
                    tokens.push_back({type, current_token, line_number});
                }
                current_token.clear();
            }
            continue;
        }

        if (isalnum(c) || c == '.' ||
            ((c == '+' || c == '-') && !current_token.empty() &&
             (current_token.back() == 'E' || current_token.back() == 'e'))) {
            current_token += c;
        } else {
            if (!current_token.empty()) {
                TokenType type = get_token_type(current_token);
                tokens.push_back({type, current_token, line_number});
                current_token.clear();
            }

            if (!isspace(c)) {
                current_token += c;
                TokenType type = get_token_type(current_token);
                tokens.push_back({type, current_token, line_number});
                current_token.clear();
            }
        }
    }

    if (!current_token.empty()) {
        TokenType type = get_token_type(current_token);
        tokens.push_back({type, current_token, line_number});
    }

    return tokens;
}

void LexicalAnalyzer::processTokens(std::vector<Token>& tokens) {
    // 处理复数常量
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == CONSTANT && i + 2 < tokens.size() &&
            (tokens[i + 1].type == OPERATOR && (tokens[i + 1].value == "+" || tokens[i + 1].value == "-")) &&
            tokens[i + 2].type == COMPLEX) {
            tokens[i].value += tokens[i + 1].value + tokens[i + 2].value;
            tokens[i].type = COMPLEX;
            tokens.erase(tokens.begin() + i + 1, tokens.begin() + i + 3);
        }
    }

    // 处理科学计数法
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == CONSTANT && i + 2 < tokens.size()) {
            if (tokens[i + 1].type == IDENTIFIER &&
                (tokens[i + 1].value == "E" || tokens[i + 1].value == "e")) {
                if (tokens[i + 2].type == CONSTANT) {
                    tokens[i].value = tokens[i].value + "E" + tokens[i + 2].value;
                    tokens.erase(tokens.begin() + i + 1, tokens.begin() + i + 3);
                }
                else if (i + 3 < tokens.size() &&
                         tokens[i + 2].type == OPERATOR &&
                         tokens[i + 2].value == "-" &&
                         tokens[i + 3].type == CONSTANT) {
                    tokens[i].value = tokens[i].value + "E" + tokens[i + 2].value + tokens[i + 3].value;
                    tokens.erase(tokens.begin() + i + 1, tokens.begin() + i + 4);
                }
                else if (i + 3 < tokens.size() &&
                         tokens[i + 2].type == OPERATOR &&
                         tokens[i + 2].value == "+" &&
                         tokens[i + 3].type == CONSTANT) {
                    tokens[i].value = tokens[i].value + "E" + tokens[i + 3].value;
                    tokens.erase(tokens.begin() + i + 1, tokens.begin() + i + 4);
                }
            }
        }
    }
}