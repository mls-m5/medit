

#pragma once

#include "archive.h"
#include "nlohmann/json.hpp"
#include <ostream>
#include <vector>

class OutArchive : public Archive {
public:
    bool beginChild(Sv) override;
    bool beginList(Sv, size_t &) override;
    void endChild() override;

    bool handle(Sv, long long &value) override;
    bool handle(Sv, double &value) override;
    bool handle(Sv, std::string &value) override;

    OutArchive(std::ostream &stream);

    OutArchive(const OutArchive &) = delete;
    OutArchive(OutArchive &&) = delete;
    OutArchive &operator=(const OutArchive &) = delete;
    OutArchive &operator=(OutArchive &&) = delete;
    ~OutArchive() override;

private:
    template <typename T>
    bool handleInternal(Sv name, T &value);

    std::ostream *stream = nullptr;

    nlohmann::json json;

    std::vector<nlohmann::json *> stack = {&json};

    nlohmann::json &current();
};
