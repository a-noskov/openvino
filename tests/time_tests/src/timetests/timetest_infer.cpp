// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <inference_engine.hpp>
#include <ie_plugin_config.hpp>
#include <iostream>

#include "common_utils.h"
#include "timetests_helper/timer.h"
#include "timetests_helper/utils.h"
using namespace InferenceEngine;


/**
 * @brief Function that contain executable pipeline which will be called from
 * main(). The function should not throw any exceptions and responsible for
 * handling it by itself.
 */
int runPipeline(const std::string &model, const std::string &device, const std::string &performanceHint,
                const bool isCacheEnabled, const std::string &vpuCompiler) {
  auto pipeline = [](const std::string &model, const std::string &device, const std::string &performanceHint,
                     const bool isCacheEnabled, const std::string &vpuCompiler) {
    Core ie;
    CNNNetwork cnnNetwork;
    ExecutableNetwork exeNetwork;
    InferRequest inferRequest;
    size_t batchSize = 0;

    if (!performanceHint.empty()) {
      std::vector<std::string> supported_config_keys = ie.GetMetric(device, METRIC_KEY(SUPPORTED_CONFIG_KEYS));

      // enables performance hint for specified device
      std::string performanceConfig;
      if (performanceHint == "THROUGHPUT")
        performanceConfig = CONFIG_VALUE(THROUGHPUT);
      else if (performanceHint == "LATENCY")
        performanceConfig = CONFIG_VALUE(LATENCY);

      if (std::find(supported_config_keys.begin(), supported_config_keys.end(), "PERFORMANCE_HINT") ==
          supported_config_keys.end()) {
        std::cerr << "Device " << device << " doesn't support config key 'PERFORMANCE_HINT'!\n"
                  << "Performance config was not set.";
      }
      else
        ie.SetConfig({{CONFIG_KEY(PERFORMANCE_HINT), performanceConfig}}, device);
    }

    // set config for VPUX device
    std::map<std::string, std::string> vpuConfig = {};
    if (vpuCompiler == "MCM")
      vpuConfig = {{"VPUX_COMPILER_TYPE", "MCM"}};
    else if (vpuCompiler == "MLIR")
      vpuConfig = {{"VPUX_COMPILER_TYPE", "MLIR"}};

    // first_inference_latency = time_to_inference + first_inference
    {
      SCOPED_TIMER(time_to_inference);
      {
        SCOPED_TIMER(load_plugin);
        ie.GetVersions(device);

        if (isCacheEnabled)
          ie.SetConfig({{CONFIG_KEY(CACHE_DIR), "models_cache"}});
      }
      {
        if (!isCacheEnabled) {
          SCOPED_TIMER(create_exenetwork);

          if (TimeTest::fileExt(model) == "blob") {
            SCOPED_TIMER(import_network);
            exeNetwork = ie.ImportNetwork(model, device);
          }
          else {
            {
              SCOPED_TIMER(read_network);
              cnnNetwork = ie.ReadNetwork(model);
              batchSize = cnnNetwork.getBatchSize();
            }

            {
              SCOPED_TIMER(load_network);
              exeNetwork = ie.LoadNetwork(cnnNetwork, device, vpuConfig);
            }
          }
        }
        else {
          SCOPED_TIMER(load_network);
          exeNetwork = ie.LoadNetwork(model, device);
        }
      }
      inferRequest = exeNetwork.CreateInferRequest();
    }

    {
      SCOPED_TIMER(first_inference);
      {
        SCOPED_TIMER(fill_inputs);
        const InferenceEngine::ConstInputsDataMap inputsInfo(exeNetwork.GetInputsInfo());
        batchSize = batchSize != 0 ? batchSize : 1;
        fillBlobs(inferRequest, inputsInfo, batchSize);
      }
      inferRequest.Infer();
    }
  };

  try {
    pipeline(model, device, performanceHint, isCacheEnabled, vpuCompiler);
  } catch (const InferenceEngine::Exception &iex) {
    std::cerr
        << "Inference Engine pipeline failed with Inference Engine exception:\n"
        << iex.what();
    return 1;
  } catch (const std::exception &ex) {
    std::cerr << "Inference Engine pipeline failed with exception:\n"
              << ex.what();
    return 2;
  } catch (...) {
    std::cerr << "Inference Engine pipeline failed\n";
    return 3;
  }
  return 0;
}
