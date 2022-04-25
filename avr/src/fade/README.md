When trying to flash to the ATTiny45, there is a double definition on 
line 79 which needs to be commented out.  

Alternatively, to avoid needing the external `iocompat.h` file, comment
out its `include` statement in `fade.c` and repeatedly attempt to 
compile until successful.  
