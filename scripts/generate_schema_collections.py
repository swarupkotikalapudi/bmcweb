#!/usr/bin/env python3

# Script to generate top level resource collection URIs
# Parses the Redfish schema to determine what are the top level collection URIs
# and writes them to a generated .hpp file as an unordered_set.  Also generates
# a map of URIs that contain a top level collection as part of their subtree.
# Those URIs as well as those of their immediate children as written as an
# unordered_map.  These URIs are need by Redfish Aggregation

import os
import xml.etree.ElementTree as ET

WARNING = """/****************************************************************
 *                 READ THIS WARNING FIRST
 * This is an auto-generated header which contains definitions
 * for Redfish DMTF defined schemas.
 * DO NOT modify this registry outside of running the
 * update_schemas.py script.  The definitions contained within
 * this file are owned by DMTF.  Any modifications to these files
 * should be first pushed to the relevant registry in the DMTF
 * github organization.
 ***************************************************************/"""

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
REDFISH_SCHEMA_DIR = os.path.realpath(
    os.path.join(SCRIPT_DIR, "..", "static", "redfish", "v1", "schema")
)
CPP_OUTFILE = os.path.realpath(
    os.path.join(SCRIPT_DIR, "..", "redfish-core", "include",
                 "aggregation_utils.hpp")
)


# Odata string types
EDMX = "{http://docs.oasis-open.org/odata/ns/edmx}"
EDM = "{http://docs.oasis-open.org/odata/ns/edm}"


def parse_node(path, collection_parents, top_collections, xml_file):
    collection_in_subtree = False
    filepath = os.path.join(REDFISH_SCHEMA_DIR, xml_file)
    tree = ET.parse(filepath)
    root = tree.getroot()

    # Map xml URIs to their associated namespace
    xml_map = {}
    for ref in root.findall(EDMX + "Reference"):
        uri = ref.get("Uri")
        if uri is None:
            continue
        file = uri.split("/").pop()
        for ref_child in ref.iter(EDMX + "Include"):
            ns = ref_child.get("Namespace")
            if ns is None:
                continue
            xml_map[ns] = file

    ds = root.find(EDMX + "DataServices")
    local_col = set()
    for ds_child in ds.iter(EDM + "Schema"):
        for navprop in ds_child.iter(EDM + "NavigationProperty"):
            nav_name = navprop.get("Name")
            nav_type_split = navprop.get("Type").split(".")
            if (nav_type_split[0].endswith("Collection") or
                    nav_type_split[0].startswith("Collection")):
                new_path = path + "/" + nav_name
                top_collections.add(new_path)
                local_col.add(new_path)
                collection_in_subtree = True
                print("Added top level collection: " + new_path)
            else:
                # Traverse the children until we hit top level collection
                new_path = path + "/" + nav_type_split[0]
                if (parse_node(new_path, collection_parents, top_collections,
                               xml_map[nav_type_split[0]])):
                    collection_in_subtree = True
                    local_col.add(new_path)

    if len(local_col) > 0:
        collection_parents[path] = local_col

    return collection_in_subtree


def main():
    # We need to separately track top level resources as well as all URIs that
    # are upstream from a top level resource.  We shouldn't combine these into
    # a single structure because:
    #
    # 1) We want direct lookup of top level collections for prefix handling
    # purposes.
    #
    # 2) A top level collection will not always be one level below the service
    # root.  For example, we need to aggregate
    # /redfish/v1/CompositionService/ActivePool and we do not currently support
    # CompositionService.  If a satellite BMC implements it then we would need
    # to display a link to CompositionService under /redfish/v1 even though
    # CompositionService is not a top level collection.

    # key = URI that's upstream from a top level resource
    # val = list of URIs that are top level resources or on path to one
    collection_parents = {}

    # Contains URIs for all top level collections
    top_collections = set()

    # Begin parsing from the Service Root
    curr_path = "/redfish/v1"
    parse_node(curr_path, collection_parents, top_collections,
               "ServiceRoot_v1.xml")

    print("Finished traversal!")

    keys = []
    for key in collection_parents:
        keys.append(key)

    with open(CPP_OUTFILE, "w") as hpp_file:
        hpp_file.write(
            "#pragma once\n"
            "{WARNING}\n"
            "// clang-format off\n"
            "#include <array>\n"
            "#include <unordered_map>\n"
            "#include <unordered_set>\n"
            "\n"
            "namespace redfish\n"
            "{{\n"
            "constexpr std::array "
            "topCollections{{\n".format(WARNING=WARNING)
        )

        for collection in sorted(top_collections):
            hpp_file.write('    "{}",\n'.format(collection))

        hpp_file.write(
            "};\n\n"
            "const std::unordered_map<std::string, "
            "std::unordered_set<std::string>>\n"
            "    topCollectionsParents = {\n"
        )

        for key in sorted(keys):
            hpp_file.write('        {{"{}",\n'
                           '         {{\n'.format(key)
                           )
            for val in sorted(collection_parents[key]):
                hpp_file.write('             "{}",\n'.format(val))
            hpp_file.write(
                '         }},\n'
            )

        hpp_file.write("};\n" "} // namespace redfish\n")


if __name__ == '__main__':
    main()
