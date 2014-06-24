/**
 * @file messageBuilder_mock.h
 * @author Rafal Slota
 * @copyright (C) 2013 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in 'LICENSE.txt'
 */

#ifndef MESSAGE_BUILDER_MOCK_H
#define MESSAGE_BUILDER_MOCK_H

#include "messageBuilder.h"

#include <gmock/gmock.h>
 
#include <memory>

class MockMessageBuilder: public veil::client::MessageBuilder
{
public:
    MockMessageBuilder(std::shared_ptr<Context> context)
        : MessageBuilder{std::move(context)}
    {
    }

    MOCK_METHOD4(packFuseMessage, ClusterMsg(const std::string&, const std::string&, const std::string&, const std::string&));
    MOCK_METHOD1(decodeAtomAnswer, std::string(protocol::communication_protocol::Answer&));
};

#endif // MESSAGE_BUILDER_MOCK_H
