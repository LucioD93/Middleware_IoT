FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    git gcc cmake gdb libsctp-dev uuid-dev \
    python3 htop nano zsh

WORKDIR /code

CMD ["zsh"]
