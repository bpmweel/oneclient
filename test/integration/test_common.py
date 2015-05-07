import os
import random
import string
import sys

_script_dir = os.path.dirname(os.path.realpath(__file__))

# Define variables for use in tests
project_dir = os.path.dirname(os.path.dirname(_script_dir))
appmock_dir = os.path.join(project_dir, 'appmock')
docker_dir = os.path.join(project_dir, 'bamboos', 'docker')

# Append useful modules to the path
sys.path = [appmock_dir, docker_dir] + sys.path


def random_int():
    return random.randint(1, 100)


def random_str():
    return ''.join(random.choice(string.ascii_uppercase + string.digits) for
                   _ in range(random_int()))
