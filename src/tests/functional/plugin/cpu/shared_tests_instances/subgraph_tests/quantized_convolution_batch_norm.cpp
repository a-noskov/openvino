// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "subgraph_tests/quantized_convolution_batch_norm.hpp"

using namespace SubgraphTestsDefinitions;

namespace {

INSTANTIATE_TEST_SUITE_P(smoke_QuantizedConvolutionBatchNorm, QuantizedConvolutionBatchNorm,
                         ::testing::Values(CommonTestUtils::DEVICE_CPU),
                         QuantizedConvolutionBatchNorm::getTestCaseName);

} // namespace
