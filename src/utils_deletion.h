#pragma once

#include <vector>
#include <algorithm>
#include <functional>

/**
 * @brief Helper to manage deferred deletions from a container/list during iteration.
 */
struct DeletionQueue {
    std::vector<int> items;

    void Queue(int index) {
        items.push_back(index);
    }

    bool HasItems() const {
        return !items.empty();
    }

    /**
     * @brief Applies the deletion logic.
     * @param deleter A callable that takes an index (int) and performs the deletion.
     * 
     * The deleter is called for indices in descending order to ensure stability 
     * when removing from vectors.
     */
    template <typename F>
    void Apply(F&& deleter) {
        if (items.empty()) return;
        
        // Sort descending to keep indices valid during erasure
        std::sort(items.begin(), items.end(), std::greater<int>());
        
        // Remove duplicates
        auto last = std::unique(items.begin(), items.end());
        items.erase(last, items.end());
        
        for (int index : items) {
            deleter(index);
        }
        items.clear();
    }
};
