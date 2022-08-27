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

variable "client_region" {
  type        = string
  description = "Region to spawn the client server in"

  validation {
    condition     = contains(["nyc1", "sfo3", "ams2", "sgp1", "lon1", "fra1", "tor1", "blr1"], var.client_region)
    error_message = "Valid values for client_region var: test_variable are (nyc1, sfo3, ams2, sgp1, lon1, fra1, tor1, blr1)."
  }
}

variable "master_server_size" {
  type        = string
  description = "Droplet size for the master server"
  default     = "s-8vcpu-16gb"

  validation {
    condition     = contains(["s-2vcpu-4gb", "s-8vcpu-16gb", "c-8", "c-16"], var.master_server_size)
    error_message = "Valid values for client_region var: test_variable are (2vcpu-4gb, s-8vcpu-16gb, c-8, c-16)."
  }
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
  client_region = var.client_region
  new_york      = "nyc1"
  san_francisco = "sfo3"
  amsterdam     = "ams2"
  singapore     = "sgp1"
  london        = "lon1"
  frankfurt     = "fra1"
  toronto       = "tor1"
  bangalore     = "blr1"

  // Droplet sizes
  local_worker_droplet_size  = "s-1vcpu-1gb"
  remote_worker_droplet_size = "s-1vcpu-2gb"
  medium_droplet_size        = "s-8vcpu-16gb"
  large_droplet_size         = "c-8"
}

# Configure the DigitalOcean Provider
provider "digitalocean" {
  token = var.do_token
}

resource "digitalocean_ssh_key" "default" {
  name       = "SSH Key"
  public_key = file("/Users/luciomederos/.ssh/id_rsa.pub")
}
