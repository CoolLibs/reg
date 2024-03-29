#pragma once
#include <vector>

namespace reg::internal {

template<typename Key, typename Value>
class OrderPreservingMap {
public:
    [[nodiscard]] auto begin() const { return _map.begin(); }
    [[nodiscard]] auto begin() { return _map.begin(); }
    [[nodiscard]] auto end() const { return _map.end(); }
    [[nodiscard]] auto end() { return _map.end(); }

    [[nodiscard]] auto find(Key const& key) const
    {
        for (auto it = begin(); it != end(); ++it)
        {
            if (it->first == key)
                return it;
        }
        return end();
    }

    [[nodiscard]] auto find(Key const& key)
    {
        for (auto it = begin(); it != end(); ++it)
        {
            if (it->first == key)
                return it;
        }
        return end();
    }

    void insert(std::pair<Key, Value> const& key_value_pair)
    {
        _map.push_back(key_value_pair);
    }

    void insert(std::pair<Key, Value>&& key_value_pair)
    {
        _map.push_back(std::move(key_value_pair));
    }

    void erase(Key const& key)
    {
        auto it = find(key);
        if (it != end())
            _map.erase(it);
    }

    [[nodiscard]] auto empty() const -> bool
    {
        return _map.empty();
    }

    void clear()
    {
        _map.clear();
    }

    [[nodiscard]] auto underlying_container() const -> std::vector<std::pair<Key, Value>> const& { return _map; }
    [[nodiscard]] auto underlying_container() -> std::vector<std::pair<Key, Value>>& { return _map; }

private:
    std::vector<std::pair<Key, Value>> _map;
};

} // namespace reg::internal