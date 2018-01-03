#!/bin/bash


mm
echo -e 'build media done..........................\n'
mmm src/Media/Codec/base4
echo -e 'build codec done..........................\n'
mmm src/Media/Camera/cam4
echo -e 'build Camera done..........................\n'

plib=../../../../out/target/product/wing-k70/obj/lib
cp $plib/libCodecBase.so src/bin/libCodecBase4.so
cp $plib/libCamera.so src/bin/libCamera4.so
cp $plib/libgreat_media.so src/bin
echo -e 'copy libs done...........................\n'



