# Build compiler image
build-compiler:
	docker build --tag middleware-compiler middleware/.

# Compilers
compile-master: build-compiler
	docker-compose run compile

# End compilers

# Runners
run-master: compile-master
	docker-compose run compile ./master

# End runners

# Utils

clean: build-compiler
	docker-compose run compile make clean

bash: build-compiler
	docker compose run bash
