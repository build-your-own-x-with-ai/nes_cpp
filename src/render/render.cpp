#include "render/render.h"
#include "render/palette.h"
#include <cstdio>

namespace render {

static std::array<uint8_t, 4> bg_palette(const NesPPU& ppu, const uint8_t* attribute_table, int tile_column, int tile_row) {
    int attr_table_idx = (tile_row / 4) * 8 + (tile_column / 4);
    uint8_t attr_byte = attribute_table[attr_table_idx];

    int attr_quad_x = (tile_column % 4) / 2;
    int attr_quad_y = (tile_row % 4) / 2;
    int attr_shift = (attr_quad_y * 2 + attr_quad_x) * 2;
    uint8_t palette_idx = (attr_byte >> attr_shift) & 0x03;

    int palette_start = 1 + palette_idx * 4;
    return {
        ppu.palette_table[0],
        ppu.palette_table[palette_start],
        ppu.palette_table[palette_start + 1],
        ppu.palette_table[palette_start + 2]
    };
}

static std::array<uint8_t, 4> sprite_palette(const NesPPU& ppu, uint8_t palette_idx) {
    int start = 0x11 + palette_idx * 4;
    return {
        0,
        ppu.palette_table[start],
        ppu.palette_table[start + 1],
        ppu.palette_table[start + 2]
    };
}

struct Rect {
    int x1, y1, x2, y2;
    Rect(int a, int b, int c, int d) : x1(a), y1(b), x2(c), y2(d) {}
};

static void render_name_table(const NesPPU& ppu, Frame& frame, const uint8_t* name_table,
                              const Rect& view_port, int shift_x, int shift_y) {
    uint16_t bank = ppu.ctrl.bknd_pattern_addr();
    const uint8_t* attribute_table = &name_table[0x3c0];

    for (int i = 0; i < 0x3c0; i++) {
        int tile_column = i % 32;
        int tile_row = i / 32;
        uint8_t tile_idx = name_table[i];
        const uint8_t* tile = &ppu.chr_rom[bank + tile_idx * 16];
        auto palette = bg_palette(ppu, attribute_table, tile_column, tile_row);

        for (int y = 0; y <= 7; y++) {
            uint8_t upper = tile[y];
            uint8_t lower = tile[y + 8];

            for (int x = 7; x >= 0; x--) {
                uint8_t value = ((lower & 1) << 1) | (upper & 1);
                upper >>= 1;
                lower >>= 1;

                std::array<uint8_t, 3> rgb;
                switch (value) {
                    case 0:
                        rgb = Palette::SYSTEM_PALETTE[ppu.palette_table[0] & 0x3F];
                        break;
                    case 1:
                        rgb = Palette::SYSTEM_PALETTE[palette[1] & 0x3F];
                        break;
                    case 2:
                        rgb = Palette::SYSTEM_PALETTE[palette[2] & 0x3F];
                        break;
                    case 3:
                        rgb = Palette::SYSTEM_PALETTE[palette[3] & 0x3F];
                        break;
                    default:
                        rgb = {0, 0, 0};
                        break;
                }

                int pixel_x = tile_column * 8 + x;
                int pixel_y = tile_row * 8 + y;

                if (pixel_x >= view_port.x1 && pixel_x < view_port.x2 &&
                    pixel_y >= view_port.y1 && pixel_y < view_port.y2) {
                    frame.set_pixel(shift_x + pixel_x, shift_y + pixel_y, rgb);
                }
            }
        }
    }
}

void render(const NesPPU& ppu, Frame& frame) {
    uint8_t scroll_x = ppu.scroll.scroll_x;
    uint8_t scroll_y = ppu.scroll.scroll_y;

    const uint8_t* main_nametable;
    const uint8_t* second_nametable;

    uint16_t nt_addr = ppu.ctrl.nametable_addr();

    if ((ppu.mirroring == Mirroring::Vertical && (nt_addr == 0x2000 || nt_addr == 0x2800)) ||
        (ppu.mirroring == Mirroring::Horizontal && (nt_addr == 0x2000 || nt_addr == 0x2400))) {
        main_nametable = &ppu.vram[0];
        second_nametable = &ppu.vram[0x400];
    } else {
        main_nametable = &ppu.vram[0x400];
        second_nametable = &ppu.vram[0];
    }

    render_name_table(ppu, frame, main_nametable,
                      Rect(scroll_x, scroll_y, 256, 240),
                      -scroll_x, -scroll_y);

    if (scroll_x > 0) {
        render_name_table(ppu, frame, second_nametable,
                          Rect(0, 0, scroll_x, 240),
                          256 - scroll_x, 0);
    } else if (scroll_y > 0) {
        render_name_table(ppu, frame, second_nametable,
                          Rect(0, 0, 256, scroll_y),
                          0, 240 - scroll_y);
    }

    for (int i = 252; i >= 0; i -= 4) {
        uint8_t tile_idx = ppu.oam_data[i + 1];
        uint8_t tile_x = ppu.oam_data[i + 3];
        uint8_t tile_y = ppu.oam_data[i];

        bool flip_vertical = (ppu.oam_data[i + 2] & 0x80) != 0;
        bool flip_horizontal = (ppu.oam_data[i + 2] & 0x40) != 0;
        uint8_t palette_idx = ppu.oam_data[i + 2] & 0x03;
        auto spr_palette = sprite_palette(ppu, palette_idx);
        uint16_t bank = ppu.ctrl.sprt_pattern_addr();

        const uint8_t* tile = &ppu.chr_rom[bank + tile_idx * 16];

        for (int y = 0; y <= 7; y++) {
            uint8_t upper = tile[y];
            uint8_t lower = tile[y + 8];

            for (int x = 7; x >= 0; x--) {
                uint8_t value = ((lower & 1) << 1) | (upper & 1);
                upper >>= 1;
                lower >>= 1;

                if (value == 0) {
                    continue;
                }

                std::array<uint8_t, 3> rgb;
                switch (value) {
                    case 1:
                        rgb = Palette::SYSTEM_PALETTE[spr_palette[1] & 0x3F];
                        break;
                    case 2:
                        rgb = Palette::SYSTEM_PALETTE[spr_palette[2] & 0x3F];
                        break;
                    case 3:
                        rgb = Palette::SYSTEM_PALETTE[spr_palette[3] & 0x3F];
                        break;
                    default:
                        continue;
                }

                int px, py;
                if (!flip_horizontal && !flip_vertical) {
                    px = tile_x + x;
                    py = tile_y + y;
                } else if (flip_horizontal && !flip_vertical) {
                    px = tile_x + 7 - x;
                    py = tile_y + y;
                } else if (!flip_horizontal && flip_vertical) {
                    px = tile_x + x;
                    py = tile_y + 7 - y;
                } else {
                    px = tile_x + 7 - x;
                    py = tile_y + 7 - y;
                }

                if (px >= 0 && px < 256 && py >= 0 && py < 240) {
                    frame.set_pixel(px, py, rgb);
                }
            }
        }
    }
}

}
