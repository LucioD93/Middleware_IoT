#!/usr/bin/env python3

import os

from scp import SCPClient

from ssh import (
    ssh_connection,
    san_francisco_local_worker_ip,
    san_francisco_master_ip,
    new_york_client_ip,
    frankfurt_remote_worker_ip,
    singapore_remote_worker_ip,
)


def get_logs(host_url: str, log_folder: str, user = "root"):
    with ssh_connection(host_url) as ssh:
        with SCPClient(ssh.get_transport()) as scp:
            scp.get("logs", f"logs/{log_folder}", recursive=True)


if __name__ == "__main__":
    os.makedirs("logs", exist_ok=True)
    get_logs(new_york_client_ip, "ny_client_logs")
    get_logs(san_francisco_master_ip, "sf_master")
    get_logs(san_francisco_local_worker_ip, "sf_worker")
    get_logs(frankfurt_remote_worker_ip, "fr_worker")
    get_logs(singapore_remote_worker_ip, "sp_worker")
