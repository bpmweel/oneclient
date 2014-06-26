/**
 * @file storageHelperFactory.cc
 * @author Rafal Slota
 * @copyright (C) 2013 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in 'LICENSE.txt'
 */

#include "helpers/storageHelperFactory.h"
#include "directIOHelper.h"
#include "clusterProxyHelper.h"
#include "communicationHandler.h"

#include <boost/algorithm/string.hpp>

namespace veil {
namespace helpers {

BufferLimits::BufferLimits(const size_t wgl, const size_t rgl, const size_t wfl,
                           const size_t rfl, const size_t pbs)
    : writeBufferGlobalSizeLimit{wgl}
    , readBufferGlobalSizeLimit{rgl}
    , writeBufferPerFileSizeLimit{wfl}
    , readBufferPerFileSizeLimit{rfl}
    , preferedBlockSize{pbs}
{
}

namespace utils {

    std::string tolower(std::string input) {
        boost::algorithm::to_lower(input);
        return std::move(input);
    }

} // namespace utils

StorageHelperFactory::StorageHelperFactory(std::shared_ptr<SimpleConnectionPool> connectionPool,
                                           const BufferLimits &limits)
    : m_connectionPool{std::move(connectionPool)}
    , m_limits{limits}
{
}

StorageHelperFactory::~StorageHelperFactory()
{
}

std::shared_ptr<IStorageHelper> StorageHelperFactory::getStorageHelper(const std::string &sh_name,
                                                                       const IStorageHelper::ArgsMap &args) {
    if(sh_name == "DirectIO")
        return std::make_shared<DirectIOHelper>(args);

    if(sh_name == "ClusterProxy")
        return std::make_shared<ClusterProxyHelper>(m_connectionPool, m_limits, args);

    return {};
}

std::string srvArg(const int argno)
{
    return "srv_arg" + std::to_string(argno);
}

} // namespace helpers
} // namespace veil
