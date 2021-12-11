# H4 over network

`h4net` transfers H4 packets between Bluetooth controller (its serial port) and remote host over TCP. The main feature of `h4net` is incoming H4 packet defragmentation and separation. If Bluetooth controller doesn't have issues with H4 packet parsing then `socat` should be used instead of `h4net`.
