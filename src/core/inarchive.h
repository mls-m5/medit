#pragma once

#include "archive.h"
#include "nlohmann/json.hpp"
#include <istream>
#include <vector>

class InArchive : public Archive {
public:
    void beginChild(Sv) override;
    void endChild() override;

    void handle(Sv, long long &) override;
    void handle(Sv, double &) override;
    void handle(Sv, std::string &) override;

    InArchive(std::istream &stream);

    InArchive(const InArchive &) = delete;
    InArchive(InArchive &&) = delete;
    InArchive &operator=(const InArchive &) = delete;
    InArchive &operator=(InArchive &&) = delete;
    ~InArchive() override;

private:
    nlohmann::json json;

    std::vector<nlohmann::json *> stack = {&json};

    nlohmann::json &current();
};
