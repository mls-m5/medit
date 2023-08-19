#pragma once

#include <concepts>
#include <string_view>

class Archive;

template <typename T>
concept HasVisit = requires(T value, Archive &archive) {
    { value.visit(archive) };
};

class Archive {
public:
    using Sv = std::string_view;

    virtual void beginChild(Sv) = 0;
    virtual void endChild() = 0;

    virtual void handle(Sv, long long &) = 0;
    virtual void handle(Sv, double &) = 0;

    template <typename T>
        requires HasVisit<T>
    void operator()(Sv name, T &value) {
        value.visit(*this);
    }

    template <std::integral T>
    void operator()(Sv name, T &value) {
        long long tmp = value;
        handle(name, tmp);
        if (tmp != value) {
            value = static_cast<T>(tmp);
        }
    }

    template <std::floating_point T>
    void operator()(Sv name, T &value) {
        double tmp = value;
        handle(name, tmp);
        if (tmp != value) {
            value = static_cast<T>(tmp);
        }
    }

    Archive() = default;
    Archive(const Archive &) = delete;
    Archive(Archive &&) = delete;
    Archive &operator=(const Archive &) = delete;
    Archive &operator=(Archive &&) = delete;
    virtual ~Archive() = default;
};
