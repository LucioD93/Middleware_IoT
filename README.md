# Middleware_IoT

#### Repositorio para proyecto de Tesis

## Desarrollo

El proyecto esta diseñado para ser ejecutado dentro de un ambiente de Docker para permitir el desarrollo sin importar el ambiente local.

## Instrucciones para ejecutar

El proyecto cuenta con un `Makefile` global para facilitar la conexión al contenedor de Docker.
Para crear una terminal ejecutar `make bash` para construir la imagen de Docker, iniciar el contedor y arrancar una terminal `bash`.

Una vez dentro del contenedor se puede ejecutar `make master` para compilar el proceso del servidor maestro y `make worker` para compilar el proceso del servidor trabajador
