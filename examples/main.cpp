#include <uJSON/ujson.h>
#include <iostream>
#include <fstream>
#include <cassert>

using json = uJSON::Value;

void test_parsing() {
    std::cout << "[Test] Parsing start" << std::endl;
    std::string json_str = R"(
        {
            "name": "uJSON",
            "version": 1.0,
            "features": ["fast", "lightweight", "no-dependencies"],
            "is_awesome": true,
            "meta": {
                "author": "Trae",
                "year": 2025
            }
        }
    )";

    json j = json::parse(json_str);
    std::cout << "[Test] Parse success" << std::endl;

    assert(j.is_object());
    std::cout << "[Test] is_object check passed" << std::endl;
    
    std::string name = j["name"].get<std::string>();
    std::cout << "[Test] Name: " << name << std::endl;
    assert(name == "uJSON");
    
    double version = j["version"].get<double>();
    std::cout << "[Test] Version: " << version << std::endl;
    assert(version == 1.0);
    
    assert(j["is_awesome"].get<bool>() == true);
    
    assert(j["features"].is_array());
    assert(j["features"].size() == 3);
    
    std::string f1 = j["features"][0].get<std::string>();
    std::cout << "[Test] Feature 1: " << f1 << std::endl;
    assert(f1 == "fast");
    
    // Test nested object
    std::string author = j["meta"]["author"].get<std::string>();
    std::cout << "[Test] Author: " << author << std::endl;
    assert(author == "Trae");
    
    std::cout << "Parsing test passed!" << std::endl;
}

void test_generation() {
    std::cout << "[Test] Generation start" << std::endl;
    json j = json::object();
    j["id"] = 123;
    j["name"] = "Test";
    j["tags"] = json::array();
    j["tags"].push_back("a");
    j["tags"].push_back("b");
    
    std::cout << "Generated JSON: " << j << std::endl;
}

void test_iterator() {
    std::cout << "[Test] Iterator start" << std::endl;
    json j = json::object();
    j["a"] = 1;
    j["b"] = 2;
    
    std::cout << "Iterating object:" << std::endl;
    for (auto it = j.begin(); it != j.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}

void test_error_handling() {
    std::cout << "[Test] Error handling start" << std::endl;
    std::string invalid_json = "{ \"key\": "; // Missing value
    try {
        json::parse(invalid_json);
        std::cerr << "Should have thrown ParseError" << std::endl;
        exit(1);
    } catch (const uJSON::ParseError& e) {
        std::cout << "Caught expected ParseError: " << e.what() << std::endl;
    }

    json j = json::object();
    try {
        j.get<int>(); // j is object, not int
        std::cerr << "Should have thrown TypeError" << std::endl;
        exit(1);
    } catch (const uJSON::TypeError& e) {
        std::cout << "Caught expected TypeError: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Running uJSON example v1.3 (Exceptions)..." << std::endl;
    try {
        test_parsing();
        test_generation();
        test_iterator();
        test_error_handling();
        std::cout << "All tests passed!" << std::endl;
    } catch (const uJSON::Exception& e) {
        std::cerr << "uJSON Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Standard Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
