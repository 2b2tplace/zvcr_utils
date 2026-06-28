#pragma once

#include <memory>
#include <unordered_map>
#include <list>
#include <mutex>

namespace zvcr {

    template<class Key, class Value, class KeyHash = std::hash<Key>>
    class LRUCache {
    public:
        using Ptr = std::shared_ptr<Value>;

        explicit LRUCache(const size_t capacity): capacity_(std::max<size_t>(1, capacity)) {}

        [[nodiscard]]
        auto get(const Key &key) -> Ptr {
            std::lock_guard lock(mutex);
            auto it = map.find(key);
            if (it == map.end()) return nullptr;
            touch(it);
            return it->second.value;
        }

        auto put(const Key &key, const Ptr valuePtr) -> void {
            std::lock_guard lock(mutex);
            auto it = map.find(key);
            if (it != map.end()) {
                it->second.value = valuePtr;
                touch(it);
                return;
            }
            if (list.size() >= capacity_) {
                auto lruKey = list.back();
                list.pop_back();
                map.erase(lruKey);
            }
            list.push_front(key);
            map.emplace(key, Node{ list.begin(), valuePtr });
        }

        [[nodiscard]]
        auto size() const -> size_t {
            std::lock_guard lock(mutex);
            return map.size();
        }

        [[nodiscard]]
        auto capacity() const -> size_t {
            return capacity_;
        }

    private:
        struct Node {
            std::list<Key>::iterator it;
            Ptr value;
        };

        auto touch(const std::unordered_map<Key, Node, KeyHash>::iterator it) -> void {
            list.erase(it->second.it);
            list.push_front(it->first);
            it->second.it = list.begin();
        }

        size_t capacity_;
        mutable std::mutex mutex;
        std::list<Key> list;
        std::unordered_map<Key, Node, KeyHash> map;
    };

}
