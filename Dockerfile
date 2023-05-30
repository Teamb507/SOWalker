FROM ubuntu:22.04

# Install development tools and libraries.
RUN apt-get -y update && \
    apt-get -y install \
      git \
      wget \
      pkg-config \
      build-essential \
      cmake \
      autoconf \
      libtool \
      zlib1g-dev \
      lsb-release \
      gcc \
      libomp-dev

# Clone and install OR-Tools.
RUN git clone -b v9.2 https://github.com/google/or-tools.git && \
    cd or-tools && \
    cmake -S . -B build -DBUILD_DEPS=ON && \
    cmake --build build --config Release --target install -v && \ 
    echo "export LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH:/usr/local/lib/\"" >> ~/.bashrc && \
    /bin/bash -c "source /etc/profile" && \
    cd ..

# Clean up.
RUN rm -rf /or-tools && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory.
WORKDIR /app

# Copy your application code into the image.
COPY . /app

# Run the application.
CMD ["./my_app"]
