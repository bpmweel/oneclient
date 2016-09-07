/**
 * @file fsSubscriptions.cc
 * @author Krzysztof Trzepla
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#include "fsSubscriptions.h"
#include "events/eventManager.h"
#include "events/subscriptions/fileAttrSubscription.h"
#include "events/subscriptions/fileLocationSubscription.h"
#include "events/subscriptions/permissionChangedSubscription.h"
#include "events/subscriptions/quotaSubscription.h"
#include "scheduler.h"

#include <functional>

namespace one {
namespace client {

FsSubscriptions::FsSubscriptions(events::EventManager &eventManager)
    : m_eventManager{eventManager}
{
}

void FsSubscriptions::addFileLocationSubscription(const std::string &fileUuid)
{
    typename decltype(m_fileLocationSubscriptions)::accessor acc;
    if (m_fileLocationSubscriptions.insert(acc, fileUuid))
        acc->second = sendFileLocationSubscription(fileUuid);
}

void FsSubscriptions::removeFileLocationSubscription(
    const std::string &fileUuid)
{
    typename decltype(m_fileLocationSubscriptions)::accessor acc;
    if (m_fileLocationSubscriptions.find(acc, fileUuid)) {
        sendSubscriptionCancellation(acc->second);
        m_fileLocationSubscriptions.erase(acc);
    }
}

void FsSubscriptions::addPermissionChangedSubscription(
    const std::string &fileUuid)
{
    typename decltype(m_permissionChangedSubscriptions)::accessor acc;
    if (m_permissionChangedSubscriptions.insert(acc, fileUuid))
        acc->second = sendPermissionChangedSubscription(fileUuid);
}

void FsSubscriptions::removePermissionChangedSubscription(
    const std::string &fileUuid)
{
    typename decltype(m_permissionChangedSubscriptions)::accessor acc;
    if (m_permissionChangedSubscriptions.find(acc, fileUuid)) {
        sendSubscriptionCancellation(acc->second);
        m_permissionChangedSubscriptions.erase(acc);
    }
}

void FsSubscriptions::addFileRemovalSubscription(const std::string &fileUuid)
{
    typename decltype(m_fileRemovalSubscriptions)::accessor acc;
    if (m_fileRemovalSubscriptions.insert(acc, fileUuid))
        acc->second = sendFileRemovalSubscription(fileUuid);
}

void FsSubscriptions::removeFileRemovalSubscription(const std::string &fileUuid)
{
    typename decltype(m_fileRemovalSubscriptions)::accessor acc;
    if (m_fileRemovalSubscriptions.find(acc, fileUuid)) {
        sendSubscriptionCancellation(acc->second);
        m_fileRemovalSubscriptions.erase(acc);
    }
}

void FsSubscriptions::addFileRenamedSubscription(const std::string &fileUuid)
{
    typename decltype(m_fileRenamedSubscriptions)::accessor acc;
    if (m_fileRenamedSubscriptions.insert(acc, fileUuid))
        acc->second = sendFileRenamedSubscription(fileUuid);
}

void FsSubscriptions::removeFileRenamedSubscription(const std::string &fileUuid)
{
    typename decltype(m_fileRenamedSubscriptions)::accessor acc;
    if (m_fileRenamedSubscriptions.find(acc, fileUuid)) {
        sendSubscriptionCancellation(acc->second);
        m_fileRenamedSubscriptions.erase(acc);
    }
}

void FsSubscriptions::addFileAttrSubscription(const std::string &fileUuid)
{
    typename decltype(m_fileAttrSubscriptions)::accessor acc;
    if (m_fileAttrSubscriptions.insert(acc, fileUuid))
        acc->second = sendFileAttrSubscription(fileUuid);
}

void FsSubscriptions::removeFileAttrSubscription(const std::string &fileUuid)
{
    typename decltype(m_fileAttrSubscriptions)::accessor acc;
    if (m_fileAttrSubscriptions.find(acc, fileUuid)) {
        sendSubscriptionCancellation(acc->second);
        m_fileAttrSubscriptions.erase(acc);
    }
}

void FsSubscriptions::addQuotaSubscription()
{
    m_quotaSubscription = sendQuotaSubscription();
}

void FsSubscriptions::removeQuotaSubscription()
{
    if (m_quotaSubscription) {
        sendSubscriptionCancellation(m_quotaSubscription);
        m_quotaSubscription = 0;
    }
}

std::int64_t FsSubscriptions::sendFileAttrSubscription(
    const std::string &fileUuid)
{
    DLOG(INFO) << "Sending subscription for change of attributes of file: "
               << fileUuid;
    events::FileAttrSubscription clientSubscription{fileUuid, {}, std::chrono::milliseconds{500}};
    events::FileAttrSubscription serverSubscription{fileUuid, {}, std::chrono::milliseconds{500}};
    return m_eventManager.subscribe(
        std::move(clientSubscription), std::move(serverSubscription));
}

std::int64_t FsSubscriptions::sendFileLocationSubscription(
    const std::string &fileUuid)
{
    DLOG(INFO) << "Sending subscription for change of location of file: "
               << fileUuid;
    events::FileLocationSubscription clientSubscription{fileUuid, {}, std::chrono::milliseconds{500}};
    events::FileLocationSubscription serverSubscription{fileUuid, {}, std::chrono::milliseconds{500}};
    return m_eventManager.subscribe(
        std::move(clientSubscription), std::move(serverSubscription));
}

std::int64_t FsSubscriptions::sendPermissionChangedSubscription(
    const std::string &fileUuid)
{
    DLOG(INFO) << "Sending subscription for change of permissions of file: "
               << fileUuid;
    events::PermissionChangedSubscription clientSubscription{fileUuid};
    events::PermissionChangedSubscription serverSubscription{fileUuid};
    return m_eventManager.subscribe(
        std::move(clientSubscription), std::move(serverSubscription));
}

std::int64_t FsSubscriptions::sendFileRemovalSubscription(
    const std::string &fileUuid)
{
    DLOG(INFO) << "Sending subscription for removing file: " << fileUuid;
    events::FileRemovalSubscription clientSubscription{fileUuid};
    events::FileRemovalSubscription serverSubscription{fileUuid};
    return m_eventManager.subscribe(
        std::move(clientSubscription), std::move(serverSubscription));
}

std::int64_t FsSubscriptions::sendQuotaSubscription()
{
    DLOG(INFO) << "Sending subscription for quota";
    events::QuotaSubscription clientSubscription{};
    events::QuotaSubscription serverSubscription{};
    return m_eventManager.subscribe(
        std::move(clientSubscription), std::move(serverSubscription));
}

std::int64_t FsSubscriptions::sendFileRenamedSubscription(
    const std::string &fileUuid)
{
    DLOG(INFO) << "Sending subscription for renaming file: " << fileUuid;
    events::FileRenamedSubscription clientSubscription{fileUuid};
    events::FileRenamedSubscription serverSubscription{fileUuid};
    return m_eventManager.subscribe(
        std::move(clientSubscription), std::move(serverSubscription));
}

void FsSubscriptions::sendSubscriptionCancellation(std::int64_t id)
{
    m_eventManager.unsubscribe(id);
}

} // namespace client
} // namespace one
