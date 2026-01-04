#!/bin/bash
set -e

EXECUTABLE="corsair-780t-fan-control-mod"
SERVICE="corsair-780t-fan-control-mod-daemon.service"

echo "Building the executable..."
mkdir -p src/build && cd src/build
cmake ..
make -j$(nproc)

echo "Copying executable to /usr/local/bin..."
sudo cp "./$EXECUTABLE" /usr/local/bin/
sudo chmod +x /usr/local/bin/$EXECUTABLE

echo "Creating systemd service..."
SERVICE_FILE="/etc/systemd/system/$SERVICE"
sudo bash -c "cat > $SERVICE_FILE" <<EOL
[Unit]
Description=Corsair 780T Fan Control mod (runs as root)
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/$EXECUTABLE
Restart=always
RestartSec=5
User=root

[Install]
WantedBy=multi-user.target
EOL

echo "Reloading systemd daemon..."
sudo systemctl daemon-reload

echo "Enabling service to start at boot..."
sudo systemctl enable $SERVICE

echo "Starting service..."
sudo systemctl start $SERVICE

echo "Done! Check status with:"
echo "  sudo systemctl status $SERVICE"
echo "Follow logs with:"
echo "  journalctl -fu $SERVICE"
