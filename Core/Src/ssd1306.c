/**
 * SSD1306 OLED 驱动实现 (I2C, 128x64)
 * 适用于 STM32F103C8T6 + 4针 I2C OLED
 */

#include "ssd1306.h"

/* ========== 全局变量 ========== */
static I2C_HandleTypeDef *ssd1306_hi2c;
uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];  // 1024 字节显存

/* ========== 辅助: I2C 发送 ========== */
static void SSD1306_I2C_Write(uint8_t addr, uint8_t *data, uint16_t size)
{
    HAL_I2C_Mem_Write(ssd1306_hi2c, SSD1306_I2C_ADDR, addr, 1, data, size, 100);
}

/* ========== 写命令 ========== */
void SSD1306_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};  // 0x00 = 控制字节: 下一个字节是命令
    HAL_I2C_Master_Transmit(ssd1306_hi2c, SSD1306_I2C_ADDR, buf, 2, 100);
}

/* ========== 写数据 ========== */
void SSD1306_WriteData(uint8_t *data, uint16_t size)
{
    SSD1306_I2C_Write(0x40, data, size);  // 0x40 = 控制字节: 后续字节是数据
}

/* ========== 初始化序列 ========== */
void SSD1306_Init(I2C_HandleTypeDef *hi2c)
{
    ssd1306_hi2c = hi2c;

    // 等待上电稳定
    HAL_Delay(100);

    // -------- 初始化命令序列 --------
    SSD1306_WriteCmd(0xAE);  // 关闭显示

    SSD1306_WriteCmd(0x20);  // 设置内存寻址模式
    SSD1306_WriteCmd(0x00);  // 水平寻址模式

    SSD1306_WriteCmd(0xB0);  // 设置页起始地址 (PAGE0)

    SSD1306_WriteCmd(0xC8);  // COM扫描方向: 从COM63到COM0 (上下翻转用0xC0)

    SSD1306_WriteCmd(0x00);  // 列低起始地址
    SSD1306_WriteCmd(0x10);  // 列高起始地址

    SSD1306_WriteCmd(0x40);  // 显示起始行 = 0

    SSD1306_WriteCmd(0x81);  // 对比度设置
    SSD1306_WriteCmd(0xFF);  // 对比度 = 255 (最大)

    SSD1306_WriteCmd(0xA1);  // 段重映射: column 127 = SEG0 (左右翻转用0xA0)

    SSD1306_WriteCmd(0xA6);  // 正常显示 (非反显)

    SSD1306_WriteCmd(0xA8);  // 多路复用比
    SSD1306_WriteCmd(0x3F);  // 1/64 duty (64行)

    SSD1306_WriteCmd(0xA4);  // 全屏显示关闭 (跟随RAM内容)

    SSD1306_WriteCmd(0xD3);  // 显示偏移
    SSD1306_WriteCmd(0x00);  // 无偏移

    SSD1306_WriteCmd(0xD5);  // 显示时钟分频/振荡器频率
    SSD1306_WriteCmd(0xF0);  // 分频比1, 频率最高

    SSD1306_WriteCmd(0xD9);  // 预充电周期
    SSD1306_WriteCmd(0x22);  //

    SSD1306_WriteCmd(0xDA);  // COM引脚硬件配置
    SSD1306_WriteCmd(0x12);  // 替代引脚配置 (128x64用0x12)

    SSD1306_WriteCmd(0xDB);  // VCOMH反压级别
    SSD1306_WriteCmd(0x20);  // ~0.77 x VCC

    SSD1306_WriteCmd(0x8D);  // 电荷泵设置
    SSD1306_WriteCmd(0x14);  // 启用电荷泵 (DC/DC)

    SSD1306_WriteCmd(0xAF);  // 打开显示

    // 清屏
    SSD1306_Clear();
    SSD1306_UpdateScreen();
}

/* ========== 刷新屏幕 (缓冲区 → OLED) ========== */
void SSD1306_UpdateScreen(void)
{
    // 水平寻址模式: 连续写入所有1024字节
    SSD1306_WriteData(SSD1306_Buffer, SSD1306_BUFFER_SIZE);
}

/* ========== 清空缓冲区 ========== */
void SSD1306_Clear(void)
{
    memset(SSD1306_Buffer, 0x00, SSD1306_BUFFER_SIZE);
}

/* ========== 填充缓冲区 ========== */
void SSD1306_Fill(uint8_t color)
{
    memset(SSD1306_Buffer, color, SSD1306_BUFFER_SIZE);
}

/* ========== 画点 ========== */
void SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    if (color)
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    else
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

/* ========== 绘制位图 ========== */
void SSD1306_DrawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                        uint8_t w, uint8_t h, uint8_t color)
{
    int16_t byteWidth = (w + 7) / 8;  // 每行字节数
    for (int16_t row = 0; row < h; row++)
    {
        for (int16_t col = 0; col < w; col++)
        {
            // 位图按行组织, 每字节从高位到低位对应像素从左到右
            if (bitmap[row * byteWidth + (col / 8)] & (0x80 >> (col % 8)))
            {
                SSD1306_SetPixel(x + col, y + row, color);
            }
        }
    }
}

/* ========== 直接显示一帧 (已经是1024字节SSD1306格式) ========== */
void SSD1306_DisplayFrame(const uint8_t *frame)
{
    memcpy(SSD1306_Buffer, frame, SSD1306_BUFFER_SIZE);
    SSD1306_UpdateScreen();
}
