#include <iostream>
#include "store.h"

namespace store {


void ObjectStore::insert(const std::string& key, std::shared_ptr<Object> value) {
    hashmap_[key] = value;
};

std::shared_ptr<Object> ObjectStore::get(const std::string& key) {
    if (hashmap_.count(key) == 1) { 
        return hashmap_[key];
    } else {
        return nullptr; 
    }
};

void ObjectStore::create_namespace(const std::string& vector_index_name, const std::string& namespace_name) {
    // TODO: implememnt vector index (session)
    // check if its there
    if (namespace_multimedia_vectors_.count(namespace_name) == 0) {
        namespace_multimedia_vectors_[namespace_name] = std::vector<multimedia_vector>();
    }

}


}
