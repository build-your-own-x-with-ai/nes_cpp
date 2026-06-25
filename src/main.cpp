#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>
#include "cartridge.h"
#include "bus.h"
#include "cpu.h"
#include "joypad.h"
#include "ppu/ppu.h"
#include "render/frame.h"
#include "render/render.h"

const int TILE_SIZE = 8;
const int TILES_PER_ROW = 16;
const int TILE_PADDING = 2;
const int TILE_VIEWER_OFFSET_X = 256 + 16;
const int TILE_VIEWER_OFFSET_Y = 8;
const int TEXTURE_WIDTH = 256 * 2;
const int TEXTURE_HEIGHT = 240;

void render_tile_viewer(NesPPU& ppu, std::vector<uint8_t>& texture_data) {
    for (int bank = 0; bank < 2; bank++) {
        int bank_start = bank * 0x1000;
        int bank_offset_y = TILE_VIEWER_OFFSET_Y + bank * (128 + TILE_PADDING * 2);

        for (int tile_idx = 0; tile_idx < 256; tile_idx++) {
            int tile_x = (tile_idx % TILES_PER_ROW) * (TILE_SIZE + TILE_PADDING);
            int tile_y = (tile_idx / TILES_PER_ROW) * (TILE_SIZE + TILE_PADDING);

            int tile_start = bank_start + tile_idx * 16;

            for (int y = 0; y < 8; y++) {
                uint8_t upper = ppu.chr_rom[tile_start + y];
                uint8_t lower = ppu.chr_rom[tile_start + y + 8];

                for (int x = 7; x >= 0; x--) {
                    uint8_t value = ((upper & 1) << 1) | (lower & 1);
                    upper >>= 1;
                    lower >>= 1;

                    uint8_t r, g, b;
                    switch (value) {
                        case 0: r = 32; g = 32; b = 32; break;
                        case 1: r = 96; g = 96; b = 96; break;
                        case 2: r = 160; g = 160; b = 160; break;
                        case 3: r = 255; g = 255; b = 255; break;
                        default: r = 0; g = 0; b = 0; break;
                    }

                    int px = TILE_VIEWER_OFFSET_X + tile_x + x;
                    int py = bank_offset_y + tile_y + y;

                    if (px >= 0 && px < TEXTURE_WIDTH && py >= 0 && py < TEXTURE_HEIGHT) {
                        int idx = (py * TEXTURE_WIDTH + px) * 3;
                        texture_data[idx] = r;
                        texture_data[idx + 1] = g;
                        texture_data[idx + 2] = b;
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom_file>" << std::endl;
        return 1;
    }

    std::string rom_path = argv[1];

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Tile viewer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        TEXTURE_WIDTH * 2, TEXTURE_HEIGHT * 2,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        TEXTURE_WIDTH, TEXTURE_HEIGHT
    );
    if (!texture) {
        std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Cartridge cartridge;
    if (!cartridge.load_from_file(rom_path)) {
        std::cerr << "Failed to load ROM file: " << rom_path << std::endl;
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<uint8_t> texture_data(TEXTURE_WIDTH * TEXTURE_HEIGHT * 3, 0);
    Frame frame;
    bool running = true;

    std::unordered_map<SDL_Keycode, JoypadButton> key_map = {
        {SDLK_DOWN, JoypadButton::DOWN},
        {SDLK_UP, JoypadButton::UP},
        {SDLK_RIGHT, JoypadButton::RIGHT},
        {SDLK_LEFT, JoypadButton::LEFT},
        {SDLK_SPACE, JoypadButton::SELECT},
        {SDLK_RETURN, JoypadButton::START},
        {SDLK_a, JoypadButton::BUTTON_A},
        {SDLK_s, JoypadButton::BUTTON_B},
    };

    Bus bus(cartridge.get_rom(), [&](NesPPU& ppu, Joypad& joypad) {
        render::render(ppu, frame);

        const std::vector<uint8_t>& frame_data = frame.get_data();
        for (int y = 0; y < 240; y++) {
            for (int x = 0; x < 256; x++) {
                int src_idx = (y * 256 + x) * 3;
                int dst_idx = (y * TEXTURE_WIDTH + x) * 3;
                texture_data[dst_idx] = frame_data[src_idx];
                texture_data[dst_idx + 1] = frame_data[src_idx + 1];
                texture_data[dst_idx + 2] = frame_data[src_idx + 2];
            }
        }

        render_tile_viewer(ppu, texture_data);

        SDL_UpdateTexture(texture, nullptr, texture_data.data(), TEXTURE_WIDTH * 3);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    } else {
                        auto it = key_map.find(event.key.keysym.sym);
                        if (it != key_map.end()) {
                            joypad.set_button_pressed(it->second, true);
                        }
                    }
                    break;
                case SDL_KEYUP: {
                    auto it = key_map.find(event.key.keysym.sym);
                    if (it != key_map.end()) {
                        joypad.set_button_pressed(it->second, false);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    });

    CPU cpu(bus);
    cpu.reset();

    try {
        cpu.run_with_callback([&](CPU*) {
            if (!running) {
                throw std::runtime_error("quit");
            }
        });
    } catch (const std::runtime_error&) {
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
