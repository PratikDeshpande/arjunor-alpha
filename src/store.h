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

// Vector index is analogous to a sessioin (session id or index id)
// Namespace is analogous to a workspace (namespace id)
// 

struct multimedia_vector {
    std::string id;
    std::string metadata_json;
    std::vector<float> values;
};

class ObjectStore {
    public:
        void insert(const std::string& key, std::shared_ptr<Object> value);
        std::shared_ptr<Object> get(const std::string& key);

        // TODO: Use a different store class for this rather than object store
        //void create_vector_index(const std::string& vector_index_name);
        void create_namespace(const std::string& vector_index_name, const std::string& namespace_name);
        // Make a way to insert multiple vectors
        void insert_multimedia_vector(const std::string& namespace_name, const multimedia_vector& vector);


    private:
        std::unordered_map<std::string, std::shared_ptr<Object>> hashmap_;
        //std::unordered_map<std::string, std::string> index_namespace_map_;
        std::unordered_map<std::string, std::vector<multimedia_vector>> namespace_multimedia_vectors_;

};
}