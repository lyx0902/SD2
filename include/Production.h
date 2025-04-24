#ifndef SD2_PRODUCTION_H
#define SD2_PRODUCTION_H

#include "Symbol.h"

struct Production {
    Symbol left;
    std::vector<Symbol> right;
    int index;

    Production(Symbol l = Symbol(), std::vector<Symbol> r = {}, int i = -1)
        : left(l), right(r), index(i) {}

    std::string toString() const;
    bool operator==(const Production& other) const;
    bool operator<(const Production& other) const;
};

#endif //SD2_PRODUCTION_H