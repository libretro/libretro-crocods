#!/bin/bash

# script to install crocods on recalbox
# exec it with the command
#
# curl https://raw.githubusercontent.com/redbug26/libretro-crocods/master/install_recalbox.sh | sh
#
#

#CONFIG=es_systems.cfg
CONFIG=/recalbox/share_init/system/.emulationstation/es_systems.cfg

mount -o remount, rw /

# Copy the crocods core in the libretro folder
curl https://github.com/redbug26/libretro-crocods/releases/download/version/crocods_libretro.so -o /usr/lib/libretro/crocods_libretro.so

# Add .kcr file extension in emulation station
xml ed -inplace -u "/systemList/system[name='amstradcpc']/extension" -v ".dsk .DSK .zip .ZIP .kcr .KCR" $CONFIG

# Add crocods core in emulation station if not exist yet
count=`xml sel -t -v "count(/systemList/system[name='amstradcpc']/emulators/emulator/cores[core='crocods'])" $CONFIG`
if [ "$count" == "0" ]; then
        xml ed -inplace -s "/systemList/system[name='amstradcpc']/emulators/emulator/cores" -t elem -n core -v "crocods" -i core -t attr -n class -v com.foo $CONFIG > $CONFIG.new
fi


# Verify the config
#xmlstarlet sel -t -c "/systemList/system[name='amstradcpc']" $CONFIG
