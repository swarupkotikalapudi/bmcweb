#pragma once
/****************************************************************
 *                 READ THIS WARNING FIRST
 * This is an auto-generated header which contains definitions
 * for Redfish DMTF defined messages.
 * DO NOT modify this registry outside of running the
 * parse_registries.py script.  The definitions contained within
 * this file are owned by DMTF.  Any modifications to these files
 * should be first pushed to the relevant registry in the DMTF
 * github organization.
 ***************************************************************/
#include <array>
#include <cstdint>
#include <string_view>

namespace redfish::privileges
{
constexpr const size_t maxPrivilegeCount = 64;
constexpr std::array<std::string_view, 5> basePrivileges = {
    "Login", "ConfigureManager", "ConfigureUsers", "ConfigureComponents",
    "ConfigureSelf"};
enum class EntityTag : int
{
    tagAccelerationFunction = 0,
    tagAccelerationFunctionCollection = 1,
    tagAccountService = 2,
    tagActionInfo = 3,
    tagAddressPool = 4,
    tagAddressPoolCollection = 5,
    tagAggregate = 6,
    tagAggregateCollection = 7,
    tagAggregationService = 8,
    tagAggregationSource = 9,
    tagAggregationSourceCollection = 10,
    tagAllowDeny = 11,
    tagAllowDenyCollection = 12,
    tagAssembly = 13,
    tagBattery = 14,
    tagBatteryCollection = 15,
    tagBatteryMetrics = 16,
    tagBios = 17,
    tagBootOption = 18,
    tagBootOptionCollection = 19,
    tagCable = 20,
    tagCableCollection = 21,
    tagCertificate = 22,
    tagCertificateCollection = 23,
    tagCertificateLocations = 24,
    tagCertificateService = 25,
    tagChassis = 26,
    tagChassisCollection = 27,
    tagCircuit = 28,
    tagCircuitCollection = 29,
    tagCompositionReservation = 30,
    tagCompositionReservationCollection = 31,
    tagCompositionService = 32,
    tagComputerSystem = 33,
    tagComputerSystemCollection = 34,
    tagConnection = 35,
    tagConnectionCollection = 36,
    tagConnectionMethod = 37,
    tagConnectionMethodCollection = 38,
    tagControl = 39,
    tagControlCollection = 40,
    tagDrive = 41,
    tagDriveCollection = 42,
    tagEndpoint = 43,
    tagEndpointCollection = 44,
    tagEndpointGroup = 45,
    tagEndpointGroupCollection = 46,
    tagEnvironmentMetrics = 47,
    tagEthernetInterface = 48,
    tagEthernetInterfaceCollection = 49,
    tagEventDestination = 50,
    tagEventDestinationCollection = 51,
    tagEventService = 52,
    tagExternalAccountProvider = 53,
    tagExternalAccountProviderCollection = 54,
    tagFabric = 55,
    tagFabricCollection = 56,
    tagFabricAdapter = 57,
    tagFabricAdapterCollection = 58,
    tagFacility = 59,
    tagFacilityCollection = 60,
    tagFan = 61,
    tagFanCollection = 62,
    tagGraphicsController = 63,
    tagGraphicsControllerCollection = 64,
    tagHostInterface = 65,
    tagHostInterfaceCollection = 66,
    tagJob = 67,
    tagJobCollection = 68,
    tagJobService = 69,
    tagJsonSchemaFile = 70,
    tagJsonSchemaFileCollection = 71,
    tagKey = 72,
    tagKeyCollection = 73,
    tagKeyPolicy = 74,
    tagKeyPolicyCollection = 75,
    tagKeyService = 76,
    tagLogEntry = 77,
    tagLogEntryCollection = 78,
    tagLogService = 79,
    tagLogServiceCollection = 80,
    tagManager = 81,
    tagManagerCollection = 82,
    tagManagerAccount = 83,
    tagManagerAccountCollection = 84,
    tagManagerDiagnosticData = 85,
    tagManagerNetworkProtocol = 86,
    tagMediaController = 87,
    tagMediaControllerCollection = 88,
    tagMemory = 89,
    tagMemoryCollection = 90,
    tagMemoryChunks = 91,
    tagMemoryChunksCollection = 92,
    tagMemoryDomain = 93,
    tagMemoryDomainCollection = 94,
    tagMemoryMetrics = 95,
    tagMessageRegistryFile = 96,
    tagMessageRegistryFileCollection = 97,
    tagMetricDefinition = 98,
    tagMetricDefinitionCollection = 99,
    tagMetricReport = 100,
    tagMetricReportCollection = 101,
    tagMetricReportDefinition = 102,
    tagMetricReportDefinitionCollection = 103,
    tagNetworkAdapter = 104,
    tagNetworkAdapterCollection = 105,
    tagNetworkAdapterMetrics = 106,
    tagNetworkDeviceFunction = 107,
    tagNetworkDeviceFunctionCollection = 108,
    tagNetworkDeviceFunctionMetrics = 109,
    tagNetworkInterface = 110,
    tagNetworkInterfaceCollection = 111,
    tagNetworkPort = 112,
    tagNetworkPortCollection = 113,
    tagOperatingConfig = 114,
    tagOperatingConfigCollection = 115,
    tagOutlet = 116,
    tagOutletCollection = 117,
    tagOutletGroup = 118,
    tagOutletGroupCollection = 119,
    tagPCIeDevice = 120,
    tagPCIeDeviceCollection = 121,
    tagPCIeFunction = 122,
    tagPCIeFunctionCollection = 123,
    tagPCIeSlots = 124,
    tagPort = 125,
    tagPortCollection = 126,
    tagPortMetrics = 127,
    tagPower = 128,
    tagPowerDistribution = 129,
    tagPowerDistributionCollection = 130,
    tagPowerDistributionMetrics = 131,
    tagPowerDomain = 132,
    tagPowerDomainCollection = 133,
    tagPowerEquipment = 134,
    tagPowerSubsystem = 135,
    tagPowerSupply = 136,
    tagPowerSupplyCollection = 137,
    tagPowerSupplyMetrics = 138,
    tagProcessor = 139,
    tagProcessorCollection = 140,
    tagProcessorMetrics = 141,
    tagResourceBlock = 142,
    tagResourceBlockCollection = 143,
    tagRole = 144,
    tagRoleCollection = 145,
    tagRouteEntry = 146,
    tagRouteEntryCollection = 147,
    tagRouteSetEntry = 148,
    tagRouteSetEntryCollection = 149,
    tagSecureBoot = 150,
    tagSecureBootDatabase = 151,
    tagSecureBootDatabaseCollection = 152,
    tagSensor = 153,
    tagSensorCollection = 154,
    tagSerialInterface = 155,
    tagSerialInterfaceCollection = 156,
    tagServiceRoot = 157,
    tagSession = 158,
    tagSessionCollection = 159,
    tagSessionService = 160,
    tagSignature = 161,
    tagSignatureCollection = 162,
    tagSimpleStorage = 163,
    tagSimpleStorageCollection = 164,
    tagSoftwareInventory = 165,
    tagSoftwareInventoryCollection = 166,
    tagStorage = 167,
    tagStorageCollection = 168,
    tagStorageController = 169,
    tagStorageControllerCollection = 170,
    tagSwitch = 171,
    tagSwitchCollection = 172,
    tagTask = 173,
    tagTaskCollection = 174,
    tagTaskService = 175,
    tagTelemetryService = 176,
    tagThermal = 177,
    tagThermalMetrics = 178,
    tagThermalSubsystem = 179,
    tagTriggers = 180,
    tagTriggersCollection = 181,
    tagUpdateService = 182,
    tagUSBController = 183,
    tagUSBControllerCollection = 184,
    tagVCATEntry = 185,
    tagVCATEntryCollection = 186,
    tagVLanNetworkInterface = 187,
    tagVLanNetworkInterfaceCollection = 188,
    tagVirtualMedia = 189,
    tagVirtualMediaCollection = 190,
    tagVolume = 191,
    tagVolumeCollection = 192,
    tagZone = 193,
    tagZoneCollection = 194,
    none = 195,
};
constexpr std::array<std::string_view, 195> entities = {
    "AccelerationFunction",
    "AccelerationFunctionCollection",
    "AccountService",
    "ActionInfo",
    "AddressPool",
    "AddressPoolCollection",
    "Aggregate",
    "AggregateCollection",
    "AggregationService",
    "AggregationSource",
    "AggregationSourceCollection",
    "AllowDeny",
    "AllowDenyCollection",
    "Assembly",
    "Battery",
    "BatteryCollection",
    "BatteryMetrics",
    "Bios",
    "BootOption",
    "BootOptionCollection",
    "Cable",
    "CableCollection",
    "Certificate",
    "CertificateCollection",
    "CertificateLocations",
    "CertificateService",
    "Chassis",
    "ChassisCollection",
    "Circuit",
    "CircuitCollection",
    "CompositionReservation",
    "CompositionReservationCollection",
    "CompositionService",
    "ComputerSystem",
    "ComputerSystemCollection",
    "Connection",
    "ConnectionCollection",
    "ConnectionMethod",
    "ConnectionMethodCollection",
    "Control",
    "ControlCollection",
    "Drive",
    "DriveCollection",
    "Endpoint",
    "EndpointCollection",
    "EndpointGroup",
    "EndpointGroupCollection",
    "EnvironmentMetrics",
    "EthernetInterface",
    "EthernetInterfaceCollection",
    "EventDestination",
    "EventDestinationCollection",
    "EventService",
    "ExternalAccountProvider",
    "ExternalAccountProviderCollection",
    "Fabric",
    "FabricCollection",
    "FabricAdapter",
    "FabricAdapterCollection",
    "Facility",
    "FacilityCollection",
    "Fan",
    "FanCollection",
    "GraphicsController",
    "GraphicsControllerCollection",
    "HostInterface",
    "HostInterfaceCollection",
    "Job",
    "JobCollection",
    "JobService",
    "JsonSchemaFile",
    "JsonSchemaFileCollection",
    "Key",
    "KeyCollection",
    "KeyPolicy",
    "KeyPolicyCollection",
    "KeyService",
    "LogEntry",
    "LogEntryCollection",
    "LogService",
    "LogServiceCollection",
    "Manager",
    "ManagerCollection",
    "ManagerAccount",
    "ManagerAccountCollection",
    "ManagerDiagnosticData",
    "ManagerNetworkProtocol",
    "MediaController",
    "MediaControllerCollection",
    "Memory",
    "MemoryCollection",
    "MemoryChunks",
    "MemoryChunksCollection",
    "MemoryDomain",
    "MemoryDomainCollection",
    "MemoryMetrics",
    "MessageRegistryFile",
    "MessageRegistryFileCollection",
    "MetricDefinition",
    "MetricDefinitionCollection",
    "MetricReport",
    "MetricReportCollection",
    "MetricReportDefinition",
    "MetricReportDefinitionCollection",
    "NetworkAdapter",
    "NetworkAdapterCollection",
    "NetworkAdapterMetrics",
    "NetworkDeviceFunction",
    "NetworkDeviceFunctionCollection",
    "NetworkDeviceFunctionMetrics",
    "NetworkInterface",
    "NetworkInterfaceCollection",
    "NetworkPort",
    "NetworkPortCollection",
    "OperatingConfig",
    "OperatingConfigCollection",
    "Outlet",
    "OutletCollection",
    "OutletGroup",
    "OutletGroupCollection",
    "PCIeDevice",
    "PCIeDeviceCollection",
    "PCIeFunction",
    "PCIeFunctionCollection",
    "PCIeSlots",
    "Port",
    "PortCollection",
    "PortMetrics",
    "Power",
    "PowerDistribution",
    "PowerDistributionCollection",
    "PowerDistributionMetrics",
    "PowerDomain",
    "PowerDomainCollection",
    "PowerEquipment",
    "PowerSubsystem",
    "PowerSupply",
    "PowerSupplyCollection",
    "PowerSupplyMetrics",
    "Processor",
    "ProcessorCollection",
    "ProcessorMetrics",
    "ResourceBlock",
    "ResourceBlockCollection",
    "Role",
    "RoleCollection",
    "RouteEntry",
    "RouteEntryCollection",
    "RouteSetEntry",
    "RouteSetEntryCollection",
    "SecureBoot",
    "SecureBootDatabase",
    "SecureBootDatabaseCollection",
    "Sensor",
    "SensorCollection",
    "SerialInterface",
    "SerialInterfaceCollection",
    "ServiceRoot",
    "Session",
    "SessionCollection",
    "SessionService",
    "Signature",
    "SignatureCollection",
    "SimpleStorage",
    "SimpleStorageCollection",
    "SoftwareInventory",
    "SoftwareInventoryCollection",
    "Storage",
    "StorageCollection",
    "StorageController",
    "StorageControllerCollection",
    "Switch",
    "SwitchCollection",
    "Task",
    "TaskCollection",
    "TaskService",
    "TelemetryService",
    "Thermal",
    "ThermalMetrics",
    "ThermalSubsystem",
    "Triggers",
    "TriggersCollection",
    "UpdateService",
    "USBController",
    "USBControllerCollection",
    "VCATEntry",
    "VCATEntryCollection",
    "VLanNetworkInterface",
    "VLanNetworkInterfaceCollection",
    "VirtualMedia",
    "VirtualMediaCollection",
    "Volume",
    "VolumeCollection",
    "Zone",
    "ZoneCollection"};
constexpr std::array<uint64_t, 198> getBasePrivilegesBitmaps = {
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00010, 0b00010,
    0b00010, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00010, 0b00100, 0b10000, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00000, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001};
constexpr std::array<uint64_t, 195> getBasePrivilegesLength = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
constexpr std::array<uint64_t, 196> headBasePrivilegesBitmaps = {
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00010, 0b00010,
    0b00010, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00000, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001,
    0b00001, 0b00001, 0b00001, 0b00001};
constexpr std::array<uint64_t, 195> headBasePrivilegesLength = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
constexpr std::array<uint64_t, 199> patchBasePrivilegesBitmaps = {
    0b01000, 0b01000, 0b00100, 0b00010, 0b01000, 0b01000, 0b00010, 0b01000,
    0b00010, 0b01000, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b01000, 0b01000, 0b00010, 0b10000, 0b00010, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b00010, 0b01000, 0b01000, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00100,
    0b00100, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b00010, 0b01000,
    0b01000, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b00010, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000};
constexpr std::array<uint64_t, 195> patchBasePrivilegesLength = {
    1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
constexpr std::array<uint64_t, 199> putBasePrivilegesBitmaps = {
    0b01000, 0b01000, 0b00100, 0b00010, 0b01000, 0b01000, 0b00010, 0b01000,
    0b00010, 0b01000, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b01000, 0b01000, 0b00010, 0b10000, 0b00010, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b00010, 0b01000, 0b01000, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00100,
    0b00100, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b00010, 0b01000,
    0b01000, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b00010, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000};
constexpr std::array<uint64_t, 195> putBasePrivilegesLength = {
    1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
constexpr std::array<uint64_t, 200> deleteBasePrivilegesBitmaps = {
    0b01000, 0b01000, 0b00100, 0b00010, 0b01000, 0b01000, 0b00010, 0b01000,
    0b00010, 0b01000, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b01000, 0b01000, 0b00010, 0b10000, 0b00010, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b00010, 0b01000, 0b01000, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00100,
    0b00100, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b00010, 0b01000,
    0b01000, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b00010, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b10000, 0b00010, 0b00010, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000};
constexpr std::array<uint64_t, 195> deleteBasePrivilegesLength = {
    1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
constexpr std::array<uint64_t, 199> postBasePrivilegesBitmaps = {
    0b01000, 0b01000, 0b00100, 0b00010, 0b01000, 0b01000, 0b00010, 0b01000,
    0b00010, 0b01000, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b01000, 0b01000, 0b00010, 0b10000, 0b00010, 0b01000,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b00010, 0b01000, 0b01000, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00100,
    0b00100, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b00010, 0b01000,
    0b01000, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b01000, 0b01000, 0b01000,
    0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b00010, 0b00010, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b00001, 0b00010, 0b01000, 0b01000, 0b01000,
    0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000,
    0b01000, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010,
    0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b00010,
    0b00010, 0b00010, 0b00010, 0b01000, 0b01000, 0b01000, 0b01000};
constexpr std::array<uint64_t, 195> postBasePrivilegesLength = {
    1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
} // namespace redfish::privileges