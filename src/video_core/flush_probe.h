// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// FLUSHPROBE (diagnostic build, not for merging): why guest CPU reads of GPU-written memory still
// flush. The read's GetFlushArea records the branch that decided it on the reading thread;
// GPU::OnCPURead counts the flushes per branch with the time the read spent waiting for each.

#pragma once

#include <array>
#include <atomic>
#include <cstddef>

#include "common/common_types.h"

namespace VideoCommon::FlushProbe {

enum class Reason : u8 {
    None,
    Texture,     ///< The texture cache holds GPU data there (its area comes first)
    FirstRead,   ///< Not preflushable yet: the read marks it and flushes
    Uncommitted, ///< Preflushable, GPU writes not committed to a fence yet
    Committed,   ///< Preflushable, committed but not downloading yet
    NoDownload,  ///< Preflushable and GPU-modified, no download in flight
    WaitTimeout, ///< Still downloading after OnCPURead's waits
    Count,
};

inline constexpr std::array<const char*, static_cast<size_t>(Reason::Count)> names{
    "none", "texture cache", "first read", "uncommitted writes", "committed, not downloading",
    "no download in flight", "wait timed out",
};

inline thread_local Reason t_reason{};

inline std::array<std::atomic<u64>, static_cast<size_t>(Reason::Count)> flushes{};
inline std::array<std::atomic<u64>, static_cast<size_t>(Reason::Count)> flush_ns{};
inline std::atomic<u64> reads{};
inline std::atomic<u64> waits{};
inline std::atomic<u64> wait_ns{};

inline void Set(Reason reason) noexcept {
    t_reason = reason;
}

} // namespace VideoCommon::FlushProbe
