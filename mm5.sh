#!/bin/bash


mm
echo -e 'build media done..........................\n'
mmm src/Media/Codec/base5
echo -e 'build codec done..........................\n'
mmm src/Media/Camera/cam5
echo -e 'build Camera done..........................\n'

plib=../../../../out/target/product/generic/obj/lib
cp $plib/libCodecBase.so src/bin/libCodecBase5.so
cp $plib/libCamera.so src/bin/libCamera5.so
cp $plib/libgreat_media.so src/bin
echo -e 'copy libs done...........................\n'



