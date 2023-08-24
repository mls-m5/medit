#include "outarchive.h"
#include "nlohmann/json.hpp"
#include <memory>
#include <ostream>
#include <vector>

struct OutArchive::Impl {
    Impl(std::ostream &stream)
        : stream{&stream} {}

    template <typename T>
    bool handleInternal(Sv name, T &value) {
        auto &c = current();
        if (c.is_array()) {
            c.push_back(value);
            return false;
        }
        c[name] = value;
        return false;
    }

    bool beginChild(Sv name) {
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

    bool beginList(Sv name, size_t &size) {
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

    void endChild() {
        stack.pop_back();
    }

    void finalize() {
        if (stream) {
            *stream << json;
        }
        stream = nullptr;
    }

    std::ostream *stream = nullptr;

    nlohmann::json json;

    std::vector<nlohmann::json *> stack = {&json};

    nlohmann::json &current() {
        return *stack.back();
    }
};

OutArchive::OutArchive(std::ostream &stream)
    : Archive{Archive::Out}
    , _impl{std::make_unique<Impl>(stream)} {}

bool OutArchive::beginChild(Sv name) {
    return _impl->beginChild(name);
}

bool OutArchive::beginList(Sv name, size_t &size) {
    return _impl->beginList(name, size);
}

void OutArchive::endChild() {
    _impl->endChild();
}

bool OutArchive::handle(Sv name, long long &value) {
    return _impl->handleInternal(name, value);
}

bool OutArchive::handle(Sv name, double &value) {
    return _impl->handleInternal(name, value);
}

bool OutArchive::handle(Sv name, std::string &value) {
    return _impl->handleInternal(name, value);
}

OutArchive::~OutArchive() {
    finalize();
}

void OutArchive::finalize() {
    _impl->finalize();
}
