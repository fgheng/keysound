# Keysound

![pic](./pic/piano.jpg)

一个linux下的按键音效程序

## Motive

当我使用vim编程的时，我找到了一个有趣的插件[skywind3000/vim-keysound](https://github.com/skywind3000/vim-keysound)，这个插件会在你进行输入的时候，发出类似机械键盘敲击的声音，我觉得非常有趣，不过，该插件只能在vim中使用，不能在其他软件中使用，也就是无法全局使用，而且该插件不支持混音，当你连续按下两个按键的时候，第二个按键的声效会终止第一个按键的声效，体验不是太好。

我之前写过一次[keysound](./old/README.md)，当时只写了全局按键音效，依赖SDL2播放音频，存在很多很多的问题，例如cpu占用高，无法检测键盘的热插拔，没有混音等，体验感极差。我一直想完善一下该项目，正好最近学习c++，可以通过完善该项目练习c++。

该项目虽然很简单，但是我从中学到了很多，如音频方面，我了解了wav格式如何解析，理解了采样率，通道数，比特率，混音等概念，会计算一段数据的播放时长；系统编程方面，学习了多线程，设备热插拔的监控，命令行参数的解析，信号等等；该项目甚至让我学会了弹钢琴，哈哈哈。



## Advatage

1. 全局按键音效，无论用户在哪个程序下敲击键盘，都可以发声。
2. 自定义，用户可以为每个按键自定义音效。
3. 混音，支持混音，同时按下多个按键，多个按键的音效会同时播放。
4. 热插拔，支持动态监控键盘的插入与拔出。
5. 多个音频播放后端，音频播放可以选择使用alsa（存在问题），pulse，sdl2进行音频的播放，默认使用pulse



## Build

Makefile文件是找的模板，写的不是太好，后续需要更改一下。

### depends

默认使用的是pulse，所以我们需要安装libpulse，如果使用SDL2，那么就需要安装SDL2

ubuntu:
```
# pulse
sudo apt install libpulse-dev
# sdl2
sudo apt install libsdl2-dev
```

fedora:
```
# 这个我还没有尝试
```

arch: arch在安装pulseaudio的时候默认安装libpulse了
```
# pulse
sudo pacman -S libpulse
# sdl2
sudo pacman -S sdl2
```

### make

```
git clone https://github.com/fgheng/keysound

cd keysound

make
```

默认使用的是pulse播放，执行make之后会在Makefile文件所在的目录下生成一个可执行文件keysound。

### change owner

编译完成之后我们还需要进行如下的操作才可以运行：

```
sudo chown root ./keysound

sudo chmod u+s ./keysound
```

因为键盘的检测读取的是`/dev/input/`下面的`event`文件，所以必须使用root运行，但是使用root运行的时候pulse和SDL2会出现音频无法播放的问题，所以必须使用上面的语句，执行完上面的语句之后就可以运行了。

这是我一直存在的一个疑问，我不清楚为什么会出现这个问题。



## Basic Usage

本程序支持单独的音频文件，支持json配置文件，支持目录

```
  -f, --file=WAV_FILE        要播放的音频
  -j, --json=JSON_FILE       json配置文件
  -d, --dir=DIR              目录
  -l, --log=LOG_FILE         日志输出文件
  -D, --daemon               后台运行
  -k, --kill                 结束进程
  -?, -h, --help             帮助
```

例如：

```
./keysound -f ./audio/typewriter-key.wav
```

所有的按键都会使用`typewriter-key.wav`这个音效。

```
./keysound -d ./audio/dir
```

程序会去`./audio/dir`目录下寻找合适的音频，比如`a.wav`表示按键`a`的音效，`lshift.wav`是左shift的音效，`;.wav`是按键`;`的音效等等。

```
./keysound -j ./audio/piano.json
```

该命令会让程序读取`./audio/piano.json`配置文件，下面是json文件的一个例子：

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

其中`”dir"`必须要有，他表示的是音频文件所在的目录，接下来的是各个按键对应的音频，如果存在`default`关键字，那么所有没有设置的按键以及音频读取失败的按键都将使用`default`所配置的音频。

```
./keysound -j ./audio/piano.json -D
```

让程序在后台运行，如果已经有程序在运行了，那么新进程会向旧进程发送中断信号结束旧进程。

```
./keysound -k
```

结束运行中的进程，包括前台和后台的进程。



## Principle

本程序的原理很简单，通过读取每个键盘的event文件来检测按键事件，当按键事件发生的时候将音频添加到混音器中进行混音，音频播放器从混音器中读取数据进行播放。

程序有这么几个线程，其一是检测键盘热插拔的线程，该线程通过netlink实现键盘热插拔的检测。当有键盘插入的时候，自动启动一个新的键盘监控线程，键盘监控线程检测是否有按键按下，如果有按键按下，那么就会从Audio中获取该按键对应的音频加入到混音器中；当有键盘拔出的时候，键盘热插拔检测程序会通知对应的键盘监控线程安全退出。

其二是音频播放线程，该线程可以选择使用alsa（有问题）pulse以及sdl2中的一种作为后端进行音频的播放，当音频播放器缓存中的数据播放完毕后，音频播放线程会去混音器中获取新的数据，直到用户给定停止播放的信号。

混音部分的实现是一段循环buffer。

混音器主要有两个功能，一个功能是存数据，一个功能是取数据。当检测到按键事件的时候，按键检测线程会将音频数据存入到混音器的buffer中，并设定当前数据的起始地址pos，当播放器从混音器取数据的时候会从pos处取走固定大小的数据，取走数据后，混音器应该将buffer中取走的部分置为0，同时pos要移动到还未读取的数据部分。如果此时又有按键按下，那么新的音频应该从pos处开始存储，同时要与buffer中已经存在的还未取走的数据进行合并，这就是混音。我使用的方案是[A+B-A*B>>XX](http://blog.sina.com.cn/s/blog_4d61a7570101arsr.html)，溢出之后使用最大值，我使用for循环实现，我认为这样实现效率比较低，这里可以继续进行优化。



## example

`audio`目录下有一个`piano.json`配置文件，该配置文件是一个按键与钢琴音效对应的配置文件，该配置只使用了三个八度的音，我们可以根据自己的情况配置更多的音，使用该配置后，我们的键盘就变成了一个简单的钢琴，执行下面的命令：

```
./keysound -j ./audio/piano.json
```

此时我们可以尝试自己弹奏钢琴曲了，我不会钢琴，下面的我都是按照简谱敲的，有可能敲错了，后面的是音乐的视频链接，首先要知道怎么唱才能听出弹的是什么歌，哈哈哈：

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

8. [更多](https://github.com/fgheng/keysound/issues/2)



## Want help

1. 只使用root但不使用`chown root`与`chmod u+s`两条命令。本程序必须要使用这两条命令才可以正常使用，当然如果使用alsa的话就可以不使用这两条命令，但是使用alsa播放音频，会出现音频设备独占的情况，其他音频软件无法播放声音。
2. 不使用root。普通用户可以直接检测键盘输入然后播放音频。



## TODO

- [ ] 重新编写README
- [ ] 重新编写Makefile，可以选择使用alsa，pulse还是sdl2，检测依赖是否缺失
- [ ] 增加日志输出
- [ ] 优化混音部分的计算：1. 使用更好的方式实现混音算法 2. 设计一个更好的buffer
- [ ] 混音部分增加float支持
- [ ] 混音部分增加大小端判断
- [ ] 不需要sudo chown root 以及 sudo chmod u+s，可以直接root启动
- [ ] 增加音量调节
- [ ] 增加一个界面，可以在任务栏显示
- [ ] 修复alsa无法播放的bug
- [ ] 添加到各种系统的软件仓库中？
- [ ] 增加按键抬起，重复等音效的自定义



## Thanks

[skywind3000/vim-keysound](https://github.com/skywind3000/vim-keysound)

[mattogodoy/hacker-sounds](https://github.com/mattogodoy/hacker-sounds)

[timmyreilly/TypewriterNoises-VSCode](https://github.com/timmyreilly/TypewriterNoises-VSCode)
