/**
* @file event.h
* @author Krzysztof Trzepla
* @copyright (C) 2015 ACK CYFRONET AGH
* @copyright This software is released under the MIT license cited in
* 'LICENSE.txt'
*/

#ifndef ONECLIENT_EVENTS_TYPES_EVENT_H
#define ONECLIENT_EVENTS_TYPES_EVENT_H

#include <google/protobuf/message.h>

#include <memory>

namespace one {

namespace clproto {
namespace communication_protocol {
class Answer;
}
}

namespace client {
namespace events {

class EventSerializer;

class Event {
public:
    Event()
        : m_counter{1} {};

    virtual ~Event() = default;

    virtual void emit() = 0;

    virtual std::unique_ptr<EventSerializer> serializer() const = 0;

protected:
    size_t m_counter;
};

class EventSerializer {
public:
    using EventMessage = one::clproto::communication_protocol::Answer;

    virtual std::unique_ptr<google::protobuf::Message>
    serialize(unsigned long long id, const Event &event) const = 0;
};

} // namespace events
} // namespace client
} // namespace one

#endif