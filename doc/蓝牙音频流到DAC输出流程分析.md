# AC696x 蓝牙音箱SDK：APP_BT_TASK 接收蓝牙音频流并通过 DAC 输出的完整流程

## 一、总体架构概述

整个数据流转路径可以概括为：

```
手机蓝牙 -> BT协议栈(L2CAP/A2DP) -> A2DP接收回调 -> A2DP解码器(SBC/AAC) -> 音频流节点链(音效/DRC/EQ) -> Mixer -> DAC通道 -> DAC硬件 -> 扬声器
```

涉及的核心模块层次：
- **应用层**：APP_BT_TASK 任务主循环
- **蓝牙事件层**：BT_STATUS_A2DP_MEDIA_START 触发解码
- **解码管理层**：audio_dec_bt.c 中的 a2dp_dec_open / a2dp_dec_start
- **解码器层**：a2dp_decoder (lib库) 负责 SBC/AAC 解码
- **音频流层**：audio_stream 串联各处理节点
- **输出层**：mixer -> default_dac -> DAC硬件

---

## 二、阶段一：系统初始化与任务创建

### 2.1 音频解码系统初始化

文件：`cpu/br25/audio_dec/audio_dec.c`，第620-912行

函数 `audio_dec_init()` 完成以下初始化：

1. **创建解码任务**（第627行）：`audio_decoder_task_create(&decode_task, "audio_dec")`
2. **初始化音频输出**（第645行）：`app_audio_output_init()`
3. **创建DAC通道**（第781-787行）：
   ```c
   audio_dac_new_channel(&dac_hdl, &default_dac);
   struct audio_dac_channel_attr attr;
   audio_dac_channel_get_attr(&default_dac, &attr);
   attr.delay_time = 50;
   attr.protect_time = 5;
   attr.write_mode = WRITE_MODE_BLOCK;
   audio_dac_channel_set_attr(&default_dac, &attr);
   ```
4. **初始化Mixer**（第674行）：`audio_mixer_open(&mixer)`
5. **创建Mixer输出数据流**（第793-794行）：
   ```c
   mixer.stream = audio_stream_open(&mixer, audio_mixer_stream_resume);
   audio_stream_add_list(mixer.stream, entries, entry_cnt);
   ```
   其中 entries 包含：`mixer.entry -> [音效节点] -> default_dac.entry`

全局变量声明（第76-77行）：
```c
struct audio_decoder_task   decode_task = {0};
struct audio_mixer          mixer = {0};
```

### 2.2 APP 主循环与任务调度

文件：`apps/soundbox/app_main.c`

- 第52-113行：`app_task_loop()` 是一个无限 while(1) 循环，根据 `app_curr_task` 变量切换到对应任务函数
- 第64-66行：当 `app_curr_task == APP_BT_TASK` 时调用 `app_bt_task()`
- `APP_BT_TASK` 的值为 3（定义在 `app_task.h` 第12行）

---

## 三、阶段二：APP_BT_TASK 主循环

### 3.1 任务入口

文件：`apps/soundbox/task_manager/bt/bt.c`，第1167-1267行

```c
void app_bt_task()
{
    bt_task_init();          // 初始化变量、时钟、显示
    // ...播放提示音...
    bt_task_start();         // 启动蓝牙协议栈
    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);  // 阻塞等待消息
        switch (msg[0]) {
        case APP_MSG_SYS_EVENT:
            bt_sys_event_handler((struct sys_event *)(msg + 1));
            break;
        }
        // 退出检测逻辑...
    }
}
```

### 3.2 任务初始化

文件：`apps/soundbox/task_manager/bt/bt_switch_fun.c`，第955-980行

`bt_task_init()` 设置时钟、初始化UI显示、设置变量标志位。

### 3.3 蓝牙协议栈启动

同文件，第989-1034行，`bt_task_start()` 调用链：
```
bt_task_start()
  -> bt_function_select_init()    // 配置协议栈参数(支持AAC/SBC、音量同步表等)
  -> bredr_handle_register()      // 注册协议栈回调函数(SPP、音量同步等)
  -> btstack_init()               // 初始化蓝牙协议栈
  -> BT_STATE_INIT()              // 初始化蓝牙控制器
```

---

## 四、阶段三：蓝牙连接建立

### 4.1 连接状态事件处理

文件：`apps/soundbox/task_manager/bt/bt.c`，第477-603行

`bt_connction_status_event_handler()` 处理所有蓝牙连接状态事件，关键事件：
- `BT_STATUS_INIT_OK`（第492行）：蓝牙初始化完成
- `BT_STATUS_FIRST_CONNECTED`/`BT_STATUS_SECOND_CONNECTED`（第514-520行）：设备连接成功
- **`BT_STATUS_A2DP_MEDIA_START`**（第553-555行）：A2DP音频流开始，触发解码
- `BT_STATUS_A2DP_MEDIA_STOP`（第557-559行）：A2DP音频流停止
- `BT_STATUS_SCO_STATUS_CHANGE`（第561-563行）：通话音频链路变化

---

## 五、阶段四：A2DP 音频流接收与解码启动

### 5.1 A2DP 媒体开始事件

文件：`apps/soundbox/task_manager/bt/bt_event_fun.c`，第1693-1717行

```c
void bt_status_a2dp_media_start(struct bt_event *bt)
{
    __this->call_flag = 0;
    __this->a2dp_start_flag = 1;
    // ...TWS相关处理...
    a2dp_dec_open(bt->value);   // bt->value = 媒体类型(SBC=0, AAC=2)
}
```

这是整个流程的关键触发点：蓝牙协议栈通知 APP 层 A2DP 音频流已开始，APP 调用 `a2dp_dec_open()` 启动解码。

### 5.2 打开 A2DP 解码

文件：`cpu/br25/audio_dec/audio_dec_bt.c`，第823-881行

```c
int a2dp_dec_open(int media_type)
{
    dec = zalloc(sizeof(*dec));    // 分配 a2dp_dec_hdl

    switch (media_type) {
    case A2DP_CODEC_SBC:           // SBC编码
        dec->dec.coding_type = AUDIO_CODING_SBC;
        clock_add(DEC_SBC_CLK);
        break;
    case A2DP_CODEC_MPEG24:        // AAC编码
        dec->dec.coding_type = AUDIO_CODING_AAC;
        clock_add(DEC_AAC_CLK);
        break;
    }

    bt_a2dp_dec = dec;             // 保存全局句柄
    dec->wait.priority = 1;
    dec->wait.handler = a2dp_wait_res_handler;  // 资源等待回调
    audio_decoder_task_add_wait(&decode_task, &dec->wait);  // 注册到解码任务
}
```

`audio_decoder_task_add_wait()` 会触发资源获取回调 `a2dp_wait_res_handler()`。

### 5.3 资源等待回调

同文件，第799-814行：

```c
static int a2dp_wait_res_handler(struct audio_res_wait *wait, int event)
{
    if (event == AUDIO_RES_GET) {
        err = a2dp_dec_start();     // 获取到资源，开始解码
    } else if (event == AUDIO_RES_PUT) {
        a2dp_audio_res_close();     // 释放资源
    }
}
```

### 5.4 A2DP 解码启动（核心函数）

同文件，第467-789行，`a2dp_dec_start()` 是整个音频数据流构建的核心函数。执行流程：

**步骤1：打开解码器**（第482行）
```c
err = a2dp_decoder_open(&dec->dec, &decode_task);
```

**步骤2：获取解码格式**（第490行）
```c
err = audio_decoder_get_fmt(&dec->dec.decoder, &fmt);
```

**步骤3：配置输出声道**（第496-498行）
```c
ch_num = audio_output_channel_num();
ch_type = audio_output_channel_type();
a2dp_decoder_set_output_channel(&dec->dec, ch_num, ch_type);
```

**步骤4：配置Mixer通道**（第500-508行）
```c
audio_mode_main_dec_open(AUDIO_MODE_MAIN_STATE_DEC_A2DP);
audio_mixer_ch_open_head(&dec->mix_ch, &mixer);
audio_mixer_ch_set_src(&dec->mix_ch, 1, 0);
audio_mixer_ch_set_no_wait(&dec->mix_ch, 1, 20);
audio_mixer_ch_sample_sync_enable(&dec->mix_ch, 1);
audio_mixer_ch_set_sample_rate(&dec->mix_ch, fmt->sample_rate);
```

**步骤5：打开音效处理**（第511-554行）
包括：环绕声(surround)、虚拟低音(vbass)、EQ、DRC等

**步骤6：初始化同步模块**（第557-564行）
```c
dec->sync = a2dp_output_sync_open(fmt->sample_rate, ...);
a2dp_decoder_stream_sync_enable(&dec->dec, dec->sync->context, fmt->sample_rate, CONFIG_A2DP_DELAY_TIME);
```

**步骤7：构建音频数据流节点链**（第587-691行）
```c
struct audio_stream_entry *entries[16] = {NULL};
entries[entry_cnt++] = &dec->dec.decoder.entry;    // 解码器输出
// ... 数字音量 ...
// ... sync同步 ...
// ... 虚拟低音 ...
// ... 高低音EQ ...
// ... 环绕声 ...
// ... 音乐EQ/DRC ...
entries[entry_cnt++] = &dec->mix_ch.entry;          // Mixer通道入口

dec->stream = audio_stream_open(dec, a2dp_dec_out_stream_resume);
audio_stream_add_list(dec->stream, entries, entry_cnt);
```

完整数据流节点链（启用所有音效时）：
```
decoder.entry -> digital_vol -> sync.entry -> sync.resample_entry
-> vbass_prev_gain -> ns_gate -> vbass -> high_bass -> hb_drc
-> gain -> surround -> eq -> drc -> convert -> ext_eq -> mix_ch.entry
```

**步骤8：启动解码**（第710行）
```c
dec->dec.start = 1;
err = audio_decoder_start(&dec->dec.decoder);
```

---

## 六、阶段五：音频数据从蓝牙到 DAC 的流转

### 6.1 蓝牙接收回调

文件：`cpu/br25/audio_dec/audio_dec_bt.c`，第386-391行

```c
void a2dp_rx_notice_to_decode(void)
{
    if (bt_a2dp_dec && bt_a2dp_dec->dec.start) {
        a2dp_decoder_resume_from_bluetooth(&bt_a2dp_dec->dec);
    }
}
```

这是蓝牙协议栈收到 A2DP 数据后的回调。当手机发送音频数据时，BT协议栈底层调用此函数通知解码器有新数据到达。

### 6.2 数据流激活回调

同文件，第372-377行：

```c
static void a2dp_dec_out_stream_resume(void *p)
{
    struct a2dp_dec_hdl *dec = (struct a2dp_dec_hdl *)p;
    audio_decoder_resume(&dec->dec.decoder);
}
```

这是 `audio_stream_open` 时注册的回调。当下游节点（Mixer/DAC）准备好接收数据时，会调用此回调唤醒解码器。

### 6.3 解码器内部数据获取

解码器 `a2dp_decoder` 是库函数，其内部流程大致为：
1. `a2dp_decoder_resume_from_bluetooth()` 被调用后，解码器从 BT 协议栈的 A2DP 媒体缓冲区获取编码数据（SBC/AAC帧）
2. 调用对应的解码算法（SBC解码器或AAC解码器）将编码数据解码为PCM数据
3. 解码后的PCM数据通过 `decoder.entry` 的 `data_handler` 输出到音频流

### 6.4 音频流节点链数据传递

解码后的PCM数据沿着 `a2dp_dec_start()` 中构建的节点链依次流过：

```
decoder.entry (解码输出PCM)
  -> digital_vol_entry (数字音量调节)
  -> sync->entry (无线同步处理)
  -> sync->resample_entry (变采样处理)
  -> [音效节点: vbass, eq, drc, surround 等]
  -> mix_ch.entry (Mixer通道入口)
```

每个节点的 `data_handler` 回调处理数据后，调用 `audio_stream_run()` 将数据传递给下一个节点。

### 6.5 Mixer 到 DAC

文件：`cpu/br25/audio_dec/audio_dec.c`，第744-794行

在 `audio_dec_init()` 中，Mixer 的输出数据流已经配置好：
```c
entries[entry_cnt++] = &mixer.entry;           // Mixer输出
// ... 音效节点(响度、频谱、人声消除等) ...
entries[entry_cnt++] = &default_dac.entry;     // DAC通道入口

mixer.stream = audio_stream_open(&mixer, audio_mixer_stream_resume);
audio_stream_add_list(mixer.stream, entries, entry_cnt);
```

Mixer 将来自不同解码源（A2DP、提示音、按键音等）的音频数据混合后，通过数据流传递给 `default_dac.entry`。

### 6.6 DAC 通道输出

`default_dac` 是 `struct audio_dac_channel` 类型，在 `audio_dec_init()` 中创建（第781行）：
```c
audio_dac_new_channel(&dac_hdl, &default_dac);
```

DAC 通道的属性配置：
- `delay_time = 50`：启动延时50ms
- `protect_time = 5`：保护时间5ms
- `write_mode = WRITE_MODE_BLOCK`：阻塞写入模式

DAC 通道的 `entry.data_handler` 由底层库实现，负责将 PCM 数据写入 DAC 硬件的 FIFO 缓冲区。DAC 硬件通过 DMA 从 FIFO 读取数据，经过数模转换后输出模拟音频信号到扬声器。

### 6.7 同步模块

文件：`cpu/br25/audio_dec/audio_sync.c`，第31-68行

`a2dp_output_sync_open()` 创建同步模块：
```c
audio_dac_channel_sync_enable(&default_dac, sample_sync);  // 同步模块接入DAC
info.target = AUDIO_SYNC_TARGET_DAC;
info.protocol = WL_PROTOCOL_RTP;
```

同步模块的作用是根据蓝牙传输的不均匀性，动态调整 DAC 的采样率，保证音频播放的连续性和同步性。

---

## 七、阶段六：A2DP 音频流停止

### 7.1 停止事件

文件：`apps/soundbox/task_manager/bt/bt_event_fun.c`，第1726-1741行

```c
void bt_status_a2dp_media_stop(struct bt_event *bt)
{
    __this->a2dp_start_flag = 0;
    bt_drop_a2dp_frame_stop();
    a2dp_dec_close();
}
```

### 7.2 关闭解码

文件：`cpu/br25/audio_dec/audio_dec_bt.c`，第891-909行

`a2dp_dec_close()` 调用链：
```
a2dp_dec_close()
  -> a2dp_audio_res_close()       // 关闭所有音效、同步、Mixer通道、数据流
  -> a2dp_dec_release()           // 释放解码任务等待、移除时钟、释放内存
```

`a2dp_audio_res_close()`（第246-319行）按以下顺序关闭：
1. `a2dp_decoder_close()` - 关闭解码器
2. 关闭各音效节点（surround、vbass、eq、drc等）
3. `a2dp_output_sync_close()` - 关闭同步模块
4. `audio_mixer_ch_close()` - 关闭Mixer通道
5. `audio_stream_close()` - 关闭数据流

---

## 八、完整函数调用链总结

```
[手机播放音乐]
    |
    v
[BT协议栈接收A2DP数据]
    |
    v
[BT_STATUS_A2DP_MEDIA_START 事件产生]
    |
    v
bt_connction_status_event_handler()         -- bt.c:553
    |
    v
bt_status_a2dp_media_start()                -- bt_event_fun.c:1693
    |
    v
a2dp_dec_open(media_type)                   -- audio_dec_bt.c:823
    |  - 分配 a2dp_dec_hdl
    |  - 设置编码类型(SBC/AAC)
    |  - audio_decoder_task_add_wait()
    v
a2dp_wait_res_handler(AUDIO_RES_GET)        -- audio_dec_bt.c:805
    |
    v
a2dp_dec_start()                            -- audio_dec_bt.c:467
    |  - a2dp_decoder_open()                // 打开解码器
    |  - audio_decoder_get_fmt()            // 获取格式
    |  - a2dp_decoder_set_output_channel()  // 设置声道
    |  - audio_mixer_ch_open_head()         // 配置Mixer通道
    |  - a2dp_output_sync_open()            // 打开同步
    |  - audio_stream_open() + audio_stream_add_list()  // 构建数据流
    |  - audio_decoder_start()              // 启动解码
    v
[解码器运行中，等待蓝牙数据]
    |
    v
a2dp_rx_notice_to_decode()                  -- audio_dec_bt.c:386  [蓝牙底层回调]
    |  - a2dp_decoder_resume_from_bluetooth()
    v
[解码器从BT缓冲区获取SBC/AAC帧，解码为PCM]
    |
    v
[PCM数据流经节点链]
    decoder.entry -> digital_vol -> sync -> [音效] -> mix_ch.entry
    |
    v
[Mixer混合]
    mixer.entry
    |
    v
[DAC通道]
    default_dac.entry -> DAC硬件FIFO
    |
    v
[DAC数模转换 -> 扬声器输出]
```

---

## 九、关键文件索引

| 文件 | 功能 |
|------|------|
| `apps/soundbox/app_main.c` | 应用主循环，任务调度 |
| `apps/soundbox/include/app_task.h` | 任务ID定义(APP_BT_TASK=3) |
| `apps/soundbox/task_manager/bt/bt.c` | BT任务主循环、事件分发 |
| `apps/soundbox/task_manager/bt/bt_switch_fun.c` | BT任务创建/启动/退出 |
| `apps/soundbox/task_manager/bt/bt_event_fun.c` | BT状态事件处理(A2DP开始/停止等) |
| `apps/soundbox/include/task_manager/bt/bt.h` | BT模块头文件(app_bt_opr结构体) |
| `cpu/br25/audio_dec/audio_dec_bt.c` | A2DP/ESCO解码管理(核心) |
| `cpu/br25/audio_dec/audio_dec_bt.h` | BT解码接口声明 |
| `cpu/br25/audio_dec/audio_dec.c` | 音频解码系统初始化、Mixer/DAC配置 |
| `cpu/br25/audio_dec/audio_dec.h` | 解码系统头文件 |
| `cpu/br25/audio_dec/audio_sync.c` | 无线同步模块(A2DP/ESCO到DAC的同步) |
| `cpu/br25/audio_common/app_audio.c` | DAC硬件初始化、音量管理 |
| `include_lib/media/media_develop/media/a2dp_decoder.h` | A2DP解码器接口定义 |

---

## 十、回调函数汇总

| 回调函数 | 注册位置 | 触发时机 | 作用 |
|----------|----------|----------|------|
| `a2dp_rx_notice_to_decode()` | BT协议栈内部 | 蓝牙收到A2DP数据包 | 唤醒解码器 |
| `a2dp_dec_out_stream_resume()` | `audio_stream_open()` | 下游节点准备就绪 | 唤醒解码器 |
| `a2dp_wait_res_handler()` | `audio_decoder_task_add_wait()` | 解码资源可用 | 启动/停止解码 |
| `a2dp_dec_event_handler()` | `audio_decoder_set_event_handler()` | 解码结束 | 关闭解码 |
| `a2dp_to_dac_probe_handler()` | `default_dac.entry.prob_handler` | 数据到达DAC前 | 设置同步标志 |
| `mixer_event_handler()` | `audio_mixer_set_event_handler()` | Mixer通道开关 | 时钟管理 |
| `audio_mixer_stream_resume()` | `audio_stream_open()` | Mixer输出就绪 | 唤醒Mixer输出 |
| `bt_connction_status_event_handler()` | `bt_sys_event_office()` | 蓝牙状态变化 | 分发处理各状态 |

---

## 十一、数据流节点链详解

### 11.1 A2DP音频数据流（解码到Mixer）

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        A2DP音频数据流节点链                                  │
└─────────────────────────────────────────────────────────────────────────────┘

┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│ decoder.entry │ -> │ digital_vol  │ -> │ sync.entry   │
│ (解码器输出)  │    │ (数字音量)    │    │ (同步处理)    │
└──────────────┘    └──────────────┘    └──────────────┘
                                                │
                                                v
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   vbass      │ <- │ resample     │ <- │ vbass_prev   │
│ (虚拟低音)    │    │ (重采样)      │    │ (增益预处理)  │
└──────────────┘    └──────────────┘    └──────────────┘
      │
      v
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│ high_bass    │ -> │ hb_drc       │ -> │   gain       │
│ (高低音EQ)    │    │ (动态范围控制) │    │ (增益)        │
└──────────────┘    └──────────────┘    └──────────────┘
      │
      v
┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│  surround    │ -> │     eq       │ -> │     drc      │
│ (环绕声)      │    │ (均衡器)      │    │ (动态压缩)    │
└──────────────┘    └──────────────┘    └──────────────┘
      │
      v
┌──────────────┐    ┌──────────────┐
│   convert    │ -> │  mix_ch.entry │
│ (格式转换)    │    │ (Mixer通道)   │
└──────────────┘    └──────────────┘
```

### 11.2 Mixer到DAC数据流

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        Mixer到DAC数据流节点链                                │
└─────────────────────────────────────────────────────────────────────────────┘

┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│ mixer.entry  │ -> │ loudness     │ -> │  spectrum    │
│ (Mixer输出)   │    │ (响度增强)    │    │ (频谱显示)    │
└──────────────┘    └──────────────┘    └──────────────┘
      │
      v
┌──────────────┐    ┌──────────────┐
│ voice_cancel │ -> │default_dac   │
│ (人声消除)    │    │ (DAC通道)     │
└──────────────┘    └──────────────┘
                          │
                          v
                  ┌──────────────┐
                  │  DAC硬件FIFO  │
                  │ (DMA传输)     │
                  └──────────────┘
                          │
                          v
                  ┌──────────────┐
                  │   扬声器输出   │
                  │ (模拟音频)    │
                  └──────────────┘
```

---

## 十二、音频数据格式

### 12.1 蓝牙编码格式

| 编码格式 | 采样率 | 声道 | 比特率 | 说明 |
|----------|--------|------|--------|------|
| SBC | 44.1/48kHz | 立体声 | 328kbps | 标准蓝牙音频编码 |
| AAC | 44.1/48kHz | 立体声 | 256kbps | 高质量音频编码 |

### 12.2 解码后PCM格式

| 参数 | 值 | 说明 |
|------|-----|------|
| 采样率 | 44100/48000 Hz | 与编码格式匹配 |
| 位深 | 16bit | 标准CD质量 |
| 声道 | 立体声 | 左右声道 |
| 字节序 | 小端 | Little Endian |

### 12.3 DAC输出格式

| 参数 | 值 | 说明 |
|------|-----|------|
| 采样率 | 8000-48000 Hz | 根据音频源自动切换 |
| 位深 | 16bit | DAC硬件分辨率 |
| 声道 | 立体声 | 左右声道输出 |
| 输出模式 | LR OUTPUT | 左右声道独立输出 |

---

## 十三、关键数据结构

### 13.1 A2DP解码控制块

```c
struct a2dp_dec_hdl {
    struct a2dp_dec dec;                    // 解码器句柄
    struct audio_mixer_ch mix_ch;           // Mixer通道
    struct audio_stream *stream;            // 音频流
    struct audio_sync *sync;                // 同步模块
    struct audio_res_wait wait;             // 资源等待

    // 音效句柄
    struct audio_eq *eq;                    // 均衡器
    struct audio_drc *drc;                  // 动态范围控制
    struct audio_surround *surround;        // 环绕声
    struct audio_vbass *vbass;              // 虚拟低音
    struct audio_hb_drc *hb_drc;            // 高低音DRC
    // ... 其他音效
};
```

### 13.2 音频流节点

```c
struct audio_stream_entry {
    void *data;                             // 节点数据
    audio_stream_data_handler handler;      // 数据处理回调
    struct list_head entry;                 // 链表节点
};
```

### 13.3 Mixer通道

```c
struct audio_mixer_ch {
    struct audio_mixer *mixer;              // 所属Mixer
    struct audio_stream_entry entry;        // 流节点
    u8 sample_sync;                         // 采样同步标志
    s16 sample_rate;                        // 采样率
    // ... 其他参数
};
```

---

## 十四、时序图

```
时间轴 →

手机      │ 开始播放音乐
          │
蓝牙协议栈│ 接收A2DP数据包
          │
          │ BT_STATUS_A2DP_MEDIA_START
          v
APP层     │ bt_connction_status_event_handler()
          │   -> bt_status_a2dp_media_start()
          │     -> a2dp_dec_open(media_type)
          │       -> 分配解码控制块
          │       -> audio_decoder_task_add_wait()
          │
          │ AUDIO_RES_GET
          v
解码管理层 │ a2dp_wait_res_handler()
          │   -> a2dp_dec_start()
          │     -> a2dp_decoder_open()
          │     -> 配置Mixer通道
          │     -> 构建数据流节点链
          │     -> audio_decoder_start()
          v
解码器层  │ 等待蓝牙数据
          │
          │ a2dp_rx_notice_to_decode() [蓝牙回调]
          v
          │ a2dp_decoder_resume_from_bluetooth()
          │   -> 从BT缓冲区获取SBC/AAC帧
          │   -> 解码为PCM数据
          v
音频流层  │ PCM数据沿节点链流动
          │ decoder -> digital_vol -> sync -> [音效] -> mix_ch
          v
Mixer层   │ 多路音频混合
          │ mixer -> default_dac
          v
DAC层     │ PCM数据写入FIFO
          │ DMA从FIFO读取数据
          │ DAC数模转换
          v
硬件层    │ 模拟音频信号输出
          │ 扬声器发声
```

---

## 十五、调试方法

### 15.1 关键日志过滤

```bash
# 过滤A2DP相关日志
grep "A2DP" log.txt

# 过滤音频解码日志
grep "audio_dec" log.txt

# 过滤DAC相关日志
grep "AUDIO-DAC" log.txt

# 过滤蓝牙状态事件
grep "bt_connction_status_event_handler" log.txt
```

### 15.2 常见问题排查

| 问题 | 可能原因 | 排查方法 |
|------|----------|----------|
| 无声 | 解码器未启动 | 检查 `a2dp_dec_start` 是否执行 |
| 杂音 | 同步模块异常 | 检查 `audio_sync` 日志 |
| 断断续续 | 缓冲区溢出/欠载 | 检查 FIFO 状态 |
| 音量异常 | 数字音量配置错误 | 检查 `digital_vol` 参数 |

### 15.3 调试断点建议

1. `a2dp_dec_open()` - 解码开始
2. `a2dp_dec_start()` - 数据流构建
3. `a2dp_rx_notice_to_decode()` - 数据接收
4. `a2dp_dec_out_stream_resume()` - 数据流恢复
5. `audio_mixer_ch_write()` - Mixer写入

---

## 版本信息

- **SDK版本**: ac696x_soundbox_sdk_v1.7.0
- **分析日期**: 2026-06-02
- **分析人**: 欧礼洪

---

## 修订记录

| 日期 | 版本 | 修订内容 |
|------|------|----------|
| 2026-06-02 | V1.0 | 初版，完整分析蓝牙音频流到DAC输出流程 |
