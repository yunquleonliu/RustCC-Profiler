// Rust C/C++ Profiler - Rule Implementation
// Rust C/C++ 分析器 - 规则实现

#include "tcc/Rule.h"
#include <algorithm>

namespace tcc {

RuleRegistry& RuleRegistry::instance() {
    static RuleRegistry registry;
    return registry;
}

void RuleRegistry::registerRule(std::unique_ptr<Rule> rule) {
    rules_.push_back(std::move(rule));
}

std::vector<Rule*> RuleRegistry::getRulesByCategory(RuleCategory category) const {
    std::vector<Rule*> result;
    for (const auto& rule : rules_) {
        if (rule->getCategory() == category) {
            result.push_back(rule.get());
        }
    }
    return result;
}

} // namespace tcc

