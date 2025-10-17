#include "obj/data/state_store.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define DATA_DIR "data"
#define LIGHTING_STATE_PATH DATA_DIR "/lighting_state.txt"
#define SECURITY_STATE_PATH DATA_DIR "/security_state.txt"

static void ensure_data_dir(void) {
    struct stat st;
    if (stat(DATA_DIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            // 已存在同名非目录，尝试创建目录（大概率失败），仅打印提示
            fprintf(stderr, "[state_store] '%s' exists and is not a directory.\n", DATA_DIR);
        }
        return;
    }
    if (mkdir(DATA_DIR, 0755) != 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "[state_store] mkdir '%s' failed: %s\n", DATA_DIR, strerror(errno));
        }
    } else {
        printf("[state_store] created directory: %s\n", DATA_DIR);
    }
}

static int read_ints_from_line(const char *line, int *a, int *b, int *c) {
    if (!line || !a) return 0;
    int n = 0;
    int x = 0, y = 0, z = 0;
    n = sscanf(line, "%d %d %d", &x, &y, &z);
    if (n >= 1) *a = x; else return 0;
    if (b) { if (n >= 2) *b = y; }
    if (c) { if (n >= 3) *c = z; }
    return n;
}

void ss_lighting_load(LightPersist *arr, int count) {
    if (!arr || count <= 0) return;
    ensure_data_dir();
    FILE *fp = fopen(LIGHTING_STATE_PATH, "r");
    if (!fp) {
        // 文件不存在：用当前传入的默认值创建文件
        printf("[state_store] lighting file not found, creating with defaults -> %s\n", LIGHTING_STATE_PATH);
        ss_lighting_save(arr, count);
        return;
    }
    printf("[state_store] loading lighting from %s\n", LIGHTING_STATE_PATH);
    char buf[256];
    int i = 0;
    while (i < count && fgets(buf, sizeof(buf), fp)) {
        int on = 0, bri = 0, ct = 0;
        int n = read_ints_from_line(buf, &on, &bri, &ct);
        if (n >= 1) arr[i].is_on = on;
        if (n >= 2) arr[i].brightness = bri;
        if (n >= 3) arr[i].color_temp = ct;
        i++;
    }
    fclose(fp);
}

void ss_lighting_save(const LightPersist *arr, int count) {
    if (!arr || count <= 0) return;
    ensure_data_dir();
    FILE *fp = fopen(LIGHTING_STATE_PATH, "w");
    if (!fp) {
        fprintf(stderr, "[state_store] failed to open %s for write: %s\n", LIGHTING_STATE_PATH, strerror(errno));
        return;
    }
    printf("[state_store] saving lighting to %s\n", LIGHTING_STATE_PATH);
    for (int i = 0; i < count; ++i) {
        fprintf(fp, "%d %d %d\n", arr[i].is_on, arr[i].brightness, arr[i].color_temp);
    }
    fclose(fp);
}

int ss_security_load(void) {
    ensure_data_dir();
    FILE *fp = fopen(SECURITY_STATE_PATH, "r");
    if (!fp) {
        // 文件不存在：创建并写入默认撤防状态
        printf("[state_store] security file not found, creating default -> %s\n", SECURITY_STATE_PATH);
        ss_security_save(0);
        return 0;
    }
    printf("[state_store] loading security from %s\n", SECURITY_STATE_PATH);
    int state = 0;
    if (fscanf(fp, "%d", &state) != 1) state = 0;
    fclose(fp);
    return state;
}

void ss_security_save(int state) {
    ensure_data_dir();
    FILE *fp = fopen(SECURITY_STATE_PATH, "w");
    if (!fp) {
        fprintf(stderr, "[state_store] failed to open %s for write: %s\n", SECURITY_STATE_PATH, strerror(errno));
        return;
    }
    printf("[state_store] saving security to %s\n", SECURITY_STATE_PATH);
    fprintf(fp, "%d\n", state);
    fclose(fp);
}
