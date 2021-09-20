// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "shared_test_classes/base/layer_test_utils.hpp"
#include "ngraph_functions/builders.hpp"
#include "ngraph_functions/utils/ngraph_helpers.hpp"

namespace SubgraphTestsDefinitions {

typedef std::tuple<
        InferenceEngine::Precision,          // Network Precision
        std::string,                         // Target Device
        std::map<std::string, std::string>,  // Configuration
        std::vector<size_t>                 // Input Shapes
> SplitConcatWith3InputsParams;

class SplitConcatWith3InputsTest : public testing::WithParamInterface<SplitConcatWith3InputsParams>,
                             public LayerTestsUtils::LayerTestsCommon {
public:
    static std::string getTestCaseName(testing::TestParamInfo<SplitConcatWith3InputsParams> obj);

protected:
    void SetUp() override;
};

}  // namespace SubgraphTestsDefinitions