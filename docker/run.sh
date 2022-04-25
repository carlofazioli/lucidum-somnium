DEVICE=$(lsusb | grep USBtiny)
DEVICE_ARR=($DEVICE)
BUS=${DEVICE_ARR[1]}
DEV=${DEVICE_ARR[3]:0:-1}

docker build . -t avr
docker run -it \
    --device=/dev/bus/usb/$BUS/$DEV \
    -v $(pwd)/src:/src \
    avr bash
