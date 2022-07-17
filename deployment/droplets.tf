# # Master
resource "digitalocean_droplet" "master_sf" {
  image      = local.ubuntu_image
  name       = "SanFrancisco-master"
  region     = local.san_francisco
  size       = local.tem_droplet_size
  ssh_keys   = [digitalocean_ssh_key.default.fingerprint]
  tags       = [local.master_server_tag]
  monitoring = true
}


# Clients
resource "digitalocean_droplet" "client_ny" {
  image      = local.ubuntu_image
  name       = "NewYork-client"
  region     = local.new_york
  size       = local.tem_droplet_size
  ssh_keys   = [digitalocean_ssh_key.default.fingerprint]
  tags       = [local.client_server_tag]
  monitoring = true
}


# Workers
resource "digitalocean_droplet" "local_worker_sf" {
  image      = local.ubuntu_image
  name       = "SanFrancisco-local-worker"
  region     = local.san_francisco
  size       = local.small_droplet_size
  ssh_keys   = [digitalocean_ssh_key.default.fingerprint]
  tags       = [local.worker_server_tag]
  monitoring = true
}

resource "digitalocean_droplet" "remote_worker_fr" {
  image      = local.ubuntu_image
  name       = "Frankfurt-remote-worker"
  region     = local.frankfurt
  size       = local.small_droplet_size
  ssh_keys   = [digitalocean_ssh_key.default.fingerprint]
  tags       = [local.worker_server_tag]
  monitoring = true
}

resource "digitalocean_droplet" "remote_worker_sp" {
  image      = local.ubuntu_image
  name       = "Singapore-remote-worker"
  region     = local.singapore
  size       = local.small_droplet_size
  ssh_keys   = [digitalocean_ssh_key.default.fingerprint]
  tags       = [local.worker_server_tag]
  monitoring = true
}
