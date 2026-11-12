#pragma once
#include <vector>
#include <string>
#include <utility>
#include <cstdint>

struct MockCapture
{
    unsigned sample_rate = 12500000;
    std::vector<unsigned long long> edges; // sample numbers of each transition; line idles high before the first
};
struct MockFrame
{
    std::string type;
    unsigned long long start, end;
    std::vector<std::pair<std::string, std::string>> kv;
};
struct MockEndOfData {};
extern MockCapture gCapture;
extern std::vector<MockFrame> gFrames;
