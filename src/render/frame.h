#pragma once

#include <cstdint>
#include <vector>
#include <array>

class Frame {
public:
    static constexpr int WIDTH = 256;
    static constexpr int HEIGHT = 240;

    Frame();

    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
    void set_pixel(int x, int y, const std::array<uint8_t, 3>& rgb);

    const std::vector<uint8_t>& get_data() const;

private:
    std::vector<uint8_t> data_;
};
