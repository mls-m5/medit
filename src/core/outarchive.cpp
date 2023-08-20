#include "outarchive.h"

OutArchive::OutArchive(std::ostream &stream)
    : Archive{Archive::Out}
    , stream{&stream} {}

void OutArchive::beginChild(Sv name) {
    auto &ref = json[name] = nlohmann::json::object();
    stack.push_back(&ref);
}

void OutArchive::endChild() {
    stack.pop_back();
}

void OutArchive::handle(Sv name, long long &value) {
    current()[name] = value;
}

void OutArchive::handle(Sv name, double &value) {
    current()[name] = value;
}

void OutArchive::handle(Sv name, std::string &value) {
    current()[name] = value;
}

nlohmann::json &OutArchive::current() {
    return *stack.back();
}

OutArchive::~OutArchive() {
    *stream << json;
}
