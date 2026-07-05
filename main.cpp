//
// Created by PC on 2026/6/25.
//
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>

#include "generate/generator.h"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    bool debug = false;
    std::string config_path;

    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        if (std::string arg = argv[i]; arg == "--debug") {
            debug = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [--debug] <config.json>\n"
                      << "Options:\n"
                      << "  --debug    Enable debug output from generator\n"
                      << "  --help     Show this help message\n";
            return 0;
        } else {
            if (config_path.empty()) {
                config_path = arg;
            } else {
                std::cerr << "Error: Multiple positional arguments provided.\n"
                          << "Usage: " << argv[0] << " [--debug] <config.json>\n";
                return 1;
            }
        }
    }

    if (config_path.empty()) {
        std::cerr << "Error: Missing config file path.\n"
                  << "Usage: " << argv[0] << " [--debug] <config.json>\n";
        return 1;
    }

    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "Failed to open config file: " << config_path << std::endl;
        return 1;
    }

    json config;
    try {
        config_file >> config;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    // 提取必需字段
    std::string script = config.value("script", "");
    std::string out = config.value("out", "");
    std::string ns = config.value("namespace", "");
    std::string name = config.value("name", "");

    if (script.empty() || out.empty() || ns.empty() || name.empty()) {
        std::cerr << "Missing required fields: script, out, namespace, name" << std::endl;
        return 1;
    }

    // 创建生成器
    cc::gen::Generator generator(name, ns);
    generator.set_print_debug_info(debug);

    try {
        generator.Build(script, out);
    } catch (const std::exception& e) {
        std::cerr << "Generation failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}