# Changelog

> [Ver em Português](CHANGELOG.pt-BR.md)

All notable changes to OBS Holyrics Finder Plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2024-01-XX

### Added
- Initial release of OBS Holyrics Finder Plugin
- Network scanner to automatically find Holyrics instances
- Manual connection test for known IP addresses
- Connection history with smart prioritization
- Bulk update of browser source URLs
- Multi-language support (English and Portuguese Brazil)
- Auto-selection of sources that need updating
- Copy IP:Port to clipboard functionality
- Persistent settings storage
- Network change detection

### Features
- Scans network range (XXX.XXX.XXX.1-254) for Holyrics instances
- Tests historical connections first for faster scanning
- Automatically detects OBS language setting
- Smart source selection (only selects sources that need updating)
- 2-second timeout per connection test
- Supports custom port configuration
- Visual feedback with status messages and progress bar

### Supported Platforms
- Windows 10/11 (64-bit)
- OBS Studio 32.0 or later

### Known Limitations
- Windows only

## [Unreleased]

### Planned Features
- Auto-create browser sources if they don't exist