#include "LR1Item.h"

std::string LR1Item::toString() const {
    std::string result = prod.left.name + " -> ";

    // 输出产生式内容
    for (size_t i = 0; i < prod.right.size(); i++) {
        if (i == dotPosition) result += "· ";
        result += prod.right[i].name + " ";
    }
    if (dotPosition == prod.right.size()) result += "· ";

    // 合并输出向前搜索符，去除epsilon
    result += ", { ";
    bool first = true;
    for (const auto& symbol : lookahead) {
        if (symbol.type != EPSILON) {
            if (!first) result += ", ";
            result += symbol.name;
            first = false;
        }
    }
    result += " }";
    return result;
}

bool LR1Item::operator==(const LR1Item& other) const {
    return prod == other.prod &&
           dotPosition == other.dotPosition &&
           lookahead == other.lookahead;
}

bool LR1Item::operator<(const LR1Item& other) const {
    if (prod.index != other.prod.index) return prod.index < other.prod.index;
    if (dotPosition != other.dotPosition) return dotPosition < other.dotPosition;
    return lookahead < other.lookahead;
}

Symbol LR1Item::getNextSymbol() const {
    if (dotPosition >= prod.right.size()) {
        return Symbol("#", END_MARKER);
    }
    return prod.right[dotPosition];
}

bool LR1Item::isComplete() const {
    return dotPosition >= prod.right.size();
}