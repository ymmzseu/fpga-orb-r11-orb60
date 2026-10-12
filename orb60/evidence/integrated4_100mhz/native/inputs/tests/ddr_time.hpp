#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

// DDR stays at the100MHz reference rate for a125MHz accelerator.
// Every HP is800MB/s; aggregate capacity is800/1600MB/s.
struct DdrTimeBudget {
    static constexpr uint64_t beat_ps=10000;
    unsigned period_ps,beats;
    uint64_t aggregate=0;
    uint64_t phase=0;
    std::array<uint64_t,4> hp{};
    DdrTimeBudget(unsigned period,unsigned count):period_ps(period),beats(count) {}
    void advance() {
        // Refill at each10ns reference epoch, quantized onto the PL clock.
        // Unused slots do not accumulate into an artificial DDR burst.
        phase+=period_ps;
        if(phase>=beat_ps) {
            phase-=beat_ps;aggregate=uint64_t(beats)*beat_ps;
            hp.fill(beat_ps);
        }
    }
    bool available(unsigned port) const { return aggregate>=beat_ps && hp[port]>=beat_ps; }
    void consume(unsigned port) { aggregate-=beat_ps;hp[port]-=beat_ps; }
    unsigned reference_cycles(unsigned cycles) const {
        return unsigned((uint64_t(cycles)*beat_ps+period_ps-1)/period_ps);
    }
};
