# Changelog
## [1.1.1] - 2-2-2026
### Fixed
- if/else compilation error when `THING_DEBUG` is not defined.

## [1.1.0] - 29-1-2026
### Added
- Added support for shared attributes, which are updated on Thingsboard and then downloaded to the device.

### Changed
- Connection status is now derived from `PubSubClient::connected()` instead of the `connected` variable.
- function `ThingGateway::is_connected()` is renamed to `ThingGateway::connected()`.
- Support for Properties 2.4, of which the function `BaseProperty::save_to_json` now accepts a `JsonObject` instead of a `JsonDocument`.
- renamed `add_attribute_store` to `add_client_attributes` and `add_telemetry_store` to `add_telemetry`. 

## [1.0.0] - 27-1-2026
### Added
- `ThingDevice`, a class that represents a device in Thingsboard. This class can upload attributes and telemetry to thingsboard.
- `ThingGateway`, a class that facilitates the communication with a Thingsboard server for a number of `ThingDevices`.

## [0.0.1] - 26-1-2026
### Added
- gitignore
- changelog
- Readme