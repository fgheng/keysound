# Keysound

![pic](./pic/piano.jpg)

<a href="./README.md">中文版</a>

A key sound program on GNU/Linux

Thanks [@MiraculousMoon](https://github.com/MiraculousMoon) for this English version.

## Motivation

I find an interesting plugin called [skywind3000/vim-keysound](https://github.com/skywind3000/vim-keysound) when I program in VIM. It can give you immediate feedback using mechanical keyboard sound when you are typing. But it can't work globally and doesn't support mixing.

So I write it to make up its defect and practice cpp.

Although the project is very simple, I have learned a lot from it. For example, in terms of audio, I understood how to parse the wav format, understood the concepts of sampling rate, number of channels, bit rate, mixing, etc., and calculate the playback time of a piece of data; system In terms of programming, I learned about multi-threading, device hot-plug monitoring, command line parameter analysis, signals, etc.; the project even taught me to play the piano, lol.

[Here are some video presentations](https://zhuanlan.zhihu.com/p/336490503)


## Advatage

1. Global key sound effects.
2. Customization. Users can customize the sound effect for each button.
3. Mixing. Support for mixing, press multiple buttons at the same time, the sound effects of multiple buttons will be played at the same time.
4. Hot plugging. Supports dynamic monitoring of keyboard insertion and removal.
5. Multiple audio playback backends. Audio playback can choose to use alsa (problems), pulse, sdl2 for audio playback.(Pulse is used by default)

### Dependency

Pulse is the default backend so `libpulse` is needed.
If you use SDL2 so the `sdl2` is needed.

ubuntu:

```
# Pulse
sudo apt install libpulse-dev
# SDL2
sudo apt install libsdl2-dev
```

arch:
```
# Pulse
sudo pacman -S libpulse
# SDL2
sudo pacman -S sdl2
```

### Make

```
git clone https://github.com/fgheng/keysound

cd keysound

# Just make，default backend is Pulse
make
# Pulse
make CFLAG=pulse
# SDL2
make CFLAG=sdl
# Alsa
make CFLAG=alsa
```

You will get a executable file called `keysound` after make.

### Permissions

Add your username to `input` group

```
sudo usermod -a -G input username
```

relogin or user this command

```
newgrp input
```


## Basic Usage

```
  -f, --file=WAV_FILE        Audio to play
  -j, --json=JSON_FILE       Json config file
  -d, --dir=DIR              Directory
  -l, --log=LOG_FILE         log(undone)
  -D, --daemon               Run as a daemon
  -k, --kill                 Kill running process
  -?, -h, --help             Help
```

Examples:

```
./keysound -f ./audio/typewriter-key.wav
```

All key will use `typewriter-key.wav`

```
./keysound -d ./audio/dir
```

Program will search suitable audioes in `./audio/dir`. For example, `a.wav` is the sound of `KEY-a`. `lshift.wav` is the sound of `KEY-left_shift`. `;.wav` is the sound of `KEY-;`.

```
./keysound -j ./audio/piano.json
```

Program will read `./audio/piano.json` to play sound.
Following is an example:

```json
{
    "dir": "./audio/piano",

    "`": "28-C-小字组.wav",
    "1": "30-D-小字组.wav",
    "2": "32-E-小字组.wav",
    "3": "33-F-小字组.wav",
    "4": "35-G-小字组.wav",
    "5": "37-A-小字组.wav",
    "6": "39-B-小字组.wav",

    "7": "40-C-小字1组.wav",
    "8": "42-D -小字1组.wav",
    "9": "44-E-小字1组.wav",
    "0": "45-F-小字1组.wav",
    "-": "47-G-小字1组.wav",
    "=": "49-A-小字1组.wav",
    "backspace": "51-B-小字1组.wav",

    "f7": "52-C-小字2组.wav",
    "f8": "54-D-小字2组.wav",
    "f9": "56-E-小字2组.wav",
    "f10": "57-F-小字2组.wav",
    "f11": "59-G-小字2组.wav",
    "f12": "61-A-小字2组.wav",
    "insert": "63-B-小字2组.wav",

    "h": "52-C-小字2组.wav",
    "n": "40-C-小字1组.wav",
    "j": "42-D -小字1组.wav",
    "k": "44-E-小字1组.wav",
    "l": "45-F-小字1组.wav",
    ";": "47-G-小字1组.wav",
    "'": "49-A-小字1组.wav",
    ".": "51-B-小字1组.wav",
    "slash": "51-B-小字1组.wav",

    "a": "28-C-小字组.wav",
    "s": "30-D-小字组.wav",
    "d": "32-E-小字组.wav",
    "f": "33-F-小字组.wav",
    "x": "35-G-小字组.wav",
    "c": "37-A-小字组.wav",
    "v": "39-B-小字组.wav",

    "y": "52-C-小字2组.wav",
    "u": "54-D-小字2组.wav",
    "i": "56-E-小字2组.wav",
    "o": "57-F-小字2组.wav",
    "p": "59-G-小字2组.wav",
    "[": "61-A-小字2组.wav",
    "]": "63-B-小字2组.wav"
}
```

`"dir"` is required. It means directory of audio files.

Below is audio file's name corresponding each key.

If you use `default` keyword, then all of keys you haven't set will use the default audio.

```
./keysound -j ./audio/piano.json -D
```

Make process running as a daemon.

```
./keysound -k
```

Kill all process running in the foreground and background.

## Principle

The principle of this program is very simple. It detects key events by reading the event file of each keyboard. When a key event occurs, the audio is added to the mixer for mixing, and the audio player reads data from the mixer to play.

There are several threads in the program, one of which is to detect keyboard hot-plugging. This thread uses netlink to detect keyboard hot-plugging. When a keyboard is inserted, a new keyboard monitoring thread is automatically started. The keyboard monitoring thread detects whether a key is pressed. If a key is pressed, the audio corresponding to the key is obtained from Audio and added to the mixer ; When the keyboard is unplugged, the keyboard hot-plug detection program will notify the corresponding keyboard monitoring thread to exit safely.

The second is the audio playback thread. This thread can choose to use one of alsa (problem) pulse and sdl2 as the backend for audio playback. When the data in the audio player buffer is played, the audio playback thread will go to mix Acquire new data from the sounder until the user gives a signal to stop playing.

The mixing part is realized through a circular buffer.

The mixer has two main functions, one is to store data, and the other is to fetch data. When a key press event is detected, the key detection thread will store the audio data into the buffer of the mixer and set the starting address pos of the current data. When the player fetches data from the mixer, it will start from pos. Remove the fixed-size data, after removing the data, the mixer should set the removed part of the buffer to 0, and at the same time pos must move to the unread data part. If a button is pressed again at this time, the new audio should be stored from pos and merged with the data already in the buffer that has not been taken away. This is the mixing. The scheme I use is [A+BA*B>>XX](http://blog.sina.com.cn/s/blog_4d61a7570101arsr.html), use the maximum value after overflow. I use a for loop to achieve it. This method's efficiency maybe relatively low, so optimization can be continued here.


## Example

`audio` directory has a `piano.json` config file binding key to the piano sound effect. This file just uses three octaves. You can set more tone level. You can turn your keyboard to a simple piano using this config.

```
./keysound -j ./audio/piano.json
```

Here are some example key sequences.

1. 青花瓷 [youtube](https://www.youtube.com/watch?v=bz4Wkyo-q4o)，[bilibili](https://www.bilibili.com/video/BV1nW411J7Dt?from=search&seid=12361052887766167699)

   ```
   ;;kjkxjk;kj  ;;kjkcjk;jn  njk;';k;kkjj  njnjnnk;k   ;;kjkcjk;kj   ;;kjkxjk;jn  njk;';k;kkjj  xkjnn
   ```

2. 菊花台 [youtube](https://www.youtube.com/watch?v=HPBtJVsLEag)，[bilibili](https://www.bilibili.com/video/BV1N7411p7Wh?from=search&seid=8246405902734662856)

   ```
   kkjkk;kjk  nnjk;kjjnj  k;k’;’;;k;  ;kjk;kjjjnj  kkjkk;kjknnjk;kjjnjk;k’;’;;k;kjk;kjjn
   njkk;’’iuyy’;   ‘;kjncnjjnjnjkk;’’iuyyuy  ;;k/ynjkjn
   ```

3. 好久不见[youtube](https://www.youtube.com/watch?v=L8BW7LCnixE)，[bilibili](https://www.bilibili.com/video/BV1UW41187Xo?from=search&seid=18165129042863151013)

   ```
   xlkdjvn  n';kjnj  clk;nnjn  ccvlk;kj  clkdjjn c';kjnj  clkl;jjn  cvnk'jn  k;k;kjk;kjn   '';k;kj  njnjn' ';jk  kjjnncj  k;k;kjk;kjn  '';;knj  njnjnc  ';jk  kjnnc  njk;kjn  kcjn
   ```

4. 斯卡布罗集市[youtube](https://www.youtube.com/watch?v=JWA1ZFvlpvM)，[bilibili](https://www.bilibili.com/video/BV1yx411e7ud?from=search&seid=8571430432893707928)

   ```
   调1:
   ''iii/y/'  ip[pioui  [[[piiuy/;   'iuy/';'
   调2:
   cckkkvnvc  k;';kljk   ''';kkjnx   ckjnvcxc    ccckkkvnvc    k;';kljk   ''';kkjnvc   kjnvcxc
   ```

5. 像我这样的人[youtube](https://www.youtube.com/watch?v=MJeLEzyX5Og) [bilibili](https://www.bilibili.com/video/BV1zx411n7wG?from=search&seid=14323209493871360281)

   ```
   jk;k;kjk  jk;k;kjk   xcncncxcn  jkjkcnj  jk;k;kjk  jk;kj;k  xcncncxcn  jjnnjk  kyy//’k; jnnnncnk kyy//’/’k; jnnnncn  jk;k;kjk  jk;k;jk  xcncncxcn  jjnnjk  ‘/y/y’/i  iuiuiiiuy   y/y/njk kyy//’/’k;  jnnnncn   ‘/y/y’/  y’y’yyu/’;  ‘;’;njk  kyy//’/’k;  jnnnncn  kyy//’/’k;jnnnnjn
   ```

6. 滚滚红尘[youtube](https://www.youtube.com/watch?v=qVm3GdOd2P8)，[bilibili](https://www.bilibili.com/video/BV17x411C7UM?from=search&seid=670334928707153697)

   ```
    k’;lkjk l;lknjkc  jjjjnjk’;lkllljklk   ‘’’/k/’;kjk’; jjjkckjjjcncv  k’;lkjk  l;lknjkc  jjjjnjk’;lkljknjxc  jjjjnjk’;lkljknjxc
   ```

7. 丁香花 [youtube]() [bilibili]()

   ```
   Dkjkjjkk  njkcncncc  cnncnjj  lllnnjk  dkjkjjkk  dkjkjncc  cnncnjj  klllnjkk  jnjnjkj  nkkjjnncc  dnnnnncn  vvvvvcv 高潮  k’k;’’’’y’;’;kjk  ckjk’’nk’’k’//  k’k;’’’ ‘y’;’;kjk   ckjkkkccvvvjnvc
   Ckjkjjkk  njkjjnncc  cnncnjj  vvvjnvc
   ```



## TODO

- [ ] Reorganize the logic of the process exiting this part.
- [X] Rewrite Makefile, you can choose to use alsa, pulse or sdl2, and check whether the dependency is missing.
- [ ] Support log output.
- [ ] Optimize the calculation of the mixing part: 1. Use a better way to implement the mixing algorithm 2. Design a better buffer.
- [ ] Float support added in the mixing section.
- [ ] The mixing section adds the size end judgment.
- [ ] Added option not to use audio mixing.
- [ ] Support volume adjustment function.
- [ ] Add an interface that can be displayed in the taskbar
- [ ] Fix the bug that alsa cannot be played.
- [ ] Add to the software warehouse of various distros like AUR.
- [ ] Add the customization of sound effects such as button up and repeat.
- [ ] Support mouse operation sound effects.
- [ ] Reduce Bluetooth headsets' latency.
- [ ] support soundfont and midi



## Thanks

Thanks [@MiraculousMoon](https://github.com/MiraculousMoon) for this English version.

some audio resources come from

[skywind3000/vim-keysound](https://github.com/skywind3000/vim-keysound)

[mattogodoy/hacker-sounds](https://github.com/mattogodoy/hacker-sounds)

[timmyreilly/TypewriterNoises-VSCode](https://github.com/timmyreilly/TypewriterNoises-VSCode)
