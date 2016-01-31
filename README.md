# Bmx2Ogg

### abstract
- convert *.bmx(bms, bml, bme, etc ...) to audio files(*.wav / ogg)
- have dependency with ```libiconv```, ```libogg```, ```libvorbis```
- original code by CHILD (specially with ```bmsbel / bmx2wav_wav / bmx2wav_wav_maker``` modules)
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
- ```-ow```
  overwrite output file *(default)*
- ```-autofn, -noautofn```
  automatically reset file name (ex: ```[artist] title.ogg```) *(default)*

#### easy use
- just drag & drop any ```*.bmx``` files to program and there'll be converted ogg file!

### known problems / TODO / etc
- current ogg quality is 0.95 (-1 ~ 1). maybe CBR 375kbps?
- ```Title/Artist``` metadata is automatically written if you encode as ogg.
- Raw audio data decoding part is still Sooooo nasty & only supports ```*.ogg/*.wav``` currently. Need to work more about this.
- Download Executable file from [here](http://kuna.wo.tc/1425).
