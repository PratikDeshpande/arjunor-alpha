#include "eval.h"

namespace eval {
    void eval_ping(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {

        if (arguments.size() >= 2) {
            std::cout << "Error: Too many arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'ping' command");
        }

        if (arguments.size() == 1) {
            std::vector<std::byte> response_byte_vector = resp::encode_bulk_string(arguments[0]);
            auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
            if (bytes_sent <= 0) {
                std::cout << "Error sending bytes" << std::endl;
                throw std::invalid_argument("Error sending bytes");
            }
        } else {
            std::vector<std::byte> response_byte_vector = resp::encode_simple_string("PONG");
            auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
            if (bytes_sent <= 0) {
                std::cout << "Error sending bytes" << std::endl;
                throw std::invalid_argument("Error sending bytes");
            }
        }

    }

    void eval_set(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {
        if (arguments.size() != 2) { // TODO: Add support for TTL option
            std::cout << "Error: Wrong number of arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'set' command");
        } else {
            
            auto key = arguments[0];
            auto value = arguments[1];
            auto object = std::make_shared<store::Object>();

            for (auto strChar: value) {
                object->data.push_back(std::byte(strChar));
            }
            object->timestamp = std::time(nullptr);
            object_store->insert(key, object);
            
            std::vector<std::byte> response_byte_vector = resp::encode_simple_string("OK");
            auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
            if (bytes_sent <= 0) {
                std::cout << "Error sending bytes" << std::endl;
                throw std::invalid_argument("Error sending bytes");
            }
                
        }
    }

    void eval_get(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {
        if (arguments.size() != 1) {
            std::cout << "Error: Wrong number of arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'get' command");
        } else {
            auto key = arguments[0];
            auto object = object_store->get(key);
            if (object != nullptr) {
                auto object_data = object->data;
                std::string data_string(  reinterpret_cast<const char*>(object_data.data()), reinterpret_cast<const char*>(object_data.data()  + object_data.size() ));

                std::vector<std::byte> response_byte_vector = resp::encode_bulk_string(data_string);
                auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
                if (bytes_sent <= 0) {
                    std::cout << "Error sending bytes" << std::endl;
                    throw std::invalid_argument("Error sending bytes");
                }
                    

            } else {
                std::vector<std::byte> response_byte_vector = resp::encode_null();
                auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
                if (bytes_sent <= 0) {
                    std::cout << "Error sending bytes" << std::endl;
                    throw std::invalid_argument("Error sending bytes");
                }
            }
        }
    }

    /*
    
    pc.create_index(
        name=index_name,
        vector_type="dense",
        dimension=1536,
        metric="cosine",
    )
    
    */

    void eval_vectorindex(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {
        // Extract args
        // name, vector_type, dimension, metric
        if (arguments.size() != 4) {
            std::cout << "Error: Wrong number of arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'vectorindex' command");
        } else {
            auto name = arguments[0];
            auto vector_type = arguments[1];
            auto dimension = arguments[2];
            auto metric = arguments[3];

            std::ostringstream user_params;
            user_params << "name: " << name << " vector_type: " << vector_type << " dimension: " << dimension << " metric: " << metric << std::endl;
            std::string user_params_log = user_params.str();
            std::cout << user_params_log << std::endl;

            // TODO: Figure out how to represent an index and store this in memory

            std::vector<std::byte> response_byte_vector = resp::encode_simple_string("OK");
            auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
            if (bytes_sent <= 0) {
                std::cout << "Error sending bytes" << std::endl;
                throw std::invalid_argument("Error sending bytes");
            }
        }
    }

    /*


    // for each index, create {}        // Index is a session
        // within an index you have a name space (analogous to a workspace)
         //     upert upsert vectors into workspace
    
    index.upsert(
  vectors=[
    {
      "id": "A", 
      "values": [0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1], 
      "metadata": {"genre": "comedy", "year": 2020}
    },
    {
      "id": "B", 
      "values": [0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2],
      "metadata": {"genre": "documentary", "year": 2019}
    },
    {
      "id": "C", 
      "values": [0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3],
      "metadata": {"genre": "comedy", "year": 2019}
    },
    {
      "id": "D", 
      "values": [0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4],
      "metadata": {"genre": "drama"}
    }
  ],
  namespace="example-namespace"
)
    
    */

    // what if you can receive JSON directly and process that?
    void eval_upsert(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {
        // expect namespace, vectors (array of vectors)
        // Vector: id, values, metadata

        // vector format:
        // '["Deimos", {"crashes": 0}, null]'
        // '[ {"id": "A", "values": [0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1], "metadata": {"genre": "comedy", "year": 2020} }, {"id": "B", "values":  [0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2], "metadata": {"genre": "documentary", "year": 2019} } ]'

        if (arguments.size() != 2) {
            std::cout << "Error: Wrong number of arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'upsert' command");
        }

        auto name_space = arguments[0];
        auto vectors = arguments[1];
        // Parse the vectors json

        std::ostringstream user_params;
        user_params << "namespace: " << name_space << " vectors: " << vectors << std::endl;
        std::string user_params_log = user_params.str();
        std::cout << user_params_log << std::endl;

        
        // figure out how to store vectors in memory
        // for now, just return OK

        std::vector<std::byte> response_byte_vector = resp::encode_simple_string("OK");
        auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
        if (bytes_sent <= 0) {
            std::cout << "Error sending bytes" << std::endl;
            throw std::invalid_argument("Error sending bytes");
        }
    }


    /*
    
    index.query(
    namespace="example-namespace",
    vector=[0.0236663818359375,-0.032989501953125, ..., -0.01041412353515625,0.0086669921875], 
    top_k=3,
    include_metadata=True,
    include_values=False
    )

    RESULT:

    {'matches': [{'id': 'rec3',
              'metadata': {'category': 'immune system',
                           'chunk_text': 'Rich in vitamin C and other '
                                          'antioxidants, apples contribute to '
                                          'immune health and may reduce the '
                                          'risk of chronic diseases.'},
              'score': 0.82026422,
              'values': []},
             {'id': 'rec1',
              'metadata': {'category': 'digestive system',
                           'chunk_text': 'Apples are a great source of '
                                          'dietary fiber, which supports '
                                          'digestion and helps maintain a '
                                          'healthy gut.'},
              'score': 0.793068111,
              'values': []},
             {'id': 'rec4',
              'metadata': {'category': 'endocrine system',
                           'chunk_text': 'The high fiber content in apples '
                                          'can also help regulate blood sugar '
                                          'levels, making them a favorable '
                                          'snack for people with diabetes.'},
              'score': 0.780169606,
              'values': []}],
 'namespace': 'example-namespace',
 'usage': {'read_units': 6}}

    
    */

    void eval_search(std::vector<std::string> arguments, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {
        if (arguments.size() != 3) {
            std::cout << "Error: Wrong number of arguments" << std::endl;
            throw std::invalid_argument("ERR wrong number of arguments for 'search' command");
        }

        auto name_space = arguments[0];
        auto vector = arguments[1];
        auto top_k = arguments[2];

        std::ostringstream user_params;
        user_params << "namespace: " << name_space << " vector: " << vector << " top_k: " << top_k << std::endl;
        std::string user_params_log = user_params.str();
        std::cout << user_params_log << std::endl;


        // TODO: initialize namespace if not initialized
        //      - load all vectors into memory
        //      - for vector in vectors, perform euclidean distance calculation
        //      - return top k vectors

        struct multimedia_vector {
            std::string id;
            std::string metadata;
            //float score;
            std::vector<float> values;
        };

        // map: id: score. find the one with highest score
        //std::unordered_map<std::string, float> map;


        //std::vector<float> vector_floats;

        // figure out how to search vectors in memory
        // for now, just return OK

        std::vector<std::byte> response_byte_vector = resp::encode_simple_string("OK");
        auto bytes_sent = send(new_fd, response_byte_vector.data(), response_byte_vector.size(), 0);
        if (bytes_sent <= 0) {
            std::cout << "Error sending bytes" << std::endl;
            throw std::invalid_argument("Error sending bytes");
        }
    }

    void eval_and_respond(std::shared_ptr<cmd::RedisCommand> command, int new_fd, std::shared_ptr<store::ObjectStore> object_store) {

        if (command->name == cmd::CommandName::Ping) {
            eval_ping(command->arguments, new_fd, object_store);
        } else if (command->name == cmd::CommandName::Set) {
            eval_set(command->arguments, new_fd, object_store);
        } else if (command->name == cmd::CommandName::Get) {
            eval_get(command->arguments, new_fd, object_store);
        } else if (command->name == cmd::CommandName::VectorIndex) {
            eval_vectorindex(command->arguments, new_fd, object_store);
        } else if (command->name == cmd::CommandName::Upsert) {
            eval_upsert(command->arguments, new_fd, object_store);
        } else if (command->name == cmd::CommandName::Search) {
            eval_search(command->arguments, new_fd, object_store);
        } else {
            eval_ping(command->arguments, new_fd, object_store);
        }


    }
}