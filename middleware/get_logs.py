#!/usr/bin/env python3

import os

from scp import SCPClient

from ssh import (
    ssh_connection,
    san_francisco_local_worker_ip,
    san_francisco_master_ip,
    new_york_client_ip,
    sanfrancisco_client_ip,
    toronto_client_ip,
    frankfurt_client_ip,
    london_client_ip,
    amsterdam_client_ip,
    singapore_client_ip,
    bangalore_client_ip,
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
        (new_york_client_ip, "ny_client"),
        (sanfrancisco_client_ip, "sf_client"),
        (toronto_client_ip, "tor_client"),
        (frankfurt_client_ip, "fr_client"),
        (london_client_ip, "ln_client"),
        (amsterdam_client_ip, "ams_client"),
        (singapore_client_ip, "sg_client"),
        (bangalore_client_ip, "bg_client"),
        (san_francisco_local_worker_ip, "sf_worker"),
        (frankfurt_remote_worker_ip, "fr_worker"),
        (singapore_remote_worker_ip, "sp_worker"),
    ]

    for server, name in servers:
        if not server:
            continue
        get_logs(server, name)
