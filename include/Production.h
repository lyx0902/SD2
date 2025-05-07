#ifndef SD2_PRODUCTION_H
#define SD2_PRODUCTION_H

#include "Symbol.h"

struct Production {
    Symbol left;
    std::vector<Symbol> right;
    int index;

    Production(Symbol l = Symbol(), std::vector<Symbol> r = {}, int i = -1)
        : left(l), right(r), index(i) {}
    // 修改构造函数，使用引用和新的参数类型，重载toString()函数与对应的运算符
    std::string toString() const;
    bool operator==(const Production& other) const;
    bool operator<(const Production& other) const;
};

#endif //SD2_PRODUCTION_H