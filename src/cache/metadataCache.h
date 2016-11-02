/**
 * @file metadataCache.h
 * @author Konrad Zemek
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#ifndef ONECLIENT_METADATA_CACHE_H
#define ONECLIENT_METADATA_CACHE_H

#include "communication/communicator.h"
#include "messages/fuse/fileAttr.h"
#include "messages/fuse/fileLocation.h"
#include "messages/clientMessage.h"
#include "messages/fuse/getFileAttr.h"
#include "messages/fuse/resolveGuid.h"

#include <boost/filesystem/path.hpp>
#include <boost/functional/hash.hpp>
#include <tbb/concurrent_hash_map.h>

#include <condition_variable>
#include <helpers/IStorageHelper.h>
#include <unordered_map>

namespace std {
template <> struct hash<one::helpers::Flag> {
    size_t operator()(const one::helpers::Flag &p) const
    {
        return std::hash<int>()(static_cast<int>(p));
    }
};
}

namespace one {
namespace client {

/**
 * @c MetadataCache is responsible for retrieving and caching path<->uuid,
 * uuid<->fileAttrs, and uuid<->fileLocation mappings.
 */
class MetadataCache {
public:
    using Path = boost::filesystem::path;
    using FileAttr = messages::fuse::FileAttr;
    using FileLocation = messages::fuse::FileLocation;
    enum FileState { normal, removedUpstream, renamedUpstream };

    /**
     * @c Metadata holds metadata of a file.
     */
    struct Metadata {
        boost::optional<Path> path;
        boost::optional<FileAttr> attr;
        std::unordered_map<one::helpers::Flag, FileLocation> locations;
        FileState state = normal;
    };

private:
    struct PathHash {
        static std::size_t hash(const Path &);
        static bool equal(const Path &, const Path &);
    };

    tbb::concurrent_hash_map<Path, std::string, PathHash> m_pathToUuid;
    tbb::concurrent_hash_map<std::string, Metadata> m_metaCache;
    tbb::concurrent_hash_map<std::string,
        std::pair<std::unique_ptr<std::mutex>,
                                 std::unique_ptr<std::condition_variable>>>
        m_mutexConditionPairMap;

public:
    using ConstUuidAccessor = decltype(m_pathToUuid)::const_accessor;
    using ConstMetaAccessor = decltype(m_metaCache)::const_accessor;
    using ConstMutexAccessor =
        decltype(m_mutexConditionPairMap)::const_accessor;
    using UuidAccessor = decltype(m_pathToUuid)::accessor;
    using MetaAccessor = decltype(m_metaCache)::accessor;
    using MutexAccessor = decltype(m_mutexConditionPairMap)::accessor;

    /**
     * Constructor.
     * @param communicator Communicator instance used for fetching missing
     * data.
     */
    MetadataCache(communication::Communicator &communicator);

    MetadataCache(MetadataCache &&) = delete;

    /**
     * Sets metadata accessor for a given uuid, without consulting the remote
     * endpoint in any case.
     * @param metaAcc Metadata accessor.
     * @param uuid UUID of the file to retrieve metadata of.
     * @return True if metadata has been found in the cache, false otherwise.
     */
    bool get(MetaAccessor &metaAcc, const std::string &uuid);

    /**
     * Retrieves attributes for a given path.
     * @param path The path of a file to retrieve attributes of.
     * @return Attributes of the file.
     */
    FileAttr getAttr(const Path &path);

    /**
     * Retrieves attributes of a file with given uuid.
     * @param uuid The uuid of a file to retrieve attributes of.
     * @return Attributes of the file.
     */
    FileAttr getAttr(const std::string &uuid);

    /**
     * Retrieves location data about a file with given uuid.
     * @param uuid The uuid of a file to retrieve location data about.
     * @param flags The open flags.
     * @return Location data about the file.
     */
    FileLocation getLocation(
        const std::string &uuid, const one::helpers::FlagsSet flags);

    /**
     * Sets metadata accessor for a given path, first ensuring that path<->UUID
     * mapping is present in the cache and attributes are set.
     * @param metaAcc Metadata accessor.
     * @param path The path of a file to retrieve attributes of.
     */
    void getAttr(MetaAccessor &metaAcc, const Path &path);

    /**
     * Sets UUID and metadata accessors for a given path, first ensuring that
     * path<->UUID mapping is present in the cache and attributes are set in
     * the metadata.
     * @param uuidAcc UUID accessor.
     * @param metaAcc Metadata accessor.
     * @param path The path of a file to retrieve attributes of.
     */
    void getAttr(
        UuidAccessor &uuidAcc, MetaAccessor &metaAcc, const Path &path);

    /**
     * Sets metadata accessor for a given UUID, first ensuring that attributes
     * are set.
     * @param metaAcc Metadata accessor.
     * @param uuid The UUID of a file to retrieve attributes of.
     */
    void getAttr(MetaAccessor &metaAcc, const std::string &uuid);

    /**
     * Sets metadata accessor for a given UUID, first ensuring that location
     * data is set.
     * @param metaAcc Metadata accessor.
     * @param uuid The UUID of a file to retrieve location data about.
     * @param flags Open flags.
     */
    void getLocation(MetaAccessor &metaAcc, const std::string &uuid,
        const one::helpers::FlagsSet flags);

    /**
     * Adds an arbitrary path<->UUID mapping to the cache.
     * @param path The path of the mapping.
     * @param uuid The UUID of the mapping.
     */
    void map(Path path, std::string uuid);

    /**
     * Adds an arbitrary path<->UUID and UUID<->fileLocation to the cache.
     * @param path The path of the mapping.
     * @param location The location data to put in the metadata.
     * @param flags Open flags.
     */
    void map(
        Path path, FileLocation location, const one::helpers::FlagsSet flags);

    /**
     * Renames a file in the cache through changing or removing mappings
     * and returns vector of pairs representing UUIDs changes.
     * @param oldPath Path to rename from.
     * @param newPath Path to rename to.
     * @return Vector of pairs representing UUIDs changes.
     */
    std::vector<std::pair<std::string, std::string>> rename(
        const Path &oldPath, const Path &newPath);

    /**
     * Changes path and uuid of single file and deletes its location.
     * @param oldMetaAcc Accessor to metadata mapping to update.
     * @param newUuidAcc Accessor to new UUID mapping.
     * @param oldUuid Old UUID of updated file.
     * @param newUuid New UUID of updated file.
     * @param newPath New path of updated file.
     */
    void remapFile(MetaAccessor &oldMetaAcc, UuidAccessor &newUuidAcc,
        const std::string &oldsUuid, const std::string &newUuid,
        const Path &newPath);

    /**
     * Changes path and uuid of single file and deletes its location.
     * @param oldUuid Old UUID of updated file.
     * @param newUuid New UUID of updated file.
     * @param newPath New path of updated file.
     */
    void remapFile(const std::string &oldUuid, const std::string &newUuid,
        const Path &newPath);

    /**
     * Removes a UUID and metadata entries from the cache.
     * @param uuidAcc Accessor to UUID mapping to remove.
     * @param metaAcc Accessor to metadata mapping to remove.
     */
    void remove(UuidAccessor &uuidAcc, MetaAccessor &metaAcc);

    /**
     * Removes a UUID entry (path mapping) from the cache.
     * @param uuidAcc Accessor to UUID mapping to remove.
     * @param metaAcc Accessor to metadata mapping.
     */
    void removePathMapping(UuidAccessor &uuidAcc, MetaAccessor &metaAcc);

    /**
     * Removes a metadata entry and UUID mapping (if exists) from the cache.
     * @param uuid UUID of the entry to remove.
     */
    void remove(const std::string &uuid);

    /**
     * Waits for file location update on given condition.
     * @param uuid The UUID of file
     * @param range Range of data to wait for
     * @param timeout Timeout to wait for condition
     * @param flags Flags for opening the file if its closed
     * @return true if file has been successfully synchronized
     */
    bool waitForNewLocation(const std::string &uuid,
        const boost::icl::discrete_interval<off_t> &range,
        const std::chrono::milliseconds &timeout,
        const one::helpers::FlagsSet flags);

    /**
     * Notifies waiting processes that the new file location has arrived
     * @param The UUID of file
     */
    void notifyNewLocationArrived(const std::string &uuid);

    /**
     * Filters given flags set to one of RDONLY, WRONLY or RDWR.
     * Returns RDONLY if flag value is zero.
     * @param Flags value
     */
    one::helpers::Flag static filterFlagsForLocation(
        one::helpers::FlagsSet flagsSet)
    {
        if (flagsSet.count(one::helpers::Flag::RDONLY))
            return one::helpers::Flag::RDONLY;
        else if (flagsSet.count(one::helpers::Flag::WRONLY))
            return one::helpers::Flag::WRONLY;
        else if (flagsSet.count(one::helpers::Flag::RDWR))
            return one::helpers::Flag::RDWR;
        else
            return one::helpers::Flag::RDONLY;
    }

private:
    /**
     * Retrieves mutex and condition assigned to file with given uuid. Creates
     * them, if they are not found.
     * @param uuid The uuid of a file to get mutex and condition.
     * @return Pair: mutex, condition.
     */
    std::pair<std::mutex &, std::condition_variable &> getMutexConditionPair(
        const std::string &uuid);

    FileAttr fetchAttr(messages::ClientMessage &&request);

    communication::Communicator &m_communicator;
};

} // namespace one
} // namespace client

#endif // ONECLIENT_METADATA_CACHE_H
