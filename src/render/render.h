#pragma once

#include "ppu/ppu.h"
#include "render/frame.h"

namespace render {
    void render(const NesPPU& ppu, Frame& frame);
}
