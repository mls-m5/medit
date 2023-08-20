

#pragma once

#include "archive.h"
#include "nlohmann/json.hpp"
#include <ostream>
#include <vector>

class OutArchive : public Archive {
public:
    void beginChild(Sv) override;
    void endChild() override;

    void handle(Sv, long long &value) override;
    void handle(Sv, double &value) override;
    void handle(Sv, std::string &value) override;

    OutArchive(std::ostream &stream);

    OutArchive(const OutArchive &) = delete;
    OutArchive(OutArchive &&) = delete;
    OutArchive &operator=(const OutArchive &) = delete;
    OutArchive &operator=(OutArchive &&) = delete;
    ~OutArchive() override;

private:
    std::ostream *stream = nullptr;

    nlohmann::json json;

    std::vector<nlohmann::json *> stack = {&json};

    nlohmann::json &current();
};
