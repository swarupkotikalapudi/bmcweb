/*
// Copyright (c) 2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
/****************************************************************
 * This is an auto-generated header which contains definitions
 * for Redfish DMTF defined messages.
 ***************************************************************/
#pragma once
#include <registries.hpp>

namespace redfish::message_registries::resource_event
{
const Header header = {
    .copyright = "Copyright 2014-2018 DMTF in cooperation with the Storage "
                 "Networking Industry Association (SNIA). All rights reserved.",
    .type = "#MessageRegistry.1.2.0.MessageRegistry",
    .id = "ResourceEventRegistry.1.2.0",
    .name = "Resource Event Message Registry",
    .language = "en",
    .description =
        "This registry defines the messages to use for resource events.",
    .registryPrefix = "ResourceEvent",
    .registryVersion = "1.0.0",
    .owningEntity = "DMTF",
};
const std::array registry = {
    MessageEntry{
        "LicenseAdded",
        {
            .description = "A license for %1 has been added.",
            .message = "A license for %1 has been added. The following message "
                       "was returned %2.",
            .severity = "OK",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "string",
                },
            .resolution =
                "See vendor specific instructions for specific actions.",
        }},
    MessageEntry{
        "LicenseChanged",
        {
            .description = "A license for %1 has changed.",
            .message = "A license for %1 has changed. The following message "
                       "was returned %2.",
            .severity = "Warning",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "string",
                },
            .resolution =
                "See vendor specific instructions for specific actions.",
        }},
    MessageEntry{
        "LicenseExpired",
        {
            .description = "A license for %1 has expired.",
            .message = "A license for %1 has expired. The following message "
                       "was returned %2.",
            .severity = "Warning",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "string",
                },
            .resolution =
                "See vendor specific instructions for specific actions.",
        }},
    MessageEntry{
        "ResourceChanged",
        {
            .description =
                "One or more resource properties have changed. This is not "
                "used whenever there is another event message for that "
                "specific change, such as only the state has changed.",
            .message = "One or more resource properties have changed.",
            .severity = "OK",
            .numberOfArgs = 0,
            .paramTypes = {},
            .resolution = "None.",
        }},
    MessageEntry{"ResourceCreated",
                 {
                     .description =
                         "Indicates that all conditions of a successful "
                         "creation operation have been met.",
                     .message = "The resource has been created successfully.",
                     .severity = "OK",
                     .numberOfArgs = 0,
                     .paramTypes = {},
                     .resolution = "None",
                 }},
    MessageEntry{
        "ResourceErrorThresholdCleared",
        {
            .description = "The resource property %1 has cleared the error "
                           "threshold of value %2. Examples would be drive IO "
                           "errors, or network link errors.",
            .message = "The resource property %1 has cleared the error "
                       "threshold of value %2.",
            .severity = "OK",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "value",
                },
            .resolution = "None.",
        }},
    MessageEntry{
        "ResourceErrorThresholdExceeded",
        {
            .description = "The resource property %1 has exceeded error "
                           "threshold of value %2. Examples would be drive IO "
                           "errors, or network link errors.",
            .message = "The resource property %1 has exceeded error threshold "
                       "of value %2.",
            .severity = "Critical",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "value",
                },
            .resolution = "None.",
        }},
    MessageEntry{
        "ResourceErrorsCorrected",
        {
            .description =
                "The resource property %1 has corrected errors of type %2. "
                "Examples would be drive IO errors, or network link errors.",
            .message =
                "The resource property %1 has corrected errors of type %2.",
            .severity = "OK",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "string",
                },
            .resolution = "None.",
        }},
    MessageEntry{
        "ResourceErrorsDetected",
        {
            .description =
                "The resource property %1 has detected errors of type %2. "
                "Examples would be drive IO errors, or network link errors.",
            .message =
                "The resource property %1 has detected errors of type %2.",
            .severity = "Warning",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "string",
                },
            .resolution = "Resolution dependent upon error type.",
        }},
    MessageEntry{"ResourceRemoved",
                 {
                     .description =
                         "Indicates that all conditions of a successful "
                         "remove operation have been met.",
                     .message = "The resource has been removed successfully.",
                     .severity = "OK",
                     .numberOfArgs = 0,
                     .paramTypes = {},
                     .resolution = "None",
                 }},
    MessageEntry{"ResourceSelfTestCompleted",
                 {
                     .description = "A self-test has completed.",
                     .message = "A self-test has completed.",
                     .severity = "OK",
                     .numberOfArgs = 0,
                     .paramTypes = {},
                     .resolution = "None.",
                 }},
    MessageEntry{
        "ResourceSelfTestFailed",
        {
            .description = "A self-test has failed. Suggested resolution may "
                           "be provided as OEM data.",
            .message = "A self-test has failed. The following message was "
                       "returned %1.",
            .severity = "Critical",
            .numberOfArgs = 1,
            .paramTypes =
                {
                    "string",
                },
            .resolution =
                "See vendor specific instructions for specific actions.",
        }},
    MessageEntry{
        "ResourceStatusChangedCritical",
        {
            .description =
                "The state of resource %1 has changed to state type %2. The "
                "state types shall be used from Resource.State.",
            .message = "The state of resource %1 has changed to state type %2.",
            .severity = "Critical",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "string",
                },
            .resolution = "None.",
        }},
    MessageEntry{
        "ResourceStatusChangedOK",
        {
            .description =
                "The state of resource %1 has changed to state type %2. The "
                "state types shall be used from Resource.State.",
            .message = "The state of resource %1 has changed to state type %2.",
            .severity = "OK",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "string",
                },
            .resolution = "None.",
        }},
    MessageEntry{
        "ResourceStatusChangedWarning",
        {
            .description =
                "The state of resource %1 has changed to state type %2. The "
                "state types shall be used from Resource.State.",
            .message = "The state of resource %1 has changed to state type %2.",
            .severity = "Warning",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "string",
                },
            .resolution = "None.",
        }},
    MessageEntry{
        "ResourceVersionIncompatible",
        {
            .description = "An incompatible version of software %1 has been "
                           "detected. Examples may be after a component or "
                           "system level software update.",
            .message =
                "An incompatible version of software %1 has been detected.",
            .severity = "Warning",
            .numberOfArgs = 1,
            .paramTypes =
                {
                    "string",
                },
            .resolution = "Compare the version of the resource with the "
                          "compatible version of the software.",
        }},
    MessageEntry{
        "ResourceWarningThresholdCleared",
        {
            .description =
                "The resource property %1 has cleared the warning threshold of "
                "value %2. Examples would be drive IO errors, or network link "
                "errors. Suggested resolution may be provided as OEM data.",
            .message = "The resource property %1 has cleared the warning "
                       "threshold of value %2.",
            .severity = "OK",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "value",
                },
            .resolution = "None.",
        }},
    MessageEntry{
        "ResourceWarningThresholdExceeded",
        {
            .description =
                "The resource property %1 has cleared its warning threshold of "
                "value %2. Examples would be drive IO errors, or network link "
                "errors. Suggested resolution may be provided as OEM data.",
            .message = "The resource property %1 has cleared its warning "
                       "threshold of value %2.",
            .severity = "Warning",
            .numberOfArgs = 2,
            .paramTypes =
                {
                    "string",
                    "value",
                },
            .resolution = "None.",
        }},
    MessageEntry{
        "URIForResourceChanged",
        {
            .description =
                "The URI for the model resource has changed. Examples for this "
                "would be physical component replacement or redistribution.",
            .message = "The URI for the resource has changed.",
            .severity = "OK",
            .numberOfArgs = 0,
            .paramTypes = {},
            .resolution = "None.",
        }},
};
} // namespace redfish::message_registries::resource_event
