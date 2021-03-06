## Output master
output "SanFrancisco_master_IP" {
  value = digitalocean_droplet.master_sf.ipv4_address
}

output "SanFrancisco_master_ssh" {
  value = "root@${digitalocean_droplet.master_sf.ipv4_address}"
}

## Output client
output "NewYork_client_IP" {
  value = digitalocean_droplet.client_ny.ipv4_address
}

output "NewYork_client_ssh" {
  value = "root@${digitalocean_droplet.client_ny.ipv4_address}"
}


## Output workers
# San Francisco
output "SanFrancisco_local_worker_IP" {
  value = digitalocean_droplet.local_worker_sf.ipv4_address
}

output "SanFrancisco_local_worker_ssh" {
  value = "root@${digitalocean_droplet.local_worker_sf.ipv4_address}"
}

# # Frankfurt
output "Frankfurt_remote_worker_IP" {
  value = digitalocean_droplet.remote_worker_fr.ipv4_address
}

output "Frankfurt_remote_worker_ssh" {
  value = "root@${digitalocean_droplet.remote_worker_fr.ipv4_address}"
}
