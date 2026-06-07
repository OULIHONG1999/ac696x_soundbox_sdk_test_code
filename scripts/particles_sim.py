#!/usr/bin/env python3
"""
AC696x LED 粒子效果模拟器
在电脑上调试效果，再烧录到开发板。
"""

import tkinter as tk
import math
import random

# ============ 配置参数（和 C 代码一致，改这里即可同步）============
LINE_MAX = 255
LED_POS = [32, 96, 160, 224]
LED_NUM = 4

# 粒子颜色池（最多支持 6 个粒子）
P_COLORS = [
    (255, 0, 0),    # 0: R
    (0, 255, 0),    # 1: G
    (0, 0, 255),    # 2: B
    (255, 255, 0),  # 3: 黄
    (255, 0, 255),  # 4: 紫
    (0, 255, 255),  # 5: 青
]
P_NAMES = ["R", "G", "B", "Y", "M", "C"]

MAX_PARTICLES = 6

# ============ Canvas 绘图参数 ============
WIN_W = 900
WIN_H = 780
LINE_Y = 120          # 0~255 线的 Y 坐标
LINE_X0 = 100         # 线的左边界
LINE_X1 = 800         # 线的右边界
LED_DISPLAY_Y = 230   # 颜色显示 Y
P_RADIUS = 12         # 粒子绘制半径
LED_RADIUS = 18       # LED 绘制半径


class ParticleSimulator:
    def __init__(self):
        self.reset()

    def reset(self):
        self.particle_count = 3           # 当前粒子数
        self.field_r = 8                  # 场半径（可调）
        self.p_pos = [32, 96, 192]        # 位置
        self.p_dir = [1, 1, 1]            # 方向
        self.p_speed_base = [1, 2, 3]     # 各粒子速度（可调）
        self.p_intensity = [200, 200, 200]  # 亮度
        self.music_state = 1
        self.beat_flash = 0
        self.speed_factor = 1.0
        self.step = 0
        self.energy = 0
        self.low_energy = 0
        self.auto_music = True

    def add_particle(self):
        """增加一个粒子"""
        if self.particle_count >= MAX_PARTICLES:
            return
        idx = self.particle_count
        # 随机位置
        self.p_pos.append(random.randint(0, LINE_MAX))
        self.p_dir.append(1 if random.random() > 0.5 else -1)
        self.p_speed_base.append(2)
        self.p_intensity.append(200)
        self.particle_count += 1

    def remove_particle(self):
        """减少一个粒子"""
        if self.particle_count <= 1:
            return
        self.particle_count -= 1
        self.p_pos.pop()
        self.p_dir.pop()
        self.p_speed_base.pop()
        self.p_intensity.pop()

    def step_simulate(self):
        """一帧更新"""
        self.step += 1
        n = self.particle_count

        # ---- 模拟频谱能量 ----
        if self.auto_music:
            t = self.step * 0.02
            self.energy = max(0, 50 + 40 * math.sin(t) + 20 * math.sin(t * 2.3) + 10 * random.gauss(0, 1))
            self.low_energy = max(0, 20 + 30 * math.sin(t * 1.7) + 15 * random.gauss(0, 1))

        # ---- 音乐状态 ----
        total = self.energy
        low = self.low_energy
        target_state = 1
        if total < 10:
            target_state = 0
        elif total > 80:
            target_state = 3
        if low > 40:
            target_state = 2
            self.beat_flash = 255
        self.music_state = target_state

        if self.beat_flash > 0:
            self.beat_flash = self.beat_flash * 6 // 8
            if self.beat_flash < 3:
                self.beat_flash = 0

        # ---- 速度因子 ----
        target_speed = {0: 0.3, 1: 1.0, 2: 0.5, 3: 1.8}[self.music_state]
        self.speed_factor += (target_speed - self.speed_factor) * 0.1

        # ---- 粒子亮度 ----
        target_intensity = 150
        if self.music_state == 0:
            target_intensity = 60
        elif self.music_state == 3:
            target_intensity = 240

        for i in range(n):
            ti = target_intensity
            if self.beat_flash > 0:
                ti = 255
                if (self.step % 5) == i:
                    self.p_pos[i] += (self.p_dir[i] * 30)
            self.p_intensity[i] += (ti - self.p_intensity[i]) >> 3

        # ---- 粒子移动 ----
        for i in range(n):
            speed = max(1, int(self.speed_factor * self.p_speed_base[i]))
            self.p_pos[i] += self.p_dir[i] * speed
            if self.p_pos[i] > LINE_MAX:
                self.p_pos[i] = LINE_MAX - (self.p_pos[i] - LINE_MAX)
                self.p_dir[i] = -1
            elif self.p_pos[i] < 0:
                self.p_pos[i] = -self.p_pos[i]
                self.p_dir[i] = 1

        # ---- 计算 LED 颜色 ----
        led_rgb = []
        for led in range(LED_NUM):
            r = g = b = 0
            for p in range(n):
                dist = abs(self.p_pos[p] - LED_POS[led])
                if dist < self.field_r:
                    inf = (self.field_r - dist) * 255 // self.field_r
                    inf = inf * self.p_intensity[p] // 255
                    cr, cg, cb = P_COLORS[p % len(P_COLORS)]
                    r += cr * inf // 255
                    g += cg * inf // 255
                    b += cb * inf // 255
            if self.beat_flash > 10:
                bf = self.beat_flash
                r += (255 - r) * bf // 255
                g += (255 - g) * bf // 255
                b += (255 - b) * bf // 255
            r = min(r, 255)
            g = min(g, 255)
            b = min(b, 255)
            led_rgb.append((r, g, b))

        return led_rgb


class LEDSimApp:
    def __init__(self):
        self.sim = ParticleSimulator()
        self.running = False
        self.after_id = None
        self.speed_sliders = []
        self.speed_labels = []
        self.p_count_label = None

        self.root = tk.Tk()
        self.root.title("AC696x LED 粒子效果模拟器")
        self.root.geometry(f"{WIN_W}x{WIN_H}")
        self.root.resizable(False, False)

        # Canvas
        self.cv = tk.Canvas(self.root, width=WIN_W, height=WIN_H - 200,
                            bg="#1a1a1a")
        self.cv.pack()

        # ======== 第一行：运行控制 ========
        ctrl1 = tk.Frame(self.root, bg="#333")
        ctrl1.pack(fill=tk.X)

        self.btn_start = tk.Button(ctrl1, text="▶ 运行", command=self.start,
                                   bg="#2a2a2a", fg="white", width=8)
        self.btn_start.pack(side=tk.LEFT, padx=3, pady=3)

        self.btn_stop = tk.Button(ctrl1, text="⏹ 停止", command=self.stop,
                                  bg="#2a2a2a", fg="white", width=8,
                                  state=tk.DISABLED)
        self.btn_stop.pack(side=tk.LEFT, padx=3, pady=3)

        self.btn_reset = tk.Button(ctrl1, text="↺ 重置", command=self.reset,
                                   bg="#2a2a2a", fg="white", width=8)
        self.btn_reset.pack(side=tk.LEFT, padx=3, pady=3)

        tk.Label(ctrl1, text="  音乐:", fg="white", bg="#333").pack(side=tk.LEFT)
        self.music_var = tk.BooleanVar(value=True)
        tk.Radiobutton(ctrl1, text="自动", variable=self.music_var,
                       value=True, bg="#333", fg="white",
                       selectcolor="#555").pack(side=tk.LEFT)
        tk.Radiobutton(ctrl1, text="手动", variable=self.music_var,
                       value=False, bg="#333", fg="white",
                       selectcolor="#555").pack(side=tk.LEFT)

        tk.Label(ctrl1, text="  能量:", fg="white", bg="#333").pack(side=tk.LEFT)
        self.energy_scale = tk.Scale(ctrl1, from_=0, to=120, orient=tk.HORIZONTAL,
                                     length=100, bg="#333", fg="white",
                                     highlightthickness=0, showvalue=0)
        self.energy_scale.set(50)
        self.energy_scale.pack(side=tk.LEFT)

        tk.Label(ctrl1, text="  低频:", fg="white", bg="#333").pack(side=tk.LEFT)
        self.low_scale = tk.Scale(ctrl1, from_=0, to=80, orient=tk.HORIZONTAL,
                                  length=80, bg="#333", fg="white",
                                  highlightthickness=0, showvalue=0)
        self.low_scale.set(20)
        self.low_scale.pack(side=tk.LEFT)

        tk.Button(ctrl1, text="💥节拍", command=self.trigger_beat,
                  bg="#2a2a2a", fg="white", width=6).pack(side=tk.LEFT, padx=5)

        self.state_label = tk.Label(ctrl1, text="状态: 正常", fg="#0f0",
                                    bg="#333", font=("Arial", 10, "bold"))
        self.state_label.pack(side=tk.LEFT, padx=8)

        # ======== 第二行：场半径 + 粒子数量 ========
        ctrl2 = tk.Frame(self.root, bg="#444")
        ctrl2.pack(fill=tk.X)

        tk.Label(ctrl2, text="  场半径:", fg="white", bg="#444").pack(side=tk.LEFT)
        self.field_r_scale = tk.Scale(ctrl2, from_=1, to=40, orient=tk.HORIZONTAL,
                                      length=120, bg="#444", fg="white",
                                      highlightthickness=0, showvalue=1)
        self.field_r_scale.set(self.sim.field_r)
        self.field_r_scale.pack(side=tk.LEFT)
        self.field_r_label = tk.Label(ctrl2, text=str(self.sim.field_r),
                                      fg="#ff0", bg="#444", width=3)
        self.field_r_label.pack(side=tk.LEFT)
        self.field_r_scale.config(command=lambda v: self.field_r_label.config(text=v))

        # 粒子数量控制
        tk.Label(ctrl2, text="  粒子:", fg="white", bg="#444").pack(side=tk.LEFT, padx=(20, 0))
        tk.Button(ctrl2, text="－", command=self.remove_particle,
                  bg="#2a2a2a", fg="white", width=3,
                  font=("Arial", 12)).pack(side=tk.LEFT, padx=2)
        self.p_count_label = tk.Label(ctrl2, text=str(self.sim.particle_count),
                                      fg="#0ff", bg="#444",
                                      font=("Arial", 12, "bold"), width=3)
        self.p_count_label.pack(side=tk.LEFT)
        tk.Button(ctrl2, text="＋", command=self.add_particle,
                  bg="#2a2a2a", fg="white", width=3,
                  font=("Arial", 12)).pack(side=tk.LEFT, padx=2)

        # ======== 第三行：各粒子速度 ========
        ctrl3 = tk.Frame(self.root, bg="#555")
        ctrl3.pack(fill=tk.X)

        colors = ["#f44", "#4f4", "#44f", "#ff4", "#f4f", "#4ff"]
        for i in range(MAX_PARTICLES):
            frame = tk.Frame(ctrl3, bg="#555")
            frame.pack(side=tk.LEFT, padx=2)

            name_label = tk.Label(frame, text=f"  {P_NAMES[i]}:",
                                  fg=colors[i], bg="#555", width=2)
            name_label.pack(side=tk.LEFT)

            s = tk.Scale(frame, from_=0, to=10, orient=tk.HORIZONTAL,
                         length=70, bg="#555", fg=colors[i],
                         highlightthickness=0, showvalue=0,
                         sliderrelief=tk.FLAT)
            s.set(2)
            s.pack(side=tk.LEFT)
            self.speed_sliders.append(s)

            lbl = tk.Label(frame, text="2", fg=colors[i], bg="#555", width=2)
            lbl.pack(side=tk.LEFT)
            self.speed_labels.append(lbl)
            s.config(command=lambda v, lb=lbl: lb.config(text=v))

        self._refresh_speed_style()

    # ---------- 粒子数量控制 ----------
    def _refresh_speed_style(self):
        """更新速度滑块的样式（激活/非激活）"""
        n = self.sim.particle_count
        colors_active = ["#f44", "#4f4", "#44f", "#ff4", "#f4f", "#4ff"]
        for i in range(MAX_PARTICLES):
            is_active = (i < n)
            fg = colors_active[i] if is_active else "#444"
            bg = "#555"  # 统一背景
            for child in self.speed_sliders[i].master.winfo_children():
                try:
                    child.config(fg=fg)
                except:
                    pass
            self.speed_sliders[i].config(fg=fg, bg=bg, troughcolor="#333" if is_active else "#2a2a2a")
        self.p_count_label.config(text=str(n))

    def add_particle(self):
        self.sim.add_particle()
        self._refresh_speed_style()
        idx = self.sim.particle_count - 1
        if idx < len(self.speed_sliders):
            self.speed_sliders[idx].set(self.sim.p_speed_base[idx])
            self.speed_labels[idx].config(text=str(self.sim.p_speed_base[idx]))

    def remove_particle(self):
        self.sim.remove_particle()
        self._refresh_speed_style()

    # ---------- 运行控制 ----------
    def start(self):
        self.running = True
        self.btn_start.config(state=tk.DISABLED)
        self.btn_stop.config(state=tk.NORMAL)
        self.sim.auto_music = not self.music_var.get()
        self._update()

    def stop(self):
        self.running = False
        if self.after_id:
            self.root.after_cancel(self.after_id)
            self.after_id = None
        self.btn_start.config(state=tk.NORMAL)
        self.btn_stop.config(state=tk.DISABLED)

    def reset(self):
        self.sim.reset()
        self.field_r_scale.set(self.sim.field_r)
        self.field_r_label.config(text=str(self.sim.field_r))
        for i in range(MAX_PARTICLES):
            v = self.sim.p_speed_base[i] if i < self.sim.particle_count else 2
            self.speed_sliders[i].set(v)
            self.speed_labels[i].config(text=str(v))
        self.energy_scale.set(50)
        self.low_scale.set(20)
        self._refresh_speed_style()
        self.draw_frame([(0, 0, 0)] * 4)

    def trigger_beat(self):
        self.sim.beat_flash = 255
        self.sim.low_energy = 60

    def _update(self):
        if not self.running:
            return

        # 从滑块同步参数
        self.sim.field_r = self.field_r_scale.get()
        n = self.sim.particle_count
        for i in range(n):
            self.sim.p_speed_base[i] = self.speed_sliders[i].get()

        if not self.sim.auto_music:
            self.sim.energy = self.energy_scale.get()
            self.sim.low_energy = self.low_scale.get()

        led_rgb = self.sim.step_simulate()
        self.draw_frame(led_rgb)
        self.update_state_label()

        self.after_id = self.root.after(30, self._update)

    # ---------- 绘制 ----------
    def map_x(self, pos):
        return LINE_X0 + pos * (LINE_X1 - LINE_X0) // LINE_MAX

    def draw_frame(self, led_rgb):
        self.cv.delete("all")
        self.cv.create_rectangle(0, 0, WIN_W, WIN_H, fill="#1a1a1a", outline="")

        # 0~255 线
        self.cv.create_line(LINE_X0, LINE_Y, LINE_X1, LINE_Y,
                            fill="#444", width=2)
        for v in range(0, 256, 32):
            x = self.map_x(v)
            self.cv.create_line(x, LINE_Y - 4, x, LINE_Y + 4, fill="#666")
            self.cv.create_text(x, LINE_Y + 18, text=str(v),
                                fill="#888", font=("Arial", 8))

        # 粒子的场范围
        n = self.sim.particle_count
        for p in range(n):
            x = self.map_x(self.sim.p_pos[p])
            r, g, b = P_COLORS[p % len(P_COLORS)]
            l = max(0, self.sim.p_pos[p] - self.sim.field_r)
            r2 = min(LINE_MAX, self.sim.p_pos[p] + self.sim.field_r)
            fx0 = self.map_x(l)
            fx1 = self.map_x(r2)
            color = f"#{r:02x}{g:02x}{b:02x}"
            self.cv.create_oval(fx0 - 15, LINE_Y - 15, fx1 + 15, LINE_Y + 15,
                                fill=color, stipple="gray25", outline="")

        # 粒子
        colors_hx = ["#f44", "#4f4", "#44f", "#ff4", "#f4f", "#4ff"]
        for p in range(n):
            x = self.map_x(self.sim.p_pos[p])
            r, g, b = P_COLORS[p % len(P_COLORS)]
            alpha = self.sim.p_intensity[p] / 255
            size = int(P_RADIUS * (0.5 + 0.5 * alpha))
            color = f"#{r:02x}{g:02x}{b:02x}"
            self.cv.create_oval(x - size, LINE_Y - size,
                                x + size, LINE_Y + size,
                                fill=color, outline="white", width=1)
            self.cv.create_text(x, LINE_Y, text=P_NAMES[p % len(P_NAMES)],
                                fill="white", font=("Arial", 9, "bold"))

        # LED
        for i in range(LED_NUM):
            x = self.map_x(LED_POS[i])
            r, g, b = led_rgb[i]
            self.cv.create_rectangle(x - 3, LINE_Y - 8, x + 3, LINE_Y + 8,
                                     fill="#aaa", outline="")
            self.cv.create_text(x, LINE_Y + 35, text=f"LED{i}",
                                fill="#aaa", font=("Arial", 9))
            color = f"#{r:02x}{g:02x}{b:02x}"
            gr = LED_RADIUS
            self.cv.create_oval(x - gr - 6, LED_DISPLAY_Y - gr - 6,
                                x + gr + 6, LED_DISPLAY_Y + gr + 6,
                                fill=color, stipple="gray25", outline="")
            self.cv.create_oval(x - gr, LED_DISPLAY_Y - gr,
                                x + gr, LED_DISPLAY_Y + gr,
                                fill=color, outline="white", width=2)
            self.cv.create_text(x, LED_DISPLAY_Y + gr + 16,
                                text=f"({r},{g},{b})",
                                fill="#ccc", font=("Arial", 8))

        # 信息
        info = [
            f"Step: {self.sim.step}",
            f"能量: {self.sim.energy:.0f}",
            f"低频: {self.sim.low_energy:.0f}",
            f"速度因子: {self.sim.speed_factor:.2f}",
            f"节拍: {self.sim.beat_flash}",
        ]
        for idx, text in enumerate(info):
            self.cv.create_text(WIN_W - 10, 20 + idx * 20,
                                text=text, fill="#aaa",
                                font=("Courier", 10), anchor=tk.NE)

    def update_state_label(self):
        names = {0: "😐 平静", 1: "🎵 正常", 2: "💥 节拍", 3: "🔥 高潮"}
        cols = {0: "#888", 1: "#0f0", 2: "#ff0", 3: "#f44"}
        s = self.sim.music_state
        self.state_label.config(text=f"状态: {names.get(s, '?')}",
                                fg=cols.get(s, "#fff"))

    def run(self):
        self.draw_frame([(0, 0, 0)] * 4)
        self.root.mainloop()


if __name__ == "__main__":
    app = LEDSimApp()
    app.run()
