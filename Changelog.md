# Changelog
## [1.3.0] - 3-2-2026
### Added
- `ThingDevice`s can now be provided with a default enable state in the constructor.
- `ThingDevice`'s internal enable property is now only protected so derived classes can have access to it as well.

## [1.2.0] - 2-2-2026
### Added
- `ThingGateway` can now through its own device representation in Thingsboard enable or disable connected devices using shared bool attributes.

### Changed
- `ThingDevice` now uses a `BooleanProperty` to store its desired enable/disable state.

### Fixed
- local variable `devicename` in `ThingGateway::process_attribute` renamed to `name` to prevent potential conflict with `ThingGateway::devicename`.

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