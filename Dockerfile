# .dockerignore
# .git
# .pio
# build
# dist
# *.log
# .env
# platformio.ini
# README.md
# LICENSE

FROM python:3.12-slim

# Install PlatformIO Core and dependencies
# Using pip to install platformio as it's the standard way for embedded projects
# Note: pio run -e tlora_pager is the entry point, so we need the full pio environment
RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    curl \
    python3-dev \
    build-essential \
    && pip install --no-cache-dir platformio \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Expose no ports as this is an embedded firmware build tool, not a web server
# If web simulation UI is used, uncomment below:
# EXPOSE 8080

# Set environment variables to ensure PIO works correctly in container
ENV PIO_HOME=/app/.pio
ENV PATH=$PATH:/root/.local/bin

# Run the build command for the specified environment
CMD ["pio", "run", "-e", "tlora_pager"]