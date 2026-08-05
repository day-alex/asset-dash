//
// Created by Alex Day on 7/28/26.
//

#ifndef ASSET_DASH_VALUE_GRAPH_H
#define ASSET_DASH_VALUE_GRAPH_H

#include <algorithm>  // for clamp, minmax_element
#include <cstddef>
#include <utility>
#include <vector>

class ValueGraph {
public:
    explicit ValueGraph(std::vector<float> data);

    std::vector<int> operator()(int width, int height) const;

private:
    std::vector<float> data_;
    float min_ = 0.f, max_ = 1.f;
};


#endif //ASSET_DASH_VALUE_GRAPH_H
