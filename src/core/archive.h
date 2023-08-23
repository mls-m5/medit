#pragma once

#include <concepts>
#include <map>
#include <stdexcept>
#include <string_view>
#include <vector>

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

/// Note that the handdle functions returns true for when a element is found in
/// inarchive, and always returns false for out archives (the value has not been
/// written to
class Archive {
public:
    enum Direction {
        In,
        Out,
    };

    Direction direction;

    using Sv = std::string_view;

    /// Return true if the child is successfully created/found
    virtual bool beginChild(Sv) = 0;

    /// Return true if the child is successfully created/found
    virtual bool beginList(Sv, size_t &) = 0;

    /// Only call this if begin statement has returned true
    virtual void endChild() = 0;

    virtual bool handle(Sv, long long &) = 0;
    virtual bool handle(Sv, double &) = 0;
    virtual bool handle(Sv, std::string &) = 0;

    template <typename T>
        requires HasVisit<T>
    bool operator()(Sv name, T &value) {
        if (!beginChild(name)) {
            return false;
        }

        value.visit(*this);
        endChild();
        return direction == In;
    }

    template <typename T>
        requires HasGlobalVisit<T>
    bool operator()(Sv name, T &value) {
        if (!beginChild(name)) {
            return false;
        }
        visit(*this, value);
        endChild();
        return direction == In;
    }

    template <typename T>
        requires HasGlobalSaveLoad<T>
    bool operator()(Sv name, T &value) {
        if (!beginChild(name)) {
            return false;
        }
        if (direction == In) {
            load(*this, value);
        }
        else {
            save(*this, value);
        }
        endChild();
        return direction == In;
    }

    template <std::integral T>
    bool operator()(Sv name, T &value) {
        long long tmp = value;
        if (handle(name, tmp)) {
            value = static_cast<T>(tmp);
            return direction == In;
        }
        return false;
    }

    template <Enum T>
    bool operator()(Sv name, T &value) {
        long long tmp = static_cast<long long>(value);
        if (handle(name, tmp)) {
            value = static_cast<T>(tmp);
            return direction == In;
        }
        return false;
    }

    template <std::floating_point T>
    bool operator()(Sv name, T &value) {
        double tmp = value;
        if (handle(name, tmp)) {
            value = static_cast<T>(tmp);
            return direction == In;
        }
        return false;
    }

    bool operator()(Sv name, std::string &value) {
        return handle(name, value);
    }

    template <typename T>
    bool operator()(Sv name, std::vector<T> &v) {
        size_t size = v.size();
        if (!beginList(name, size)) {
            throw std::runtime_error{"could not begin list"};
            return false;
        }
        if (direction == In) {
            v.resize(size);
        }
        for (auto &element : v) {
            (*this)("", element);
        }
        endChild();
        return direction == In;
    }

    //    template <typename T>
    //    bool operator()(Sv name, std::map<std::string, T> &m) {
    //        if (!beginChild(name)) {
    //            throw std::runtime_error{"could not begin list"};
    //            return false;
    //        }
    //        for (auto &element : v) {
    //            (*this)("", element);
    //        }
    //        endChild();
    //        return direction == In;
    //    }

    /// Shorthand for reading a value
    template <typename T>
    T get(Sv name) {
        T ret;
        (*this)(name, ret);
        return ret;
    }

    /// Shorthand for reading a value
    /// The argument acts as temporary
    template <typename T>
    void set(Sv name, T value) {
        (*this)(name, value);
    }

    Archive(Direction d)
        : direction{d} {}
    Archive(const Archive &) = delete;
    Archive(Archive &&) = delete;
    Archive &operator=(const Archive &) = delete;
    Archive &operator=(Archive &&) = delete;
    virtual ~Archive() = default;
};
