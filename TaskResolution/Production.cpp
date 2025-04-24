#include "Production.h"

std::string Production::toString() const {
    std::string result = left.name + " -> ";
    for (const auto& symbol : right) {
        result += symbol.name + " ";
    }
    return result;
}

bool Production::operator==(const Production& other) const {
    return left == other.left && right == other.right;
}

bool Production::operator<(const Production& other) const {
    if (left != other.left) return left < other.left;
    return right < other.right;
}