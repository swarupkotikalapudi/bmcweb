#ifndef PHYSICALCONTEXT_V1
#define PHYSICALCONTEXT_V1

enum class PhysicalContext_v1_PhysicalContext
{
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
};
enum class PhysicalContext_v1_PhysicalSubContext
{
    Input,
    Output,
};
#endif
