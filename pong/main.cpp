//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"
#include <ctime>
#include <cstdlib>

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);

}

void DrawLine(int x0, int y0, int z0, int x1, int y1, int z1)
{
    int deltax = abs(x1 - x0);
    int deltay = abs(y1 - y0);
    int error = 0;
    int deltaerr = (deltay + 1);
    int dirx, diry;
    if (deltax != 0)
    {
        dirx = (x1 - x0) / deltax;
    }
    else
    {
        dirx = 0;
    }
    if (deltay != 0)
    {
        diry = (y1 - y0) / deltay;
    }
    else
    {
        diry = 0;
    }
        
    if (deltax > deltay)
    {
        int y = y0;
        if (dirx > 0)
        {
            for (int x = x0; x < x1; x++)
            {
                SetPixel(window.context, x, y, RGB(255, 255, 255));
                error += deltaerr;
                if (error >= deltax + 1)
                {
                    y += diry;
                    error -= deltax + 1;
                }
            }
        }
        else
        {
            for (int x = x0; x > x1; x--)
            {
                SetPixel(window.context, x, y, RGB(255, 255, 255));
                error += deltaerr;
                if (error >= deltax + 1)
                {
                    y += diry;
                    error -= deltax + 1;
                }
            }
        }
    }
    else
    {
        int x = x0;
        if (diry > 0)
        {
            for (int y = y0; y < y1; y++)
            {
                SetPixel(window.context, x, y, RGB(255, 255, 255));
                error += deltaerr;
                if (error >= deltay + 1)
                {
                    x += dirx;
                    error -= deltay + 1;
                }
            }
        }
        else
        {
            for (int y = y0; y > y1; y--)
            {
                SetPixel(window.context, x, y, RGB(255, 255, 255));
                error += deltaerr;
                if (error >= deltay + 1)
                {
                    x += dirx;
                    error -= deltay + 1;
                }
            }
        }
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    const int fps = 60;
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        DrawLine(860, 440, 0, 1060, 440, 0);
        DrawLine(1060, 440, 0, 1060, 640, 0);
        DrawLine(1060, 640, 0, 860, 640, 0);
        DrawLine(860, 640, 0, 860, 440, 0);
        Sleep(1000. / fps);//ждем 16 милисекунд (1/количество кадров в секунду)
    }

}
