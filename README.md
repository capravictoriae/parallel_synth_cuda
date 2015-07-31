# Simple Synthesizer that uses CUDA to process.

Synth emulation of simple analog monofonic synthesizer using standard modern C++ and CUDA to increase it's performance.

Built using [WDL-OL](https://github.com/olilarkin/wdl-ol)

**TFM - Plácido Fernández - Universidad Carlos III de Madrid**

WDL-OL uses this other 3rd party libraries
- [JNetLib](http://www.nullsoft.com/free/jnetlib)
- [LibPNG](http://www.libpng.org/pub/png)
- [GifLib](http://sourceforge.net/projects/libungif)
- [JPEGLib](http://www.ijg.org/)
- [zlib](http://www.zlib.net/)

In order to compile the standalone APP and the VST plugin the following SDKs are needed:
- [VST3 SDK](http://www.steinberg.net/en/company/developer.html) - Register needed.
- [RtAudio](http://www.music.mcgill.ca/~gary/rtaudio/)
- ASIO SDK - Included with *RtAudio*
- [CUDA SDK](https://developer.nvidia.com/cuda-downloads)


