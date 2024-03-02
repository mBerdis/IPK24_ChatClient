# Changelog

All features and notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),

## [0.0.4] - 2023-03-02

### Added
- handling interrupt signal
- using pool to listen for keyboard and socket input

## [0.0.3] - 2023-02-29

### Added
- parsing arguments into ConnectionSettings
- TCPConnection class

### Deprecated 
- WINDOWS support. From this point the app wont support Windows. (Because of testing and implementation simplicity)

## [0.0.2] - 2023-02-25

### Added
- CHANGELOG.md file

## [0.0.1] - 2023-02-25

### Added
- added Makefile
- made Abstract and UDP connection classes (not final)
- sending UDP msg for testing