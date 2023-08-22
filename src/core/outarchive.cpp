#include "outarchive.h"
#include "nlohmann/json.hpp"
#include <vector>

OutArchive::OutArchive(std::ostream &stream)
    : Archive{Archive::Out}
    , stream{&stream} {}

bool OutArchive::beginChild(Sv name) {
    auto &c = current();

    if (c.is_array()) {
        c.push_back(nlohmann::json::object());
        stack.push_back(&c.back());
        return true;
    }

    auto &ref = c[name] = nlohmann::json::object();
    stack.push_back(&ref);
    return true;
}

bool OutArchive::beginList(Sv name, size_t &size) {
    auto &c = current();
    nlohmann::json jsonArray = nlohmann::json::array();
    auto arr = nlohmann::json::array();
    arr.get_ptr<std::vector<nlohmann::json> *>()->reserve(size);

    if (c.is_array()) {
        c.push_back(std::move(arr));
        stack.push_back(&c.back());
        return true;
    }

    auto &ref = c[name] = std::move(arr);
    stack.push_back(&ref);
    return true;
}

void OutArchive::endChild() {
    stack.pop_back();
}

bool OutArchive::handle(Sv name, long long &value) {
    return handleInternal(name, value);
}

bool OutArchive::handle(Sv name, double &value) {
    return handleInternal(name, value);
}

bool OutArchive::handle(Sv name, std::string &value) {
    return handleInternal(name, value);
}

nlohmann::json &OutArchive::current() {
    return *stack.back();
}

OutArchive::~OutArchive() {
    finalize();
}

void OutArchive::finalize() {
    if (stream) {
        *stream << json;
    }
    stream = nullptr;
}

template <typename T>
bool OutArchive::handleInternal(Sv name, T &value) {
    auto &c = current();
    if (c.is_array()) {
        c.push_back(value);
        return false;
    }
    c[name] = value;
    return false;
}
