//
// Created by Alex Day on 7/28/26.
//

#include "value_graph.h"

#include <algorithm>  // for clamp, minmax_element
#include <cstddef>
#include <utility>
#include <vector>

ValueGraph::ValueGraph(std::vector<float> data) : data_(std::move(data)) {
    if (data_.empty()) return;
    auto [lo, hi] = std::minmax_element(data_.begin(), data_.end());
    min_ = *lo;
    max_ = *hi;
    if (max_ <= min_) max_ = min_ + 1.0f;  // flat series -> avoid /0
}

std::vector<int> ValueGraph::operator()(int width, int height) const {
    std::vector<int> out(std::max(width, 0), 0);
    if (data_.empty() || width <= 0 || height <= 0) return out;

    const auto n = data_.size();
    for (int x = 0; x < width; ++x) {
        // Column -> fractional index into the series.
        const float t = (width == 1) ? 0.f : float(x) / float(width - 1);
        const float pos = t * float(n - 1);
        const auto i0 = std::size_t(pos);
        const auto i1 = std::min(i0 + 1, n - 1);
        const float frac = pos - float(i0);
        const float v = data_[i0] * (1.f - frac) + data_[i1] * frac;

        const float norm = (v - min_) / (max_ - min_);
        out[x] = std::clamp(int(norm * float(height - 1)), 0, height - 1);
    }
    return out;
}
