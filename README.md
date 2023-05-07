#### Build
```sh
make && make iso
```

#### Run
```sh
qemu-system-x86_64 -serial file:logfile.json -cdrom dist/NOS.iso
```