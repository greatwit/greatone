#!/bin/bash


mm
echo -e 'build media done..........................\n'
mmm src/Media/Codec/base6
echo -e 'build codec done..........................\n'
mmm src/Media/Camera/cam6
echo -e 'build Camera done..........................\n'

plib=../../../../out/target/product/generic/obj/lib
cp $plib/libCodecBase.so src/bin/libCodecBase6.so
cp $plib/libCamera.so src/bin/libCamera6.so
cp $plib/libgreat_media.so src/bin
cp $plib/libgreat_sender.so src/bin
echo -e 'copy libs done...........................\n'



