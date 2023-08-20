#pragma once

#include <concepts>
#include <string_view>

#define ARCH_PAIR(x) arch(#x, x)

class Archive;

template <typename T>
concept HasVisit = requires(T value, Archive &archive) {
    { value.visit(archive) };
};

template <typename T>
concept HasGlobalVisit = requires(T value, Archive &archive) {
    { visit(archive, value) };
};

template <typename T>
concept HasGlobalSaveLoad = requires(T value, Archive &archive) {
    { save(archive, value), load(archive, value) };
};

template <typename T>
concept Enum = std::is_enum_v<T>;

class Archive {
public:
    enum Direction {
        In,
        Out,
    };

    Direction direction;

    using Sv = std::string_view;

    virtual void beginChild(Sv) = 0;
    virtual void endChild() = 0;

    virtual void handle(Sv, long long &) = 0;
    virtual void handle(Sv, double &) = 0;
    virtual void handle(Sv, std::string &) = 0;

    template <typename T>
        requires HasVisit<T>
    void operator()(Sv name, T &value) {
        beginChild(name);
        value.visit(*this);
        endChild();
    }

    template <typename T>
        requires HasGlobalVisit<T>
    void operator()(Sv name, T &value) {
        beginChild(name);
        visit(*this, value);
        endChild();
    }

    template <typename T>
        requires HasGlobalSaveLoad<T>
    void operator()(Sv name, T &value) {
        beginChild(name);
        if (direction == In) {
            load(*this, value);
        }
        else {
            save(*this, value);
        }
        endChild();
    }

    template <std::integral T>
    void operator()(Sv name, T &value) {
        long long tmp = value;
        handle(name, tmp);
        if (tmp != value) {
            value = static_cast<T>(tmp);
        }
    }

    template <Enum T>
    void operator()(Sv name, T &value) {
        long long tmp = static_cast<long long>(value);
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

    void operator()(Sv name, std::string &value) {
        handle(name, value);
    }

    Archive(Direction d)
        : direction{d} {}
    Archive(const Archive &) = delete;
    Archive(Archive &&) = delete;
    Archive &operator=(const Archive &) = delete;
    Archive &operator=(Archive &&) = delete;
    virtual ~Archive() = default;
};
