/**
 * @file fuseResponse.h
 * @author Konrad Zemek
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#ifndef HELPERS_MESSAGES_FUSE_MESSAGES_FUSE_RESPONSE_H
#define HELPERS_MESSAGES_FUSE_MESSAGES_FUSE_RESPONSE_H

#include "messages/serverMessage.h"

#include <memory>
#include <string>
#include <vector>

namespace one {
namespace messages {
namespace fuse {

/**
 * The FuseResponse class represents a response to a fuse request.
 */
class FuseResponse : public ServerMessage {
public:
    /**
     * Constructor.
     * @param serverMessage Protocol Buffers message representing
     * @c FuseResponse counterpart.
     */
    FuseResponse(const std::unique_ptr<ProtocolServerMessage> &serverMessage);

    virtual std::string toString() const override;
};

} // namespace fuse
} // namespace messages
} // namespace one

#endif // HELPERS_MESSAGES_FUSE_MESSAGES_FUSE_RESPONSE_H
