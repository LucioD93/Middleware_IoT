resource "digitalocean_droplet" "local_worker_sf" {
    image = local.ubuntu_image
    name = "SanFrancisco-local-worker"
    region = local.san_francisco
    size = local.small_droplet_size
    ssh_keys = [digitalocean_ssh_key.default.fingerprint]
}

resource "digitalocean_droplet" "remote_worker_fr" {
    image = local.ubuntu_image
    name = "Frankfurt-remote-worker"
    region = local.frankfurt
    size = local.small_droplet_size
    ssh_keys = [digitalocean_ssh_key.default.fingerprint]
}
