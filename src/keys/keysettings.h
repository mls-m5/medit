#pragma once

#include "keys/keyevent.h"

#include <algorithm>
#include <string>
#include <vector>

class KeySettings {
private:
    std::vector<std::pair<KeyEvent, std::string>> binding;

public:
    auto findBinding(KeyEvent event) {
        return std::find_if(
            binding.begin(), binding.end(), [&event](auto &&value) {
                return event == value.first;
            });
    }

    auto findBinding(std::string action) {
        return std::find_if(
            binding.begin(), binding.end(), [&action](auto &&value) {
                return action == value.second;
            });
    }

    std::string action(KeyEvent ev) {
        auto it = findBinding(ev);

        if (it == binding.end()) {
            return {};
        }
        else {
            return it->second;
        }
    }

    void setBinding(std::string action, KeyEvent event) {
        auto it = findBinding(action);
        if (it != binding.end()) {
            it->first = event;
        }
        else {
            binding.emplace_back(event, action);
        }
    }
};
