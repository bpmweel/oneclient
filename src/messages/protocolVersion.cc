/**
 * @file protocolVersion.cc
 * @author Krzysztof Trzepla
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#include "messages/protocolVersion.h"

#include "server_messages.pb.h"

namespace one {
namespace messages {

ProtocolVersion::ProtocolVersion(
    std::unique_ptr<ProtocolServerMessage> serverMessage)
{
    auto &protocolVersionMsg = serverMessage->protocol_version();
    m_major = protocolVersionMsg.major();
    m_minor = protocolVersionMsg.minor();
}

uint32_t ProtocolVersion::major() const { return m_major; }

uint32_t ProtocolVersion::minor() const { return m_minor; }

} // namespace messages
} // namespace one
