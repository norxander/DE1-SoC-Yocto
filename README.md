# DE1-SoC-Yocto
Yocto recipes to build apps/libs/modules/tools for the Altera DE1-SoC board
Currently it contains the following:
  - Linux kernel driver module to request physical contiguous DDR3 SDRAM memory
  - Linux library to communicate to the driver and a custom malloc to dynamically allocate/free memory using malloc/free.
