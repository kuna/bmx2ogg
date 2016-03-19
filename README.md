# Bmx2Ogg

[![travis](https://travis-ci.org/kuna/bmx2ogg.svg)](https://travis-ci.org/kuna/bmx2ogg)
[![Build status](https://ci.appveyor.com/api/projects/status/j23imi9b8q66qc0w?svg=true)](https://ci.appveyor.com/project/kuna/bmx2ogg)

### abstract
- convert *.bmx(bms, bml, bme, etc ...) to audio files(*.wav / ogg)
- have dependency with ```libiconv```, ```libogg```, ```libvorbis```
- original project by CHILD *(named after bmx2wav)*
- cross-platform (win32 / linux)

### how to use

#### command lines
- ```-oc```
  output file to current directory *(default)*
- ```-ob```
  output file to bms directory
- ```-o```
  output file to my custom path *(you need to enter path argument)*
- ```-wav```
  output audio as wav
- ```-ogg```
  output audio as ogg *(default)*
- ```-flac```
  output audio as flac
- ```-q```
  set encoding quality (-1 ~ 1, default: 0.95 - about VBR 400kbps)
- ```-rate```
  set audio file speed rate (higher means faster)
- ```-ow```
  overwrite output file *(default)*
- ```-autofn, -noautofn```
  automatically reset file name (ex: ```[artist] title.ogg```) *(default)*

### dependency
- bmsbelplus
- libiconv (glibc)
- libsndfile

#### easy use
- just drag & drop any ```*.bmx``` files to program and there'll be converted ogg file!
- if you want to encode file as flac by default, rename file name including *flac* (ex: ```bmx2wav-flag.exe```)

### etc
- ```Title/Artist/Genre``` metadata is automatically written if you encode as ogg/flac.
- Download Executable file from [here](http://kuna.wo.tc/1425).
