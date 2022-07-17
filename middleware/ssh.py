#!/usr/bin/env python3

from contextlib import contextmanager
import logging
import os

from dotenv import load_dotenv
from paramiko import SSHClient, AutoAddPolicy

load_dotenv()

san_francisco_master_ip = os.environ.get("SanFrancisco_master_IP")
print("San Francisco master ip", san_francisco_master_ip)
new_york_client_ip = os.environ.get("NewYork_client_IP")
print("New Yor client ip", new_york_client_ip)
frankfurt_remote_worker_ip = os.environ.get("Frankfurt_remote_worker_IP")
print("Frankfurt worker ip", frankfurt_remote_worker_ip)
san_francisco_local_worker_ip = os.environ.get("SanFrancisco_local_worker_IP")
print("San Francisco worker ip", san_francisco_local_worker_ip)

logger = logging.getLogger()

@contextmanager
def ssh_connection(host_url: str, user = "root"):
    ssh = SSHClient()
    ssh.load_system_host_keys()
    ssh.set_missing_host_key_policy(AutoAddPolicy())
    ssh.connect(host_url, username=user)
    yield ssh
    ssh.exec_command("ulimit -n 8192")
    ssh.close()