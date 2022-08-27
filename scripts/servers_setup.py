#!/usr/bin/env python3

import os
import logging

from scp import SCPClient

from ssh import (
    ssh_connection,
    san_francisco_local_worker_ip,
    master_ip,
    client_ip,
    frankfurt_remote_worker_ip,
    singapore_remote_worker_ip,
)

ROOT_DIR = os.path.realpath(os.path.join(os.path.dirname(__file__), '..'))

logger = logging.getLogger()

def get_full_path(file: str, directory = "middleware"):
    return os.path.join(ROOT_DIR, directory, file)


def client_setup(host_url: str, user = "root"):
    print(f"Setting up client {host_url}")
    with ssh_connection(host_url) as ssh:
        transport = ssh.get_transport()
        if transport:
            with SCPClient(transport) as scp:
                scp.put(get_full_path("run_tests_client.sh", "scripts"), "run_tests_client.sh")
                scp.put(get_full_path(".env", "scripts"), ".env")
                scp.put(get_full_path("set_env.sh", "scripts"), "set_env.sh")
                scp.put(get_full_path("client"), "client")
                scp.put(get_full_path("client.jpg"), "client.jpg")
                scp.put(get_full_path("client.txt"), "client.txt")


def master_setup(host_url: str, user = "root"):
    print(f"Setting up master {host_url}")
    with ssh_connection(host_url) as ssh:
        transport = ssh.get_transport()
        if transport:
            with SCPClient(transport) as scp:
                scp.put(get_full_path("start_master.sh", "scripts"), "start_master.sh")
                scp.put(get_full_path(".env", "scripts"), ".env")
                scp.put(get_full_path("set_env.sh", "scripts"), "set_env.sh")
                scp.put(get_full_path("master"), "master")


def worker_setup(host_url: str, user = "root"):
    print(f"Setting up worker {host_url}")
    with ssh_connection(host_url) as ssh:
        transport = ssh.get_transport()
        if transport:
            with SCPClient(transport) as scp:
                scp.put(get_full_path("start_worker.sh", "scripts"), "start_worker.sh")
                scp.put(get_full_path(".env", "scripts"), ".env")
                scp.put(get_full_path("set_env.sh", "scripts"), "set_env.sh")
                scp.put(get_full_path("worker"), "worker")
    ssh.close()


if __name__ == "__main__":
    masters = [master_ip]
    clients = [
        client_ip,
    ]
    workers = [
        san_francisco_local_worker_ip,
        frankfurt_remote_worker_ip,
        singapore_remote_worker_ip
    ]

    for master in masters:
        if not master:
            continue
        master_setup(master)

    for client in clients:
        if not client:
            continue
        client_setup(client)

    for worker in workers:
        if not worker:
            continue
        worker_setup(worker)
