make TARGET=nrf BOARD=nrf5340/dk/network mobile-node.upload
make TARGET=nrf BOARD=nrf5340/dk/network node.upload

make TARGET=nrf BOARD=nrf5340/dk/network root.upload

# cp build/nrf/nrf5340/dk/network/node.nrf build/nrf/nrf5340/dk/network/node.hex
# objcopy  -O binary build/nrf/nrf5340/dk/network/node.nrf build/nrf/nrf5340/dk/network/node.i16hex
# srec_cat build/nrf/nrf5340/dk/network/node.i16hex build/nrf/nrf5340/dk/network/node.elf

# objcopy  build/nrf/nrf5340/dk/network/root.nrf build/nrf/nrf5340/dk/network/root.i16hex
# srec_cat build/nrf/nrf5340/dk/network/root.i16hex build/nrf/nrf5340/dk/network/root.elf
