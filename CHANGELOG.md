# Changelog

All features and notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

WINDOWS support was deprecated in version [0.0.3] due to amount of testing it would require and uncompatible libraries. 
Legacy windows support code is still in the app and implementing it is possible.

## [0.1.5] - 2023-03-13

### Added 
- (docs): Problematique section

## [0.1.4] - 2023-03-13

### Added 
- (docs): Build & Run section

## [0.1.3] - 2023-03-10

### Fixed
- (udp) sending confirm only once
- (udp) processing of reply message
- (udp) bye message would not be send when received SIGINT

## [0.1.2] - 2023-03-09

### Added
- (udp) join, auth, receive funcionality

## [0.1.1] - 2023-03-08

### Added
- (udp) added messageID
- (udp) laid work for sending messages
- minor code refactor

## [0.1.0] - 2023-03-08

### Fixed
- byte size of message type

## [0.0.9] - 2023-03-05

### Added
- (tcp) main tcp funcionality should work, tested on ref server
- translating hostname into ip address

## [0.0.8] - 2023-03-04

### Added
- (tcp) reacting to BYE message from server
- (tcp) handling interrupt signal from user while waiting for auth verification
- (tcp) added (5sec) timeout to waiting for auth verification

## [0.0.7] - 2023-03-04

### Added
- (tcp) processing messages sent by server
- (tcp) waiting for auth verification

## [0.0.6] - 2023-03-03

### Added
- preparing functions for communication
- added exceptions

## [0.0.5] - 2023-03-03

### Added
- handling user input

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