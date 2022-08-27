#!/usr/bin/env python3

import os

from scp import SCPClient

from ssh import (
    ssh_connection,
    san_francisco_local_worker_ip,
    san_francisco_master_ip,
    client_ip,
    frankfurt_remote_worker_ip,
    singapore_remote_worker_ip,
)


def get_logs(host_url: str, log_folder: str, user = "root"):
    with ssh_connection(host_url) as ssh:
        transport = ssh.get_transport()
        if transport:
            with SCPClient(transport) as scp:
                scp.get("logs", f"logs/{log_folder}", recursive=True)


if __name__ == "__main__":
    os.makedirs("logs", exist_ok=True)

    servers = [
        (san_francisco_master_ip, "sf_master"),
        (client_ip, "client"),
        (san_francisco_local_worker_ip, "sf_worker"),
        (frankfurt_remote_worker_ip, "fr_worker"),
        (singapore_remote_worker_ip, "sp_worker"),
    ]

    for server, name in servers:
        if not server:
            continue
        get_logs(server, name)
