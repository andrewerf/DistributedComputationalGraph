FROM archlinux:base as build

RUN pacman-key --init
RUN pacman-key --populate archlinux
RUN pacman -Syu --noconfirm
RUN pacman -S --noconfirm gcc cmake make eigen nlohmann-json git hiredis librdkafka

WORKDIR DistributedComputationalGraph
COPY 3rdparty 3rdparty
COPY src src
COPY CMakeLists.txt CMakeLists.txt

WORKDIR build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release
RUN make -j8


FROM archlinux:base as manager

RUN pacman-key --init
RUN pacman-key --populate archlinux
RUN pacman -Syu --noconfirm
RUN pacman -S --noconfirm eigen nlohmann-json hiredis librdkafka

WORKDIR distribution
COPY --from=build DistributedComputationalGraph/build/manager .
COPY --from=build DistributedComputationalGraph/build/client .
COPY --from=build DistributedComputationalGraph/build/worker .

ENTRYPOINT /distribution/manager $KAFKA_HOST $REDIS_URI -v 6


FROM archlinux:base as worker

RUN pacman-key --init
RUN pacman-key --populate archlinux
RUN pacman -Syu --noconfirm
RUN pacman -S --noconfirm eigen nlohmann-json hiredis librdkafka

WORKDIR distribution
COPY --from=build DistributedComputationalGraph/build/manager .
COPY --from=build DistributedComputationalGraph/build/client .
COPY --from=build DistributedComputationalGraph/build/worker .

ENTRYPOINT /distribution/worker $KAFKA_HOST $REDIS_URI $MANAGER_HOST -v 6

