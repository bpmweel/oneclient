/**
* @file eventEmissionConfirmation.cc
* @author Krzysztof Trzepla
* @copyright (C) 2015 ACK CYFRONET AGH
* @copyright This software is released under the MIT license cited in
* 'LICENSE.txt'
*/

#include "logging.h"

#include "events.pb.h"
#include "communication_protocol.pb.h"

#include "events/eventBuffer.h"
#include "events/messages/eventEmissionConfirmation.h"

namespace one {
namespace client {
namespace events {

EventEmissionConfirmation::EventEmissionConfirmation(unsigned long long id)
    : m_id{id}
{
}

void EventEmissionConfirmation::process(std::weak_ptr<EventBuffer> buffer) const
{
    buffer.lock()->removeSentMessages(m_id);
}

std::unique_ptr<EventEmissionConfirmation>
EventEmissionConfirmationSerializer::deserialize(const Message &message) const
{
    one::clproto::events::EventEmissionConfirmation eventEmissionConfirmation{};
    if (eventEmissionConfirmation.ParseFromString(message.worker_answer())) {
        return std::make_unique<one::client::events::EventEmissionConfirmation>(
            eventEmissionConfirmation.id());
    }
    LOG(WARNING) << "Cannot deserialize message of type: '"
                 << message.message_type()
                 << "' with ID: " << message.message_id();
    return nullptr;
}

} // namespace events
} // namespace client
} // namespace one
