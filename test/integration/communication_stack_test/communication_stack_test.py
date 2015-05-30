"""Documentation for communication_stack module."""

__author__ = "Konrad Zemek"
__copyright__ = """(C) 2015 ACK CYFRONET AGH,
This software is released under the MIT license cited in 'LICENSE.txt'."""

import os
import sys
import time

import pytest

script_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.dirname(script_dir))
from test_common import *
from performance import *

# noinspection PyUnresolvedReferences
from environment import appmock, common, docker
import communication_stack
import appmock_client


# noinspection PyClassHasNoInit
class TestCommunicator:
    @classmethod
    def setup_class(cls):
        cls.result = appmock.up(image='onedata/builder', bindir=appmock_dir,
                                dns='none', uid=common.generate_uid(),
                                config_path=os.path.join(script_dir,
                                                         'env.json'))

        [container] = cls.result['docker_ids']
        cls.ip = docker.inspect(container)['NetworkSettings']['IPAddress']. \
            encode('ascii')

    @classmethod
    def teardown_class(cls):
        docker.remove(cls.result['docker_ids'], force=True, volumes=True)

    @performance({'repeats': 3,
                  'configs': [
                      {'name': 'test_config'}
                  ]})
    def test_send(self, parameters):
        """Documentation for test_send test."""
        com = communication_stack.Communicator(3, self.ip, 5555, False)
        com.connect()

        sent_bytes = com.send("this is a message")
        time.sleep(0.5)

        assert 0 < appmock_client.tcp_server_message_count(self.ip, 5555,
                                                           sent_bytes)

    @performance(skip=True)
    def test_communicate(self, parameters):
        com = communication_stack.Communicator(1, self.ip, 5555, False)
        com.connect()

        request = com.communicate("this is a request")

        reply = communication_stack.prepareReply(request, "this is a reply")
        time.sleep(0.5)

        assert 1 == appmock_client.tcp_server_message_count(self.ip, 5555,
                                                            request)
        appmock_client.tcp_server_send(self.ip, 5555, reply)

        assert com.communicateReceive() == reply

    @performance(skip=True)
    def test_successful_handshake(self, parameters):
        com = communication_stack.Communicator(1, self.ip, 5555, False)
        handshake = com.setHandshake("handshake", False)
        com.connect()

        request = com.send("this is another request")

        time.sleep(0.5)
        assert 1 == appmock_client.tcp_server_message_count(self.ip, 5555,
                                                            handshake)
        assert 0 == appmock_client.tcp_server_message_count(self.ip, 5555,
                                                            request)

        reply = communication_stack.prepareReply(handshake, "handshakeReply")
        appmock_client.tcp_server_send(self.ip, 5555, reply)

        assert com.handshakeResponse() == reply
        assert 1 == appmock_client.tcp_server_message_count(self.ip, 5555,
                                                            request)

    @performance(skip=True)
    def test_unsuccessful_handshake(self, parameters):
        com = communication_stack.Communicator(3, self.ip, 5555, False)
        handshake = com.setHandshake("anotherHanshake", True)
        com.connect()

        time.sleep(0.5)

        assert 3 == appmock_client.tcp_server_message_count(self.ip, 5555,
                                                            handshake)

        reply = communication_stack.prepareReply(handshake, "anotherHandshakeR")
        appmock_client.tcp_server_send(self.ip, 5555, reply)

        # The connections should now be recreated and another handshake sent
        time.sleep(1.5)
        assert 6 == appmock_client.tcp_server_message_count(self.ip, 5555,
                                                            handshake)

    @performance(skip=True)
    def test_exception_on_unsuccessful_handshake(self, parameters):
        com = communication_stack.Communicator(3, self.ip, 5555, True)
        handshake = com.setHandshake("oneMoreHanshake", True)
        com.connect()

        com.communicate("communication")

        reply = communication_stack.prepareReply(handshake, "oneMoreHandshakeR")
        appmock_client.tcp_server_send(self.ip, 5555, reply)

        with pytest.raises(RuntimeError) as exc:
            com.communicateReceive()
        assert "ConnectionError" in str(exc.value)
        assert "handshake" in str(exc.value)

    @performance(skip=True)
    def test_exception_on_connection_error(self, parameters):
        com = communication_stack.Communicator(3, self.ip,
                                               9876, True)  # note the port
        com.setHandshake("secondMoreHandshake", True)
        com.connect()

        com.communicate("communication2")
        with pytest.raises(RuntimeError) as exc:
            com.communicateReceive()
        assert "ConnectionError" in str(exc.value)
        assert "failed to establish" in str(exc.value)
