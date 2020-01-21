#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_stdinc.h>
#include <ctype.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static const char *file_keyany = "./sources/keyany.wav";
static char name[64];

// 音频结构, 任意
static struct {
  SDL_AudioSpec spec;
  Uint8 *sound;    // 存储音频地址
  Uint32 soundlen; // 音频大小
  int soundpos;    // 播放到的位置
} wave_keyany;

// sdl 退出
static void quit(int rc) {
  SDL_Quit();
  exit(rc);
}

// 回调
// unused: 未使用
// stream: 音频流，
// len: stream的大小
// 将内容复制到stream中，sdl会播放stream中的内容
static void SDLCALL fillerup(void *unused, Uint8 *stream, int len) {
  Uint8 *waveptr;
  int waveleft;

  // 定位到播放到的位置
  waveptr = wave_keyany.sound + wave_keyany.soundpos;
  // 剩余未播放的音频长度
  waveleft = wave_keyany.soundlen - wave_keyany.soundpos;

  // 复制len大小的内容进入stream
  // 如果剩余的音频长度小于len
  // 那么从头读取，填满

  if (waveleft <= len) {
    // 如果剩余的音频长度小于len
    // 将剩余的内容复制到stream
    SDL_memcpy(stream, waveptr, waveleft);

    // 然后soundpos归0
    wave_keyany.soundpos = 0;
    waveptr = wave_keyany.sound + wave_keyany.soundpos;

    SDL_memcpy(stream, waveptr, len - waveleft);

    SDL_PauseAudio(1);
  } else {
    // 向后读取
    SDL_memcpy(stream, waveptr, len);
    wave_keyany.soundpos += len;
  }
}

// 初始化音频
void init_audio() {

  // 初始化sdl
  // SDL_INIT_AUDIO 初始化音频子系统
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL 初始化失败: %s\n",
                 SDL_GetError());
    return;
  }

  // 载入音频文件
  if (SDL_LoadWAV(file_keyany, &wave_keyany.spec, &wave_keyany.sound,
                  &wave_keyany.soundlen) == NULL) {
    quit(1);
  }

  //设置回调函数
  wave_keyany.spec.callback = fillerup;

  // 打开音频设备
  if (SDL_OpenAudio(&wave_keyany.spec, NULL) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "音频设备打开失败: %s\n",
                 SDL_GetError());
    SDL_FreeWAV(wave_keyany.sound);
    quit(2);
  }
}

void finish_audio() {
  SDL_CloseAudio();               //关掉音频进程以及音频设备
  SDL_FreeWAV(wave_keyany.sound); //释放数据由SDL_LoadWAV申请的
  SDL_Quit();
}

// 读取键盘输入
void detect_press(char *path) {
  int fd;
  char ret[2];

  struct input_event ie;
  // 读取文件
  if ((fd = open((char *)path, O_RDONLY)) > 0) {
    // 读取成功
    while (1) {
      // 按照input_event读取文件
      if (read(fd, &ie, sizeof(struct input_event)) ==
          sizeof(struct input_event)) {

        if (ie.type == EV_KEY) {
          if (ie.value == 1) {
            // 按下, 播放按键声音
            SDL_PauseAudio(0);
          }
        }
      }
    }
    close(fd);
  } else {
    return;
  }
}

static void str_to_lower(char *str, int len) {
  for (int i = 0; i < len; i++) {
    str[i] = tolower(str[i]);
  }
}

// 获取键盘文件
// 即/dev/input/event**, **表示数字
void get_event_device() {
  char buf[256] = {
      0,
  };

  int fd = 0;

  for (int i = 0; i < 64; i++) {
    sprintf(name, "/dev/input/event%d", i);
    // 打开input文件
    if ((fd = open(name, O_RDONLY, 0)) >= 0) {
      // 获取设备name的名称，存储到buf中
      ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
      str_to_lower(buf, strlen(buf));
      if (strstr(buf, "keyboard")) {
        printf("[%d]: %s\n", i, buf);
      }
      printf("");
    }
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  int i;

  printf("please input device number: \n");
  get_event_device();
  scanf("%d", &i);
  printf("your number is %d\n", i);

  sprintf(name, "/dev/input/event%d", i);

  if (fork() > 0) {
    // 父进程退出
    exit(0);
  }

  // 初始化音频设备等
  init_audio();
  // 监控键盘
  detect_press(name);
  finish_audio();
}
