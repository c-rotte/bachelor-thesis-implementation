#pragma once

#include <benchmark/benchmark.h>

namespace bm = benchmark;

namespace ucsb {

/**
 * @brief Trivial Google Benchmark wrapper.
 * No added value here :)
 */
struct timer_ref_t {
    inline timer_ref_t(bm::State& state) : state_(&state) {}

    inline void pause() { state_->PauseTiming(); }
    inline void resume() { state_->ResumeTiming(); }

  private:
    bm::State* state_;
};

} // namespace ucsb