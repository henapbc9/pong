//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"
#include <ctime>
#include <cstdlib>

// секция данных игры  
typedef struct {
    float x, y, width, height, rad, dx, dy, speed;
    bool active, debugActive;
    HBITMAP hBitmap;//хэндл к спрайту шарика 
} sprite;

const int horizontalBlockCount = 10;
const int verticalBlockCount = 5;

sprite racket;//ракетка игрока
sprite blocks[horizontalBlockCount][verticalBlockCount];
sprite ball;//шарик

struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения

//cекция кода

const int fps = 120;
const float pi = 3.14159;
int freezeSpeed = 1;
void InitGame()
{
    srand(time(0));

    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------

    racket.width = window.width / 6;
    racket.height = window.height / 20;
    racket.speed = window.width / fps / freezeSpeed;//скорость перемещения ракетки
    racket.x = window.width / 2.;//ракетка посередине окна
    racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки

    for (int x = 0; x < horizontalBlockCount; x++) 
    {
        for (int y = 0; y < verticalBlockCount; y++)
        {
            blocks[x][y].width = window.width / horizontalBlockCount;
            blocks[x][y].height = window.height / verticalBlockCount / 3;
            blocks[x][y].x = blocks[x][y].width * x;
            blocks[x][y].y = blocks[x][y].height * y + window.height / 3;
            blocks[x][y].active = true;
            blocks[x][y].debugActive = true;
            if (x < horizontalBlockCount / 2 && y > verticalBlockCount / 2)
            {
                blocks[x][y].active = false;
                blocks[x][y].debugActive = false;
            }
        }
    }

    //ball.dy = -(rand() % 65 + 35) / 100.;//формируем вектор полета шарика
    //ball.dx = (rand() % 90 + 10) / 100. * pow(-1, rand());
    ball.dy = -0.5;
    ball.dx = 0.5;
    ball.speed = window.width / 3. / fps / freezeSpeed;
    ball.rad = window.width / 96.;
    ball.x = racket.x;//x координата шарика - на середие ракетки
    ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки

    game.score = 0;
    game.balls = 9;
    game.action = false;
}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
}

void ShowScore()
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//буфер для текста
    _itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));

    _itoa_s(ball.x, txt, 10);
    TextOutA(window.context, 10, 200, "X", 1);
    TextOutA(window.context, 200, 200, (LPCSTR)txt, strlen(txt));

    _itoa_s(ball.y, txt, 10);
    TextOutA(window.context, 10, 300, "Y", 1);
    TextOutA(window.context, 200, 300, (LPCSTR)txt, strlen(txt));

    _itoa_s(ball.dx, txt, 10);
    TextOutA(window.context, 10, 400, "DX", 2);
    TextOutA(window.context, 200, 400, (LPCSTR)txt, strlen(txt));

    _itoa_s(ball.dy, txt, 10);
    TextOutA(window.context, 10, 500, "DY", 2);
    TextOutA(window.context, 200, 500, (LPCSTR)txt, strlen(txt));
}

void ProcessInput()
{
    if (!GetAsyncKeyState(VK_LSHIFT))
    {
        freezeSpeed = 1;
    }
    else
    {
        freezeSpeed = 10;
    }

    racket.speed = window.width / fps / freezeSpeed;
    ball.speed = window.width / 3. / fps / freezeSpeed;

    if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;

    if (!game.action && GetAsyncKeyState(VK_SPACE))
    {
        game.action = true;
        ProcessSound("bounce.wav");
    }
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}

void ShowRacketBallAndBlocks()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ракетка игрока
    for (int x = 0; x < horizontalBlockCount; x++)
    {
        for (int y = 0; y < verticalBlockCount; y++)
        {
            if (blocks[x][y].active) {
                ShowBitmap(window.context, blocks[x][y].x, blocks[x][y].y, blocks[x][y].width, blocks[x][y].height, racket.hBitmap);
            }
        }
    }
    ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик
}

void LimitRacket()
{
    racket.x = max(racket.x, racket.width / 2.);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
    racket.x = min(racket.x, window.width - racket.width / 2.);//аналогично для правого угла
}

int ProcessRoom(void)
{
    float ballX = ball.x;
    float ballY = ball.y;
    float ballDx = ball.dx;
    float ballDy = ball.dy;
    float ballSpeed = ball.speed;
    float a = pow(ballDx * ballSpeed, 2);
    float b = pow(ballDy * ballSpeed, 2);
    float ballTravelDistance = sqrt(a + b);

    for (int depth = 0; depth < 150; depth++)
    {
        for (int i = 0; i < ballTravelDistance; i++)
        {
            bool collisionDone = false;
            float x1 = ballX + ballDx * ballSpeed / ballTravelDistance * i;
            float y1 = ballY + ballDy * ballSpeed / ballTravelDistance * i;
            
            if (depth > 0)
            {
                SetPixel(window.context, x1, y1, RGB(255, 255, 255));
            }
            else
            {
                SetPixel(window.context, x1, y1, RGB(0, 255, 0));
            }

            float ballVectorAngle = atan2(ballDy, ballDx);
            int spherePrecision = 7;
            for (int sphereTracePoint = 0; sphereTracePoint < spherePrecision; sphereTracePoint++)
            {
                float sphereVectorAngle = ballVectorAngle - pi / 2. + pi * sphereTracePoint / (spherePrecision - 1);
                float sphereX = x1 + ball.rad * cos(sphereVectorAngle);
                float sphereY = y1 + ball.rad * sin(sphereVectorAngle);

                if (i % 1 == 0)
                {
                    if (depth > 0)
                    {
                        SetPixel(window.context, sphereX, sphereY, RGB(0, 255, 255));
                    }
                    else
                    {
                        SetPixel(window.context, sphereX, sphereY, RGB(255, 0, 255));
                    }
                }

                if (!collisionDone)
                {
                    if (sphereY > window.height && depth == 0)
                    {
                        collisionDone = true;
                        game.balls--;

                        ProcessSound("fail.wav");

                        game.action = false;

                        ball.x = racket.x;
                        ball.y = racket.y - ball.rad;
                        ball.dy = -(rand() % 65 + 35) / 100.;//задаем новый случайный вектор для шарика
                        ball.dx = (rand() % 90 + 10) / 100. * pow(-1, rand());

                        if (game.balls < 0) {

                            MessageBoxA(window.hWnd, "game over", "", MB_OK);
                            InitGame();
                            return 0;
                        }
                        break;
                    }
                    else if (sphereX > racket.x - racket.width / 2. && sphereX < racket.x + racket.width / 2. && sphereY > racket.y && sphereY < racket.y + racket.height)
                    {
                        collisionDone = true;
                        ballDy *= -1;
                        ballY = window.height - racket.height;
                        ballY += 2. * (y1 - ballY);
                        if (game.action && depth == 0)
                        {
                            ProcessSound("bounce.wav");
                        }
                    }
                    else if (sphereY < 0)
                    {
                        collisionDone = true;
                        ballDy *= -1;
                        ballY += 2. * (y1 - ballY);
                        if (game.action && depth == 0)
                        {
                            ProcessSound("bounce.wav");
                        }
                    }
                    else if (sphereX < 0 || sphereX > window.width)
                    {
                        collisionDone = true;
                        ballDx *= -1;
                        ballX += 2. * (x1 - ballX);
                        if (game.action && depth == 0)
                        {
                            ProcessSound("bounce.wav");
                        }
                    }
                    else
                    {
                        for (int x = 0; x < horizontalBlockCount; x++)
                        {
                            for (int y = 0; y < verticalBlockCount; y++)
                            {
                                if (blocks[x][y].debugActive)
                                { // active block
                                    if (sphereX >= blocks[x][y].x && sphereX <= blocks[x][y].x + blocks[x][y].width && sphereY >= blocks[x][y].y && sphereY <= blocks[x][y].y + blocks[x][y].height)
                                    { // and checking if there is a collision with a ball
                                        collisionDone = true;
                                        float minhor = round(min(abs(x1 - blocks[x][y].x), abs(blocks[x][y].x + blocks[x][y].width - x1)));
                                        float minver = round(min(abs(y1 - blocks[x][y].y), abs(blocks[x][y].y + blocks[x][y].height - y1)));
                                        float what = minhor - minver;
                                        if (minhor < minver)
                                        { // horizontal
                                            if ((x > 0 && blocks[x - 1][y].debugActive && ballDx > 0) || (x < horizontalBlockCount - 1 && blocks[x + 1][y].debugActive && ballDx < 0))
                                            {
                                                ballDy *= -1;
                                                ballY += 2. * (y1 - ballY);
                                            }
                                            else
                                            {
                                                ballDx *= -1;
                                                ballX += 2. * (x1 - ballX);
                                            }
                                        }
                                        else
                                        { // or vertical
                                            if ((y > 0 && blocks[x][y - 1].debugActive && ballDy > 0) || (y < verticalBlockCount - 1 && blocks[x][y + 1].debugActive && ballDy < 0))
                                            {
                                                ballDx *= -1;
                                                ballX += 2. * (x1 - ballX);
                                            }
                                            else
                                            {
                                                ballDy *= -1;
                                                ballY += 2. * (y1 - ballY);
                                            }
                                        }
                                        if (game.action && depth == 0)
                                        {
                                                ProcessSound("bounce.wav");
                                                blocks[x][y].active = false; // removing the block the ball collided with
                                                game.score++; // adding 1 score
                                        }
                                        blocks[x][y].debugActive = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (depth == 0)
        {
            if (game.action)
            {
                ball.x = ballX + ballDx * ballSpeed;
                ball.y = ballY + ballDy * ballSpeed;
                ball.dx = ballDx;
                ball.dy = ballDy;
                ball.speed = ballSpeed;
            }
            else
            {
                ball.x = racket.x;
            }
        }
        ballX += ballDx * ballSpeed;
        ballY += ballDy * ballSpeed;
    }
    for (int x = 0; x < horizontalBlockCount; x++)
    {
        for (int y = 0; y < verticalBlockCount; y++)
        {
            if (blocks[x][y].active) blocks[x][y].debugActive = true;
        }
    }
}

void ProcessBall()
{
    if (game.action)
    {
        //если игра в активном режиме - перемещаем шарик
        ball.x += ball.dx * ball.speed;
        ball.y += ball.dy * ball.speed;
    }
    else
    {
        //иначе - шарик "приклеен" к ракетке
        ball.x = racket.x;
    }
}

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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    //mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        POINT ggg;
        GetCursorPos(&ggg);
        //ggg.x = 632;
        //ggg.y = 1006;
        //ball.x = ggg.x;
        //ball.y = ggg.y;
        ShowRacketBallAndBlocks();//рисуем фон, ракетку и шарик

        ProcessInput();//опрос клавиатуры
        LimitRacket();//проверяем, чтобы ракетка не убежала за экран
        ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в каретку
        
        ShowScore();//рисуем очик и жизни
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(1000. / fps);//ждем 16 милисекунд (1/количество кадров в секунду)

    }

}
