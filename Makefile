# Build compiler image
build-compiler:
	docker compose build

up:
	docker compose up -d

down:
	docker compose down

# Attach bash shell
bash: up
	docker compose exec compiler /bin/bash
