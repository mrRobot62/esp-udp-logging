# Changelog

## Unreleased

- added a second PlatformIO environment for `esp32-s3-devkitc-1`
- moved shared PlatformIO settings into a global `[env]` block
- both board targets now use the same UDP logging, CSV logging and client log flags
- demo loop currently emits logs faster, which makes testing with `udp-viewer` easier
