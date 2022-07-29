#!/usr/bin/env python3

import logging

from scp import SCPClient

from ssh import (
    ssh_connection,
    san_francisco_local_worker_ip,
    san_francisco_master_ip,
    new_york_client_ip,
    frankfurt_remote_worker_ip,
    singapore_remote_worker_ip,
)

logger = logging.getLogger()


def client_setup(host_url: str, user = "root"):
    print(f"Setting up client {host_url}")
    with ssh_connection(host_url) as ssh:
        with SCPClient(ssh.get_transport()) as scp:
            scp.put(".env", ".env")
            scp.put("set_env.sh", "set_env.sh")
            scp.put("client", "client")
            scp.put("run_tests_client.sh", "run_tests_client.sh")
            scp.put("client.jpg", "client.jpg")
            scp.put("client.txt", "client.txt")


def master_setup(host_url: str, user = "root"):
    print(f"Setting up master {host_url}")
    with ssh_connection(host_url) as ssh:
        with SCPClient(ssh.get_transport()) as scp:
            scp.put("start_master.sh", "start_master.sh")
            scp.put(".env", ".env")
            scp.put("set_env.sh", "set_env.sh")
            scp.put("master", "master")


def worker_setup(host_url: str, user = "root"):
    print(f"Setting up worker {host_url}")
    with ssh_connection(host_url) as ssh:
        with SCPClient(ssh.get_transport()) as scp:
            scp.put("start_worker.sh", "start_worker.sh")
            scp.put(".env", ".env")
            scp.put("set_env.sh", "set_env.sh")
            scp.put("worker", "worker")
    ssh.close()


if __name__ == "__main__":
    client_setup(new_york_client_ip)
    master_setup(san_francisco_master_ip)
    worker_setup(san_francisco_local_worker_ip)
    worker_setup(frankfurt_remote_worker_ip)
    worker_setup(singapore_remote_worker_ip)
