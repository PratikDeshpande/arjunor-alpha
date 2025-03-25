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
}

}
