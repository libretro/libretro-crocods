# libretro-crocods

[![Build Status](https://travis-ci.org/libretro/libretro-crocods.svg?branch=master)](https://travis-ci.org/travis-ci/travis-api)

Look for .kcr file format to use CrocoDS at its best: https://github.com/redbug26/crocods-core/wiki/kcr

## Install on Recalbox

- SSH on your recalbox
- Type the following command:
``` 
curl https://raw.githubusercontent.com/libretro/libretro-crocods/master/install_recalbox.sh | sh
``` 
- Restart your recalbox
- Choose crocods core in the Amstrad settings in Emulation Station

(Repeat each time you update your recalbox)

## Build instructions on other systems

``` 
git clone https://github.com/libretro/libretro-crocods.git
cd libretro-crocods/
git submodule update --init --recursive
``` 

Compile for osX
``` 
make platform="osx" -j2 CC="cc" 
```

Compile for linux
``` 
make platform="linux" -j2 CC="cc" 
```

Compile for win (using mingw)
``` 
make platform="win" -j2 CC="cc"
```

Compile for raspberry (from Ubuntu)
```
sudo apt-get install gcc-arm-linux-gnueabihf make
make platform="unix" -j2 CC="arm-linux-gnueabihf-gcc"
```

