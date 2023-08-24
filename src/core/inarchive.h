#pragma once

#include "archive.h"
#include <istream>
#include <memory>
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
    struct Impl; // For hiding json.hpp
    std::unique_ptr<Impl> _impl;
};
