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
#include <string_view>

namespace redfish::privileges
{
constexpr std::string_view basePrivilegeStr = R"({
  "@Redfish.Copyright": "Copyright 2015-2022 DMTF. All rights reserved.",
  "@odata.type": "#PrivilegeRegistry.v1_1_4.PrivilegeRegistry",
  "Id": "Redfish_1.3.0_PrivilegeRegistry",
  "Name": "Privilege Mapping array collection",
  "PrivilegesUsed": [
    "Login",
    "ConfigureManager",
    "ConfigureUsers",
    "ConfigureComponents",
    "ConfigureSelf"
  ],
  "OEMPrivilegesUsed": [],
  "Mappings": [
    {
      "Entity": "AccelerationFunction",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AccelerationFunctionCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AccountService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ActionInfo",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AddressPool",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AddressPoolCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Aggregate",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AggregateCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AggregationService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AggregationSource",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AggregationSourceCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AllowDeny",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "AllowDenyCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Assembly",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Battery",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "BatteryCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "BatteryMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Bios",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "BootOption",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "BootOptionCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Cable",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "CableCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Certificate",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      },
      "SubordinateOverrides": [
        {
          "Targets": [
            "ComputerSystem"
          ],
          "OperationMap": {
            "GET": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "HEAD": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        }
      ]
    },
    {
      "Entity": "CertificateCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      },
      "SubordinateOverrides": [
        {
          "Targets": [
            "ComputerSystem"
          ],
          "OperationMap": {
            "GET": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "HEAD": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        }
      ]
    },
    {
      "Entity": "CertificateLocations",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "CertificateService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Chassis",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ChassisCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Circuit",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "CircuitCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "CompositionReservation",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "CompositionReservationCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "CompositionService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ComputerSystem",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ComputerSystemCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Connection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ConnectionCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ConnectionMethod",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ConnectionMethodCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Control",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ControlCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Drive",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "DriveCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Endpoint",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "EndpointCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "EndpointGroup",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "EndpointGroupCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "EnvironmentMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      },
      "SubordinateOverrides": [
        {
          "Targets": [
            "Processor"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        },
        {
          "Targets": [
            "Memory"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        },
        {
          "Targets": [
            "Drive"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        },
        {
          "Targets": [
            "PCIeDevice"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        },
        {
          "Targets": [
            "StorageController"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        },
        {
          "Targets": [
            "Port"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        }
      ]
    },
    {
      "Entity": "EthernetInterface",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      },
      "SubordinateOverrides": [
        {
          "Targets": [
            "Manager",
            "EthernetInterfaceCollection"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureManager"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureManager"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureManager"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureManager"
                ]
              }
            ]
          }
        }
      ]
    },
    {
      "Entity": "EthernetInterfaceCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "EventDestination",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureSelf"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureSelf"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureSelf"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureSelf"
            ]
          }
        ]
      }
    },
    {
      "Entity": "EventDestinationCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "EventService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ExternalAccountProvider",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ExternalAccountProviderCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Fabric",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "FabricCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "FabricAdapter",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "FabricAdapterCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Facility",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "FacilityCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Fan",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "FanCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "GraphicsController",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "GraphicsControllerCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "HostInterface",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "HostInterfaceCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Job",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "JobCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "JobService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "JsonSchemaFile",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "JsonSchemaFileCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Key",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "KeyCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "KeyPolicy",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "KeyPolicyCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "KeyService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "LogEntry",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      },
      "SubordinateOverrides": [
        {
          "Targets": [
            "ComputerSystem",
            "LogServiceCollection",
            "LogService",
            "LogEntryCollection"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        },
        {
          "Targets": [
            "Chassis",
            "LogServiceCollection",
            "LogService",
            "LogEntryCollection"
          ],
          "OperationMap": {
            "GET": [
              {
                "Privilege": [
                  "Login"
                ]
              }
            ],
            "HEAD": [
              {
                "Privilege": [
                  "Login"
                ]
              }
            ],
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        }
      ]
    },
    {
      "Entity": "LogEntryCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      },
      "SubordinateOverrides": [
        {
          "Targets": [
            "ComputerSystem",
            "LogServiceCollection",
            "LogService"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        },
        {
          "Targets": [
            "Chassis",
            "LogServiceCollection",
            "LogService"
          ],
          "OperationMap": {
            "GET": [
              {
                "Privilege": [
                  "Login"
                ]
              }
            ],
            "HEAD": [
              {
                "Privilege": [
                  "Login"
                ]
              }
            ],
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        }
      ]
    },
    {
      "Entity": "LogService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      },
      "SubordinateOverrides": [
        {
          "Targets": [
            "ComputerSystem",
            "LogServiceCollection"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        },
        {
          "Targets": [
            "Chassis",
            "LogServiceCollection"
          ],
          "OperationMap": {
            "GET": [
              {
                "Privilege": [
                  "Login"
                ]
              }
            ],
            "HEAD": [
              {
                "Privilege": [
                  "Login"
                ]
              }
            ],
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        }
      ]
    },
    {
      "Entity": "LogServiceCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      },
      "SubordinateOverrides": [
        {
          "Targets": [
            "ComputerSystem"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        },
        {
          "Targets": [
            "Chassis"
          ],
          "OperationMap": {
            "GET": [
              {
                "Privilege": [
                  "Login"
                ]
              }
            ],
            "HEAD": [
              {
                "Privilege": [
                  "Login"
                ]
              }
            ],
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "PUT": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "DELETE": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ],
            "POST": [
              {
                "Privilege": [
                  "ConfigureComponents"
                ]
              }
            ]
          }
        }
      ]
    },
    {
      "Entity": "Manager",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ManagerCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ManagerAccount",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          },
          {
            "Privilege": [
              "ConfigureSelf"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ]
      },
      "PropertyOverrides": [
        {
          "Targets": [
            "Password"
          ],
          "OperationMap": {
            "PATCH": [
              {
                "Privilege": [
                  "ConfigureUsers"
                ]
              },
              {
                "Privilege": [
                  "ConfigureSelf"
                ]
              }
            ]
          }
        }
      ]
    },
    {
      "Entity": "ManagerAccountCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureUsers"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ManagerDiagnosticData",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ManagerNetworkProtocol",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MediaController",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MediaControllerCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Memory",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MemoryCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MemoryChunks",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MemoryChunksCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MemoryDomain",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MemoryDomainCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MemoryMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MessageRegistryFile",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MessageRegistryFileCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MetricDefinition",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MetricDefinitionCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MetricReport",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MetricReportCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MetricReportDefinition",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "MetricReportDefinitionCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkAdapter",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkAdapterCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkAdapterMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkDeviceFunction",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkDeviceFunctionCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkDeviceFunctionMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkInterface",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkInterfaceCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkPort",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "NetworkPortCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "OperatingConfig",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "OperatingConfigCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Outlet",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "OutletCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "OutletGroup",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "OutletGroupCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PCIeDevice",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PCIeDeviceCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PCIeFunction",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PCIeFunctionCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PCIeSlots",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Port",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PortCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PortMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Power",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerDistribution",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerDistributionCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerDistributionMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerDomain",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerDomainCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerEquipment",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerSubsystem",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerSupply",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerSupplyCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "PowerSupplyMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Processor",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ProcessorCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ProcessorMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ResourceBlock",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ResourceBlockCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Role",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "RoleCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "RouteEntry",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "RouteEntryCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "RouteSetEntry",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "RouteSetEntryCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SecureBoot",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SecureBootDatabase",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SecureBootDatabaseCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Sensor",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SensorCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SerialInterface",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SerialInterfaceCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ServiceRoot",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          },
          {
            "Privilege": [
              "NoAuth"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          },
          {
            "Privilege": [
              "NoAuth"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Session",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          },
          {
            "Privilege": [
              "ConfigureSelf"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SessionCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SessionService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Signature",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SignatureCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SimpleStorage",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SimpleStorageCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SoftwareInventory",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SoftwareInventoryCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Storage",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "StorageCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "StorageController",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "StorageControllerCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Switch",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "SwitchCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Task",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "TaskCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "TaskService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "TelemetryService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Thermal",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ThermalMetrics",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ThermalSubsystem",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Triggers",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "TriggersCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "UpdateService",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "USBController",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "USBControllerCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "VCATEntry",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "VCATEntryCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "VLanNetworkInterface",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "VLanNetworkInterfaceCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "VirtualMedia",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "VirtualMediaCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureManager"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Volume",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "VolumeCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "Zone",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    },
    {
      "Entity": "ZoneCollection",
      "OperationMap": {
        "GET": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "HEAD": [
          {
            "Privilege": [
              "Login"
            ]
          }
        ],
        "PATCH": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "POST": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "PUT": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ],
        "DELETE": [
          {
            "Privilege": [
              "ConfigureComponents"
            ]
          }
        ]
      }
    }
  ]
})";
} // namespace redfish::privileges