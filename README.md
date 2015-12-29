# Bmx2Ogg

### abstract
- convert *.bmx(bms, bml, bme, etc ...) to audio files(*.wav / ogg)
- have dependency with ```libiconv```, ```libogg```, ```libvorbis```
- original code by CHILD (specially with ```bmsbel / bmx2wav_wav / bmx2wav_wav_maker``` modules)

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
- ```-ow```
  overwrite output file *(default)*
- ```-autofn, -noautofn```
  automatically reset file name (ex: ```[artist] title.ogg```) *(default)*

#### easy use
- just drag & drop any ```*.bmx``` files to program and there'll be converted ogg file!

### known problems / TODO / etc
- current ogg quality is 0.95 (-1 ~ 1). maybe CBR 375kbps?