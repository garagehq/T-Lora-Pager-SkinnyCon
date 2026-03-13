# .dockerignore
# .git
# .pio
# build
# dist
# *.bin
# *.elf
# *.log
# .env
# README.md
# LICENSE

FROM python:3.12-slim

# Install build dependencies for PlatformIO and ESP32 toolchain
RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    curl \
    python3-dev \
    build-essential \
    cmake \
    ninja-build \
    libncurses-dev \
    libffi-dev \
    && rm -rf /var/lib/apt/lists/*

# Install PlatformIO Core
RUN pip install --no-cache-dir platformio

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Install project dependencies (if any Python dependencies exist in requirements.txt)
# Assuming standard PIO project structure where dependencies are managed via platformio.ini
# If a requirements.txt exists for Python scripts, uncomment the next line:
# RUN pip install --no-cache-dir -r requirements.txt

# Set environment variables for PlatformIO
ENV PIO_HOME=/app/.pio
ENV PIO_CORE_HOME=/app/.pio

# Expose no ports as this is an embedded firmware build project
# EXPOSE 8080

# Run the build command
CMD ["pio", "run", "-e", "tlora_pager"]