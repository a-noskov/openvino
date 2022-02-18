// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <list>
#include <memory>
#include <vector>

#include "openvino/core/core_visibility.hpp"
#include "openvino/core/deprecated.hpp"
#include "openvino/core/enum_mask.hpp"
#include "openvino/core/model.hpp"
#include "openvino/core/node.hpp"
#include "openvino/pass/pass_config.hpp"

namespace ov {
namespace pass {
enum class PassProperty : uint32_t {
    // Pass requires node shapes to be static
    REQUIRE_STATIC_SHAPE = 0x1,
    // Pass transformation will change the function's dynamic state
    CHANGE_DYNAMIC_STATE = 1 << 1,
};

using PassPropertyMask = ov::EnumMask<PassProperty>;

class OPENVINO_API PassBase {
    friend class Manager;

public:
    PassBase();
    virtual ~PassBase() = default;
    /// Check if this pass has all the pass properties.
    bool get_property(const PassPropertyMask& prop_mask) const;

    void set_name(const std::string& name) {
        m_name = name;
    }
    std::string get_name() const;

    /// \brief Set callback for particular transformation type.
    /// This method set global callback. For more details see PassConfig class
    /// documentation.
    /// \param callback lambda function that takes node and returns bool
    void set_callback(const param_callback& callback);

    /// \brief Set PassConfig for particular transformation instance
    /// \param pass_config is a PassConfig shared_ptr
    virtual void set_pass_config(const std::shared_ptr<PassConfig>& pass_config) {
        m_pass_config = pass_config;
    }

    /// \brief Allows to access PassConfig shared instance
    /// \return Shared instance of PassConfig class
    std::shared_ptr<PassConfig> get_pass_config() {
        return m_pass_config;
    }
    /// \brief Applies callback for given node. By default callback returns false.
    /// This method remains here only for backward compatibility and will be removed
    /// after all transformations are moved to transformation_callback() method.
    /// \return result of callback execution for given node
    OPENVINO_DEPRECATED("Please use transformation_callback method instead")
    bool m_transformation_callback(const std::shared_ptr<const Node>& node) {
        return m_pass_config->get_callback(get_type_info())(node);
    }

    /// \brief Applies callback for given node. By default callback returns false.
    /// \param node which will be used inside callback
    /// \return result of callback execution for given node
    bool transformation_callback(const std::shared_ptr<const Node>& node) {
        return m_pass_config->get_callback(get_type_info())(node);
    }

    using type_info_t = DiscreteTypeInfo;

    virtual const type_info_t& get_type_info() const = 0;

protected:
    void set_property(const PassPropertyMask& prop, bool value);

private:
    PassPropertyMask m_property;

    std::string m_name;
    std::shared_ptr<PassConfig> m_pass_config;
};

class OPENVINO_API ModelPass : public PassBase {
public:
    OPENVINO_RTTI("ov::pass::ModelPass");
    ~ModelPass() override;
    OPENVINO_DEPRECATED("run_on_function() method is deprecated. Please use run_on_model() instead.")
    virtual bool run_on_function(std::shared_ptr<ov::Model> m);
    virtual bool run_on_model(const std::shared_ptr<ov::Model>& m);

private:
    bool call_on_function{false};
    bool call_on_model{false};
};

}  // namespace pass
}  // namespace ov
