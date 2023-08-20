#include "inarchive.h"

InArchive::InArchive(std::istream &stream)
    : Archive{Archive::In} {
    stream >> json;
}

InArchive::~InArchive() {}

nlohmann::json &InArchive::current() {
    return *stack.back();
}

void InArchive::beginChild(Sv name) {
    stack.push_back(&current().at(name));
}

void InArchive::endChild() {
    stack.pop_back();
}

void InArchive::handle(Sv name, long long &value) {
    current()[name].get_to(value);
}

void InArchive::handle(Sv name, double &value) {
    current()[name].get_to(value);
}

void InArchive::handle(Sv name, std::string &value) {
    current()[name].get_to(value);
}
