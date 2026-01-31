Build286
Ken Silverman's Build Engine for 16-bit DOS computers
https://github.com/FrenkelS/Build286

Ken Silverman's Build Engine was originally designed for 32-bit DOS computers.
It's written in C code with assembly code.

Build286 is a port for PCs with a 16-bit processor like a 286, and with VGA or MCGA graphics.
It uses 64 kB of EMS memory, if available, as an upper memory block to store data.

Controls:
|Action                           |Keys            |
|---------------------------------|----------------|
|Walk                             |Arrow keys      |
|Run                              |Left Shift      |
|Walk slowly                      |F1, F2, F9, F10 |
|Strafe                           |Right Ctrl      |
|Strafe left and right            |<, >            |
|Strafe left and right slowly     |F3, F4          |
|Jump                             |A               |
|Crouch                           |Z               |
|Look up and down                 |Left Ctrl + A, Z|
|Change brightness                |F11             |
|Change aspect ratio              |9, 0, +, -      |
|Render Build Engine sprite       |1, 2, 3, 4      |
|Rotate Build Engine sprite       |5, 6            |
|Quit to DOS                      |Esc             |

Command line arguments:
|Command line argument|Effect               |
|---------------------|---------------------|
|<file>               |Load custom map      |
|-noems               |Disable EMS          |
