#!/usr/bin/env python3

import logging
import os

from dotenv import load_dotenv
from paramiko import SSHClient
from scp import SCPClient

load_dotenv()

Frankfurt_remote_worker_IP = os.environ.get("Frankfurt_remote_worker_IP")
SanFrancisco_local_worker_IP = os.environ.get("SanFrancisco_local_worker_IP")

logger = logging.getLogger()


def client_setup(host_url: str, user = "root"):
    logger.info(f"Setting up client {host_url}")
    ssh = SSHClient()
    ssh.load_system_host_keys()
    ssh.connect(host_url, username=user)
    with SCPClient(ssh.get_transport()) as scp:
        scp.put('.env', '.env')
        scp.put('client', 'client')
        # scp.put('client.jpg', 'client.jpg')
        # scp.put('client.txt', 'client.txt')
    ssh.close()

def master_setup(host_url: str, user = "root"):
    logger.info(f"Setting up master {host_url}")
    ssh = SSHClient()
    ssh.load_system_host_keys()
    ssh.connect(host_url, username=user)
    with SCPClient(ssh.get_transport()) as scp:
        scp.put('.env', '.env')
        scp.put('master', 'master')
    ssh.close()


def worker_setup(host_url: str, user = "root"):
    logger.info(f"Setting up worker {host_url}")
    ssh = SSHClient()
    ssh.load_system_host_keys()
    ssh.connect(host_url, username=user)
    with SCPClient(ssh.get_transport()) as scp:
        scp.put('.env', '.env')
        scp.put('env', 'env')
    ssh.close()


if __name__ == "__main__":
    client_setup(SanFrancisco_local_worker_IP)
    master_setup(Frankfurt_remote_worker_IP)
