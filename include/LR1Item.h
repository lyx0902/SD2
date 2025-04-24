#ifndef SD2_LR1ITEM_H
#define SD2_LR1ITEM_H

#include "Production.h"
#include <set>

struct LR1Item {
    const Production& prod;  // 改为引用类型
    size_t dotPosition;     // 使用size_t类型
    std::set<Symbol> lookahead;

    // 修改构造函数，使用引用和新的参数类型
    LR1Item(const Production& p, size_t pos, const std::set<Symbol>& look)
        : prod(p), dotPosition(pos), lookahead(look) {}

    std::string toString() const;
    bool operator==(const LR1Item& other) const;
    bool operator<(const LR1Item& other) const;
    Symbol getNextSymbol() const;
    bool isComplete() const;
};

#endif //SD2_LR1ITEM_H