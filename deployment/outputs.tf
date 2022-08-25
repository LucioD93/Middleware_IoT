## Output master
output "sanfrancisco_master_ip" {
  value = digitalocean_droplet.master_sf.ipv4_address
}

output "sanfrancisco_master_ssh" {
  value = "root@${digitalocean_droplet.master_sf.ipv4_address}"
}

## Output client
# output "newyork_client_ip" {
#   value = digitalocean_droplet.client_ny.ipv4_address
# }

# output "newyork_client_ssh" {
#   value = "root@${digitalocean_droplet.client_ny.ipv4_address}"
# }

output "sanfrancisco_client_ip" {
  value = digitalocean_droplet.client_sf.ipv4_address
}

output "sanfrancisco_client_ssh" {
  value = "root@${digitalocean_droplet.client_sf.ipv4_address}"
}

output "toronto_client_ip" {
  value = digitalocean_droplet.client_tor.ipv4_address
}
output "toronto_client_ssh" {
  value = "root@${digitalocean_droplet.client_tor.ipv4_address}"
}

# output "london_client_ip" {
#   value = digitalocean_droplet.client_ln.ipv4_address
# }
# output "london_client_ssh" {
#   value = "root@${digitalocean_droplet.client_ln.ipv4_address}"
# }

# output "amsterdam_client_ip" {
#   value = digitalocean_droplet.client_ams.ipv4_address
# }
# output "amsterdam_client_ssh" {
#   value = "root@${digitalocean_droplet.client_ams.ipv4_address}"
# }

# output "singapore_client_ip" {
#   value = digitalocean_droplet.client_sg.ipv4_address
# }
# output "singapore_client_ssh" {
#   value = "root@${digitalocean_droplet.client_sg.ipv4_address}"
# }

# output "bangalore_client_ip" {
#   value = digitalocean_droplet.client_blr.ipv4_address
# }
# output "bangalore_client_ssh" {
#   value = "root@${digitalocean_droplet.client_blr.ipv4_address}"
# }

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
