# Release Checklist and Instructions

## Before Creating a Release

### 1. Update Version Numbers
- [ ] Update version in `buildspec.json`
- [ ] Update version in `CHANGELOG.md`
- [ ] Update version in this release script command

### 2. Test Everything
- [ ] Test on fresh OBS installation
- [ ] Test network scanning
- [ ] Test manual connection
- [ ] Test source updates
- [ ] Test in both English and Portuguese
- [ ] Test connection history
- [ ] Test network change detection

### 3. Build Release Package

```powershell
.\build-release.ps1 -Version "1.0.0"
```

### 4. Verify Package Contents

The ZIP should contain:
```
obs-holyrics-finder-1.0.0/
??? obs-plugins/
?   ??? 64bit/
?       ??? obs-holyrics-finder.dll
?       ??? obs-holyrics-finder.pdb
??? INSTALL.txt
??? README.md
??? LICENSE
```

## Creating the GitHub Release

### 1. Create and Push Tag

```bash
git tag -a v1.0.0 -m "Release version 1.0.0"
git push origin v1.0.0
```

### 2. Create Release on GitHub

1. Go to: https://github.com/Pelezi/obs-holyrics-plugin-finder/releases/new
2. Choose the tag you just created (v1.0.0)
3. Set release title: `v1.0.0 - Initial Release`
4. Use the template below for release notes
5. Upload the ZIP file from `release/` folder
6. Publish release

### 3. Release Notes Template

```markdown
# OBS Holyrics Finder v1.0.0

**Find and connect to Holyrics instances on your network!**

## ?? First Release

This is the initial release of the OBS Holyrics Finder plugin for OBS Studio.

## ? Features

- ?? **Network Scanner**: Automatically find Holyrics instances on your network
- ?? **Quick Test**: Test connection to specific IP addresses
- ?? **Smart History**: Remembers and prioritizes previous successful connections
- ?? **Bulk Update**: Update multiple browser sources at once
- ?? **Multi-language**: Full support for English and Português (Brasil)
- ?? **Copy to Clipboard**: Quick IP:Port copying
- ? **Smart Selection**: Only selects sources that actually need updating

## ?? Installation

1. Download `obs-holyrics-finder-1.0.0-windows-x64.zip`
2. Extract the ZIP file
3. Copy the `obs-plugins` folder to: `C:\Program Files\obs-studio\`
4. Restart OBS Studio
5. Find the plugin in **Tools** ? **Holyrics Finder**

For detailed instructions, see [INSTALL.txt](https://github.com/Pelezi/obs-holyrics-plugin-finder#installation) in the package.

## ?? Requirements

- Windows 10/11 (64-bit)
- OBS Studio 28.0 or later
- Holyrics running on your network

## ?? What's New

- Initial release with full network scanning capabilities
- Connection history with smart prioritization
- Multi-language support (English + Portuguese)
- Smart browser source detection and updating

## ?? Known Issues

None at this time. Please report any issues on the [Issues page](https://github.com/Pelezi/obs-holyrics-plugin-finder/issues).

## ?? Documentation

- [Full README](https://github.com/Pelezi/obs-holyrics-plugin-finder/blob/master/README.md)
- [Changelog](https://github.com/Pelezi/obs-holyrics-plugin-finder/blob/master/CHANGELOG.md)

## ?? Acknowledgments

Thanks to everyone who helped test and provide feedback!

---

**Full Changelog**: https://github.com/Pelezi/obs-holyrics-plugin-finder/commits/v1.0.0
```

## Post-Release

### 1. Verify Release
- [ ] Download the released ZIP
- [ ] Test installation on clean system
- [ ] Verify all features work

### 2. Announce
- [ ] Update repository description
- [ ] Share on relevant communities
- [ ] Update any external documentation

### 3. Monitor
- [ ] Watch for issues
- [ ] Respond to questions
- [ ] Plan next version based on feedback
