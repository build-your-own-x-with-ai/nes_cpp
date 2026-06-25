#include "render/frame.h"

Frame::Frame() : data_(WIDTH * HEIGHT * 3, 0) {}

void Frame::set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return;
    }
    int base = y * 3 * WIDTH + x * 3;
    data_[base] = r;
    data_[base + 1] = g;
    data_[base + 2] = b;
}

void Frame::set_pixel(int x, int y, const std::array<uint8_t, 3>& rgb) {
    set_pixel(x, y, rgb[0], rgb[1], rgb[2]);
}

const std::vector<uint8_t>& Frame::get_data() const {
    return data_;
}
