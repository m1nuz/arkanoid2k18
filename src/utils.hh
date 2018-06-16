#pragma once

#include <optional>

#include <fstream>
inline auto get_config(std::string_view path) -> std::optional<std::string> {
    using namespace std;

    ifstream fs(path.data(), ios::in | ios::binary);

    if (!fs.is_open())
        return {};

    string contents;
    fs.seekg(0, ios::end);
    contents.resize(fs.tellg());
    fs.seekg(0, ios::beg);
    fs.read(&contents[0], contents.size());
    fs.close();

    return contents;
}

#include <random>
inline auto random(const int start, const int end) -> int {
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist(start, end);

    return dist(rng);
}

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
