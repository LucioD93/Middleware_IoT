terraform {
  required_providers {
    digitalocean = {
      source = "digitalocean/digitalocean"
      version = "~> 2.0"
    }
  }
}

variable "do_token" {
  type = string
  description = "DigitalOcean API token"
}

locals {
    // Operating system
    ubuntu_image = "ubuntu-22-04-x64"

    // Regions
    new_york = "nyc2"
    san_francisco = "sfo3"
    amsterdam = "ams2"
    singapore = "sgp1"
    london = "lon1"
    frankfurt = "fra1"

    // Droplet sizes
    small_droplet_size     = "s-1vcpu-1gb"
}

# Configure the DigitalOcean Provider
provider "digitalocean" {
  token = var.do_token
}

resource "digitalocean_ssh_key" "default" {
  name       = "SSH Key"
  public_key = file("/Users/luciomederos/.ssh/id_rsa.pub")
}


output "SanFrancisco_local_worker_IP" {
    value = digitalocean_droplet.local_worker_sf.ipv4_address
}

output "SanFrancisco_local_worker_ssh" {
    value = "root@${digitalocean_droplet.local_worker_sf.ipv4_address}"
}

output "Frankfurt_remote_worker_IP" {
    value = digitalocean_droplet.remote_worker_fr.ipv4_address
}

output "Frankfurt_remote_worker_ssh" {
    value = "root@${digitalocean_droplet.remote_worker_fr.ipv4_address}"
}
