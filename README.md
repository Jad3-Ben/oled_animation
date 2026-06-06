# STM32F103C8T6 + SSD1306 I2C OLED 动图显示教程

## 📋 目录
1. [硬件连接](#硬件连接)
2. [STM32CubeIDE 工程配置](#stm32cubeide-工程配置)
3. [GIF 转换工具](#gif-转换工具)
4. [代码集成](#代码集成)
5. [播放动画](#播放动画)
6. [常见问题](#常见问题)

---

## 硬件连接

```
OLED 引脚        STM32F103C8T6 (蓝色小板 / 最小系统板)
─────────────────────────────────────────────────────
 VCC       →    3.3V
 GND       →    GND
 SCL       →    PB6  (I2C1 时钟)
 SDA       →    PB7  (I2C1 数据)
```

> 确认 OLED 是 SSD1306 芯片, 分辨率 128x64, I2C 接口 (4针)。

典型实物接线:
```
   OLED                STM32 (蓝色小板)
  ┌─────┐              ┌──────────┐
  │ VCC │──────────────│ 3.3V     │
  │ GND │──────────────│ GND      │
  │ SCL │──────────────│ PB6      │
  │ SDA │──────────────│ PB7      │
  └─────┘              └──────────┘
```

---

## STM32CubeIDE 工程配置

### 1. 新建项目
- 打开 STM32CubeIDE → **File → New → STM32 Project**
- 搜索芯片: **STM32F103C8T6**
- 输入工程名 (如 `oled_animation`) → **Finish**

### 2. 配置 I2C1
```
Pinout & Configuration → Connectivity → I2C1
  - I2C Mode:          I2C (不是 SMBus)
  - 勾选 PB6 为 SCL, PB7 为 SDA
```
在下方的 Parameter Settings 中:
| 参数              | 值    |
|-------------------|-------|
| I2C Speed Mode    | Fast Mode |
| I2C Speed Frequency | 400 KHz |

> Standard Mode (100KHz) 也可以, 只是刷屏慢一点。

### 3. 配置调试接口
```
Pinout & Configuration → System Core → SYS
  - Debug: Serial Wire (SWD)
```
> 如果用的是 ST-Link 下载器, 必须开 SWD!

### 4. 时钟配置
- 进入 **Clock Configuration** 标签页
- HCLK 设为 **72 MHz** (系统默认就是72MHz, 确认即可)
- 如果使用外部晶振, 确保 PLL 配置正确

### 5. 生成代码
- 按 **Ctrl+S** 保存配置
- 弹出对话框问是否生成代码 → **Yes**
- 或使用菜单: **Project → Generate Code**

生成后工程结构:
```
Core/
├── Inc/
│   ├── main.h
│   └── ... (其他头文件)
└── Src/
    ├── main.c
    └── ... (其他源文件)
```

---

## GIF 转换工具

### 安装依赖
```bash
pip install pillow
```

### 转换 GIF
```bash
cd D:\claud_gennerate\oled_animation

# 基本用法
python gif_to_bitmap.py your_animation.gif

# 指定输出文件名
python gif_to_bitmap.py your_animation.gif --out gif_frames.h

# 调整二值化阈值 (128 = 默认, 越小越暗)
python gif_to_bitmap.py your_animation.gif --threshold 100

# 减半帧数 (如果 Flash 不够用)
python gif_to_bitmap.py your_animation.gif --frame-skip 2
```

输出示例:
```
📂 读取 GIF: your_animation.gif
📊 提取到 15 帧
✅ 生成完成: gif_frames.h
   帧数: 15
   总大小: 15360 字节
   STM32F103C8T6 Flash: 64KB → 最多约 64 帧
```

### 容量参考
| GIF 帧数  | 占用 Flash | 占 STM32F103C8T6 总量 |
|-----------|-----------|----------------------|
| 10 帧     | 10 KB     | ~15%                 |
| 20 帧     | 20 KB     | ~31%                 |
| 30 帧     | 30 KB     | ~46%                 |
| 60 帧     | 60 KB     | ~93% ⚠️ 接近极限     |

> STM32F103C8T6 有 64KB Flash。代码本身约 2-5KB。建议 GIF 帧控制在 30 帧以内。

---

## 代码集成

### 第 1 步: 复制文件

将以下文件放入 CubeIDE 工程:

| 文件               | 放入位置          | 作用              |
|--------------------|-------------------|-------------------|
| `ssd1306.h`        | `Core/Inc/`       | OLED 驱动头文件   |
| `ssd1306.c`        | `Core/Src/`       | OLED 驱动实现     |
| `gif_frames.h`     | `Core/Inc/`       | GIF 帧数据        |
| `main.c`           | `Core/Src/`       | 主程序 (替换)     |

### 第 2 步: 确认 I2C 句柄名称

在 `Core/Src/i2c.c` (或 `main.c`) 中找到 I2C 句柄定义:
```c
I2C_HandleTypeDef hi2c1;  // ← 确认变量名是这个
```

如果变量名不是 `hi2c1`, 修改 `main.c` 中第 20 行的 `extern` 声明。

### 第 3 步: 检查 I2C 地址

大多数 SSD1306 OLED 的 I2C 地址是 `0x3C` (7位)。在 `ssd1306.h` 中:
```c
#define SSD1306_I2C_ADDR  0x78  // 0x3C << 1
```

如果想确认你屏幕的实际地址, 可以:
- 用 Arduino I2C Scanner 扫描
- 查看模块背面标记 (有的模块是 0x3D)

### 第 4 步: 编译烧录

1. **Project → Build All** (或按 `Ctrl+B`)
2. 连接 ST-Link / USB-TTL 烧录器
3. **Run → Run** (或按 F11 进入调试)

---

## 播放动画

工程内置了 3 种播放方式, 在 `main.c` 的 `while(1)` 中切换:

### 方式 1: 非阻塞播放 (推荐)

```c
while (1)
{
    Animation_Play_NonBlocking();  // 不卡主循环, 可同时处理其他任务
    // 其他任务...
}
```

自动根据 `frame_durations[]` 切换帧, 播完自动循环。

### 方式 2: 阻塞播放

```c
Animation_Play_Blocking(3);  // 循环播放 3 次后返回
```

简单但会阻塞 `HAL_Delay()`, 不适合有多个任务的情况。

### 方式 3: 弹跳小球 (无需 GIF)

```c
Animation_BouncingBall();  // 500 帧弹跳动画, 不需要任何外部数据
```

---

## 文件说明

| 文件              | 说明                                    |
|-------------------|----------------------------------------|
| `ssd1306.h`       | OLED 驱动 API: 初始化、画点、画图、刷屏 |
| `ssd1306.c`       | OLED 驱动实现, 使用 I2C 通信            |
| `gif_to_bitmap.py`| Python 脚本: GIF → C 帧数组            |
| `gif_frames.h`    | 脚本生成的帧数据 (需要自己运行脚本生成)  |
| `main.c`          | 主程序, 含 3 种播放方式和硬件初始化      |

---

## 常见问题

### Q1: 屏幕不亮, 完全没反应?
- 检查接线: VCC→3.3V (不是5V!), GND→GND, 不要接反
- 检查 I2C 地址: 试试 `0x7A` (对应的 0x3D << 1)
- 用逻辑分析仪或示波器抓 I2C 波形
- 先运行 `Animation_TestPattern()`, 看是否有任何反应

### Q2: 屏幕显示乱码或花屏?
- 确认 SSD1306_Init 的初始化序列完整
- 检查 I2C 速度: 先降回 100KHz 试试
- 检查 OLED 分辨率确实是 128x64

### Q3: 刷屏速度慢?
- I2C 设为 Fast Mode (400KHz)
- 400KHz 下每帧约 20ms, 可到约 50 FPS
- 如果还慢, 检查主循环中是否有其他耗时代码

### Q4: Flash 不够用?
- 减少 GIF 帧数
- 缩短 GIF (裁剪不重要的帧)
- 使用 SPI Flash 外挂存储 (如 W25Q32)
- 考虑换 STM32F103CBT6 (128KB Flash, 引脚兼容)

### Q5: 画面是反的 (上下颠倒)?
- 修改 `ssd1306.c` 中的 `0xC8` 改为 `0xC0` (COM扫描方向)
- 修改 `ssd1306.c` 中的 `0xA1` 改为 `0xA0` (段重映射)

### Q6: 画面是反色 (白底黑字 vs 黑底白字)?
- 在 `gif_to_bitmap.py` 中加 `--invert` 参数
- 或修改 `ssd1306.c` 中 `0xA6` 改为 `0xA7` (反显模式)

---

## 进阶: 存储更多帧

如果你的动图超过 30 帧, Flash 不够用, 有以下方案:

### 方案 A: 换大 Flash 的芯片
- STM32F103CBT6: 128KB Flash, 引脚完全兼容, 直接替换

### 方案 B: 外挂 SPI Flash (如 W25Q32)
- 4MB 存储, 可以存 4000+ 帧
- 使用 SPI2 读取帧数据到 RAM, 再显示
- 需要预先烧写 Flash 或用文件系统

### 方案 C: 运行时生成帧
- 不存完整帧, 用算法计算 (如弹跳球)
- 适合几何动画, 不适合复杂动图
