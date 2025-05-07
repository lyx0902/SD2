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
    // 修改构造函数，使用引用和新的参数类型，重载toString()函数与对应的运算符
    std::string toString() const;// 将LR1Item转换为字符串
    bool operator==(const LR1Item& other) const; // 比较两个LR1Item是否相等
    bool operator<(const LR1Item& other) const; // 比较两个LR1Item的大小
    Symbol getNextSymbol() const; // 获取下一个符号
    bool isComplete() const; // 判断LR1Item是否完成
};

#endif //SD2_LR1ITEM_H