/**
 * @file helpersCache.h
 * @author Konrad Zemek
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#ifndef ONECLIENT_HELPERS_CACHE_H
#define ONECLIENT_HELPERS_CACHE_H

#include "communication/communicator.h"
#include "helpers/storageHelper.h"
#include "helpers/storageHelperCreator.h"
#include "scheduler.h"
#include "storageAccessManager.h"

#include <asio/executor_work.hpp>
#include <asio/io_service.hpp>
#include <folly/FBString.h>
#include <folly/FBVector.h>
#include <folly/Hash.h>

#include <thread>
#include <tuple>
#include <utility>

namespace one {
namespace messages {
namespace fuse {
class StorageTestFile;
}
}
namespace client {
namespace cache {

/**
 * @c HelpersCache is responsible for creating and caching
 * @c helpers::StorageHelper instances.
 */
class HelpersCache {
public:
    using HelperPtr = std::shared_ptr<helpers::StorageHelper>;

    /**
     * Constructor.
     * Starts an @c asio::io_service instance with one worker thread for
     * @c helpers::StorageHelperCreator .
     * @param communicator Communicator instance used to fetch helper
     * parameters.
     * @param workersNumber Number of worker threads that will drive some async
     * helpers operations.
     */
    HelpersCache(communication::Communicator &communicator,
        Scheduler &scheduler, const std::size_t workersNumber = 10);

    /**
     * Destructor.
     * Stops the @c asio::io_service instance and a worker thread.
     */
    ~HelpersCache();

    /**
     * Retrieves a helper instance.
     * @param fileUuid UUID of a file for which helper will be used.
     * @param storageId Storage id for which to retrieve a helper.
     * @param forceProxyIO Determines whether to return a ProxyIO helper.
     * @return Retrieved helper instance.
     */
    HelperPtr get(const folly::fbstring &fileUuid,
        const folly::fbstring &storageId, const bool forceProxyIO);

private:
    enum class AccessType { DIRECT, PROXY };

    void requestStorageTestFileCreation(
        const folly::fbstring &fileUuid, const folly::fbstring &storageId);

    void handleStorageTestFile(
        std::shared_ptr<messages::fuse::StorageTestFile> testFile,
        const folly::fbstring &storageId, const std::size_t attempts);

    void requestStorageTestFileVerification(
        const messages::fuse::StorageTestFile &testFile,
        const folly::fbstring &storageId, const folly::fbstring &fileContent);

    void handleStorageTestFileVerification(
        const std::error_code &ec, const folly::fbstring &storageId);

    communication::Communicator &m_communicator;
    Scheduler &m_scheduler;

    asio::io_service m_helpersIoService;
    asio::executor_work<asio::io_service::executor_type> m_idleWork{
        asio::make_work(m_helpersIoService)};
    folly::fbvector<std::thread> m_helpersWorkers;

    helpers::StorageHelperCreator m_helperFactory{m_helpersIoService,
        m_helpersIoService, m_helpersIoService, m_helpersIoService,
        m_communicator};

    StorageAccessManager m_storageAccessManager{
        m_communicator, m_helperFactory};

    std::unordered_map<folly::fbstring, AccessType> m_accessType;
    std::unordered_map<std::tuple<folly::fbstring, bool>, HelperPtr> m_cache;
};

} // namespace cache
} // namespace client
} // namespace one

#endif // ONECLIENT_HELPERS_CACHE_H
