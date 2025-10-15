#ifndef SH_WEATHER_H
#define SH_WEATHER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char location[64];
  char text[32];
  char code[16];
  char temperature[16];
} sh_weather_now_t;

// 同步获取心知天气 now 接口数据（HTTP/80），成功返回 0，失败返回 -1。
// location 例如 "guangzhou" 或 城市中文名，key 为 API Key。
int sh_weather_fetch_now(const char *location, const char *key, sh_weather_now_t *out, char *err, size_t errlen);

#ifdef __cplusplus
}
#endif

#endif // SH_WEATHER_H
