#include <Windows.h>
#include <d2d1.h>
#include <map>
#include <chrono>
#include <vector>
#include <wincodec.h>
#include <chrono>
#include <math.h>
#include <d2d1effects.h>
#include <d2d1_1.h>
#include <cmath>
#include <algorithm>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")

#define VK_Q 0x51
#define pi 3.141592653589793238462643383279502884197

// Contains all filenames necessary for the game to run. Condensed into variables to improve readability in later code.
// Made a struct just so I can collapse when I have more files later
struct {
    LPCWSTR playerFrame1 = L"Sprites\\Ship\\Ship1.png";
    LPCWSTR playerFrame2 = L"Sprites\\Ship\\Ship2.png";
    LPCWSTR background = L"Sprites\\Background\\Background.png";
    LPCWSTR playerRedLightffect1 = L"Sprites\\Ship\\Ship Effects\\Red_Light_1.png";
    LPCWSTR playerRedLightffect2 = L"Sprites\\Ship\\Ship Effects\\Red_Light_2.png";
    LPCWSTR playerRedLightffect3 = L"Sprites\\Ship\\Ship Effects\\Red_Light_3.png";
    LPCWSTR playerRedLightffect4 = L"Sprites\\Ship\\Ship Effects\\Red_Light_4.png";
    LPCWSTR playerRedLightffect5 = L"Sprites\\Ship\\Ship Effects\\Red_Light_5.png";
    LPCWSTR playerRedLightffect6 = L"Sprites\\Ship\\Ship Effects\\Red_Light_6.png";
    LPCWSTR playerRedLightffect7 = L"Sprites\\Ship\\Ship Effects\\Red_Light_7.png";
    LPCWSTR playerRedLightffect8 = L"Sprites\\Ship\\Ship Effects\\Red_Light_8.png";
    LPCWSTR playerRedLightffect9 = L"Sprites\\Ship\\Ship Effects\\Red_Light_9.png";
    LPCWSTR asteroid1 = L"Sprites\\Environment\\Asteroid_1.png";
    LPCWSTR pause = L"Sprites\\Menu\\Pause.png";
    LPCWSTR pauseBackground = L"Sprites\\Menu\\Pause_Background.png";
    LPCWSTR basicShotEffect1 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_1.png";
    LPCWSTR basicShotEffect2 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_2.png";
    LPCWSTR basicShotEffect3 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_3.png";
    LPCWSTR basicShotEffect4 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_4.png";
    LPCWSTR basicShotEffect5 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_5.png";
    LPCWSTR basicShotEffectBlue1 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Blue_1.png";
    LPCWSTR basicShotEffectBlue2 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Blue_2.png";
    LPCWSTR basicShotEffectBlue3 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Blue_3.png";
    LPCWSTR basicShotEffectBlue4 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Blue_4.png";
    LPCWSTR basicShotEffectBlue5 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Blue_5.png";
    LPCWSTR basicShotEffectPurple1 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Purple_1.png";
    LPCWSTR basicShotEffectPurple2 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Purple_2.png";
    LPCWSTR basicShotEffectPurple3 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Purple_3.png";
    LPCWSTR basicShotEffectPurple4 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Purple_4.png";
    LPCWSTR basicShotEffectPurple5 = L"Sprites\\Ship\\Ship Effects\\Basic_Shot_Effect_Purple_5.png";
    LPCWSTR basicShotDefault = L"Sprites\\Projectiles\\Basic_Shot_Default.png"; 
    LPCWSTR basicShotPurple = L"Sprites\\Projectiles\\Basic_Shot_Purple.png";
    LPCWSTR basicShotBlue = L"Sprites\\Projectiles\\Basic_Shot_Blue.png";
    LPCWSTR hitBox = L"Sprites\\Utilities\\hitBox.png";
    LPCWSTR hurtBox = L"Sprites\\Utilities\\hurtBox.png";
    LPCWSTR enemyShip1 = L"Sprites\\Enemies\\Enemy_Ship_1.png";
    LPCWSTR enemyShip1Damaged = L"Sprites\\Enemies\\Enemy_Ship_1_Damaged.png";
    LPCWSTR explosion1 = L"Sprites\\Effects\\Explosion_1.png";
    LPCWSTR doubleShotPickup = L"Sprites\\Pickups\\Double_Shot_Icon.png";
    LPCWSTR base = L"Sprites\\Enemies\\Base_Outside.png";
} files;

// The starting point for using Direct2D; it's what you use to create other Direct2D resources
ID2D1Factory* D2DFactory = NULL;

// Represents an object that can receive drawing commands.
ID2D1HwndRenderTarget* renderTarget = NULL;

// A map of bitmaps that will be used for rendering
std::map<LPCWSTR, ID2D1Bitmap*> bitmaps;

// System screen horizontal and vertical resolution
int screenX = GetSystemMetrics(SM_CXSCREEN);
int screenY = GetSystemMetrics(SM_CYSCREEN);

// Art style is 256x224 resolution, so this helps with scaling later
double scalerX = double(screenX) / ((double(screenX) / double(screenY)) * 224);
double scalerY = double(screenY) / 224;

// Art style is 4:3, so this helps with boundaries later
double leftBoundary = (screenX / 2) - (128 * scalerX);
double rightBoundary = (screenX / 2) + (128 * scalerX);

std::chrono::steady_clock::time_point lastLogicUpdate = std::chrono::steady_clock::now();

bool paused = false;

std::chrono::steady_clock::time_point pauseBuffer = std::chrono::steady_clock::now();

bool modulatorTicker = true;

// Booleans for key inputs
struct
{
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool space = false;
    bool lShift = false;
    bool escape = false;
    bool q = false;
} keys;

void GetDirectionalInput(int& xDir, int& yDir, bool right, bool left, bool down, bool up) {
    xDir = (right ? 1 : 0) - (left ? 1 : 0);
    yDir = (down ? 1 : 0) - (up ? 1 : 0);
}

// Represents any visible entity, created for later
class Object
{
public:
    double xPos = 1280;
    double yPos = 720;
    double xVel = 0;
    double yVel = 0;
    LPCWSTR currentFramePath = nullptr;
    double angleRadians = 0;
    double modulator = 0;
    double modulatorDelta = 0;
    bool modulatorPositiveDelta = true;
    struct {
        double left;
        double right;
        double up;
        double down;
    } hitbox ;

    bool damaged = false;
    std::chrono::steady_clock::time_point damageBegins;

    int power;
    int health;

    bool collided = false;
    std::chrono::steady_clock::time_point explosionBegin;
    
    Object() {}

    Object(LPCWSTR fileName) {
        currentFramePath = fileName;
    }

    void UpdateHitBox() {
        ID2D1Bitmap* object = bitmaps[currentFramePath];

        D2D1_SIZE_F size = object->GetSize();

        hitbox.left = xPos;
        hitbox.right = xPos + size.width;
        hitbox.up = yPos;
        hitbox.down = yPos + size.height;
    }

    void UpdateHurtBox() {
        int later = 0;
    }

    bool CheckCollision(Object &object) {
        if (hitbox.left < object.hitbox.left + (object.hitbox.right - object.hitbox.left) &&
            hitbox.left + (hitbox.right - hitbox.left) > object.hitbox.left &&
            hitbox.up < object.hitbox.up + (object.hitbox.down - object.hitbox.up) &&
            hitbox.up + (hitbox.down - hitbox.up) > object.hitbox.up)
        {
            object.damaged = true;
            object.damageBegins = std::chrono::steady_clock::now();
            object.health -= 1;
            return true;
        }
        return false;
    }
};

class Player : public Object {
public:
    int directionX, directionY;
    LPCWSTR redLightFrame = files.playerRedLightffect1;
    LPCWSTR basicShotFrame = nullptr;
    std::chrono::steady_clock::time_point lastFrameChange = std::chrono::steady_clock::now();
    std::chrono::nanoseconds frameInterval = std::chrono::nanoseconds(166666666); // closest I can get to 1/6th of a second
    std::chrono::steady_clock::time_point lastThrusterTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastRedLightTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point fullBrightStart = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastShotEffectTime = std::chrono::steady_clock::now();
    bool loadBullet = false;
    bool doubleShot = false;
    

    Player() {
        currentFramePath = files.playerFrame1;
    }

    void updatePlayerHitbox() {

    }

    //void PlayerIdle()
    //{
    //    // cycle through a basic animation
    //    if ((std::chrono::steady_clock::now() - lastFrameChange) >= frameInterval)
    //    {
    //        lastFrameChange = std::chrono::steady_clock::now();
    //        if (currentFramePath == files.playerFrame1)
    //        {
    //            currentFramePath = files.playerFrame2;
    //        }
    //        else
    //        {
    //            currentFramePath = files.playerFrame1;
    //        }
    //    }
    //}

    //void MovePlayer(double xDir, double yDir) {
    //    xPos += xDir;
    //    yPos += yDir;
    //    angle = -1 * atan2(xDir, xDir);
    //}
};

class Enemy : public Object {
public:

};

void LoadSpritesToMemory(HWND hWnd, std::vector<LPCWSTR> spriteFilePaths) {
    HRESULT hr = S_OK;

    // If no render target yet
    if (!renderTarget) {

        // make a windows render target based on screen dimensions
        D2D1_SIZE_U size = D2D1::SizeU(screenX, screenY);
        hr = D2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size),
            &renderTarget);
    }

    if (FAILED(hr)) {
        return; 
    }

    // used to convert PNGs to ID2D1Bitmaps
    IWICImagingFactory* bitmapFactory = NULL;
    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void**>(&bitmapFactory));

    if (FAILED(hr)) {
        return;
    }

    // convert each file from the vector of filepaths
    for (LPCWSTR filePath : spriteFilePaths) {

        // If the file path isn't already in the map
        if (bitmaps.find(filePath) == bitmaps.end()) {

            // creates decoder that reads image from filepath
            IWICBitmapDecoder* decoder = NULL;
            hr = bitmapFactory->CreateDecoderFromFilename(filePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);

            if (FAILED(hr)) {
                bitmapFactory->Release();
                return;
            }

            // PNGs will always have 1 frame, but a frame must be specified for converter->Initialize to work
            IWICBitmapFrameDecode* source = NULL;
            hr = decoder->GetFrame(0, &source);

            if (FAILED(hr)) {
                bitmapFactory->Release();
                decoder->Release();
                return;
            }

            // creating the converter
            IWICFormatConverter* converter = NULL;
            hr = bitmapFactory->CreateFormatConverter(&converter);

            if (FAILED(hr)) {
                bitmapFactory->Release();
                decoder->Release();
                source->Release();
                return;
            }

            // converts source image format to 32-bit RGB image with Alpha transparency
            hr = converter->Initialize(source, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeMedianCut);

            if (FAILED(hr)) {
                bitmapFactory->Release();
                decoder->Release();
                source->Release();
                converter->Release();
                return;
            }

            // Use converted file to ID2D1Bitmap that can be stored in our map
            ID2D1Bitmap* bitmap = NULL;
            hr = renderTarget->CreateBitmapFromWicBitmap(converter, NULL, &bitmap);

            if (SUCCEEDED(hr)) {
                bitmaps[filePath] = bitmap;
            }

            // Cleanup
            decoder->Release();
            source->Release();
            converter->Release();
        }
    }
    bitmapFactory->Release();
}

void ReleaseD2DResourcesFromMemory()
{
    // cleanup
    if (renderTarget)
    {
        renderTarget->Release();
        renderTarget = NULL;
    }

    for (auto& bitmap : bitmaps)
    {
        ID2D1Bitmap* bitmaps = bitmap.second;
        if (bitmaps)
        {
            bitmaps->Release();
        }
    }
    bitmaps.clear();
}

void Render(Player &player, std::vector<Object> &bullets, Object enemy, Object pickup) {

    renderTarget->BeginDraw();

    renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    if (!paused) {
        // Camera position
        D2D1_RECT_F cameraPos = D2D1::RectF(
            player.xPos - 128,
            player.yPos - 112,
            player.xPos + 128,
            player.yPos + 112
        );

        // Keeps stuff from being rendered outside boundaries
        D2D1_RECT_F aspectEnforcer = D2D1::RectF(leftBoundary, 0, rightBoundary, screenY);

        // Push the clipping rectangle onto the render target
        renderTarget->PushAxisAlignedClip(aspectEnforcer, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        // Pulls bitmap for background
        ID2D1Bitmap* backgroundBitmap = bitmaps[files.background];

        if (backgroundBitmap) {

            // Gets the size of the background 
            D2D1_SIZE_F size = backgroundBitmap->GetSize();
            D2D1_RECT_F screen = D2D1::RectF(leftBoundary, 0, rightBoundary, screenY);

            // Render a slice of the background equal to the camera coords, with no interpolation or transparency, to the defined display bounds
            renderTarget->DrawBitmap(backgroundBitmap, screen, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, cameraPos);
        }

        ID2D1Bitmap* bPickup = bitmaps[pickup.currentFramePath];
        if (bPickup) {
            D2D1_SIZE_F size = bPickup->GetSize();

            D2D1_RECT_F position = D2D1::RectF(
                ((screenX / 2) + ((pickup.xPos - player.xPos) * scalerX)) - ((size.width / 2) * scalerX),
                ((screenY / 2) + ((pickup.yPos - player.yPos) * scalerY)) - ((size.height / 2) * scalerY),
                ((screenX / 2) + ((pickup.xPos - player.xPos) * scalerX)) + ((size.width / 2) * scalerX),
                ((screenY / 2) + ((pickup.yPos - player.yPos) * scalerY)) + ((size.height / 2) * scalerY)
            );

            renderTarget->DrawBitmap(bPickup, position, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        // Ship and and effects will need to have uniform rotation, so setting that upfront...
        // Finds direction angle based on inputs
        double angleRadians = atan2(player.directionX, player.directionY);
        double angleDegrees = angleRadians * (180.0 / 3.14159265359);
        player.angleRadians = angleRadians + (0 * 0.785398163397448);
        D2D1_POINT_2F center = D2D1::Point2F(screenX / 2, screenY / 2);

        ID2D1Bitmap* baseBitmap = bitmaps[files.base];

        if (baseBitmap) {
            D2D1_SIZE_F size = baseBitmap->GetSize();

            D2D1_RECT_F position = D2D1::RectF(
                ((screenX / 2) + ((1800 - player.xPos) * scalerX)) - ((size.width / 2) * scalerX),
                ((screenY / 2) + ((720 - player.yPos) * scalerY)) - ((size.height / 2) * scalerY),
                ((screenX / 2) + ((1800 - player.xPos) * scalerX)) + ((size.width / 2) * scalerX),
                ((screenY / 2) + ((720 - player.yPos) * scalerY)) + ((size.height / 2) * scalerY)
            );

            renderTarget->DrawBitmap(baseBitmap, position, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }


        // Rotates sprite. Makes art less consistent but saves time
        D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(angleDegrees, center);

        LPCWSTR filename;
        if (enemy.damaged) {
            enemy.currentFramePath = files.enemyShip1Damaged;
        }
        ID2D1Bitmap* enemyBitmap = bitmaps[enemy.currentFramePath];

        if (enemyBitmap) {
            D2D1_SIZE_F size = enemyBitmap->GetSize();

            D2D1_RECT_F position = D2D1::RectF(
                ((screenX / 2) + ((enemy.xPos - player.xPos) * scalerX)) - ((size.width / 2) * scalerX),
                ((screenY / 2) + ((enemy.yPos - player.yPos) * scalerY)) - ((size.height / 2) * scalerY),
                ((screenX / 2) + ((enemy.xPos - player.xPos) * scalerX)) + ((size.width / 2) * scalerX),
                ((screenY / 2) + ((enemy.yPos - player.yPos) * scalerY)) + ((size.height / 2) * scalerY)
            );

            double angleRadians1 = atan2(player.yPos - enemy.yPos, player.xPos - enemy.xPos) + (1.5707963267948966192313216916398);
            double angleDegrees1 = angleRadians1 * (180.0 / 3.141592653589793238462643383279502884197);
            D2D1_POINT_2F center1 = D2D1::Point2F((position.right + position.left) / 2, (position.bottom + position.top) / 2);

            // Rotates sprite. Makes art less consistent but saves time
            D2D1_MATRIX_3X2_F rotation1 = D2D1::Matrix3x2F::Rotation(angleDegrees1, center1);

            // Rotates bitmap based on rotation calculations
            renderTarget->SetTransform(rotation1);
            renderTarget->DrawBitmap(enemyBitmap, position, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

            //ID2D1Bitmap* hitBox = bitmaps[files.hitBox];

            //if (hitBox) {
            //    // Resets the rotation transformation on renderTarget
            //    renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
            //    D2D1_RECT_F perimeter = D2D1::RectF(
            //        ((screenX / 2) + ((enemy.xPos - player.xPos) * scalerX)) - ((size.width / 2) * scalerX),
            //        ((screenY / 2) + ((enemy.yPos - player.yPos) * scalerY)) - ((size.height / 2) * scalerY),
            //        ((screenX / 2) + ((enemy.xPos - player.xPos) * scalerX)) + ((size.width / 2) * scalerX),
            //        ((screenY / 2) + ((enemy.yPos - player.yPos) * scalerY)) + ((size.height / 2) * scalerY)
            //    );

            //    renderTarget->DrawBitmap(hitBox, perimeter, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
            //}
            // Resets the rotation transformation on renderTarget
            renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        }

        // Rotates bitmap based on rotation calculations
        renderTarget->SetTransform(rotation);

        // Pulls bitmap for player ship
        ID2D1Bitmap* playerBitmap = bitmaps[player.currentFramePath];

        if (playerBitmap) {
            D2D1_SIZE_F size = playerBitmap->GetSize();

            // Correct ship size basd on scale and screen position
            D2D1_RECT_F shipDisplayPosition = D2D1::RectF(
                (screenX / 2) - ((size.width / 2) * scalerX),
                (screenY / 2) - ((size.height / 2) * scalerY),
                (screenX / 2) + ((size.width / 2) * scalerX),
                (screenY / 2) + ((size.height / 2) * scalerY)
            );

            // Render bitmap at display position with no transparency or interpolation
            renderTarget->DrawBitmap(playerBitmap, shipDisplayPosition, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        ID2D1Bitmap* redLightBitmap = bitmaps[player.redLightFrame];

        if (redLightBitmap) {
            D2D1_SIZE_F size = redLightBitmap->GetSize();

            D2D1_RECT_F displayPos1 = D2D1::RectF(
                (screenX / 2) - (((size.width / 2) - 4) * scalerX),
                (screenY / 2) - (((size.height / 2) + 2) * scalerY),
                (screenX / 2) + (((size.width / 2) + 4) * scalerX),
                (screenY / 2) + (((size.height / 2) - 2) * scalerY)
            );

            D2D1_RECT_F displayPos2 = D2D1::RectF(
                (screenX / 2) - (((size.width / 2) + 4) * scalerX),
                (screenY / 2) - (((size.height / 2) + 2) * scalerY),
                (screenX / 2) + (((size.width / 2) - 4) * scalerX),
                (screenY / 2) + (((size.height / 2) - 2) * scalerY)
            );

            renderTarget->DrawBitmap(redLightBitmap, displayPos1, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

            renderTarget->DrawBitmap(redLightBitmap, displayPos2, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        ID2D1Bitmap* basicShotEffectBitmap = bitmaps[player.basicShotFrame];

        if (basicShotEffectBitmap) {
            D2D1_SIZE_F size = basicShotEffectBitmap->GetSize();

            D2D1_RECT_F displayPos = D2D1::RectF(
                (screenX / 2) - ((size.width / 2) * scalerX),
                (screenY / 2) - (((size.height / 2) + 8) * scalerY),
                (screenX / 2) + ((size.width / 2) * scalerX),
                (screenY / 2) + (((size.height / 2) - 8) * scalerY)
            );

            renderTarget->DrawBitmap(basicShotEffectBitmap, displayPos, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        // Resets the rotation transformation on renderTarget
        renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        for (auto bullet : bullets) {

            ID2D1Bitmap* basicShotBitmap = bitmaps[bullet.currentFramePath];

            if (basicShotBitmap) {
                D2D1_SIZE_F size = basicShotBitmap->GetSize();

                D2D1_RECT_F position = D2D1::RectF(
                    ((screenX / 2) + ((bullet.xPos - player.xPos) * scalerX)) - ((size.width / 2) * scalerX),
                    ((screenY / 2) + ((bullet.yPos - player.yPos) * scalerY)) - ((size.height / 2) * scalerY),
                    ((screenX / 2) + ((bullet.xPos - player.xPos) * scalerX)) + ((size.width / 2) * scalerX),
                    ((screenY / 2) + ((bullet.yPos - player.yPos) * scalerY)) + ((size.height / 2) * scalerY)
                );

                double angleDegrees = bullet.angleRadians * (180.0 / 3.14159265359);
                D2D1_POINT_2F center = D2D1::Point2F((position.right + position.left) / 2, (position.top + position.bottom) / 2);

                // Rotates sprite. Makes art less consistent but saves time
                D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(angleDegrees, center);

                // Rotates bitmap based on rotation calculations
                renderTarget->SetTransform(rotation);

                // 
                renderTarget->DrawBitmap(basicShotBitmap, position, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);


                // Resets the rotation transformation on renderTarget
                renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

                //ID2D1Bitmap* hurtBox = bitmaps[files.hitBox];

                //if (hurtBox) {
                //    D2D1_RECT_F perimeter = D2D1::RectF(
                //        ((screenX / 2) + ((bullet.hitbox.left - player.xPos) * scalerX)) - (((bullet.hitbox.right - bullet.hitbox.left) / 2) * scalerX),
                //        ((screenY / 2) + ((bullet.hitbox.up - player.yPos) * scalerY)) - (((bullet.hitbox.down - bullet.hitbox.up) / 2) * scalerY),
                //        ((screenX / 2) + ((bullet.hitbox.left - player.xPos) * scalerX)) + (((bullet.hitbox.right - bullet.hitbox.left) / 2) * scalerX),
                //        ((screenY / 2) + ((bullet.hitbox.up - player.yPos) * scalerY)) + (((bullet.hitbox.down - bullet.hitbox.up) / 2) * scalerY)
                //    );

                //    // Rotates bitmap based on rotation calculations
                //    renderTarget->SetTransform(rotation);

                //    // 
                //    renderTarget->DrawBitmap(hurtBox, perimeter, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
                //    
                //    // Resets the rotation transformation on renderTarget
                //    renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
                //}
            }
        }

        //ID2D1Bitmap* asteroid = bitmaps[files.asteroid1];
        //if (asteroid) {
        //    D2D1_SIZE_F size = asteroid->GetSize();

        //    D2D1_RECT_F shipDisplayPosition = D2D1::RectF(
        //        (player.xPos) - ((size.width / 2) * scalerX),
        //        (player.yPos) - ((size.height / 2) * scalerY),
        //        (player.xPos) + ((size.width / 2) * scalerX),
        //        (player.yPos) + ((size.height / 2) * scalerY)
        //    );

        //    renderTarget->DrawBitmap(asteroid, shipDisplayPosition, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        //}

        // Pop the clipping rectangle off the render target
        renderTarget->PopAxisAlignedClip();
    } else {

        ID2D1Bitmap* pauseBackground = bitmaps[files.pauseBackground];
        if (pauseBackground) {
            D2D_RECT_F position = D2D1::RectF(leftBoundary, 0, rightBoundary, screenY);
            renderTarget->DrawBitmap(pauseBackground, position, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        ID2D1Bitmap* pause = bitmaps[files.pause];
        if (pause) {
            D2D1_SIZE_F size = pause->GetSize();

            D2D_RECT_F position = D2D1::RectF(
                (screenX / 2) - ((size.width / 2) * scalerX),
                (screenY / 2) - ((size.height / 2) * scalerY),
                (screenX / 2) + ((size.width / 2) * scalerX),
                (screenY / 2) + ((size.height / 2) * scalerY)
            );

            renderTarget->DrawBitmap(pause, position, 1.0F, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }
    }

    HRESULT hr = S_OK;
    hr = renderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        ReleaseD2DResourcesFromMemory();
    }
}

LRESULT CALLBACK ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

std::vector<LPCWSTR> spriteFilePaths;
std::vector<Object> bullets;
Object enemy;
Player player;
Object pickup;


void UpdateGameLogic(double deltaSeconds) {

    if (!paused) {
        double boost = 1;
        if (keys.lShift) {
            boost = 1.4;
        }
        if (keys.up) {
            player.yPos -= (boost * deltaSeconds);
        }
        if (keys.down) {
            player.yPos += (boost * deltaSeconds);
        }
        if (keys.right) {
            player.xPos += (boost * deltaSeconds);
        }
        if (keys.left) {
            player.xPos -= (boost * deltaSeconds);
        }

        if (keys.right || keys.left || keys.up || keys.down) {
            player.directionX = keys.right - keys.left;
            player.directionY = keys.up - keys.down;
        }

        if (std::chrono::steady_clock::now() - player.lastThrusterTime >= std::chrono::milliseconds(100)) {
            player.lastThrusterTime = std::chrono::steady_clock::now();
            if (player.currentFramePath == files.playerFrame1) {
                player.currentFramePath = files.playerFrame2;
            }
            else {
                player.currentFramePath = files.playerFrame1;
            }
        }

        // Ship's Red Lights effects
        if ((std::chrono::steady_clock::now() - player.lastRedLightTime) >= std::chrono::milliseconds(100)) {
            player.lastRedLightTime = std::chrono::steady_clock::now();
            if (player.redLightFrame == files.playerRedLightffect1) {
                player.redLightFrame = files.playerRedLightffect2;
            }
            else if (player.redLightFrame == files.playerRedLightffect2) {
                player.redLightFrame = files.playerRedLightffect3;
            }
            else if (player.redLightFrame == files.playerRedLightffect3) {
                player.redLightFrame = files.playerRedLightffect4;
            }
            else if (player.redLightFrame == files.playerRedLightffect4) {
                player.redLightFrame = files.playerRedLightffect5;
            }
            else if (player.redLightFrame == files.playerRedLightffect5) {
                player.redLightFrame = files.playerRedLightffect6;
            }
            else if (player.redLightFrame == files.playerRedLightffect6) {
                player.redLightFrame = files.playerRedLightffect7;
                player.fullBrightStart = std::chrono::steady_clock::now();
            }
            else if (std::chrono::steady_clock::now() - player.fullBrightStart >= std::chrono::milliseconds(500)) {
                player.redLightFrame = files.playerRedLightffect1;
            }
        }

        // Ship's shot effects
        if (keys.space && !player.basicShotFrame && 
            (std::chrono::steady_clock::now() - player.lastShotEffectTime >= std::chrono::milliseconds(250))) {
            player.lastShotEffectTime = std::chrono::steady_clock::now();

            double xPosition, yPosition;
            if (player.doubleShot) {
                double xOffset(0), yOffset(0);
                xOffset = (-1.8 * sin(player.angleRadians * 2)) - 4;
                yOffset = 4 - ((4 / sin(pi/4)) * sin(player.angleRadians));
                if (player.angleRadians > -pi / 6 && player.angleRadians < pi / 6) {
                    xOffset = -4;
                    yOffset = 4;
                } else if (player.angleRadians > (pi / 6) && player.angleRadians < (pi / 3)) { 
                    xOffset = -5.6568542494923801952067548968388;
                    yOffset = 0;
                } else if (player.angleRadians > (pi / 3) && player.angleRadians < (2 * pi / 3)) {
                    xOffset = -4;
                    yOffset = 4;
                } else if (player.angleRadians > (2 * pi / 3) && player.angleRadians < (5 * pi / 6)) {
                    xOffset = 0;
                    yOffset = -5.6568542494923801952067548968388;
                } else if (player.angleRadians > (5 * pi / 6) && player.angleRadians < (7 * pi / 6)) {
                    xOffset = 4;
                    yOffset = -4;
                } else if (player.angleRadians > (-5 * pi / 6) && player.angleRadians < (-2 * pi / 3)) {
                    xOffset = 5.6568542494923801952067548968388;
                    yOffset = 0;
                } else if (player.angleRadians > (-2 * pi / 3) && player.angleRadians < (-pi / 3)) {
                    xOffset = 4;
                    yOffset = 4;
                } else if (player.angleRadians > (-pi / 3) && player.angleRadians < 0) {
                    xOffset = 0;
                    yOffset = 5.6568542494923801952067548968388;
                }
                bullets.emplace_back(files.basicShotDefault);
                bullets.back().xPos = player.xPos + xOffset + (11 * sin(player.angleRadians));
                bullets.back().yPos = player.yPos + yOffset - (11 * cos(player.angleRadians));
                bullets.back().UpdateHitBox();
                bullets.back().angleRadians = player.angleRadians;
                bullets.back().yVel = round(-cos(player.angleRadians) * 100) / 100;
                if (abs(bullets.back().yVel) < 0.0001) {
                    bullets.back().yVel = 0;
                }
                bullets.back().xVel = round(sin(player.angleRadians) * 100) / 100;
                if (abs(bullets.back().xVel) < 0.0001) {
                    bullets.back().xVel = 0;
                }

                
                if (player.angleRadians > -pi / 6 && player.angleRadians < pi / 6) {
                    xOffset = 4;
                    yOffset = 4;
                } else if (player.angleRadians > (pi / 6) && player.angleRadians < (pi / 3)) {
                    xOffset = 0;
                    yOffset = 5.6568542494923801952067548968388;
                } else if (player.angleRadians > (pi / 3) && player.angleRadians < (2 * pi / 3)) {
                    xOffset = -4;
                    yOffset = -4;
                } else if (player.angleRadians > (2 * pi / 3) && player.angleRadians < (5 * pi / 6)) {
                    xOffset = -5.6568542494923801952067548968388;
                    yOffset = 0;
                } else if (player.angleRadians > (5 * pi / 6) && player.angleRadians < (7 * pi / 6)) {
                    xOffset = -4;
                    yOffset = -4;
                } else if (player.angleRadians > (-5 * pi / 6) && player.angleRadians < (-2 * pi / 3)) {
                    xOffset = 0;
                    yOffset = -5.6568542494923801952067548968388;
                } else if (player.angleRadians > (-2 * pi / 3) && player.angleRadians < (-pi / 3)) {
                    xOffset = 4;
                    yOffset = -4;
                } else if (player.angleRadians > (-pi / 3) && player.angleRadians < 0) {
                    xOffset = 5.6568542494923801952067548968388;
                    yOffset = 0;
                }
                xPosition = player.xPos + xOffset + (11 * sin(player.angleRadians));
                yPosition = player.yPos + yOffset - (11 * cos(player.angleRadians));
            }
            else {
                xPosition = player.xPos + (11 * sin(player.angleRadians));
                yPosition = player.yPos - (11 * cos(player.angleRadians));
            }


            // Create new bullet, position it with player, give it its velocities
            bullets.emplace_back(files.basicShotDefault);
            bullets.back().xPos = xPosition;
            bullets.back().yPos = yPosition;
            bullets.back().UpdateHitBox();
            bullets.back().angleRadians = player.angleRadians;
            bullets.back().yVel = round(-cos(player.angleRadians) * 100) / 100;
            bullets.back().modulator = double((rand() % 628)) / 100;
            bullets.back().modulatorDelta = double((rand() % 5) + 15) / 100;
            int result = (rand() % 2);
            if (modulatorTicker == true) {
                modulatorTicker = false;
                bullets.back().modulatorPositiveDelta = false;
            }
            else {
                modulatorTicker = true;
                bullets.back().modulatorPositiveDelta = true;
            }
            if (abs(bullets.back().yVel) < 0.0001) {
                bullets.back().yVel = 0;
            }
            bullets.back().xVel = round(sin(player.angleRadians) * 100) / 100;
            if (abs(bullets.back().xVel) < 0.0001) {
                bullets.back().xVel = 0;
            }

            player.basicShotFrame = files.basicShotEffect1;
        }
        else if (player.basicShotFrame == files.basicShotEffect1 &&
            (std::chrono::steady_clock::now() - player.lastShotEffectTime >= std::chrono::nanoseconds(16666666))) {
            player.lastShotEffectTime = std::chrono::steady_clock::now();
            player.basicShotFrame = files.basicShotEffect2;
        }
        else if (player.basicShotFrame == files.basicShotEffect2 &&
            (std::chrono::steady_clock::now() - player.lastShotEffectTime >= std::chrono::nanoseconds(16666666))) {
            player.lastShotEffectTime = std::chrono::steady_clock::now();
            player.basicShotFrame = files.basicShotEffect3;
        }
        else if (player.basicShotFrame == files.basicShotEffect3 &&
            (std::chrono::steady_clock::now() - player.lastShotEffectTime >= std::chrono::nanoseconds(16666666))) {
            player.lastShotEffectTime = std::chrono::steady_clock::now();
            player.basicShotFrame = files.basicShotEffect4;
        }
        else if (player.basicShotFrame == files.basicShotEffect4 &&
            (std::chrono::steady_clock::now() - player.lastShotEffectTime >= std::chrono::nanoseconds(16666666))) {
            player.lastShotEffectTime = std::chrono::steady_clock::now();
            player.basicShotFrame = files.basicShotEffect5;
        }
        else if ((player.basicShotFrame == files.basicShotEffect5) &&
            (std::chrono::steady_clock::now() - player.lastShotEffectTime >= std::chrono::nanoseconds(16666666))) {
            player.basicShotFrame = nullptr;
        }

        int i = 0;
        if (bullets.size() >= 100) {
             i++;
        }

        if (!bullets.empty()) {
            for (auto it = bullets.begin(); it != bullets.end(); ) {
                if (it->collided && std::chrono::steady_clock::now() - it->explosionBegin > std::chrono::nanoseconds(16666666 * 6)) {
                    it = bullets.erase(it);
                }
                else if (!it->collided) {
                    it->UpdateHitBox();
                    if (it->CheckCollision(enemy)) {
                        it->currentFramePath = files.explosion1;
                        it->collided = true;
                        it->explosionBegin = std::chrono::steady_clock::now();
                    }
                    else {
                        it->xPos += 7 * (deltaSeconds * it->xVel);
                        it->yPos += 7 * (deltaSeconds * it->yVel);
                    }
                    /*int sign = 1;
                    if (!bullet.modulatorPositiveDelta) {
                        sign = -1;
                    }
                    if (bullet.xVel == 0) {
                        bullet.xPos += 2 * sign * deltaSeconds * cos(bullet.modulator);
                    } else if (bullet.yVel == 0) {
                        bullet.yPos += 2 * sign * deltaSeconds * ( cos(bullet.modulator - 3.1415926535897932384626433832795));
                    } else if ((bullet.xVel) - (bullet.yVel) == 0) {
                        bullet.xPos -= cos(bullet.modulator) * sign * 0.70710678118654752440084436210485 * (double(2)/3);
                        bullet.yPos += cos(bullet.modulator) * sign * 0.70710678118654752440084436210485 * (double(2) / 3);
                    } else if ((bullet.xVel != 0) && (bullet.yVel != 0)) {
                        bullet.xPos += cos(bullet.modulator) * sign * 0.70710678118654752440084436210485 * (double(2) / 3);
                        bullet.yPos += cos(bullet.modulator) * sign * 0.70710678118654752440084436210485 * (double(2) / 3);
                    }

                    bullet.modulator += deltaSeconds * bullet.modulatorDelta;*/
                }
                if (it != bullets.end()) {
                    ++it;
                }
            }
        }
        if (enemy.currentFramePath != nullptr) {
            if (enemy.health <= 0) {
                pickup.xPos = enemy.xPos;
                pickup.yPos = enemy.yPos;
                pickup.currentFramePath = files.doubleShotPickup;
                pickup.UpdateHitBox();
                enemy.currentFramePath = nullptr;
            }
            else {
                enemy.currentFramePath = files.enemyShip1;
                double angleRadians = atan2(player.yPos - enemy.yPos, player.xPos - enemy.xPos);
                enemy.xPos += 0.33 * deltaSeconds * cos(angleRadians);
                enemy.yPos += 0.33 * deltaSeconds * sin(angleRadians);
                enemy.UpdateHitBox();
            }
        }
        
        if (std::chrono::steady_clock::now() - enemy.damageBegins >= std::chrono::nanoseconds(16666666 * 4)) {
            enemy.damaged = false;
        }
        player.UpdateHitBox();
        if (pickup.currentFramePath) {
            pickup.UpdateHitBox();
                 if (player.CheckCollision(pickup)) {
                player.doubleShot = true;
                pickup.xPos = 0;
                pickup.yPos = 0;
                pickup.currentFramePath = nullptr;
            }
        }
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    srand(time(NULL));
    spriteFilePaths.emplace_back(files.background);
    spriteFilePaths.emplace_back(files.playerFrame1);
    spriteFilePaths.emplace_back(files.playerFrame2);
    spriteFilePaths.emplace_back(files.playerRedLightffect1);
    spriteFilePaths.emplace_back(files.playerRedLightffect2);
    spriteFilePaths.emplace_back(files.playerRedLightffect3);
    spriteFilePaths.emplace_back(files.playerRedLightffect4);
    spriteFilePaths.emplace_back(files.playerRedLightffect5);
    spriteFilePaths.emplace_back(files.playerRedLightffect6);
    spriteFilePaths.emplace_back(files.playerRedLightffect7);
    spriteFilePaths.emplace_back(files.playerRedLightffect8);
    spriteFilePaths.emplace_back(files.playerRedLightffect9);
    spriteFilePaths.emplace_back(files.asteroid1);
    spriteFilePaths.emplace_back(files.pause);
    spriteFilePaths.emplace_back(files.pauseBackground);
    spriteFilePaths.emplace_back(files.basicShotEffect1);
    spriteFilePaths.emplace_back(files.basicShotEffect2);
    spriteFilePaths.emplace_back(files.basicShotEffect3);
    spriteFilePaths.emplace_back(files.basicShotEffect4);
    spriteFilePaths.emplace_back(files.basicShotEffect5);
    spriteFilePaths.emplace_back(files.basicShotDefault);
    spriteFilePaths.emplace_back(files.basicShotPurple);
    spriteFilePaths.emplace_back(files.basicShotBlue);
    spriteFilePaths.emplace_back(files.hitBox);
    spriteFilePaths.emplace_back(files.hurtBox);
    spriteFilePaths.emplace_back(files.enemyShip1);
    spriteFilePaths.emplace_back(files.basicShotEffectBlue1);
    spriteFilePaths.emplace_back(files.basicShotEffectBlue2);
    spriteFilePaths.emplace_back(files.basicShotEffectBlue3);
    spriteFilePaths.emplace_back(files.basicShotEffectBlue4);
    spriteFilePaths.emplace_back(files.basicShotEffectBlue5);
    spriteFilePaths.emplace_back(files.basicShotEffectPurple1);
    spriteFilePaths.emplace_back(files.basicShotEffectPurple2);
    spriteFilePaths.emplace_back(files.basicShotEffectPurple3);
    spriteFilePaths.emplace_back(files.basicShotEffectPurple4);
    spriteFilePaths.emplace_back(files.basicShotEffectPurple5);
    spriteFilePaths.emplace_back(files.enemyShip1Damaged);
    spriteFilePaths.emplace_back(files.explosion1);
    spriteFilePaths.emplace_back(files.doubleShotPickup);
    spriteFilePaths.emplace_back(files.base);

    enemy.xPos = 1300;
    enemy.yPos = 740;
    enemy.health = 3;
    enemy.currentFramePath = files.enemyShip1;

    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2DFactory);
    if (SUCCEEDED(hr)) {
        WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = ProcessMessage;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = L"CSS 385 Program 1";
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        if (RegisterClassEx(&wc)) {
            HWND hWnd = CreateWindowEx(0, L"CSS 385 Program 1", L"CSS 385 Program 1", WS_POPUP | WS_VISIBLE, 0, 0,
                                       screenX, screenY, NULL, NULL, hInstance, NULL);

            if (hWnd) {
                LoadSpritesToMemory(hWnd, spriteFilePaths);
                ShowWindow(hWnd, nCmdShow);

                MSG msg;
                while (GetMessage(&msg, NULL, 0, 0)) {


                    std::chrono::duration<double> cDeltaTime = std::chrono::steady_clock::now() - lastLogicUpdate;
                    double deltaTime = cDeltaTime.count() * 50;
                    lastLogicUpdate = std::chrono::steady_clock::now();
                    UpdateGameLogic(deltaTime);

                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            D2DFactory->Release();
        }
    }

    return 0;
}

LRESULT CALLBACK ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_LEFT:
            keys.left = true;
            break;
        case VK_RIGHT:
            keys.right = true;
            break;
        case VK_UP:
            keys.up = true;
            break;
        case VK_DOWN:
            keys.down = true;
            break;
        case VK_SPACE:
            keys.space = true;
            break;
        case VK_SHIFT:
            keys.lShift = true;
            break;
        case VK_ESCAPE:
            keys.escape = true;
            break;
        case VK_Q:
            keys.q = true;
            break;
        }
        break;
    }
    case WM_KEYUP:
    {
        switch (wParam)
        {
        case VK_LEFT:
            keys.left = false;
            break;
        case VK_RIGHT:
            keys.right = false;
            break;
        case VK_UP:
            keys.up = false;
            break;
        case VK_DOWN:
            keys.down = false;
            break;
        case VK_SPACE:
            keys.space = false;
            break;
        case VK_SHIFT:
            keys.lShift = false;
            break;
        case VK_ESCAPE:
            keys.escape = false;
            break;
        case VK_Q:
            keys.q = false;
            break;
        }
        break;
    }
    case WM_PAINT:
    case WM_DISPLAYCHANGE:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Render(player, bullets, enemy, pickup);

        EndPaint(hWnd, &ps);
        InvalidateRect(hWnd, NULL, FALSE);
    } break;
    case WM_DESTROY: {
        ReleaseD2DResourcesFromMemory();
        PostQuitMessage(0);
    } break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    if (keys.q && paused) {
        ReleaseD2DResourcesFromMemory();
        PostQuitMessage(0);
    }                  
    if (keys.escape && (std::chrono::steady_clock::now() - pauseBuffer >= std::chrono::milliseconds(250))) {
        pauseBuffer = std::chrono::steady_clock::now();
        paused ? paused = false : paused = true;
    }

    return 0;
}