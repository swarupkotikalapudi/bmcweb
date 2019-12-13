#pragma once

#include <app.h>

#include <boost/algorithm/string.hpp>
#include <boost/container/flat_map.hpp>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace crow
{
namespace ibm_mc_lock
{

namespace fs = std::filesystem;
using stype = std::string;

/*----------------------------------------
|Segment flags : LockFlag | SegmentLength|
------------------------------------------*/

using segmentflags = std::vector<std::pair<stype, uint32_t>>;

// Lockrequest = session-id | hmc-id | locktype | resourceid | segmentinfo
using lockrequest = std::tuple<stype, stype, stype, uint64_t, segmentflags>;

using lockrequests = std::vector<lockrequest>;
using rc =
    std::pair<bool, std::variant<uint32_t, std::pair<uint32_t, lockrequest>>>;
using rcrelaselock = std::pair<bool, std::pair<uint32_t, lockrequest>>;
using rcgetlocklist = std::pair<
    bool,
    std::variant<std::string, std::vector<std::pair<uint32_t, lockrequests>>>>;
using listoftransactionIDs = std::vector<uint32_t>;
using rcacquirelock = std::pair<bool, std::variant<rc, std::pair<bool, int>>>;
using rcreleaselockapi = std::pair<bool, std::variant<bool, rcrelaselock>>;

class lock
{
    uint32_t transactionID;
    std::map<uint32_t, lockrequests> locktable;
    bool isvalidlockrequest(lockrequest);
    bool isconflictrequest(lockrequests);
    bool isconflictrecord(lockrequest, lockrequest);
    rc isconflictwithtable(lockrequests);
    rcrelaselock isitmylock(listoftransactionIDs, std::pair<stype, stype>);
    bool validaterids(listoftransactionIDs);
    void releaselock(listoftransactionIDs);

  private:
    bool checkbyte(uint64_t, uint64_t, uint32_t);
    uint32_t generateTransactionID();

  public:
    rcacquirelock Acquirelock(lockrequests);
    rcreleaselockapi Releaselock(listoftransactionIDs, std::pair<stype, stype>);

  public:
    lock()
    {
        transactionID = 0;
    }

} lockobject;

rcreleaselockapi lock::Releaselock(listoftransactionIDs p,
                                   std::pair<stype, stype> ids)
{

    bool status = validaterids(p);

    if (!status)
    {
        // Validation of rids failed
        BMCWEB_LOG_DEBUG << "Not a Valid request id";
        return std::make_pair(false, status);
    }
    else
    {
        // Validation passed, check if all the locks are owned by the
        // requesting HMC
        auto status = isitmylock(p, std::make_pair(ids.first, ids.second));
        if (status.first)
        {
            // The current hmc owns all the locks, so we can release
            // them
            releaselock(p);
        }
        return std::make_pair(true, status);
    }
    return std::make_pair(false, status);
}

rcacquirelock lock::Acquirelock(lockrequests lockrequeststructure)
{

    // validate the lock request

    for (auto i : lockrequeststructure)
    {
        const lockrequest &j = i;
        bool status = isvalidlockrequest(j);
        if (!status)
        {
            BMCWEB_LOG_DEBUG << "Not a Valid record";
            BMCWEB_LOG_DEBUG << "Bad json in request";
            return std::make_pair(true, std::make_pair(status, 0));
        }
    }
    // check for conflict record

    const lockrequests &k = lockrequeststructure;
    bool status = isconflictrequest(k);

    if (status)
    {
        BMCWEB_LOG_DEBUG << "There is a conflict within itself";
        return std::make_pair(true, std::make_pair(status, 1));
    }
    else
    {
        BMCWEB_LOG_DEBUG << "The request is not conflicting within itself";

        // Need to check for conflict with the locktable entries.

        auto conflict = isconflictwithtable(k);

        BMCWEB_LOG_DEBUG << "Done with checking conflict with the locktable";
        return std::make_pair(false, conflict);
    }

    return std::make_pair(true, std::make_pair(true, 1));
}

void lock::releaselock(std::vector<uint32_t> refrids)
{
    for (auto id : refrids)
    {
        locktable.erase(id);
    }
}

rcrelaselock lock::isitmylock(std::vector<uint32_t> refrids,
                              std::pair<stype, stype> ids)
{
    for (auto id : refrids)
    {
        // Just need to compare the client id of the first lock records in the
        // complete lock row(in the map), because the rest of the lock records
        // would have the same client id

        std::string expectedclientid = std::get<1>(locktable[id][0]);
        std::string expectedsessionid = std::get<0>(locktable[id][0]);

        if ((expectedclientid == ids.first) &&
            (expectedsessionid == ids.second))
        {
            // It is owned by the currently request hmc
            BMCWEB_LOG_DEBUG << "Lock is owned  by the current hmc";
        }
        else
        {
            BMCWEB_LOG_DEBUG << "Lock is not owned by the current hmc";
            return std::make_pair(false, std::make_pair(id, locktable[id][0]));
        }
    }
    return std::make_pair(true, std::make_pair(0, lockrequest()));
}

bool lock::validaterids(std::vector<uint32_t> refrids)
{
    for (auto id : refrids)
    {
        auto search = locktable.find(id);

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

bool lock::isvalidlockrequest(lockrequest reflockrecord)
{

    // validate the locktype

    if (!((boost::equals(std::get<2>(reflockrecord), "Read") ||
           (boost::equals(std::get<2>(reflockrecord), "Write")))))
    {
        BMCWEB_LOG_DEBUG << "Validation of LockType Failed";
        BMCWEB_LOG_DEBUG << "Locktype : " << std::get<2>(reflockrecord);
        return false;
    }

    BMCWEB_LOG_DEBUG << static_cast<int>(std::get<4>(reflockrecord).size());

    // validate the number of segments
    // Allowed No of segments are between 2 and 6
    if ((static_cast<int>(std::get<4>(reflockrecord).size()) > 6) ||
        (static_cast<int>(std::get<4>(reflockrecord).size()) < 2))
    {
        BMCWEB_LOG_DEBUG << "Validation of Number of Segements Failed";
        BMCWEB_LOG_DEBUG << "Number of Segments provied : "
                         << sizeof(std::get<4>(reflockrecord));
        return false;
    }

    // validate the lockflags & segment length

    for (const auto &p : std::get<4>(reflockrecord))
    {

        // validate the lock flags
        // Allowed lockflags are locksame,lockall & dontlock

        if (!((boost::equals(p.first, "LockSame") ||

               (boost::equals(p.first, "LockAll")) ||
               (boost::equals(p.first, "DontLock")))))
        {
            BMCWEB_LOG_DEBUG << "Validation of lock flags failed";
            BMCWEB_LOG_DEBUG << p.first;
            return false;
        }

        // validate the segment length
        // Allowed values of segment length are between 1 and 4

        if (p.second < 1 || p.second > 4)
        {
            BMCWEB_LOG_DEBUG << "Validation of Segment Length Failed";
            BMCWEB_LOG_DEBUG << p.second;
            return false;
        }
    }

    // validate the segment length
    return true;
}

rc lock::isconflictwithtable(lockrequests reflockrequeststructure)
{

    uint32_t transactionID;

    if (locktable.empty())
    {
        transactionID = generateTransactionID();
        BMCWEB_LOG_DEBUG << transactionID;
        // Lock table is empty, so we are safe to add the lockrecords
        // as there will be no conflict
        BMCWEB_LOG_DEBUG << "Lock table is empty, so adding the lockrecords";
        locktable.emplace(std::pair<uint32_t, lockrequests>(
            transactionID, reflockrequeststructure));

        return std::make_pair(false, transactionID);
    }

    else
    {
        BMCWEB_LOG_DEBUG
            << "Lock table is not empty, check for conflict with lock table";
        // Lock table is not empty, compare the lockrequest entries with
        // the entries in the lock table

        for (auto i : reflockrequeststructure)
        {
            for (const auto &map : locktable)
            {
                for (auto k : map.second)
                {
                    const lockrequest &p = i;
                    const lockrequest &q = k;
                    bool status = isconflictrecord(p, q);
                    if (status)
                    {
                        return std::make_pair(true,
                                              std::make_pair(map.first, q));
                    }
                }
            }
        }

        // Reached here, so no conflict with the locktable, so we are safe to
        // add the request records into the lock table

        // Lock table is empty, so we are safe to add the lockrecords
        // as there will be no conflict
        BMCWEB_LOG_DEBUG << " Adding elements into lock table";
        transactionID = generateTransactionID();
        locktable.emplace(
            std::make_pair(transactionID, reflockrequeststructure));
    }
    return std::make_pair(false, transactionID);
}

bool lock::isconflictrequest(lockrequests reflockrequeststructure)
{
    // check for all the locks coming in as a part of single request
    // return conflict if any two lock requests are conflicting

    if (reflockrequeststructure.size() == 1)
    {
        BMCWEB_LOG_DEBUG << "Only single lock request, so there is no conflict";
        // This means , we have only one lock request in the current
        // request , so no conflict within the request
        return false;
    }

    else
    {
        BMCWEB_LOG_DEBUG
            << "There are multiple lock requests coming in a single request";

        // There are multiple requests a part of one request

        for (uint32_t i = 0; i < reflockrequeststructure.size(); i++)
        {
            for (uint32_t j = i + 1; j < reflockrequeststructure.size(); j++)
            {
                const lockrequest &p = reflockrequeststructure[i];
                const lockrequest &q = reflockrequeststructure[j];
                bool status = isconflictrecord(p, q);

                if (status)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// This function converts the provided uint64_t resource id's from the two
// lock requests subjected for comparision, and this function also compares
// the content by bytes mentioned by a uint32_t number.

// If all the elements in the lock requests which are subjected for comparison
// are same, then the last comparision would be to check for the respective
// bytes in the resourceid based on the segment length.

bool lock::checkbyte(uint64_t resourceid1, uint64_t resourceid2, uint32_t j)
{
    uint8_t *p = reinterpret_cast<uint8_t *>(&resourceid1);
    uint8_t *q = reinterpret_cast<uint8_t *>(&resourceid2);

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
    return true;
}

bool lock::isconflictrecord(lockrequest reflockrecord1,
                            lockrequest reflockrecord2)
{
    // No conflict if both are read locks

    if (boost::equals(std::get<2>(reflockrecord1), "Read") &&
        boost::equals(std::get<2>(reflockrecord2), "Read"))
    {
        BMCWEB_LOG_DEBUG << "Both are read locks, no conflict";
        return false;
    }

    else
    {
        uint32_t i = 0;
        for (const auto &p : std::get<4>(reflockrecord1))
        {

            // return conflict when any of them is try to lock all resources
            // under the current resource level.
            if (boost::equals(p.first, "LockAll") ||
                boost::equals(std::get<4>(reflockrecord2)[i].first, "LockAll"))
            {
                BMCWEB_LOG_DEBUG
                    << "Either of the Comparing locks are trying to lock all "
                       "resources under the current resource level";
                return true;
            }

            // determine if there is a lock-all-with-same-segment-size.
            // If the current segment sizes are the same,then we should fail.

            if ((boost::equals(p.first, "LockSame") ||
                 boost::equals(std::get<4>(reflockrecord2)[i].first,
                               "LockSame")) &&
                (p.second == std::get<4>(reflockrecord2)[i].second))
            {
                return true;
            }

            // if segment lengths are not the same, it means two different locks
            // So no conflict
            if (p.second != std::get<4>(reflockrecord2)[i].second)
            {
                BMCWEB_LOG_DEBUG << "Segment lengths are not same";
                BMCWEB_LOG_DEBUG << "Segment 1 length : " << p.second;
                BMCWEB_LOG_DEBUG << "Segment 2 length : "
                                 << std::get<4>(reflockrecord2)[i].second;
                return false;
            }

            // compare segment data

            for (uint32_t i = 0; i < p.second; i++)
            {
                // if the segment data is different , then the locks is on a
                // different resource So no conflict between the lock records
                if (!(checkbyte(std::get<3>(reflockrecord1),
                                std::get<3>(reflockrecord2), i)))
                {
                    return false;
                }
            }

            ++i;
        }
    }

    return false;
}

uint32_t lock::generateTransactionID()
{
    ++transactionID;
    return transactionID;
}

} // namespace ibm_mc_lock
} // namespace crow
