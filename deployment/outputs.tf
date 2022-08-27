## Output master
output "master_ip" {
  value = digitalocean_droplet.master.ipv4_address
}

output "master_ssh" {
  value = "root@${digitalocean_droplet.master.ipv4_address}"
}

## Output client
output "client_ip" {
  value = digitalocean_droplet.client.ipv4_address
}

output "client_ssh" {
  value = "root@${digitalocean_droplet.client.ipv4_address}"
}

## Output workers
# San Francisco
output "sanfrancisco_local_worker_ip" {
  value = digitalocean_droplet.local_worker_sf.ipv4_address
}

output "sanfrancisco_local_worker_ssh" {
  value = "root@${digitalocean_droplet.local_worker_sf.ipv4_address}"
}

# # Frankfurt
output "frankfurt_remote_worker_ip" {
  value = digitalocean_droplet.remote_worker_fr.ipv4_address
}

output "frankfurt_remote_worker_ssh" {
  value = "root@${digitalocean_droplet.remote_worker_fr.ipv4_address}"
}

# # Singapore
output "singapore_remote_worker_ip" {
  value = digitalocean_droplet.remote_worker_sp.ipv4_address
}

output "singapore_remote_worker_ssh" {
  value = "root@${digitalocean_droplet.remote_worker_sp.ipv4_address}"
}
