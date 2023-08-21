#include "inarchive.h"
#include <cassert>
#include <vector>

InArchive::InArchive(std::istream &stream)
    : Archive{Archive::In} {
    stream >> json;
}

InArchive::~InArchive() {}

nlohmann::json &InArchive::current() const {
    return *stack.back().ptr;
}

size_t InArchive::currentIndex() const {
    return stack.back().arrayIndex;
}

bool InArchive::beginChild(Sv name) {
    auto &c = current();
    if (auto f = c.find(name); f != c.end()) {
        stack.push_back({&current().at(name)});
        return true;
    }
    return false;
}

bool InArchive::beginList(Sv name, size_t &size) {
    auto &c = current();
    if (auto f = c.find(name); f != c.end()) {
        stack.push_back({&current().at(name)});
        assert(current().size() == size);
        return true;
    }
    return false;
}

void InArchive::endChild() {
    stack.pop_back();
}

template <typename T>
bool InArchive::handleInternal(Sv name, T &value) {
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

bool InArchive::handle(Sv name, long long &value) {
    return handleInternal(name, value);
}

bool InArchive::handle(Sv name, double &value) {
    return handleInternal(name, value);
}

bool InArchive::handle(Sv name, std::string &value) {
    return handleInternal(name, value);
}
