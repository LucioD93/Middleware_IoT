#!/usr/bin/env python3

from contextlib import contextmanager
import logging
import os

from dotenv import load_dotenv
from paramiko import SSHClient, AutoAddPolicy

load_dotenv()

master_ip = os.environ.get("master_ip")
client_ip = os.environ.get("client_ip")
frankfurt_remote_worker_ip = os.environ.get("frankfurt_remote_worker_ip")
san_francisco_local_worker_ip = os.environ.get("sanfrancisco_local_worker_ip")
singapore_remote_worker_ip = os.environ.get("singapore_remote_worker_ip")

logger = logging.getLogger()

@contextmanager
def ssh_connection(host_url: str, user = "root"):
    ssh = SSHClient()
    ssh.load_system_host_keys()
    ssh.set_missing_host_key_policy(AutoAddPolicy())
    ssh.connect(host_url, username=user)
    yield ssh
    ssh.close()