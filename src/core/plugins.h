#pragma once

#include "syntax/iannotation.h"
#include <algorithm>
#include <meditfwd.h>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

struct Plugins {
public:
    template <typename T>
    using ListT = std::vector<std::shared_ptr<T>>;

private:
    std::tuple<ListT<IHighlight>,
               ListT<IFormat>,
               ListT<IAnnotation>,
               ListT<INavigation>,
               ListT<ICompletionSource>>
        lists;

    template <typename T, typename V>
    void addSingle(std::shared_ptr<V> ptr) {
        if constexpr (std::is_base_of_v<T, V>) {
            if (ptr) {
                get<T>().push_back(ptr);
            }
        }
    }

    template <typename... T, typename V>
    void addMultiple(std::shared_ptr<V> ptr) {
        (addSingle<T, V>(ptr), ...);
    }

    template <std::size_t i = 0>
    void sortTuple() {
        if constexpr (i < std::tuple_size_v<decltype(lists)>) {
            auto &list = std::get<i>(lists);
            std::sort(list.begin(), list.end());
        }
    }

public:
    template <typename T>
    void loadPlugin() {
        auto ptr = std::make_shared<T>();

        addMultiple<IAnnotation,
                    IFormat,
                    IAnnotation,
                    INavigation,
                    ICompletionSource,
                    IHighlight>(ptr);
    }

    template <typename T>
    ListT<T> &get() {
        return std::get<ListT<T>>(lists);
    }

    void sort() {
        sortTuple();
    }
};
