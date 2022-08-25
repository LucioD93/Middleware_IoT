terraform {
  required_providers {
    digitalocean = {
      source  = "digitalocean/digitalocean"
      version = "~> 2.0"
    }
  }
}

variable "do_token" {
  type        = string
  description = "DigitalOcean API token"
}

locals {
  // Operating system
  ubuntu_image = "ubuntu-22-04-x64"

  // Tags
  master_server_tag = "master_server"
  client_server_tag = "client_server"
  worker_server_tag = "worker_server"

  // Executables
  worker_executable = "../middleware/worker"

  // Regions
  new_york      = "nyc1"
  san_francisco = "sfo3"
  amsterdam     = "ams2"
  singapore     = "sgp1"
  london        = "lon1"
  frankfurt     = "fra1"
  toronto       = "tor1"
  bangalore     = "blr1"

  // Droplet sizes
  small_droplet_size  = "s-1vcpu-2gb"
  medium_droplet_size = "s-8vcpu-16gb"
  large_droplet_size  = "c-8"

  tem_droplet_size = "c-4"
}

# Configure the DigitalOcean Provider
provider "digitalocean" {
  token = var.do_token
}

resource "digitalocean_ssh_key" "default" {
  name       = "SSH Key"
  public_key = file("/Users/luciomederos/.ssh/id_rsa.pub")
}
