FROM gcc:15.1

RUN apt update && apt upgrade -y && apt install cmake -y
