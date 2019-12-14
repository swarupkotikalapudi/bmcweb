#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/container/flat_map.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace crow
{
namespace ibm_mc_lock
{
namespace fs = std::filesystem;
using stype = std::string;
using segmentflags = std::vector<std::pair<std::string, uint32_t>>;
using lockrecord = std::tuple<stype, stype, stype, uint64_t, segmentflags>;
using lockrequest = std::vector<lockrecord>;
using rc = std::pair<bool, std::variant<uint32_t, lockrecord>>;
using rcrelaselock = std::pair<bool, lockrecord>;
using rcgetlocklist = std::pair<
    bool,
    std::variant<std::string, std::vector<std::pair<uint32_t, lockrequest>>>>;
static constexpr const char *filename =
    "/var/lib/obmc/bmc-console-mgmt/locks/ibm_mc_persistent_lock_data.json";

class lock
{
    uint32_t rid;
    std::map<uint32_t, lockrequest> locktable;

  private:
    lock()
    {
        loadLocks();
        rid = locktable.empty() ? 0 : locktable.end()->first;
    }

  public:
    static lock &getInstance()
    {
        static lock lockobject;
        return lockobject;
    }

    bool createPersistentLockFilePath()
    {
        // The path /var/lib/obmc will be created by initrdscripts
        // Create the directories for the persistent lock file
        std::error_code ec;
        if (!std::filesystem::is_directory("/var/lib/obmc/bmc-console-mgmt"))
        {
            std::filesystem::create_directory("/var/lib/obmc/bmc-console-mgmt",
                                              ec);
        }
        if (ec)
        {
            BMCWEB_LOG_DEBUG
                << "handleIbmPost: Failed to prepare bmc-console-mgmt "
                   "directory. ec : "
                << ec;
            return false;
        }

        if (!std::filesystem::is_directory(
                "/var/lib/obmc/bmc-console-mgmt/locks"))
        {
            std::filesystem::create_directory(
                "/var/lib/obmc/bmc-console-mgmt/locks", ec);
        }
        if (ec)
        {
            BMCWEB_LOG_DEBUG
                << "handleIbmPost: Failed to prepare persistent lock "
                   "file directory. ec : "
                << ec;
            return false;
        }
        return true;
    }

    void loadLocks()
    {
        std::ifstream persistentFile(filename);
        if (persistentFile.is_open())
        {
            auto data = nlohmann::json::parse(persistentFile, nullptr, false);
            if (data.is_discarded())
            {
                BMCWEB_LOG_ERROR
                    << "Error parsing persistent data in json file.";
            }
            else
            {
                BMCWEB_LOG_DEBUG << "The persistent lock data is available";
                for (const auto &item : data.items())
                {
                    BMCWEB_LOG_DEBUG << item.key();
                    BMCWEB_LOG_DEBUG << item.value();
                    lockrequest locks = item.value();
                    locktable.insert(std::pair<uint32_t, lockrequest>(
                        std::stoul(item.key()), locks));
                    BMCWEB_LOG_DEBUG << "The persistent lock data loaded";
                }
            }
        }
    }

    bool saveLocks()
    {
        if (!std::filesystem::is_directory(
                "/var/lib/obmc/bmc-console-mgmt/locks"))
        {
            if (!createPersistentLockFilePath())
            {
                BMCWEB_LOG_DEBUG << "Failed to create lock persistent path";
                return false;
            }
        }
        std::ofstream persistentFile(filename);
        // set the permission of the file to 640
        fs::perms permission = fs::perms::owner_read | fs::perms::owner_write |
                               fs::perms::group_read;
        fs::permissions(filename, permission);
        nlohmann::json data;
        std::map<uint32_t, lockrequest>::iterator it;
        for (it = locktable.begin(); it != locktable.end(); ++it)
        {
            data[std::to_string(it->first)] = it->second;
        }
        BMCWEB_LOG_DEBUG << "data is " << data;
        persistentFile << data;
        return true;
    }

    rcgetlocklist getlocklist(std::vector<std::string> listsessionid)
    {

        // validate the session id
        std::vector<std::pair<uint32_t, lockrequest>> locklist;

        if (!locktable.empty())
        {

            for (uint32_t i = 0; i < listsessionid.size(); i++)
            {
                std::vector<std::pair<uint32_t, lockrequest>> templist;
                auto it = locktable.begin();
                while (it != locktable.end())
                {
                    // Check if session id of this entry matches with session id
                    // given
                    if (std::get<0>(it->second[0]) == listsessionid[i])
                    {
                        BMCWEB_LOG_DEBUG
                            << "Session id is found in the locktable";

                        // Push the whole lock record into a vector for
                        // returning the json
                        locklist.push_back(
                            std::make_pair(it->first, it->second));
                        templist.push_back(
                            std::make_pair(it->first, it->second));
                    }
                    // Go to next entry in map
                    it++;
                }

                if (templist.size() == 0)
                {
                    // The session id is not found in the lock table
                    // return a validation failure
                    return std::make_pair(false, listsessionid[0]);
                }
            }

            // we found at least one entry with the given session id
            // return the json list of lock records pertaining to the
            // given session id
            return std::make_pair(true, locklist);
        }
        else
        {
            // if lock table is empty , the return the empty lock list
            return std::make_pair(true, locklist);
        }
        return std::make_pair(true, listsessionid[0]);
    }

    void releaselock(std::vector<uint32_t> *refrids)
    {
        for (uint32_t i = 0; i < refrids->size(); i++)
        {
            locktable.erase(refrids->at(i));
        }
        saveLocks();
    }

    void releaselock(const std::string &sessionID)
    {
        if (!locktable.empty())
        {
            for (auto &lockRecord : locktable)
            {
                // Check if session id of this entry matches with session id
                // given
                if (std::get<0>(lockRecord.second[0]) == sessionID)
                {
                    BMCWEB_LOG_DEBUG << "Remove the lock from the locktable "
                                        "having sessionID="
                                     << sessionID;
                    locktable.erase(lockRecord.first);
                    // save the lock in the persistent file
                    bool isSaved = saveLocks();
                    if (!isSaved)
                    {
                        BMCWEB_LOG_DEBUG
                            << "Error saving the locks in persistent";
                    }
                }
            }
        }
    }

    rcrelaselock isitmylock(std::vector<uint32_t> *refrids,
                            std::pair<stype, stype> ids)
    {
        for (uint32_t i = 0; i < refrids->size(); i++)
        {
            // Just need to compare the client id of the first lock records in
            // the complete lock row(in the map), because the rest of the lock
            // records would have the same client id

            std::string expectedclientid =
                std::get<1>(locktable[refrids->at(i)][0]);
            std::string expectedsessionid =
                std::get<0>(locktable[refrids->at(i)][0]);

            if ((expectedclientid == ids.first) &&
                (expectedsessionid == ids.second))
            {
                // It is owned by the currently request hmc
                BMCWEB_LOG_DEBUG << "Lock is owned  by the current hmc";
            }
            else
            {
                BMCWEB_LOG_DEBUG << "Lock is not owned by the current hmc";
                return std::make_pair(false, locktable[refrids->at(i)][0]);
            }
        }
        return std::make_pair(true, lockrecord());
    }

    bool validaterids(std::vector<uint32_t> *refrids)
    {
        for (uint32_t i = 0; i < refrids->size(); i++)
        {
            auto search = locktable.find(refrids->at(i));

            if (search != locktable.end())
            {
                BMCWEB_LOG_DEBUG << "Valid Transaction id";
                //  continue for the next rid
            }
            else
            {
                BMCWEB_LOG_DEBUG << "Atleast 1 inValid Request id";
                return false;
            }
        }
        return true;
    }
    /*
    void printmymap()
    {

        BMCWEB_LOG_DEBUG << "Printing the locktable";

        for (auto it = locktable.begin(); it != locktable.end(); it++)
        {
            BMCWEB_LOG_DEBUG << it->first;
            BMCWEB_LOG_DEBUG << std::get<0>(it->second);
            BMCWEB_LOG_DEBUG << std::get<1>(it->second);
            BMCWEB_LOG_DEBUG << std::get<2>(it->second);
            BMCWEB_LOG_DEBUG << std::get<3>(it->second);

            for (const auto &p : std::get<4>(it->second))
            {
                BMCWEB_LOG_DEBUG << p.first << ", " << p.second;
            }
        }
    }
    */
    bool isvalidlockrecord(lockrecord *reflockrecord)
    {

        // validate the locktype

        if (!((boost::iequals(std::get<2>(*reflockrecord), "read") ||
               (boost::iequals(std::get<2>(*reflockrecord), "write")))))
        {
            BMCWEB_LOG_DEBUG << "Locktype : " << std::get<2>(*reflockrecord);
            return false;
        }

        // validate the resource id

        // validate the lockflags & segment length

        for (const auto &p : std::get<4>(*reflockrecord))
        {

            // validate the lock flags
            if (!((boost::iequals(p.first, "locksame") ||
                   (boost::iequals(p.first, "lockall")) ||
                   (boost::iequals(p.first, "dontlock")))))
            {
                BMCWEB_LOG_DEBUG << p.first;
                return false;
            }

            // validate the segment length
            if (p.second < 1 || p.second > 4)
            {
                BMCWEB_LOG_DEBUG << p.second;
                return false;
            }
        }

        // validate the segment length
        return true;
    }

    rc isconflictwithtable(lockrequest *reflockrequeststructure)
    {

        uint32_t transactionID;
        // std::vector<uint32_t> vrid;

        if (locktable.empty())
        {
            transactionID = getmyrequestid();
            // for (uint32_t i = 0; i < reflockrequeststructure->size(); i++)
            //{
            // Lock table is empty, so we are safe to add the lockrecords
            // as there will be no conflict
            BMCWEB_LOG_DEBUG
                << "Lock table is empty, so adding the lockrecords";
            // rid = getmyrequestid();
            locktable.emplace(std::pair<uint32_t, lockrequest>(
                transactionID, *reflockrequeststructure));
            // save the lock in the persistent file
            bool isSaved = saveLocks();
            if (!isSaved)
            {
                BMCWEB_LOG_DEBUG << "Error saving the locks in persistent";
            }
            // vrid.push_back(rid);
            // }
            return std::make_pair(false, transactionID);
        }

        else
        {
            BMCWEB_LOG_DEBUG << "Lock table is not empty, check for conflict "
                                "with lock table";
            // Lock table is not empty, compare the lockrequest entries with
            // the entries in the lock table

            for (uint32_t i = 0; i < reflockrequeststructure->size(); i++)
            {
                for (auto it = locktable.begin(); it != locktable.end(); it++)
                {
                    for (uint32_t j = 0; j < it->second.size(); j++)
                    {
                        bool status = isconflictrecord(
                            &reflockrequeststructure->at(i), &it->second[j]);
                        if (status)
                        {
                            return std::make_pair(true, it->second.at(j));
                        }
                        else
                        {
                            // No conflict , we can proceed to another record
                        }
                    }
                }
            }

            // Reached here, so no conflict with the locktable, so we are safe
            // to add the request records into the lock table

            // for (uint32_t i = 0; i < reflockrequeststructure->size(); i++)
            //{
            // Lock table is empty, so we are safe to add the lockrecords
            // as there will be no conflict
            BMCWEB_LOG_DEBUG << " Adding elements into lock table";
            transactionID = getmyrequestid();
            locktable.emplace(
                std::make_pair(transactionID, *reflockrequeststructure));
            // save the lock in the persistent file
            bool isSaved = saveLocks();
            if (!isSaved)
            {
                BMCWEB_LOG_DEBUG << "Error saving the locks in persistent";
            }
            // vrid.push_back(rid);
            //}
        }
        return std::make_pair(false, transactionID);
    }

    bool isconflictrequest(lockrequest *reflockrequeststructure)
    {
        // check for all the locks coming in as a part of single request
        // return conflict if any two lock requests are conflicting

        if (reflockrequeststructure->size() == 1)
        {
            BMCWEB_LOG_DEBUG
                << "Only single lock request, so there is no conflict";
            // This means , we have only one lock request in the current
            // request , so no conflict within the request
            return false;
        }
        else
        {
            BMCWEB_LOG_DEBUG << "There are multiple lock requests coming in a "
                                "single request";

            // There are multiple requests a part of one request
            for (uint32_t i = 0; i < reflockrequeststructure->size(); i++)
            {
                for (uint32_t j = i + 1; j < reflockrequeststructure->size();
                     j++)
                {
                    bool status =
                        isconflictrecord(&reflockrequeststructure->at(i),
                                         &reflockrequeststructure->at(j));

                    if (status)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool checkbyte(uint64_t resourceid1, uint64_t resourceid2, uint32_t j)
    {
        uint8_t *p = reinterpret_cast<uint8_t *>(&resourceid1);
        uint8_t *q = reinterpret_cast<uint8_t *>(&resourceid2);

        // uint8_t result[8];
        // for(uint32_t i = 0; i < j; i++)
        // {
        BMCWEB_LOG_DEBUG << "Comparing bytes " << std::to_string(p[j]) << ","
                         << std::to_string(q[j]);
        if (p[j] != q[j])
        {
            return false;
        }

        else
        {
            return true;
        }

        //}
        return true;
    }

    bool isconflictrecord(lockrecord *reflockrecord1,
                          lockrecord *reflockrecord2)
    {
        // No conflict if both are read locks

        if (boost::iequals(std::get<2>(*reflockrecord1), "read") &&
            boost::iequals(std::get<2>(*reflockrecord2), "read"))
        {
            BMCWEB_LOG_DEBUG << "Both are read locks, no conflict";
            return false;
        }

        uint32_t i = 0;
        for (const auto &p : std::get<4>(*reflockrecord1))
        {

            // return conflict when any of them is try to lock all resources
            // under the current resource level.
            if (boost::iequals(p.first, "lockall") ||
                boost::iequals(std::get<4>(*reflockrecord2)[i].first,
                               "lockall"))
            {
                BMCWEB_LOG_DEBUG
                    << "Either of the Comparing locks are trying to lock all "
                       "resources under the current resource level";
                return true;
            }

            // determine if there is a lock-all-with-same-segment-size.
            // If the current segment sizes are the same,then we should fail.

            if ((boost::iequals(p.first, "locksame") ||
                 boost::iequals(std::get<4>(*reflockrecord2)[i].first,
                                "locksame")) &&
                (p.second == std::get<4>(*reflockrecord2)[i].second))
            {
                return true;
            }

            // if segment lengths are not the same, it means two different locks
            // So no conflict
            if (p.second != std::get<4>(*reflockrecord2)[i].second)
            {
                BMCWEB_LOG_DEBUG << "Segment lengths are not same";
                BMCWEB_LOG_DEBUG << "Segment 1 length : " << p.second;
                BMCWEB_LOG_DEBUG << "Segment 2 length : "
                                 << std::get<4>(*reflockrecord2)[i].second;
                return false;
            }

            // compare segment data

            for (uint32_t i = 0; i < p.second; i++)
            {
                // if the segment data is different , then the locks is on a
                // different resource So no conflict between the lock records
                if (!(checkbyte(std::get<3>(*reflockrecord1),
                                std::get<3>(*reflockrecord2), i)))
                {
                    return false;
                }
            }

            ++i;
        }

        return false;
    }

    uint32_t getmyrequestid()
    {
        ++rid;
        return rid;
    }
};

} // namespace ibm_mc_lock
} // namespace crow
