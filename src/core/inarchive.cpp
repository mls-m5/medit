#include "inarchive.h"
#include "nlohmann/json.hpp"
#include <cassert>
#include <vector>

struct InArchive::Impl {
    template <typename T>
    bool handleInternal(Sv name, T &value) {
        auto &c = current();

        if (name.empty() && c.is_array()) {
            c.get_ptr<std::vector<nlohmann::json> *>()
                ->at(currentIndex())
                .get_to(value);
            ++stack.back().arrayIndex;
            return true;
        }

        if (auto f = c.find(name); f != c.end()) {
            f->get_to(value);
            return true;
        }

        return false;
    }

    nlohmann::json json;

    struct StackElement {
        nlohmann::json *ptr = nullptr;
        size_t arrayIndex = 0;
    };

    std::vector<StackElement> stack = {{&json}};

    nlohmann::json &current() const {
        return *stack.back().ptr;
    }

    size_t currentIndex() const {
        return stack.back().arrayIndex;
    }

    bool beginChild(Sv name) {
        auto &c = current();

        if (name.empty() && c.is_array()) {
            stack.push_back({&(c.get_ptr<std::vector<nlohmann::json> *>()->at(
                currentIndex()))});
            ++stack.at(stack.size() - 2).arrayIndex;
            return true;
        }

        if (auto f = c.find(name); f != c.end()) {
            stack.push_back({&current().at(name)});
            return true;
        }
        return false;
    }
    bool beginList(Sv name, size_t &size) {
        if (!beginChild(name)) {
            return false;
        }

        size = current().size();
        return true;
    }

    void endChild() {
        stack.pop_back();
    }
};

InArchive::InArchive(std::istream &stream)
    : Archive{Archive::In}
    , _impl{std::make_unique<Impl>()} {
    stream >> _impl->json;
}

InArchive::~InArchive() {}

bool InArchive::beginChild(Sv name) {
    return _impl->beginChild(name);
}

bool InArchive::beginList(Sv name, size_t &size) {
    return _impl->beginList(name, size);
}

void InArchive::endChild() {
    _impl->endChild();
}

bool InArchive::handle(Sv name, long long &value) {
    return _impl->handleInternal(name, value);
}

bool InArchive::handle(Sv name, double &value) {
    return _impl->handleInternal(name, value);
}

bool InArchive::handle(Sv name, std::string &value) {
    return _impl->handleInternal(name, value);
}
