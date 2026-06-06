/**
 * SSD1306 OLED 驱动头文件 (I2C, 128x64)
 * 适用于 STM32F103C8T6 + 4针 I2C OLED
 */

#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <string.h>

/* ========== SSD1306 参数 ========== */
#define SSD1306_I2C_ADDR         0x78  // OLED I2C 地址 (0x3C << 1)
#define SSD1306_WIDTH            128
#define SSD1306_HEIGHT           64
#define SSD1306_BUFFER_SIZE      (SSD1306_WIDTH * SSD1306_HEIGHT / 8)  // 1024 bytes

/* SSD1306 显存缓冲区 (全局, 供外部访问) */
extern uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

/* ========== 初始化 ========== */
void SSD1306_Init(I2C_HandleTypeDef *hi2c);

/* ========== 基础操作 ========== */
void SSD1306_WriteCmd(uint8_t cmd);
void SSD1306_WriteData(uint8_t *data, uint16_t size);
void SSD1306_UpdateScreen(void);          // 把缓冲区刷到屏幕
void SSD1306_Clear(void);                 // 清空缓冲区
void SSD1306_Fill(uint8_t color);         // 填充缓冲区 (0x00全黑 / 0xFF全白)

/* ========== 绘图函数 ========== */
void SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t color);
void SSD1306_DrawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                        uint8_t w, uint8_t h, uint8_t color);
void SSD1306_DisplayFrame(const uint8_t *frame);  // 直接显示一帧(已格式化为1024字节)

#endif /* __SSD1306_H__ */
