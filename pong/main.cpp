//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"
#include <ctime>
#include <cstdlib>

const float pi = 3.14159;

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

class Vector2 // a 2-dimensional vector
{
public:
    float x, y;
    Vector2()
    {
        x = 0;
        y = 0;
    }

    Vector2(float x0, float y0)
    {
        x = x0;
        y = y0;
    }
};

class Vector3 // a 3-dimensional vector
{
public:
    float x, y, z;
    Vector3()
    {
        x = 0;
        y = 0;
        z = 0;
    }

    Vector3(float x0, float y0, float z0)
    {
        x = x0;
        y = y0;
        z = z0;
    }
};

void DrawLine(Vector2 A, Vector2 B, Vector3 color) // draws a line from point A(x0, y0) to point B(x1, y1) of color RGB(r, g, b)
{
    int x0 = A.x;
    int y0 = A.y;
    int x1 = B.x;
    int y1 = B.y;
    int deltax = abs(x1 - x0);
    int deltay = abs(y1 - y0);
    int error = 0;
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
        int deltaerr = (deltay + 1);
        int y = y0;
        if (dirx > 0)
        {
            for (int x = x0; x < x1; x++)
            {
                SetPixel(window.context, x, y, RGB(color.x, color.y, color.z));
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
                SetPixel(window.context, x, y, RGB(color.x, color.y, color.z));
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
        int deltaerr = (deltax + 1);
        int x = x0;
        if (diry > 0)
        {
            for (int y = y0; y < y1; y++)
            {
                SetPixel(window.context, x, y, RGB(color.x, color.y, color.z));
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
                SetPixel(window.context, x, y, RGB(color.x, color.y, color.z));
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

Vector3 RotateVector(Vector3 point, char axis, float angle) // rotates a point in a 3-dimensional coordinate system around either of 3 axis (x, y, z) by an angle (in rad)
{
    if (axis == 'x')
    {
        float rotationMatrix[3][3] =
        {
            {1, 0, 0},
            {0, cos(angle), -sin(angle)},
            {0, sin(angle), cos(angle)}
        };
        return Vector3(point.x * rotationMatrix[0][0] + point.y * rotationMatrix[1][0] + point.z * rotationMatrix[2][0],
                       point.x * rotationMatrix[0][1] + point.y * rotationMatrix[1][1] + point.z * rotationMatrix[2][1],
                       point.x * rotationMatrix[0][2] + point.y * rotationMatrix[1][2] + point.z * rotationMatrix[2][2]);
    }
    else if (axis == 'y')
    {
        float rotationMatrix[3][3] =
        {
            {cos(angle), 0, sin(angle)},
            {0, 1, 0},
            {-sin(angle), 0, cos(angle)}
        };
        return Vector3(point.x * rotationMatrix[0][0] + point.y * rotationMatrix[1][0] + point.z * rotationMatrix[2][0],
                       point.x * rotationMatrix[0][1] + point.y * rotationMatrix[1][1] + point.z * rotationMatrix[2][1],
                       point.x * rotationMatrix[0][2] + point.y * rotationMatrix[1][2] + point.z * rotationMatrix[2][2]);
    }
    else if (axis == 'z')
    {
        float rotationMatrix[3][3] =
        {
            {cos(angle), -sin(angle), 0},
            {sin(angle), cos(angle), 0},
            {0, 0, 1}
        };
        return Vector3(point.x * rotationMatrix[0][0] + point.y * rotationMatrix[1][0] + point.z * rotationMatrix[2][0],
                       point.x * rotationMatrix[0][1] + point.y * rotationMatrix[1][1] + point.z * rotationMatrix[2][1],
                       point.x * rotationMatrix[0][2] + point.y * rotationMatrix[1][2] + point.z * rotationMatrix[2][2]);
    }
}

const int dist = 200;
Vector2 crd2scr(Vector3 O) // converts the point's coordinates in my coordinate system to the screen's one (Z doesn't do anything atm)
{
    return Vector2(window.width/2 + O.x*dist/(O.z+dist), window.height/2 - O.y*dist/(O.z+dist));
}

class Rect // a rectangle which can be drawn on screen by using its draw() method
{
public:
    Vector3 vert1, vert2, vert3, vert4;
    Rect()
    {
        vert1 = Vector3(-window.width / 4, window.height / 4, 0);
        vert2 = Vector3(window.width / 4, window.height / 4, 0);
        vert3 = Vector3(window.width / 4, -window.height / 4, 0);
        vert4 = Vector3(-window.width / 4, -window.height / 4, 0);
    } 

    Rect(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4)
    {
        vert1 = v1;
        vert2 = v2;
        vert3 = v3;
        vert4 = v4;
    }
    void rotate(char axis, float angle)
    {
        vert1 = RotateVector(vert1, axis, angle);
        vert2 = RotateVector(vert2, axis, angle);
        vert3 = RotateVector(vert3, axis, angle);
        vert4 = RotateVector(vert4, axis, angle);
    }
    void draw()
    {
        SetPixel(window.context, crd2scr(vert1).x, crd2scr(vert1).y, RGB(255, 0, 0));
        SetPixel(window.context, crd2scr(vert2).x, crd2scr(vert2).y, RGB(0, 255, 0));
        SetPixel(window.context, crd2scr(vert3).x, crd2scr(vert3).y, RGB(0, 0, 255));
        SetPixel(window.context, crd2scr(vert4).x, crd2scr(vert4).y, RGB(255, 255, 255));
        DrawLine(crd2scr(vert1), crd2scr(vert2), Vector3(255, 0, 0));
        DrawLine(crd2scr(vert2), crd2scr(vert3), Vector3(0, 255, 0));
        DrawLine(crd2scr(vert3), crd2scr(vert4), Vector3(0, 0, 255));
        DrawLine(crd2scr(vert4), crd2scr(vert1), Vector3(255, 255, 255));
    }
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    const int fps = 30;
    int totalFrames = 0;
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    ShowCursor(NULL);
    float rotationAngle = 0;

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        PatBlt(window.context, 0, 0, window.width, window.height, BLACKNESS);
        SetPixel(window.context, window.width / 2, window.height / 2, RGB(255, 255, 255));

        Rect myRect(Vector3(-100, 100, 0), Vector3(100, 100, 0), Vector3(100, -100, 0), Vector3(-100, -100, 0));
        //myRect.rotate('x', rotationAngle);
        myRect.rotate('y', rotationAngle);
        //myRect.rotate('z', rotationAngle);
        myRect.draw();

        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        rotationAngle += pi / 180;
        totalFrames += 1;
        Sleep(1000. / fps);//ждем 16 милисекунд (1/количество кадров в секунду)
    }
}
