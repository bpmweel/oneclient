/**
 * @file eventManager.h
 * @author Krzysztof Trzepla
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#ifndef ONECLIENT_EVENTS_EVENT_MANAGER_H
#define ONECLIENT_EVENTS_EVENT_MANAGER_H

#include "eventStream.h"

#include <sys/types.h>

#include <memory>
#include <cstddef>
#include <functional>
#include <unordered_map>

namespace one {

namespace clproto {
class ServerMessage;
}

namespace client {

class Context;

namespace events {

class Event;
class ReadEvent;
class WriteEvent;

/**
 * The EventManager class is responsible for events management. It handles
 * server
 * push messages and provides interface for events emission.
 */
class EventManager {
public:
    /**
     * Constructor.
     * @param context A @c Context instance used to instantiate event streams
     * and
     * acquire communicator instance to register for push messages.
     */
    EventManager(std::shared_ptr<Context> context);

    ~EventManager() = default;

    /**
     * Emits a read event.
     * @param fileUuid UUID of file associated with a read operation.
     * @param offset Distance from the beginning of the file to the first byte
     * read.
     * @param size Number of bytes read.
     */
    void emitReadEvent(
        off_t offset, std::size_t size, std::string fileUuid) const;

    /**
     * Emits a write event.
     * @param fileUuid UUID of file associated with a write operation.
     * @param offset Distance from the beginning of the file to the first byte
     * written.
     * @param size Number of bytes written.
     * @param fileSize Size of file after a write operation.
     * @param storageId ID of a storage the write has been written to,
     * @param fileId ID of a file on the storage.
     */
    void emitWriteEvent(off_t offset, std::size_t size, std::string fileUuid,
        std::string storageId, std::string fileId) const;

    /**
     * Emits a truncate event.
     * @param fileUuid UUID of file associated with a truncate operation.
     * @param fileSize Size of file after a truncate operation.
     */
    void emitTruncateEvent(off_t fileSize, std::string fileUuid) const;

private:
    void handleServerMessage(const clproto::ServerMessage &msg);

    std::shared_ptr<Context> m_context;
    std::unique_ptr<EventStream<ReadEvent>> m_readEventStream;
    std::unique_ptr<EventStream<WriteEvent>> m_writeEventStream;
    std::unordered_map<uint64_t, std::function<void()>>
        m_subscriptionsCancellation;
};

} // namespace events
} // namespace client
} // namespace one

#endif // ONECLIENT_EVENTS_EVENT_MANAGER_H
