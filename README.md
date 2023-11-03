# Sauremilch
Sauremilch is an injectable library that can edit rw and r-xp Memory, either it uses hardcoded pointers
or uses hardcoded offsets to find stuff, which in theory means it won't work all the time for everyone.

I decided to carry on, it took more and more time as the code grew, while I could still work on it and improve it
I thought no and cleaned it up then put it on GitHub for learning purposes.

# Build
`g++ -O2 -fPIC -lncurses -o sauremilch.so ./sauremilch.cpp -shared`

# Injection
I personally only used LD_PRELOAD and GDB:

`LD_PRELOAD=~/path/to/library/sauremilch.so ./sauerbraten_unix`

or

`sudo gdb -n -q -batch -ex "attach PID" -ex "set \$dlopen = (void*(*)(char*, int)) dlopen" -ex "call \$dlopen(\"/home/user/path/to/libray/sauremilch.so\", 1)" -ex "detach" -ex "quit"`

# Keybinds
Q : Clip up

E : Clip down

W : Clip forward

S : Clip backward

A : Clip left

D : Clip right



P : Add Ammo

R : Rapid Fire

G : "God" Mode (desyncs you more than actual godmode, try it while playing CTF)

F : "Disable" Gravity (actual gravity still applies, its just slightly noticeable)



X : quit Sauremilch
