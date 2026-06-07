#include "system/includes.h"
#include "app_config.h"
#include "audio_spectrum.h"

// ============ LED SPI 接口声明 ============
void led_spi_init(void);
void led_spi_rgb_to_24byte(u8 r, u8 g, u8 b, u8 *buf, int idx);
void led_spi_send_rgbbuf_isr(u8 *rgb_buf, u16 led_num);

#define LED_NUM         4
static u8 led_buf[24 * LED_NUM] __attribute__((aligned(4)));

// ============ 频谱数据 ============
extern spectrum_fft_hdl *spec_hdl;
static short db_data_old[32];
static short led_on_table[32];

static void read_spectrum(void)
{
    if (!spec_hdl) {
        return;
    }
    u8 db_num = audio_spectrum_fft_get_num(spec_hdl);
    short *db_data = audio_spectrum_fft_get_val(spec_hdl);
    if (!db_data) {
        return;
    }
    for (int i = 0; i < db_num && i < 32; i++) {
        led_on_table[i] = db_data[i] - db_data_old[i];
        db_data_old[i] = db_data[i];
    }
}

// ============ 平滑状态（cur → tgt 插值）============
static u8 cur_r[LED_NUM], cur_g[LED_NUM], cur_b[LED_NUM];
static u8 tgt_r[LED_NUM], tgt_g[LED_NUM], tgt_b[LED_NUM];

static void smooth_leds(void)
{
    for (int i = 0; i < LED_NUM; i++) {
        cur_r[i] += ((int)tgt_r[i] - (int)cur_r[i]) >> 2;
        cur_g[i] += ((int)tgt_g[i] - (int)cur_g[i]) >> 2;
        cur_b[i] += ((int)tgt_b[i] - (int)cur_b[i]) >> 2;
    }
}

static void flush_leds(void)
{
    for (int i = 0; i < LED_NUM; i++) {
        led_spi_rgb_to_24byte(cur_r[i], cur_g[i], cur_b[i], led_buf, i);
    }
    led_spi_send_rgbbuf_isr(led_buf, LED_NUM);
}

// ============ 简易随机 ============
static u32 xorshift32(void)
{
    static u32 x = 246810;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

// ============ 效果①：火焰 ============
static void effect_fire(u32 step)
{
    (void)step;

    int low_energy = 0;
    for (int i = 0; i < 4; i++) {
        int v = led_on_table[i];
        low_energy += (v > 0) ? v : -v;
    }
    low_energy /= 4;
    if (low_energy > 100) {
        low_energy = 100;
    }

    int base = 30 + low_energy * 225 / 100;

    for (int i = 0; i < LED_NUM; i++) {
        int rnd = (int)(xorshift32() % 61) - 30;
        int bri = base + rnd;
        if (bri < 10)  bri = 10;
        if (bri > 255) bri = 255;

        u8 r = (u8)bri;
        u8 g = (u8)((u32)bri * (30 + i * 55) / 255);
        if (g > r) {
            g = r;
        }
        u8 b = (i >= 2) ? (u8)((u32)bri * (i - 1) * 25 / 255) : 0;

        tgt_r[i] = r;
        tgt_g[i] = g;
        tgt_b[i] = b;
    }
}

// ============ 效果②：呼吸心跳 ============
static u32 breath_beat_decay = 0;

static void effect_breath(u32 step)
{
    int tri = (int)(step % 100);
    if (tri < 50) {
        tri = tri * 255 / 50;
    } else {
        tri = (100 - tri) * 255 / 50;
    }
    if (tri < 20) {
        tri = 20;
    }

    int low_energy = 0;
    for (int i = 0; i < 4; i++) {
        int v = led_on_table[i];
        low_energy += (v > 0) ? v : -v;
    }
    if (low_energy > 30) {
        breath_beat_decay = 255;
    }
    breath_beat_decay = breath_beat_decay * 7 / 8;

    int hue = (int)(step * 1) % 256;
    u8 r = 0, g = 0, b = 0;
    if (hue < 85) {
        r = 255; g = (u8)(hue * 3);          b = 0;
    } else if (hue < 170) {
        hue -= 85;
        r = (u8)(255 - hue * 3); g = 255;    b = (u8)(hue * 3);
    } else {
        hue -= 170;
        r = (u8)(hue * 3);       g = (u8)(255 - hue * 3); b = 255;
    }

    r = (u8)((u32)r * tri / 255);
    g = (u8)((u32)g * tri / 255);
    b = (u8)((u32)b * tri / 255);

    if (breath_beat_decay > 5) {
        u8 inv = (u8)breath_beat_decay;
        r = r + ((u32)(255 - r) * inv / 255);
        g = g + ((u32)(255 - g) * inv / 255);
        b = b + ((u32)(255 - b) * inv / 255);
    }

    for (int i = 0; i < LED_NUM; i++) {
        tgt_r[i] = r;
        tgt_g[i] = g;
        tgt_b[i] = b;
    }
}

// ============ 效果③：流光弹跳 ============
static const u8 rainbow[7][3] = {
    {255, 0,   0  },
    {255, 80,  0  },
    {255, 255, 0  },
    {0,   255, 0  },
    {0,   180, 255},
    {0,   0,   255},
    {160, 0,   255},
};

static void effect_bounce(u32 step)
{
    int energy = 0;
    for (int i = 0; i < 8; i++) {
        int v = led_on_table[i];
        energy += (v > 0) ? v : -v;
    }
    energy /= 8;

    int period = 60;
    if (energy > 5) {
        period = 60 * 30 / (energy + 10);
        if (period < 12)  period = 12;
        if (period > 80)  period = 80;
    }

    int t = (int)(step % (period * 2));
    int pos100;
    if (t < period) {
        pos100 = t * 300 / period;
    } else {
        pos100 = (period * 2 - t) * 300 / period;
    }

    int ci = (int)(step / (period * 4)) % 7;
    u8 cr = rainbow[ci][0];
    u8 cg = rainbow[ci][1];
    u8 cb = rainbow[ci][2];

    for (int i = 0; i < LED_NUM; i++) {
        int dist = pos100 - i * 100;
        if (dist < 0) dist = -dist;
        dist = dist * 5 / 100;

        int bri;
        if (dist < 3) {
            bri = 255 - dist * 30;
        } else {
            bri = 50 / dist;
        }
        if (bri > 255) bri = 255;
        if (bri < 1)   bri = 1;

        tgt_r[i] = (u8)((u32)cr * bri / 255);
        tgt_g[i] = (u8)((u32)cg * bri / 255);
        tgt_b[i] = (u8)((u32)cb * bri / 255);
    }
}

// ============ 效果④：能量粒子 ============
#define LINE_MAX      255

// 4个LED在线上的固定位置
static const u8 led_pos[4] = {32, 96, 160, 224};

// 3个粒子状态
static int p_pos[3];
static int p_dir[3];
static int p_speed_base[3];

// 可调参数（运行时可通过 API 修改）
int g_field_r = 30;             // 场半径(默认30，粒子能覆盖相邻LED)
static int g_particle_num = 3;   // 粒子数量

// 粒子颜色 (R, G, B)
static const u8 p_color[3][3] = {
    {255, 0,   0  },
    {0,   255, 0  },
    {0,   0,   255},
};

static u8 p_intensity[3];
static int music_state;
static int beat_flash;
static float speed_factor;

static void effect_particles_init(void)
{
    p_pos[0] = 32;
    p_pos[1] = 96;
    p_pos[2] = 192;
    p_dir[0] = 1;
    p_dir[1] = 1;
    p_dir[2] = 1;
    p_speed_base[0] = 1;
    p_speed_base[1] = 2;
    p_speed_base[2] = 3;
    p_intensity[0] = 200;
    p_intensity[1] = 200;
    p_intensity[2] = 200;
    g_field_r = 30;
    music_state = 1;
    beat_flash = 0;
    speed_factor = 1.0f;
}

// ============ 运行时参数设置接口 ============
void led_effect_set_field_r(int r)
{
    if (r < 1) r = 1;
    if (r > 120) r = 120;
    g_field_r = r;
    printf("[LED] 场半径 -> %d\n", r);
}

void led_effect_set_speed(int idx, int speed)
{
    if (idx < 0 || idx >= 3) return;
    if (speed < 0) speed = 0;
    if (speed > 20) speed = 20;
    p_speed_base[idx] = speed;
    printf("[LED] P%d速度 -> %d\n", idx, speed);
}

void led_effect_show_status(void)
{
    int n = 3;
    const char *state_names[] = {"平静", "正常", "节拍", "高潮"};
    printf("────────── LED 粒子状态 ──────────\n");
    printf("  场半径: %d\n", g_field_r);
    for (int i = 0; i < n; i++) {
        printf("  P%d(位置%3d, 速度%2d, 亮度%3d)\n",
               i, p_pos[i], p_speed_base[i], p_intensity[i]);
    }
    printf("  音乐状态: %s  速度因子: %.1f  节拍余辉: %d\n",
           state_names[music_state], speed_factor, beat_flash);
    printf("  各LED颜色: ");
    for (int led = 0; led < LED_NUM; led++) {
        printf("LED%d(%d,%d,%d) ", led, cur_r[led], cur_g[led], cur_b[led]);
    }
    printf("\n────────────────────────────────\n");
}

static void effect_particles(u32 step)
{
    // ---- 音乐状态检测 ----
    int total_energy = 0;
    int low_energy = 0;
    for (int i = 0; i < 8; i++) {
        int v = led_on_table[i];
        if (v < 0) v = -v;
        total_energy += v;
        if (i < 4) low_energy += v;
    }

    int target_state = 1;  // normal
    if (total_energy < 10) {
        target_state = 0;  // calm
    } else if (total_energy > 80) {
        target_state = 3;  // climax
    }

    // beat 检测：低频骤增
    if (low_energy > 40) {
        target_state = 2;  // beat
        beat_flash = 255;
    }

    // 状态平滑过渡
    if (music_state != target_state) {
        music_state = target_state;
    }

    // beat 衰减
    if (beat_flash > 0) {
        beat_flash = beat_flash * 6 / 8;
        if (beat_flash < 3) beat_flash = 0;
    }

    // ---- 速度因子 ----
    float target_speed = 1.0f;
    switch (music_state) {
    case 0: target_speed = 0.3f; break;  // 平静
    case 1: target_speed = 1.0f; break;  // 正常
    case 2: target_speed = 0.5f; break;  // 节拍时短暂停滞再弹开
    case 3: target_speed = 1.8f; break;  // 高潮
    }
    speed_factor += (target_speed - speed_factor) * 0.1f;

    // ---- 粒子亮度 ----
    int target_intensity = 150;
    if (music_state == 0) target_intensity = 60;       // 平静: 微光
    else if (music_state == 3) target_intensity = 240; // 高潮: 高亮

    for (int i = 0; i < 3; i++) {
        int ti = target_intensity;

        // 节拍脉冲：粒子弹开 + 亮度瞬间提升
        if (beat_flash > 0) {
            ti = 255;
            // 粒子被弹开
            if ((step % 5) == (u32)i) {
                p_pos[i] += (p_dir[i] * 30);
            }
        }

        p_intensity[i] += (ti - (int)p_intensity[i]) >> 3;
    }

    // ---- 更新粒子位置 ----
    for (int i = 0; i < 3; i++) {
        int speed = (int)((float)p_speed_base[i] * speed_factor);
        if (speed < 1) speed = 1;

        p_pos[i] += p_dir[i] * speed;

        // 出界反弹
        if (p_pos[i] > LINE_MAX) {
            p_pos[i] = LINE_MAX - (p_pos[i] - LINE_MAX);
            p_dir[i] = -1;
        } else if (p_pos[i] < 0) {
            p_pos[i] = -p_pos[i];
            p_dir[i] = 1;
        }
    }

    // ---- 计算每个LED的RGB ----
    for (int led = 0; led < LED_NUM; led++) {
        int r = 0, g = 0, b = 0;

        for (int p = 0; p < 3; p++) {
            int dist = p_pos[p] - (int)led_pos[led];
            if (dist < 0) dist = -dist;

            if (dist < g_field_r) {
                // 场强度：距离越近越强
                int inf = (g_field_r - dist) * 255 / g_field_r;
                // 叠加粒子亮度
                inf = inf * (int)p_intensity[p] / 255;

                r += p_color[p][0] * inf / 255;
                g += p_color[p][1] * inf / 255;
                b += p_color[p][2] * inf / 255;
            }
        }

        // 叠加节拍闪白
        if (beat_flash > 10) {
            r += ((u32)(255 - r) * (u32)beat_flash / 255);
            g += ((u32)(255 - g) * (u32)beat_flash / 255);
            b += ((u32)(255 - b) * (u32)beat_flash / 255);
        }

        tgt_r[led] = (r > 255) ? 255 : (u8)r;
        tgt_g[led] = (g > 255) ? 255 : (u8)g;
        tgt_b[led] = (b > 255) ? 255 : (u8)b;
    }
}

// ============ 手动切换 ============
static volatile u8 g_switch_req = 0;

void led_effect_switch(u8 direct)
{
    g_switch_req = direct;   // 1=下一个, 0=上一个
}

static const char *effect_names[] = {
    "火焰 (Fire)",
    "呼吸心跳 (Breath)",
    "流光弹跳 (Bounce)",
    "能量粒子 (Particles)",
};
#define EFFECT_NUM  (sizeof(effect_names) / sizeof(effect_names[0]))

// ============ 任务线程 ============
static void led_task(void *priv)
{
    (void)priv;

    led_spi_init();
    effect_particles_init();

    u32 step = 0;
    u32 effect_idx = 255;

    while (1) {
        // 每 3 帧 (≈100ms) 读一次频谱
        if ((step % 3) == 0) {
            read_spectrum();
        }

        // 按键手动切换效果
        if (g_switch_req || effect_idx == 255) {
            if (effect_idx == 255) {
                effect_idx = 3;   // 默认启动粒子效果
            } else if (g_switch_req == 1) {
                effect_idx ++;
                if (effect_idx >= EFFECT_NUM) {
                    effect_idx = 0;
                }
            } else {
                if (effect_idx == 0) {
                    effect_idx = EFFECT_NUM - 1;
                } else {
                    effect_idx --;
                }
            }
            g_switch_req = 0;

            if (effect_idx == 3) {
                effect_particles_init();
            }

            printf("\n[LED] >>> 切换到: %s\n", effect_names[effect_idx]);

            // 切效果时全暗一帧，平滑过渡
            for (int i = 0; i < LED_NUM; i++) {
                tgt_r[i] = 0;
                tgt_g[i] = 0;
                tgt_b[i] = 0;
            }
        }

        // 执行当前效果
        switch (effect_idx) {
        case 0:
            effect_fire(step);
            break;
        case 1:
            effect_breath(step);
            break;
        case 2:
            effect_bounce(step);
            break;
        default:
            effect_particles(step);
            break;
        }

        smooth_leds();
        flush_leds();

        os_time_dly(3);   // ≈30ms
        wdt_clear();
        step ++;
    }
}

// ============ 对外接口 ============
void led_effect_init(void)
{
    printf("******************  led effect start  *******************\n");
    os_task_create(led_task, NULL, 20, 512, 0, "led_eff");
}
