## EXT2 File System
Implements a bash for an EXT2 filesystem on a virtual disk.

Run: `sh mk <image>`. 

If not given an `<image>`, will use `disk` by default. 

#### Known Bugs:
* Cannot `cat` more than once after performing some sort of file editing task such as cp. Offset issue maybe..?
* `rmdir` gets weird when removing deeper filesystem trees and non-empty ones.
