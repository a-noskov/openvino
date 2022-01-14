
// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <reorg_yolo_shape_inference.hpp>

#include "utils.hpp"

using namespace ov;
using namespace std;

TEST(StaticShapeInferenceTest, ReorgYoloV0) {
    size_t stride = 2;
    auto data_param = make_shared<op::v0::Parameter>(element::f32, ov::PartialShape{-1, -1, -1, -1});
    auto op = make_shared<op::v0::ReorgYolo>(data_param, stride);

    check_static_shape(op.get(), {ov::StaticShape{1, 64, 26, 26}}, {ov::StaticShape{1, 256, 13, 13}});
}