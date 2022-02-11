// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <algorithm>
#include <memory>
#include <numeric>

#include "transformations/resolve_names_collisions.hpp"
#include "openvino/op/parameter.hpp"
#include "openvino/op/result.hpp"
#include "openvino/op/sink.hpp"

bool ov::pass::ResolveNameCollisions::run_on_model(const std::shared_ptr<ov::Model>& model) {
    // Next containers are used to fix collisions in autogenerated names
    // The final list of nodes with collisions
    std::vector<Node*> nodes_with_conflicts;
    std::unordered_map<std::string, std::list<Node*>> visited_nodes;

    for (const auto& node : model->get_ordered_ops()) {
        // Detect names collisions only for nodes with autogenerated names
        const auto& friendly_name = node->get_friendly_name();
        visited_nodes[friendly_name].emplace_back(node.get());
    }

    for (const auto& l_nodes : visited_nodes) {
        if (l_nodes.second.size() == 1)
            continue;
        const size_t nodes_size = l_nodes.second.size();
        bool has_public_node = false; // Parameter, Result ans Sinks
        size_t i(0);
        for (auto* node : l_nodes.second) {
            i++;
            // Skip the last node if we don't have public nodes with collisions
            if (i == nodes_size && !has_public_node)
                break;
            if (dynamic_cast<const ov::op::v0::Result*>(node)) {
                // Result is a service node
                continue;
            }
            if (dynamic_cast<const ov::op::Sink*>(node) ||
                dynamic_cast<const ov::op::v0::Parameter*>(node)) {
                // Resolve names for public ops with autogenerated name
                if (node->m_friendly_name.empty())
                    nodes_with_conflicts.emplace_back(node);
                has_public_node = true;
                continue;
            } else {
                // For result we need to avoid changes in previous operation
                bool is_public = false;
                for (const auto& output : node->outputs()) {
                    for (const auto input : output.get_target_inputs()) {
                        if (dynamic_cast<const ov::op::v0::Result*>(input.get_node())) {
                            has_public_node = true;
                            is_public = true;
                            break;
                        }
                    }
                    if (is_public)
                        break;
                }
                if (is_public)
                    continue;
            }
            nodes_with_conflicts.emplace_back(node);
        }
    }

    // Resolve names collisions
    for (auto* node : nodes_with_conflicts) {
        size_t idx = 2;
        const auto friendly_name = node->get_friendly_name();
        while (visited_nodes.find(friendly_name + "_" + std::to_string(idx)) != visited_nodes.end())
            idx++;
        const auto new_friendly_name = friendly_name + "_" + std::to_string(idx);
        node->set_friendly_name(new_friendly_name);
        visited_nodes[new_friendly_name].emplace_back(node);
    }
    return true;
}

