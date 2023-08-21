#pragma once

#include "archive.h"
#include "nlohmann/json.hpp"
#include <istream>
#include <vector>

class InArchive : public Archive {
public:
    bool beginChild(Sv) override;
    bool beginList(Sv, size_t &) override;
    void endChild() override;

    bool handle(Sv, long long &) override;
    bool handle(Sv, double &) override;
    bool handle(Sv, std::string &) override;

    InArchive(std::istream &stream);

    InArchive(const InArchive &) = delete;
    InArchive(InArchive &&) = delete;
    InArchive &operator=(const InArchive &) = delete;
    InArchive &operator=(InArchive &&) = delete;
    ~InArchive() override;

private:
    template <typename T>
    bool handleInternal(Sv, T &t);

    nlohmann::json json;

    struct StackElement {
        nlohmann::json *ptr = nullptr;
        size_t arrayIndex = 0;
    };

    std::vector<StackElement> stack = {{&json}};

    nlohmann::json &current() const;
    size_t currentIndex() const;
};
