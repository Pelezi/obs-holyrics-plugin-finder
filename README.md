# OBS Holyrics Finder Plugin

**Find and connect to Holyrics instances on your network directly from OBS Studio!**

[![GitHub release](https://img.shields.io/github/v/release/Pelezi/obs-holyrics-plugin-finder)](https://github.com/Pelezi/obs-holyrics-plugin-finder/releases)
[![License](https://img.shields.io/github/license/Pelezi/obs-holyrics-plugin-finder)](LICENSE)

> 🇧🇷 [Leia em Português](README.pt-BR.md)

This OBS Studio plugin helps you quickly find and configure browser sources pointing to [Holyrics](https://holyrics.com.br/) stage view instances on your local network.

##  Features

-  **Network Scanner**: Automatically scan your network to find Holyrics instances
-  **Quick Test**: Test connection to a specific IP address
-  **Smart History**: Remembers successful connections and tests them first
-  **Bulk Update**: Update multiple browser sources with one click
-  **Multi-language**: Supports English and Portuguese (Brazil)
-  **Copy IP:Port**: Easy clipboard copy functionality
-  **Smart Selection**: Auto-selects only sources that need updating

##  Installation

### Requirements

- **OBS Studio** 28.0 or later
- **Windows** 10/11 (64-bit)
- **Holyrics** running on your network

### Steps

1. Download the latest release ZIP from [Releases](https://github.com/Pelezi/obs-holyrics-plugin-finder/releases)
2. Extract the ZIP file
3. Copy the `obs-plugins` folder to your OBS installation directory
   - Default: `C:\Program Files\obs-studio\`
4. Restart OBS Studio
5. Find the plugin in **Tools** > **Holyrics Finder** (or "Localizador Holyrics" in Portuguese)

##  Usage

### Finding Holyrics on Your Network

1. Open OBS Studio
2. Go to **Tools** > **Holyrics Finder**
3. Choose one of these options:
   - **Test Connection**: If you know the IP address
   - **Scan Network**: To automatically find Holyrics

### Updating Browser Sources

1. After a successful connection, sources are automatically listed
2. Sources that need updating are automatically selected
3. Sources already pointing to the correct IP:Port are unchecked
4. Click **Update Selected Sources** to apply changes

### Tips

-  The plugin remembers successful connections
-  On next scan, it tests previous IPs first (faster!)
-  If you change networks, it automatically adapts
-  Use "Copy IP:Port" to quickly share connection details

##  Supported Languages

The plugin automatically matches OBS's language setting:

-  **English**
-  **Português (Brasil)**

To change language:
1. Go to OBS **Settings** > **General** > **Language**
2. Restart OBS

##  Building from Source

### Prerequisites

- CMake 3.16 or later
- Visual Studio 2026 with C++ tools
- Qt 6
- OBS Studio 32.x build dependencies

### Build Steps

```powershell
# Clone the repository
git clone https://github.com/Pelezi/obs-holyrics-plugin-finder.git
cd obs-holyrics-plugin-finder

# Configure
cmake --preset windows-x64

# Build Release
cmake --build build_x64_ninja --config Release

# Or use the deployment script for local testing
.\build-and-deploy.ps1
```

### Creating a Release Package

```powershell
.\build-release.ps1 -Version "1.0.0"
```

This creates a ready-to-distribute ZIP file in the `release` folder.

##  How It Works

1. **Network Scanning**: Tests connections to all IPs in your subnet (XXX.XXX.XXX.1-254)
2. **Smart Detection**: Identifies Holyrics by checking HTTP responses
3. **History Priority**: Tests previously successful IPs first
4. **Auto-Update**: Only selects sources that point to different IPs

##  Troubleshooting

### Plugin doesn't appear in Tools menu

- Verify you copied files to the correct OBS directory
- Check OBS version (must be 32.0+)
- Look at OBS logs: **Help** > **Log Files** > **View Current Log**

### Can't find Holyrics instance

- Ensure Holyrics is running and accessible on your network
- Check firewall settings
- Try testing with the specific IP address instead of scanning
- Default Holyrics port is 80

### Sources not updating

- Make sure sources are browser sources with IP:Port URLs
- Check if sources are in the current scene collection
- Try clicking "Refresh List"

##  Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

##  License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](LICENSE) file for details.

##  Acknowledgments

- [OBS Studio](https://obsproject.com/) - Amazing streaming software
- [Holyrics](https://holyrics.com.br/) - Church presentation software
- All contributors and users of this plugin

##  Support

-  **Bug Reports**: [GitHub Issues](https://github.com/Pelezi/obs-holyrics-plugin-finder/issues)
-  **Feature Requests**: [GitHub Issues](https://github.com/Pelezi/obs-holyrics-plugin-finder/issues)

---

**Made with ❤️ for the worship tech community**
