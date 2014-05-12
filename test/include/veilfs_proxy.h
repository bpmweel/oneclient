/**
 * @file veilfs_proxy.h
 * @author Rafal Slota
 * @copyright (C) 2013 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in 'LICENSE.txt'
 */

#ifndef VEILFS_PROXY_H
#define VEILFS_PROXY_H

#include "veilfs.h"
#include "testCommon.h"

class ProxyVeilFS
    : public veil::client::VeilFS {
public:
    ProxyVeilFS(std::string path, boost::shared_ptr<Config> cnf, boost::shared_ptr<JobScheduler> scheduler, 
               boost::shared_ptr<FslogicProxy> fslogic,  boost::shared_ptr<MetaCache> metaCache, 
               boost::shared_ptr<StorageMapper> mapper, boost::shared_ptr<helpers::StorageHelperFactory> sh_factory,
               boost::shared_ptr<EventCommunicator> eventCommunicator)
      : VeilFS(path, cnf, scheduler, fslogic, metaCache, mapper, sh_factory, eventCommunicator)
    {

    }

    void setCachedHelper(helper_cache_idx_t idx, sh_ptr sh)
    {
        m_shCache[idx] = sh;
    }
};



#endif // VEILFS_PROXY_H