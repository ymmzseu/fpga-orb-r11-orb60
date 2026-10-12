#pragma once
#include <cstdint>

namespace orb60 {
constexpr uint32_t kLayerVersion = UINT32_C(0x00600001);
/** Counters describe this layer; dataflow_cycles excludes final statistics write.
 * AXI byte counts describe issued payload, including final 64-bit padding.
 * Host functional execution has no cycle model and produces dataflow_cycles=1.
 */
struct LayerStats {
    uint32_t version, status, level, width, height;
    uint32_t input_read_bytes, blur_write_bytes, resize_write_bytes;
    uint32_t image_pairs, blur_pairs, resize_pairs, dataflow_cycles, flags;
    uint32_t reserved[3];
};
static_assert(sizeof(LayerStats) == 64, "LayerStats ABI is 64 bytes");
}

/** One shared DDR read, exact FAST/Gaussian/Resize outputs, four AXI masters.
 * At level 7 resize output is untouched. Invalid arguments touch only summary
 * and statistics. Selected-capacity failure still produces the legal images.
 * The Point and Result byte layouts and ordering remain unchanged.
 */
extern "C" int fastv2_layer2_hls(const uint64_t* image, uint64_t* blurred,
    uint64_t* resized, uint64_t* selected, uint64_t* result, uint64_t* stats,
    int level, int budget, int high_threshold, int low_threshold,
    int dual_threshold, int selected_capacity);
