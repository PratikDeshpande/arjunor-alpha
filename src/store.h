#include <vector>
#include <string>
#include <cstddef>
#include <unordered_map>
#include <memory>

// NOTE: Not thread safe!

namespace store {
// TODO: make this singleton, flush to disk, load from disk (not redis features)
// TODO: use mutex if in multithreaded mode
struct Object {
    // TODO: store data's type info?
    std::vector<std::byte> data;
    int64_t timestamp;
};

class ObjectStore {
    public:
        void insert(const std::string& key, std::shared_ptr<Object> value);
        std::shared_ptr<Object> get(const std::string& key);

    private:
        std::unordered_map<std::string, std::shared_ptr<Object>> hashmap_;

};
}