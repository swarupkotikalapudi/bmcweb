#pragma once
#include <nlohmann/json.hpp>

namespace physical_context
{
// clang-format off

enum class PhysicalContext{
    Invalid,
    Room,
    Intake,
    Exhaust,
    LiquidInlet,
    LiquidOutlet,
    Front,
    Back,
    Upper,
    Lower,
    CPU,
    CPUSubsystem,
    GPU,
    GPUSubsystem,
    FPGA,
    Accelerator,
    ASIC,
    Backplane,
    SystemBoard,
    PowerSupply,
    PowerSubsystem,
    VoltageRegulator,
    Rectifier,
    StorageDevice,
    NetworkingDevice,
    ComputeBay,
    StorageBay,
    NetworkBay,
    ExpansionBay,
    PowerSupplyBay,
    Memory,
    MemorySubsystem,
    Chassis,
    Fan,
    CoolingSubsystem,
    Motor,
    Transformer,
    ACUtilityInput,
    ACStaticBypassInput,
    ACMaintenanceBypassInput,
    DCBus,
    ACOutput,
    ACInput,
    TrustedModule,
    Board,
    Transceiver,
    Battery,
    Pump,
};

enum class PhysicalSubContext{
    Invalid,
    Input,
    Output,
};

enum class LogicalContext{
    Invalid,
    Capacity,
    Environment,
    Network,
    Performance,
    Security,
    Storage,
};

NLOHMANN_JSON_SERIALIZE_ENUM(PhysicalContext, { //NOLINT
    {PhysicalContext::Invalid, "Invalid"},
    {PhysicalContext::Room, "Room"},
    {PhysicalContext::Intake, "Intake"},
    {PhysicalContext::Exhaust, "Exhaust"},
    {PhysicalContext::LiquidInlet, "LiquidInlet"},
    {PhysicalContext::LiquidOutlet, "LiquidOutlet"},
    {PhysicalContext::Front, "Front"},
    {PhysicalContext::Back, "Back"},
    {PhysicalContext::Upper, "Upper"},
    {PhysicalContext::Lower, "Lower"},
    {PhysicalContext::CPU, "CPU"},
    {PhysicalContext::CPUSubsystem, "CPUSubsystem"},
    {PhysicalContext::GPU, "GPU"},
    {PhysicalContext::GPUSubsystem, "GPUSubsystem"},
    {PhysicalContext::FPGA, "FPGA"},
    {PhysicalContext::Accelerator, "Accelerator"},
    {PhysicalContext::ASIC, "ASIC"},
    {PhysicalContext::Backplane, "Backplane"},
    {PhysicalContext::SystemBoard, "SystemBoard"},
    {PhysicalContext::PowerSupply, "PowerSupply"},
    {PhysicalContext::PowerSubsystem, "PowerSubsystem"},
    {PhysicalContext::VoltageRegulator, "VoltageRegulator"},
    {PhysicalContext::Rectifier, "Rectifier"},
    {PhysicalContext::StorageDevice, "StorageDevice"},
    {PhysicalContext::NetworkingDevice, "NetworkingDevice"},
    {PhysicalContext::ComputeBay, "ComputeBay"},
    {PhysicalContext::StorageBay, "StorageBay"},
    {PhysicalContext::NetworkBay, "NetworkBay"},
    {PhysicalContext::ExpansionBay, "ExpansionBay"},
    {PhysicalContext::PowerSupplyBay, "PowerSupplyBay"},
    {PhysicalContext::Memory, "Memory"},
    {PhysicalContext::MemorySubsystem, "MemorySubsystem"},
    {PhysicalContext::Chassis, "Chassis"},
    {PhysicalContext::Fan, "Fan"},
    {PhysicalContext::CoolingSubsystem, "CoolingSubsystem"},
    {PhysicalContext::Motor, "Motor"},
    {PhysicalContext::Transformer, "Transformer"},
    {PhysicalContext::ACUtilityInput, "ACUtilityInput"},
    {PhysicalContext::ACStaticBypassInput, "ACStaticBypassInput"},
    {PhysicalContext::ACMaintenanceBypassInput, "ACMaintenanceBypassInput"},
    {PhysicalContext::DCBus, "DCBus"},
    {PhysicalContext::ACOutput, "ACOutput"},
    {PhysicalContext::ACInput, "ACInput"},
    {PhysicalContext::TrustedModule, "TrustedModule"},
    {PhysicalContext::Board, "Board"},
    {PhysicalContext::Transceiver, "Transceiver"},
    {PhysicalContext::Battery, "Battery"},
    {PhysicalContext::Pump, "Pump"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(PhysicalSubContext, { //NOLINT
    {PhysicalSubContext::Invalid, "Invalid"},
    {PhysicalSubContext::Input, "Input"},
    {PhysicalSubContext::Output, "Output"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(LogicalContext, { //NOLINT
    {LogicalContext::Invalid, "Invalid"},
    {LogicalContext::Capacity, "Capacity"},
    {LogicalContext::Environment, "Environment"},
    {LogicalContext::Network, "Network"},
    {LogicalContext::Performance, "Performance"},
    {LogicalContext::Security, "Security"},
    {LogicalContext::Storage, "Storage"},
});

}
// clang-format on
