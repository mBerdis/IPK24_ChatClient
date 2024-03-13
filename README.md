# IPK24_ChatClient
Author: Maroš Berdis (xberdi01) \
License: GNU GENERAL PUBLIC LICENSE Version 3 \
[Assignment specification](https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201)

Chat client application written in ***C++*** that uses ***IPK24-CHAT*** protocol to communicate with the server.

## Build & Run
Project uses ***Makefile*** to handle build. Available commands:
|Command             |Description           |
| ---                | -----------          |
|`make`              | creates ***ipk24chat-client*** executable in project directory |
|`make clean`        | cleans build-generated files |
|`make run`          | builds and runs the app |
|`make runValgrind`  | builds and runs the app using valgrind |
|`make runUdpServCon`| builds and runs the app, will use UDP and reference server in options |
|`make runTcpServCon`| builds and runs the app, will use TCP and reference server in options |

The program ***ipk24chat-client*** accepts following CLI arguments: \
(Table is inspired by the specification in [Assignment\CLI Arguments](https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201#user-content-cli-arguments))
|Argument | Default value | Accepted values | Description |
| ---     | -----------   | ---             | ----------- |
|`-t`     | User provided | `tcp` / `udp`   | Transport protocol used for connection|
|`-s`     | User provided | IP address or hostname	   | Chat server IP/hostname |
|`-p`     | 4567 | `uint16`	   | Server port |
|`-d`     | 250 | `uint16`	   | UDP confirmation timeout |
|`-r`     | 3 | `uint8`	   | max number of UDP retransmissions  |
|`-h`     | | | Prints program help and exits  |

The program reads user-typed commands from the standard input (`stdin`). Any other input is treated as a message and sent to the server. \
(Table is inspired by the specification in [Assignment\Client behaviour, input and commands](https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201#user-content-client-behaviour-input-and-commands))

|Command | Parameters | Description |
| ---     | --------  | ----------- |
|`/auth`  | `{username}` `{secret}` `{displayName}` | Tries to authorize user with the server |
|`/join`  | `{channelID}` | Tries to join channel |
|`/rename`  | `{displayName}` | Renames authorized user, will apply to newly sent messages |
|`/help`  | | Prints help about these commands  |

## Theory

## Implementation
### 1. Abstraction
### 2. Terminating connection

## Testing

## Sources
1. [Assignment](https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201)
2. [BUT FIT - IKP24 presentation on network programming](https://moodle.vut.cz/pluginfile.php/823898/mod_folder/content/0/IPK2023-24L-04-PROGRAMOVANI.pdf)
3. [Non-blocking operations using poll()](https://pubs.opengroup.org/onlinepubs/009696799/functions/poll.html)