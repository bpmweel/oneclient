/**
 * @file getFileLocation.cc
 * @author Konrad Zemek
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#include "getFileLocation.h"

#include "messages.pb.h"

#include <memory>
#include <sstream>

namespace one {
namespace messages {
namespace fuse {

GetFileLocation::GetFileLocation(
    std::string uuid, const helpers::FlagsSet flags)
    : FileRequest{std::move(uuid)}
    , m_flags{flags}
{
}

std::string GetFileLocation::toString() const
{
    std::stringstream stream;
    stream << "type: 'GetFileLocation', uuid: " << m_contextGuid << ", flags: ";

    if (m_flags.count(helpers::Flag::RDWR))
        stream << "rdwr";
    else if (m_flags.count(helpers::Flag::RDONLY))
        stream << "read";
    else if (m_flags.count(helpers::Flag::WRONLY))
        stream << "write";

    return stream.str();
}

std::unique_ptr<ProtocolClientMessage> GetFileLocation::serializeAndDestroy()
{
    auto msg = FileRequest::serializeAndDestroy();
    auto gfl = msg->mutable_fuse_request()
                   ->mutable_file_request()
                   ->mutable_get_file_location();

    gfl->set_create_handle(true);

    if (m_flags.count(helpers::Flag::RDWR))
        gfl->set_flags(clproto::FileLocationFlags::READ_WRITE);
    else if (m_flags.count(helpers::Flag::RDONLY))
        gfl->set_flags(clproto::FileLocationFlags::READ);
    else if (m_flags.count(helpers::Flag::WRONLY))
        gfl->set_flags(clproto::FileLocationFlags::WRITE);

    return msg;
}

} // namespace fuse
} // namespace messages
} // namespace one
