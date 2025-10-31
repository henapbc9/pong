//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"
#include <ctime>
#include <cstdlib>

const float pi = 3.14159;
int zBuffer[1920][1080];

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
/*
void DrawLine(Vector3 A, Vector3 B, Vector3 color) // draws a line from point A(x0, y0) to point B(x1, y1) of color RGB(r, g, b)
{
    int x0 = A.x;
    int y0 = A.y;
    int z0 = A.z;
    int x1 = B.x;
    int y1 = B.y;
    int z1 = B.z;
    int deltax = abs(x1 - x0);
    int deltay = abs(y1 - y0);
    int deltaz = abs(z1 - z0);
    int dirx, diry, dirz;
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
    if (deltaz != 0)
    {
        dirz = (z1 - z0) / deltaz;
    }
    else
    {
        dirz = 0;
    }

    if (deltax > deltay)
    {
        int error = 0;
        int deltaerr = (deltay + 1);
        int y = y0;
        int z = z0;
        if (dirx > 0)
        {
            for (int x = x0; x <= x1; x++)
            {
                if (z <= zBuffer[x][y] && x >= 0 && y >= 0 && x < 1920 && y < 1080)
                {
                    SetPixel(window.context, x, y, RGB(color.x, color.y, color.z));
                    zBuffer[x][y] = z;
                }
                error += deltaerr;
                if (error >= deltax + 1)
                {
                    y += diry;
                    z += dirz;
                    error -= deltax + 1;
                }
            }
        }
        else
        {
            for (int x = x0; x >= x1; x--)
            {
                if (z <= zBuffer[x][y] && x >= 0 && y >= 0 && x < 1920 && y < 1080)
                {
                    SetPixel(window.context, x, y, RGB(color.x, color.y, color.z));
                    zBuffer[x][y] = z;
                }
                error += deltaerr;
                if (error >= deltax + 1)
                {
                    y += diry;
                    z += dirz;
                    error -= deltax + 1;
                }
            }
        }
    }
    else
    {
        int error = 0;
        int deltaerr = (deltax + 1);
        int x = x0;
        int z = z0;
        if (diry > 0)
        {
            for (int y = y0; y <= y1; y++)
            {
                if (z <= zBuffer[x][y] && x >= 0 && y >= 0 && x < 1920 && y < 1080)
                {
                    SetPixel(window.context, x, y, RGB(color.x, color.y, color.z));
                    zBuffer[x][y] = z;
                }
                error += deltaerr;
                if (error >= deltay + 1)
                {
                    x += dirx;
                    z += dirz;
                    error -= deltay + 1;
                }
            }
        }
        else
        {
            for (int y = y0; y >= y1; y--)
            {
                if (z <= zBuffer[x][y] && x >= 0 && y >= 0 && x < 1920 && y < 1080)
                {
                    SetPixel(window.context, x, y, RGB(color.x, color.y, color.z));
                    zBuffer[x][y] = z;
                }
                error += deltaerr;
                if (error >= deltay + 1)
                {
                    x += dirx;
                    z += dirz;
                    error -= deltay + 1;
                }
            }
        }
    }
}
*/
void DrawLine(Vector3 A, Vector3 B, Vector3 color)
{
    int x0 = A.x;
    int y0 = A.y;
    int z0 = A.z;
    int x1 = B.x;
    int y1 = B.y;
    int z1 = B.z;
    float dx = x1 - x0;
    float dy = y1 - y0;
    float dz = z1 - z0;
    int steps = max(abs(dx), abs(dy));
    float xInc = dx / steps;
    float yInc = dy / steps;
    float zInc = dz / steps;

    float x = x0, y = y0, z = z0;
    for (int i = 0; i <= steps; i++) {
        int xi = round(x), yi = round(y);
        if (xi >= 0 && yi >= 0 && xi < window.width && yi < window.height) {
            if (z <= zBuffer[xi][yi]) {
                SetPixel(window.context, xi, yi, RGB(color.x, color.y, color.z));
                zBuffer[xi][yi] = z;
            }
        }
        x += xInc;
        y += yInc;
        z += zInc;
    }
}

void DrawTriangle(Vector3 vert1, Vector3 vert2, Vector3 vert3, Vector3 color)
{
    Vector3 A, B, C;
    if (vert1.y < vert2.y && vert1.y < vert3.y)
    {
        A = vert1;
        if (vert3.y < vert2.y)
        {
            B = vert3;
            C = vert2;
        }
        else
        {
            B = vert2;
            C = vert3;
        }
    }
    else if (vert2.y < vert1.y && vert2.y < vert3.y)
    {
        A = vert2;
        if (vert3.y < vert1.y)
        {
            B = vert3;
            C = vert1;
        }
        else
        {
            B = vert1;
            C = vert3;
        }
    }
    else if (vert3.y < vert1.y && vert3.y < vert2.y)
    {
        A = vert3;
        if (vert1.y < vert2.y)
        {
            B = vert1;
            C = vert2;
        }
        else
        {
            B = vert2;
            C = vert1;
        }
    }
    else
    {
        A = vert1;
        B = vert2;
        C = vert3;
    }

    for (int y = A.y; y <= C.y; y++)
    {
        int x1, x2;
        int z1, z2;
        if (A.y != C.y)
        {
            x1 = A.x + (y - A.y) * (C.x - A.x) / (C.y - A.y);
            z1 = A.z + (y - A.y) * (C.z - A.z) / (C.y - A.y);
        }
        else
        {
            x1 = A.x;
            z1 = A.z;
        }

        if (y < B.y)
        {
            if (A.y != B.y)
            {
                x2 = A.x + (y - A.y) * (B.x - A.x) / (B.y - A.y);
                z2 = A.z + (y - A.y) * (B.z - A.z) / (B.y - A.y);
            }
            else
            {
                x2 = A.x;
                z2 = A.z;
            }
        }
        else
        {
            if (C.y == B.y)
            {
                if (B.x > C.x)
                {
                    x2 = B.x;
                    z2 = B.z;
                }
                else
                {
                    x2 = C.x;
                    z2 = C.z;
                }
            }
            else
            {
                x2 = B.x + (y - B.y) * (C.x - B.x) / (C.y - B.y);
                z2 = B.z + (y - B.y) * (C.z - B.z) / (C.y - B.y);
            }
        }
        DrawLine(Vector3(x1, y, z1), Vector3(x2, y, z2), color);
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
Vector3 crd2scr(Vector3 O) // converts the point's coordinates in my coordinate system to the screen's one (Z acts as a depth variable)
{
    return Vector3(window.width / 2 + O.x * dist / (O.z + dist), window.height / 2 - O.y * dist / (O.z + dist), O.z);
}



class Rect // a rectangle which can be drawn on screen by using its draw() method
{
    Vector3 vert1, vert2, vert3, vert4;
public:
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

class Cuboid // a cuboid which can be drawn on screen by using its draw() method
{
    Vector3 vert1, vert2, vert3, vert4, vert5, vert6, vert7, vert8;
public:
    Cuboid()
    {
        vert1 = Vector3(-window.width / 4, window.height / 4, window.height / 4);
        vert2 = Vector3(window.width / 4, window.height / 4, window.height / 4);
        vert3 = Vector3(window.width / 4, -window.height / 4, window.height / 4);
        vert4 = Vector3(-window.width / 4, -window.height / 4, window.height / 4);
        vert5 = Vector3(-window.width / 4, window.height / 4, -window.height / 4);
        vert6 = Vector3(window.width / 4, window.height / 4, -window.height / 4);
        vert7 = Vector3(window.width / 4, -window.height / 4, -window.height / 4);
        vert8 = Vector3(-window.width / 4, -window.height / 4, -window.height / 4);
    }

    Cuboid(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4, Vector3 v5, Vector3 v6, Vector3 v7, Vector3 v8)
    {
        vert1 = v1;
        vert2 = v2;
        vert3 = v3;
        vert4 = v4;
        vert5 = v5;
        vert6 = v6;
        vert7 = v7;
        vert8 = v8;
    }
    void rotate(char axis, float angle)
    {
        vert1 = RotateVector(vert1, axis, angle);
        vert2 = RotateVector(vert2, axis, angle);
        vert3 = RotateVector(vert3, axis, angle);
        vert4 = RotateVector(vert4, axis, angle);
        vert5 = RotateVector(vert5, axis, angle);
        vert6 = RotateVector(vert6, axis, angle);
        vert7 = RotateVector(vert7, axis, angle);
        vert8 = RotateVector(vert8, axis, angle);
    }
    void draw()
    {
        //SetPixel(window.context, crd2scr(vert1).x, crd2scr(vert1).y, RGB(255, 0, 0));
        //SetPixel(window.context, crd2scr(vert2).x, crd2scr(vert2).y, RGB(0, 255, 0));
        //SetPixel(window.context, crd2scr(vert3).x, crd2scr(vert3).y, RGB(0, 0, 255));
        //SetPixel(window.context, crd2scr(vert4).x, crd2scr(vert4).y, RGB(255, 255, 255));
        //SetPixel(window.context, crd2scr(vert5).x, crd2scr(vert5).y, RGB(255, 255, 255));
        //SetPixel(window.context, crd2scr(vert6).x, crd2scr(vert6).y, RGB(0, 0, 255));
        //SetPixel(window.context, crd2scr(vert7).x, crd2scr(vert7).y, RGB(0, 255, 0));
        //SetPixel(window.context, crd2scr(vert8).x, crd2scr(vert8).y, RGB(255, 0, 0));
        
        //DrawLine(crd2scr(vert1), crd2scr(vert2), Vector3(255, 0, 0));
        //DrawLine(crd2scr(vert2), crd2scr(vert3), Vector3(0, 255, 0));
        //DrawLine(crd2scr(vert3), crd2scr(vert4), Vector3(0, 0, 255));
        //DrawLine(crd2scr(vert4), crd2scr(vert1), Vector3(255, 255, 255));

        //DrawLine(crd2scr(vert1), crd2scr(vert5), Vector3(255, 0, 0));
        //DrawLine(crd2scr(vert2), crd2scr(vert6), Vector3(0, 255, 0));
        //DrawLine(crd2scr(vert3), crd2scr(vert7), Vector3(0, 0, 255));
        //DrawLine(crd2scr(vert4), crd2scr(vert8), Vector3(255, 255, 255));

        //DrawLine(crd2scr(vert5), crd2scr(vert6), Vector3(255, 0, 0));
        //DrawLine(crd2scr(vert6), crd2scr(vert7), Vector3(0, 255, 0));
        //DrawLine(crd2scr(vert7), crd2scr(vert8), Vector3(0, 0, 255));
        //DrawLine(crd2scr(vert8), crd2scr(vert5), Vector3(255, 255, 255));

        //DrawTriangle(crd2scr(vert1), crd2scr(vert2), crd2scr(vert3), Vector3(255, 255, 255));
        //DrawTriangle(crd2scr(vert1), crd2scr(vert4), crd2scr(vert3), Vector3(255, 255, 255));

        //DrawTriangle(crd2scr(vert5), crd2scr(vert6), crd2scr(vert7), Vector3(255, 255, 255));
        //DrawTriangle(crd2scr(vert5), crd2scr(vert8), crd2scr(vert7), Vector3(255, 255, 255));

        DrawTriangle(crd2scr(vert1), crd2scr(vert2), crd2scr(vert6), Vector3(255, 0, 0));
        DrawTriangle(crd2scr(vert1), crd2scr(vert5), crd2scr(vert6), Vector3(255, 0, 0));

        DrawTriangle(crd2scr(vert4), crd2scr(vert3), crd2scr(vert7), Vector3(0, 255, 0));
        DrawTriangle(crd2scr(vert4), crd2scr(vert8), crd2scr(vert7), Vector3(0, 255, 0));

        DrawTriangle(crd2scr(vert1), crd2scr(vert5), crd2scr(vert8), Vector3(0, 0, 255));
        DrawTriangle(crd2scr(vert1), crd2scr(vert4), crd2scr(vert8), Vector3(0, 0, 255));

        DrawTriangle(crd2scr(vert2), crd2scr(vert6), crd2scr(vert7), Vector3(255, 255, 255));
        DrawTriangle(crd2scr(vert2), crd2scr(vert3), crd2scr(vert7), Vector3(255, 255, 255));

    }
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    const int fps = 120;
    int totalFrames = 0;
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    ShowCursor(NULL);
    float rotationAngle = 0;
    
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        PatBlt(window.context, 0, 0, window.width, window.height, BLACKNESS);
        SetPixel(window.context, window.width / 2, window.height / 2, RGB(255, 255, 255));

        for (int x = 0; x < 1920; x++)
        {
            for (int y = 0; y < 1080; y++)
            {
                zBuffer[x][y] = 10000;
            }
        }

        Cuboid myCuboid(Vector3(-100, 100, 100), Vector3(100, 100, 100), Vector3(100, -100, 100), Vector3(-100, -100, 100), Vector3(-100, 100, -100), Vector3(100, 100, -100), Vector3(100, -100, -100), Vector3(-100, -100, -100));
        //myCuboid.rotate('x', rotationAngle);
        myCuboid.rotate('y', rotationAngle);
        //myCuboid.rotate('z', rotationAngle);
        myCuboid.draw();


        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        rotationAngle += pi / 180;
        if (rotationAngle > 2 * pi) rotationAngle -= 2 * pi;
        totalFrames += 1;
        Sleep(1000. / fps);//ждем 16 милисекунд (1/количество кадров в секунду)
    }
}
