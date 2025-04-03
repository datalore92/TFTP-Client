# Cross-Platform TFTP Client

A simple cross-platform TFTP (Trivial File Transfer Protocol) client application built with GTK3 that allows users to upload files to TFTP servers.

## Features

- Simple and intuitive GTK3-based graphical user interface
- Works on both Windows and Linux platforms
- File selection through standard file dialog
- Support for standard TFTP protocol (RFC 1350)
- Displays transfer status and results

## Requirements

### For Windows:
- MinGW or MSYS2 with GCC
- GTK3 development libraries
- CMake (3.10 or higher)

### For Linux:
- GCC or Clang
- GTK3 development libraries
- CMake (3.10 or higher)

## Building the Application

### Installing Dependencies

#### On Windows (using MSYS2):
```
pacman -S mingw-w64-x86_64-gtk3 mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc
```

#### On Ubuntu/Debian:
```bash
sudo apt-get install build-essential cmake libgtk-3-dev
```

#### On Fedora:
```bash
sudo dnf install cmake gcc gtk3-devel
```

### Building

1. Create a build directory:
```
mkdir build && cd build
```

2. Configure the project:
```
cmake ..
```

3. Build:
```
cmake --build .
```

## Quick Installation

For those who just want to use the application without building it:

1. Visit our [GitHub Releases page](https://github.com/datalore92/TFTP-Client/releases)
2. Download the appropriate version for your platform:
   - Windows users: Download `tftp-client-windows.zip`
   - Linux users: Download `tftp-client-linux.tar.gz`
3. See [INSTALLATION.md](INSTALLATION.md) for detailed instructions

## Usage

1. Launch the application
2. Enter the TFTP server address in the "Server" field
3. Click "Browse" to select a file to upload
4. Click "Send" to initiate the file transfer
5. Check the status message at the bottom of the window for transfer results

## Protocol Details

This client implements the TFTP protocol as defined in RFC 1350. It supports "octet" mode for binary file transfers and follows the standard TFTP packet structure and exchange patterns:

- WRQ (Write Request) to initiate upload
- DATA packets for transferring file contents
- ACK packets for acknowledging received blocks
- ERROR packets for handling errors

## How It Works

### The Basic Process

1. **Server Requirement**: The IP address you enter must be running a TFTP server. You cannot send files to just any IP address - the recipient must be running TFTP server software configured to accept uploads.

2. **Protocol Flow**:
   - When you click "Send File", your client sends a "Write Request" (WRQ) packet to port 69 on the server
   - If a TFTP server is listening, it responds with an acknowledgment
   - Your client then breaks the file into small chunks and sends them one by one
   - The server acknowledges each chunk
   - This continues until the entire file is transferred

3. **Security Considerations**:
   - TFTP is an insecure protocol with no authentication
   - The receiving server must be explicitly configured to accept files
   - Most networks block TFTP traffic by default as a security measure
   - Home routers and firewalls typically block incoming TFTP requests

### Common Use Cases

TFTP clients are typically used in controlled environments such as:

- Uploading firmware to network devices
- Deploying configuration files to managed devices
- Transferring files within secure local networks
- Network boot scenarios (PXE boot)

TFTP is generally not suitable for:
- General file sharing between users
- Sending files across the public internet
- Any scenario requiring security or authentication

## Testing the Application

### Setting Up a Local TFTP Server

#### For Windows using SolarWinds Free TFTP Server:
1. Download and install SolarWinds Free TFTP Server from: https://www.solarwinds.com/free-tools/free-tftp-server
2. Launch the application
3. Configure the server:
   - Click on "File" → "Configure" in the menu
   - In the "General" tab, set your "TFTP Root Directory" to a folder (e.g., "C:\TFTP-Root")
   - Go to the "Server Bindings" tab
   - Check "Use custom server binding"
   - Look at the "Currently Available Addresses" list and double-click on your actual IP address to add it to the "Custom server bindings" list
   - Click "OK" to save your settings
4. Ensure the server shows "Running" status in the main window
5. In your TFTP client:
   - Enter your actual IP address (the same one you selected in server bindings) in the "Server" field
   - Select a small test file to upload
   - Click "Send"
6. After sending, check your TFTP Root Directory (e.g., "C:\TFTP-Root"):
   - The file you selected and sent should now appear in this directory
   - The file will have the same name as the original file
   - You should also see the transfer activity logged in the SolarWinds TFTP Server main window

Note: Using your actual IP address (rather than 127.0.0.1) is often more reliable for TFTP transfers.

#### For Linux:
1. Install tftpd:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install tftpd-hpa
   
   # Fedora
   sudo dnf install tftp-server
   ```

2. Configure the server:
   ```bash
   sudo mkdir -p /srv/tftp
   sudo chmod 777 /srv/tftp
   sudo systemctl restart tftpd-hpa  # On Ubuntu/Debian
   sudo systemctl restart tftp.service  # On Fedora
   ```

3. Use "127.0.0.1" as the server address in your client

### Public TFTP Test Servers

Note: Public TFTP servers are rare these days due to security concerns. If you find one, be aware that:

- Only use them for legitimate testing purposes
- Do not upload sensitive information
- Files may be limited in size
- The server might not be accessible from all networks

Currently, there are no reliable public TFTP servers recommended for general testing. It's preferable to set up your own local server for testing.