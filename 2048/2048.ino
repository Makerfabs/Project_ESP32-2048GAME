#include <SPI.h>
#include <Wire.h>
#include <LovyanGFX.hpp>
#include "makerfabs_pin.h"
#include "FT6236.h"

#define TAB_WIDTH 80
#define BEGIN_LEN 10
#define BUTTON_LEN 60
#define BUTTON_HIG 360

struct LGFX_Config
{
    static constexpr spi_host_device_t spi_host = ESP32_TSC_9488_LCD_SPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_sclk = ESP32_TSC_9488_LCD_SCK;
    static constexpr int spi_mosi = ESP32_TSC_9488_LCD_MOSI;
    static constexpr int spi_miso = ESP32_TSC_9488_LCD_MISO;
};

static lgfx::LGFX_SPI<LGFX_Config> TFT;
static LGFX_Sprite sprite(&TFT);
static lgfx::Panel_ILI9488 panel;

int a[4][4] = {0};
int empty;

void setup()
{
    Serial.begin(115200);

    Wire.begin(ESP32_TSC_9488_I2C_SDA, ESP32_TSC_9488_I2C_SCL);
    byte error, address;

    Wire.beginTransmission(TOUCH_I2C_ADD);
    error = Wire.endTransmission();

    if (error == 0)
    {
        Serial.print("I2C device found at address 0x");
        Serial.print(TOUCH_I2C_ADD, HEX);
        Serial.println("  !");
    }
    else if (error == 4)
    {
        Serial.print("Unknown error at address 0x");
        Serial.println(TOUCH_I2C_ADD, HEX);
    }

    set_tft();
    TFT.begin();
}

void loop()
{
    game_start();
    delay(3000);
    game_init();
    show_logo();
    play();
}

void game_init()
{
    int x, y;

    empty = 15;
    srand(100);     //<stdlib.h>初始化随机数发生器，生成随机数种子
    x = rand() % 4; //生成第一个2
    y = rand() % 4;
    a[y][x] = 2;
    TFT.fillScreen(TFT_BLACK);
    draw(); //绘制字符界面
    //draw_button();
}

void play()
{
    int x, y, i, new_x, new_y, temp;
    int old_empty, move;
    char ch;

    while (1)
    {
        move = 0;          //移动标志位
        old_empty = empty; //剩余空格
        //ch = getch();      //从键盘获取操作
        ch = get_direction();
        switch (ch)
        {
        case 97:  //左移a
        case 104: //h
        case 68:  //左方向键
        case '4':
            //左至右检测，如果遇到相邻相同值则累加到左数，并清零右数
            for (y = 0; y < 4; y++) //遍历行
                for (x = 0; x < 4;) //遍历列
                {
                    if (a[y][x] == 0) //检测位置是否为空
                    {
                        x++; //为空则跳到本行下一个
                        continue;
                    }
                    else
                    {
                        for (i = x + 1; i < 4; i++) //非空检测下一个非空数
                        {
                            if (a[y][i] == 0)
                            {
                                continue;
                            }
                            else
                            {
                                if (a[y][x] == a[y][i]) //如果x与i相符
                                {
                                    a[y][x] += a[y][i]; //将x与i合并
                                    a[y][i] = 0;        //将i清零
                                    empty++;            //空格+1
                                    break;              //跳出for
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                        x = i; //i为下一个x，继续执行检测。
                    }
                }
            //将累加后的格子全部左移，不留空格
            for (y = 0; y < 4; y++)
                for (x = 0; x < 4; x++)
                {
                    if (a[y][x] == 0)
                    {
                        continue;
                    }
                    else
                    {
                        for (i = x; (i > 0) && (a[y][i - 1] == 0); i--) //依次左移，知道左数非空或到最左
                        {
                            a[y][i - 1] = a[y][i];
                            a[y][i] = 0;
                            move = 1;
                        }
                    }
                }
            break;
            //下面分别是右上下三个方向的操作，只有顺序不同，逻辑想通
        case 100:
        case 108:
        case 67:
        case '6':
            for (y = 0; y < 4; y++)
                for (x = 3; x >= 0;)
                {
                    if (a[y][x] == 0)
                    {
                        x--;
                        continue;
                    }
                    else
                    {
                        for (i = x - 1; i >= 0; i--)
                        {
                            if (a[y][i] == 0)
                            {
                                continue;
                            }
                            else if (a[y][x] == a[y][i])
                            {
                                a[y][x] += a[y][i];
                                a[y][i] = 0;
                                empty++;
                                break;
                            }
                            else
                            {
                                break;
                            }
                        }
                        x = i;
                    }
                }
            for (y = 0; y < 4; y++)
                for (x = 3; x >= 0; x--)
                {
                    if (a[y][x] == 0)
                    {
                        continue;
                    }
                    else
                    {
                        for (i = x; (i < 3) && (a[y][i + 1] == 0); i++)
                        {
                            a[y][i + 1] = a[y][i];
                            a[y][i] = 0;
                            move = 1;
                        }
                    }
                }
            break;
        case 119:
        case 107:
        case 65:
        case '8':
            for (x = 0; x < 4; x++)
                for (y = 0; y < 4;)
                {
                    if (a[y][x] == 0)
                    {
                        y++;
                        continue;
                    }
                    else
                    {
                        for (i = y + 1; i < 4; i++)
                        {
                            if (a[i][x] == 0)
                            {
                                continue;
                            }
                            else if (a[y][x] == a[i][x])
                            {
                                a[y][x] += a[i][x];
                                a[i][x] = 0;
                                empty++;
                                break;
                            }
                            else
                            {
                                break;
                            }
                        }
                        y = i;
                    }
                }
            for (x = 0; x < 4; x++)
                for (y = 0; y < 4; y++)
                {
                    if (a[y][x] == 0)
                    {
                        continue;
                    }
                    else
                    {
                        for (i = y; (i > 0) && (a[i - 1][x] == 0); i--)
                        {
                            a[i - 1][x] = a[i][x];
                            a[i][x] = 0;
                            move = 1;
                        }
                    }
                }
            break;
        case 115:
        case 106:
        case 66:
        case '2':
            for (x = 0; x < 4; x++)
                for (y = 3; y >= 0;)
                {
                    if (a[y][x] == 0)
                    {
                        y--;
                        continue;
                    }
                    else
                    {
                        for (i = y - 1; i >= 0; i--)
                        {
                            if (a[i][x] == 0)
                            {
                                continue;
                            }
                            else if (a[y][x] == a[i][x])
                            {
                                a[y][x] += a[i][x];
                                a[i][x] = 0;
                                empty++;
                                break;
                            }
                            else
                            {
                                break;
                            }
                        }
                        y = i;
                    }
                }
            for (x = 0; x < 4; x++)
                for (y = 3; y >= 0; y--)
                {
                    if (a[y][x] == 0)
                    {
                        continue;
                    }
                    else
                    {
                        for (i = y; (i < 3) && (a[i + 1][x] == 0); i++)
                        {
                            a[i + 1][x] = a[i][x];
                            a[i][x] = 0;
                            move = 1;
                        }
                    }
                }
            break;
        case 'Q':
        case 'q':
            game_over();
            break;
        default:
            continue;
            break;
        }
        //无空格游戏结束
        if (empty <= 0)
            game_over();
        //如果有累加或移动操作，则随机添加一个2到空格
        if ((empty != old_empty) || (move == 1))
        {
            //找到一个空位
            do
            {
                new_x = rand() % 4;
                new_y = rand() % 4;
            } while (a[new_y][new_x] != 0);

            //随机生成2或4，赋给新空位
            do
            {
                temp = rand() % 4;
            } while (temp == 0 || temp == 2);
            a[new_y][new_x] = temp + 1;
            empty--;
        }
        draw(); //绘制游戏界面
        delay(1000);
    }
}

void game_over()
{
    TFT.fillScreen(TFT_BLACK);
    TFT.setTextColor(TFT_WHITE);
    TFT.setTextSize(4);
    // half width - num int * int width in pixels
    TFT.setCursor(40, 180);
    TFT.println("GAME OVER");
    while (1)
        ;
}

void game_start()
{
    TFT.setTextColor(TFT_WHITE);
    TFT.setTextSize(5);
    // half width - num int * int width in pixels
    TFT.setCursor(0, 100);
    TFT.println("MAKERFABS");
    TFT.setTextSize(2);
    TFT.setCursor(0, 200);
    TFT.println("ESP32 TOUCH CAMERA");
    TFT.setCursor(0, 250);
    TFT.println("2048 GAME");
    
}

void show_logo()
{
    TFT.setTextColor(TFT_WHITE);
    TFT.setTextSize(3);
    // half width - num int * int width in pixels
    TFT.setCursor(0, 350);
    TFT.println("MAKERFABS");
    TFT.setTextSize(2);
    TFT.setCursor(0, 410);
    TFT.println("ESP32 TOUCH CAMERA");
    TFT.setCursor(0, 430);
    TFT.println("2048 GAME");
}

void draw()
{
    int n, m, x, y;

    //TFT.fillRect(0, 0, 320, 320, TFT_BLACK);

    for (y = 0; y < 4; y++)
    {
        for (x = 0; x < 4; x++)
        {
            draw_one(y, x); //绘制数字
        }
    }
}

void draw_one(int x, int y)
{
    int i, posx, posy;
    i = a[y][x]; //从全局变量读取值
    int color = 0;

    switch (i)
    {
    case 0:
        color = TFT_DARKGREY;
        break;
    case 2:
        color = TFT_WHITE;
        break;
    case 4:
        color = TFT_CYAN;
        break;
    case 8:
        color = TFT_YELLOW;
        break;
    case 16:
        color = TFT_LIGHTGREY;
        break;
    case 32:
        color = TFT_BLUE;
        break;
    case 64:
        color = TFT_BROWN;
        break;
    case 128:
        color = TFT_MAROON;
        break;
    case 256:
        color = TFT_GREEN;
        break;
    case 512:
        color = TFT_RED;
        break;
    case 1024:
        color = TFT_GOLD;
        break;
    }

    posx = TAB_WIDTH * x;
    posy = TAB_WIDTH * y;

    TFT.setTextColor(0xff - color);
    TFT.setTextSize(3);

    TFT.fillRect(BEGIN_LEN + posx - 10, BEGIN_LEN + posy - 10, TAB_WIDTH - 10, TAB_WIDTH - 10, color);
    TFT.setCursor(BEGIN_LEN + posx, BEGIN_LEN + posy + 10);
    if (i != 0)
        TFT.println(i);
}

/*
void draw_button()
{
    TFT.setTextColor(TFT_BLACK);
    TFT.setTextSize(1);
    TFT.fillRect(10, BUTTON_HIG, BUTTON_LEN, BUTTON_LEN, TFT_WHITE);
    TFT.setCursor(10, BUTTON_HIG + 20);
    TFT.println("  UP");

    TFT.fillRect(90, BUTTON_HIG, BUTTON_LEN, BUTTON_LEN, TFT_WHITE);
    TFT.setCursor(90, BUTTON_HIG + 20);
    TFT.println("  DOWN");

    TFT.fillRect(170, BUTTON_HIG, BUTTON_LEN, BUTTON_LEN, TFT_WHITE);
    TFT.setCursor(170, BUTTON_HIG + 20);
    TFT.println("  LEFT");

    TFT.fillRect(250, BUTTON_HIG, BUTTON_LEN, BUTTON_LEN, TFT_WHITE);
    TFT.setCursor(250, BUTTON_HIG + 20);
    TFT.println("  RIGHT");
}
*/
char get_direction()
{
    char data = '5';
    int pos[2] = {0, 0};
    ft6236_pos(pos);
    if (80 < pos[0] && pos[0] < 240)
    {
        if (0 < pos[1] && pos[1] < 80)
        {
            data = '8';
        }
    }

    if (80 < pos[0] && pos[0] < 240)
    {
        if (240 < pos[1] && pos[1] < 320)
        {
            data = '2';
        }
    }

    if (pos[0] < 80 && pos[0] > 0)
    {
        if (80 < pos[1] && pos[1] < 240)
        {
            data = '4';
        }
    }

    if (pos[0] < 320 && pos[0] > 240)
    {
        if (80 < pos[1] && pos[1] < 240)
        {
            data = '6';
        }
    }
    return data;
}

void set_tft()
{
    // パネルクラスに各種設定値を代入していきます。
    // （LCD一体型製品のパネルクラスを選択した場合は、
    //   製品に合った初期値が設定されているので設定は不要です）

    // 通常動作時のSPIクロックを設定します。
    // ESP32のSPIは80MHzを整数で割った値のみ使用可能です。
    // 設定した値に一番近い設定可能な値が使用されます。
    panel.freq_write = 60000000;
    //panel.freq_write = 20000000;

    // 単色の塗り潰し処理時のSPIクロックを設定します。
    // 基本的にはfreq_writeと同じ値を設定しますが、
    // より高い値を設定しても動作する場合があります。
    panel.freq_fill = 60000000;
    //panel.freq_fill  = 27000000;

    // LCDから画素データを読取る際のSPIクロックを設定します。
    panel.freq_read = 16000000;

    // SPI通信モードを0~3から設定します。
    panel.spi_mode = 0;

    // データ読み取り時のSPI通信モードを0~3から設定します。
    panel.spi_mode_read = 0;

    // 画素読出し時のダミービット数を設定します。
    // 画素読出しでビットずれが起きる場合に調整してください。
    panel.len_dummy_read_pixel = 8;

    // データの読取りが可能なパネルの場合はtrueを、不可の場合はfalseを設定します。
    // 省略時はtrueになります。
    panel.spi_read = true;

    // データの読取りMOSIピンで行うパネルの場合はtrueを設定します。
    // 省略時はfalseになります。
    panel.spi_3wire = false;

    // LCDのCSを接続したピン番号を設定します。
    // 使わない場合は省略するか-1を設定します。
    panel.spi_cs = ESP32_TSC_9488_LCD_CS;

    // LCDのDCを接続したピン番号を設定します。
    panel.spi_dc = ESP32_TSC_9488_LCD_DC;

    // LCDのRSTを接続したピン番号を設定します。
    // 使わない場合は省略するか-1を設定します。
    panel.gpio_rst = ESP32_TSC_9488_LCD_RST;

    // LCDのバックライトを接続したピン番号を設定します。
    // 使わない場合は省略するか-1を設定します。
    panel.gpio_bl = ESP32_TSC_9488_LCD_BL;

    // バックライト使用時、輝度制御に使用するPWMチャンネル番号を設定します。
    // PWM輝度制御を使わない場合は省略するか-1を設定します。
    panel.pwm_ch_bl = -1;

    // バックライト点灯時の出力レベルがローかハイかを設定します。
    // 省略時は true。true=HIGHで点灯 / false=LOWで点灯になります。
    panel.backlight_level = true;

    // invertDisplayの初期値を設定します。trueを設定すると反転します。
    // 省略時は false。画面の色が反転している場合は設定を変更してください。
    panel.invert = false;

    // パネルの色順がを設定します。  RGB=true / BGR=false
    // 省略時はfalse。赤と青が入れ替わっている場合は設定を変更してください。
    panel.rgb_order = false;

    // パネルのメモリが持っているピクセル数（幅と高さ）を設定します。
    // 設定が合っていない場合、setRotationを使用した際の座標がずれます。
    // （例：ST7735は 132x162 / 128x160 / 132x132 の３通りが存在します）
    panel.memory_width = ESP32_TSC_9488_LCD_WIDTH;
    panel.memory_height = ESP32_TSC_9488_LCD_HEIGHT;

    // パネルの実際のピクセル数（幅と高さ）を設定します。
    // 省略時はパネルクラスのデフォルト値が使用されます。
    panel.panel_width = ESP32_TSC_9488_LCD_WIDTH;
    panel.panel_height = ESP32_TSC_9488_LCD_HEIGHT;

    // パネルのオフセット量を設定します。
    // 省略時はパネルクラスのデフォルト値が使用されます。
    panel.offset_x = 0;
    panel.offset_y = 0;

    // setRotationの初期化直後の値を設定します。
    panel.rotation = 0;

    // setRotationを使用した時の向きを変更したい場合、offset_rotationを設定します。
    // setRotation(0)での向きを 1の時の向きにしたい場合、 1を設定します。
    panel.offset_rotation = 0;

    // 設定を終えたら、LGFXのsetPanel関数でパネルのポインタを渡します。
    TFT.setPanel(&panel);
}