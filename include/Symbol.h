#ifndef SD2_SYMBOL_H
#define SD2_SYMBOL_H

#include <string>
#include <vector>
#include <set>

enum SymbolType {
    TERMINAL,       // 终结符
    NON_TERMINAL,   // 非终结符
    EPSILON,        // 空符号ε
    END_MARKER      // 结束符号#
};

struct Symbol {
    std::string name;
    SymbolType type;

    Symbol(std::string n = "", SymbolType t = TERMINAL) : name(n), type(t) {}

    bool operator==(const Symbol& other) const {
        return name == other.name && type == other.type;
    }

    bool operator<(const Symbol& other) const {
        if (type != other.type) return type < other.type;
        return name < other.name;
    }
};

#endif //SD2_SYMBOL_H