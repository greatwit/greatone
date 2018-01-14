#!/bin/bash


mm
echo -e 'build media done..........................\n'
mmm src/Media/Codec/base7
echo -e 'build codec done..........................\n'
mmm src/Media/Camera/cam7
echo -e 'build Camera done..........................\n'

plib=../../../../out/target/product/generic/obj/lib
cp $plib/libCodecBase.so src/bin/libCodecBase7.so
cp $plib/libCamera.so src/bin/libCamera7.so
cp $plib/libgreat_media.so src/bin
echo -e 'copy libs done...........................\n'



