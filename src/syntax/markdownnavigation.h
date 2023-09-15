#pragma once

#include "navigation/inavigation.h"

class MarkdownNavigation : public INavigation {
public:
    MarkdownNavigation() = default;
    ~MarkdownNavigation() override = default;

    MarkdownNavigation(const MarkdownNavigation &) = delete;
    MarkdownNavigation(MarkdownNavigation &&) = delete;
    MarkdownNavigation &operator=(const MarkdownNavigation &) = delete;
    MarkdownNavigation &operator=(MarkdownNavigation &&) = delete;

    // @see INavigation
    bool gotoSymbol(std::shared_ptr<IEnvironment> env) override;
};
