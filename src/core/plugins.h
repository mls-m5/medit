#pragma once

#include "plugin/idebugger.h"
#include "syntax/iannotation.h"
#include "syntax/ihighlight.h"
#include "syntax/irename.h"
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
               ListT<ICompletionSource>,
               ListT<IRename>,
               ListT<IDebugger>>
        lists;

    template <typename T, typename V>
    void addSingle(std::shared_ptr<V> ptr) {
        if constexpr (std::is_base_of_v<T, V>) {
            if (ptr) {
                get<T>().push_back(ptr);
            }
        }
    }

    /// Try to add a plugin as any possible plugin
    template <typename... T, typename V>
    void addMultiple(std::shared_ptr<V> ptr) {
        (addSingle<T, V>(ptr), ...);
    }

    template <std::size_t i = 0>
    void sortTuple() {
        if constexpr (i < std::tuple_size_v<decltype(lists)>) {
            auto &list = std::get<i>(lists);
            std::sort(list.begin(), list.end(), [](auto &a, auto &b) {
                return a->priority() >
                       b->priority(); // note that it is inverted
            });
        }
    }

public:
    /// Create a plugin and add it to all matching lists
    template <typename T, typename... Args>
    void createPlugin(Args &&...args) {
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);

        addMultiple<IAnnotation,
                    IFormat,
                    INavigation,
                    ICompletionSource,
                    IHighlight,
                    IRename,
                    IDebugger>(std::move(ptr));
    }

    template <typename T>
    ListT<T> &get() {
        return std::get<ListT<T>>(lists);
    }

    void sort() {
        sortTuple();
    }
};
