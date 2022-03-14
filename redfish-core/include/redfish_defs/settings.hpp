#pragma once
#include <nlohmann/json.hpp>

namespace settings{
// clang-format off

enum class ApplyTime{
    Invalid,
    Immediate,
    OnReset,
    AtMaintenanceWindowStart,
    InMaintenanceWindowOnReset,
};

NLOHMANN_JSON_SERIALIZE_ENUM(ApplyTime, {
    {ApplyTime::Invalid, "Invalid"},
    {ApplyTime::Immediate, "Immediate"},
    {ApplyTime::OnReset, "OnReset"},
    {ApplyTime::AtMaintenanceWindowStart, "AtMaintenanceWindowStart"},
    {ApplyTime::InMaintenanceWindowOnReset, "InMaintenanceWindowOnReset"},
});

enum class OperationApplyTime{
    Invalid,
    Immediate,
    OnReset,
    AtMaintenanceWindowStart,
    InMaintenanceWindowOnReset,
    OnStartUpdateRequest,
};

NLOHMANN_JSON_SERIALIZE_ENUM(OperationApplyTime, {
    {OperationApplyTime::Invalid, "Invalid"},
    {OperationApplyTime::Immediate, "Immediate"},
    {OperationApplyTime::OnReset, "OnReset"},
    {OperationApplyTime::AtMaintenanceWindowStart, "AtMaintenanceWindowStart"},
    {OperationApplyTime::InMaintenanceWindowOnReset, "InMaintenanceWindowOnReset"},
    {OperationApplyTime::OnStartUpdateRequest, "OnStartUpdateRequest"},
});

}
// clang-format on
