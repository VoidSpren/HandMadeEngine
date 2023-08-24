#pragma once

#include <Windows.h>
#include <Xinput.h>

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <math.h>
#include <assert.h>

#include "utilDefs.h"
#include "LinearAlg.h"
#include "PixelDefs.h"
#include "Image.h"

namespace voi{

	template <typename T>
	struct LoopFunc { typedef void(T::* loop)(); typedef void(T::* first)(); typedef void(T::* last)(); };

	struct WinScreenBuffInf {
		BITMAPINFO info;
		void* buffer;
		int width;
		int height;
		int pxBytes;
	};

	struct MouseInf {
		Vec2i pos;
		Vec2l dPos;
		i32 dWheel;
		bool lmb, rmb, mmb, x1mb, x2mb;
	};

	struct Dimension { int w; int h; };

	typedef enum : ui8 {
		VK_LMB = 0x01, VK_RMB, CANCEL, VK_MMB, VK_X1MB, VK_X2MB, BACK = 0x08, TAB, CLEAR = 0x0C, RETURN, SHIFT = 0x10, CTRL, ALT, PAUSE, CAPS_LOCK,
		KANA, IME_ON, JUNJA, FINAL, KANJI, IME_OFF, ESC, CONVERT, NONCONVERT, ACCEPT, MODECHANGE, SPACE, PAGE_UP, PAGE_DOWN, END,
		HOME, LEFT, UP, RIGHT, DOWN, SELECT, PRINT, EXECUTE, PRINT_SCREEN, INS, DEL, HELP, K0, K1, K2, K3, K4, K5, K6, K7, K8, K9,
		A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, LWIN, RWIN, APPS, SLEEP = 0x5F, NPAD0,
		NPAD1, NPAD2, NPAD3, NPAD4, NPAD5, NPAD6, NPAD7, NPAD8, NPAD9, MULT, PLUS, SEPARATOR, SUB, DECIMAL, DIV, F1, F2, F3, F4, F5,
		F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, NUM_LOCK = 0x090, SCROLL_LOCK,
		LSHIFT = 0xA0, RSHIFT, LCTRL, RCTRL, LALT, RALT, BROWSER_BACK, BROWSER_FORDWARD, BROWSER_REFRESH, BROWSER_STOP, BROWSER_SEARCH,
		BROWSER_FAV, BROWSER_HOME, VOL_MUTE, VOL_DOWN, VOL_UP, NEXT_TRACK, PREV_TRACK, MEDIA_STOP, MEDIA_PLAY, MAIL, MEDIA_SELECT,
		APP1, APP2, OEM1 = 0xBA, OEM_PLUS, OEM_COMMA, OEM_MINUS, OEM_PERIOD, OEM2, OEM3, OEM4 = 0xDB, OEM5, OEM6, OEM7, OEM8,
		OEM102 = 0xE2, PROCESS_KEY = 0xE5, PACKET = 0XE7, ATTN = 0XF6, CRSEL, EXSEL, ERASE_EOF, PLAY, ZOOM, NONAME, PA1, OEM_CLEAR
	} KeyAccess;
	typedef enum : ui8 {
		LMB = 0x01, RMB, VMS_SHIFT = 0x04, VMS_CTRL = 0x08, MMB = 0x10, X1MB = 0x20, X2MB = 0x40
	} MouseAccess;

#ifdef THREADEDVOIENGINE
#undef THREADEDVOIENGINE

/*----------------------------------------------------------------------------------------------------------------------------------------------*/
	/*#################################*/
	/*#         Render Window         #*/
	/*#################################*/
/*----------------------------------------------------------------------------------------------------------------------------------------------*/

	class WindowHandler {

		WNDCLASS win{};

		bool run = true;

	protected:

		HWND winHandle;

		std::mutex mtx;

		std::atomic<bool> render = true;

		WinScreenBuffInf buffInf;

		int _width;
		int _height;

		ui8 keyState[254] = { 0 };

		static WindowHandler *ownHandle;

		WindowHandler() {}

		/*---------- Window class an Window Handle setup ------------*/
		bool Construct(HINSTANCE instance, const wchar_t* name, ui32 w, ui32 h, ui32 wSize, ui32 hSize){

			win.style = CS_HREDRAW | CS_VREDRAW;
			win.hInstance = instance;
			win.lpszClassName = name;
			win.lpfnWndProc = WinProc;

			_width = w;
			_height = h;

			if (ownHandle) return false;
			ownHandle = this;

			if (RegisterClass(&win)) {

				RECT size{};
				size.right = w * wSize;
				size.bottom = h * hSize;
				AdjustWindowRect(&size, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_SIZEBOX, 0);

				winHandle = CreateWindowExW(
					0, win.lpszClassName, win.lpszClassName,
					WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
					CW_USEDEFAULT, CW_USEDEFAULT, size.right - size.left, size.bottom - size.top,
					0, 0, win.hInstance, 0
				);

				return !!winHandle;
			}
			return false;
		}

		/*---------- Window Messages loop and render loop initialization ------------*/
		template <typename T>
		void WindowLoopInit(typename LoopFunc<T>::loop engineLoop, T *renderEngine) {

			if (winHandle) {
				MSG msg;

				while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {

					if (msg.message == WM_QUIT) run = false;

					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				std::thread renderLoop(engineLoop, renderEngine);

				while (run) {
					while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {

						if (msg.message == WM_QUIT) run = false;

						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}

				render = false;
				renderLoop.join();
			}
		}

		/*---------- Updates the screen with the data of the rendering buffer ------------*/
		void UpdateScreen(HDC context) {

			mtx.lock();

			Dimension d = GetClientDim();

			StretchDIBits(
				context,
				0, 0, d.w, d.h,
				0, 0, buffInf.width, buffInf.height,
				buffInf.buffer,
				&(buffInf.info),
				DIB_RGB_COLORS, SRCCOPY
			);

			mtx.unlock();
		}

		virtual void OnKeyUp(KeyAccess key) {};
		virtual void OnKeyDown(KeyAccess key) {};
		bool KeyPressedConsult(KeyAccess key) {
			bool result;

			mtx.lock();

			result = (keyState[key] & 1);

			mtx.unlock();

			return result;
		};

	private:

		/*---------- Gets client dimension { width, height } of given window handle ------------*/
		static Dimension GetClientDim(HWND hwnd) {
			RECT cr;
			GetClientRect(hwnd, &cr);

			return { cr.right - cr.left, cr.bottom - cr.top };
		}

		/*---------- Gets client dimension { width, height } ------------*/
		Dimension GetClientDim() {
			RECT cr;
			GetClientRect(winHandle, &cr);

			return { cr.right - cr.left, cr.bottom - cr.top };
		}

		/*---------- Calls the KeyUp event function after locking the thread ------------*/
		void KeyUpCall(KeyAccess key) {
			mtx.lock();

			keyState[key] &= 0xfe;
			OnKeyUp(key);

			mtx.unlock();
		}

		/*---------- Calls the KeyDown event function after locking the thread ------------*/
		void KeyDownCall(KeyAccess key) {
			mtx.lock();

			keyState[key] |= 1;
			OnKeyDown(key);

			mtx.unlock();
		}

		/*---------- Sets and configurates buffer info with the given width and height ------------*/
		void SetBufferSize(int w, int h) {
			buffInf.width = w;
			buffInf.height = h;
			buffInf.pxBytes = 4;
			
			if (buffInf.buffer) {
				VirtualFree(buffInf.buffer, 0, MEM_RELEASE);
			}
			
			buffInf.info.bmiHeader = {
				sizeof(buffInf.info.bmiHeader),
				w, -h,
				1, 32,
				BI_RGB 
			};
			
			buffInf.buffer = VirtualAlloc(0, w * h * buffInf.pxBytes, MEM_COMMIT, PAGE_READWRITE);
		}

		/*---------- Window Messages handler procedure ------------*/
		static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			LRESULT result = 0;
			
			switch (msg) {
			case WM_DESTROY: {
					PostQuitMessage(0);
				}break;
			
			case WM_CLOSE: {
					DestroyWindow(hwnd);
				}break;
			
			case WM_SIZE: {
					ownHandle->mtx.lock();

					if (!(ownHandle->buffInf.buffer)) {
						ownHandle->SetBufferSize(ownHandle->_width, ownHandle->_height);
					}

					ownHandle->mtx.unlock();
				}break;
			
			case WM_PAINT: {

					PAINTSTRUCT paint;
					HDC context = BeginPaint(hwnd, &paint);
			
					ownHandle->UpdateScreen(context);
			
					EndPaint(hwnd, &paint);
				}break;

			/*------------------- Key Messages Handle ------------------*/

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN: {
					ui8 key = 0xFF & wParam;
					ownHandle->KeyDownCall((KeyAccess)key);
				}break;
			case WM_KEYUP:{
					ui8 key = 0xFF & wParam;
					ownHandle->KeyUpCall((KeyAccess)key);
				}break;
			 
			/*------------------- Default Messages Handle ------------------*/
			default:{
					result = DefWindowProc(hwnd, msg, wParam, lParam);
				}break;
			}
			
			return result;
		}
	};

	WindowHandler* WindowHandler::ownHandle;

	/*----------------------------------------------------------------------------------------------------------------------------------------------*/
		/*#################################*/
		/*#         Render Engine         #*/
		/*#################################*/
	/*----------------------------------------------------------------------------------------------------------------------------------------------*/

	class VoiEngine : WindowHandler{


		MapPixel* pixelBuffer;
		ui64 _frameCount = 0;

		XINPUT_STATE _padState{ 0 };
		bool _padConnected = false;

		Image font;
		int fontW, fontH, charsXWidth;
		float fontWHratio;

	public:

		Pixel colorSet{0xFF, 0xFF, 0xFF};
		MapPixel clearColor{};

		VoiEngine(): WindowHandler(){}

		/*---------- Starts engine and render loop ------------*/
		void Start() {
			this->WindowLoopInit(&VoiEngine::Loop, this);
		}

		bool Construct(HINSTANCE instance, const wchar_t* name, ui32 w, ui32 h, ui32 wSize = 1, ui32 hSize = 1) {

			font.ReadDecodeImage("consolas_font_mid_res.bmp");

			Pixel* fontData = (Pixel*)font.data;
			Pixel black = { 0,0,0 };

			for (int y = 0; y < font.V5.height; y++) {
				if (fontData[y * font.V5.width + (font.V5.width - 1)].u != black.u) {
					fontH = y;
					break;
				}
			}

			for (int x = font.V5.width - 1; x >= 0; x--) {
				if (fontData[x].u != black.u) {
					fontW = font.V5.width - x - 1;
					break;
				}
			}

			charsXWidth = font.V5.width / fontW;
			fontWHratio = (float)fontW / (float)fontH;

			return this->WindowHandler::Construct(instance, name, w, h, wSize, hSize);
		}



		virtual void OnCreate() = 0;

		virtual void OnUpdate(float deltaTime) = 0;

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       Keyboard Functions       #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/

		virtual void OnKeyDown(KeyAccess key) override {

		}

		virtual void OnKeyUp(KeyAccess key) override {

		}

		bool IsKeyPressed(KeyAccess key) {
			return KeyPressedConsult(key);
		}

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       GamePad Functions       #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/

		void PullPadState() {
			DWORD result;
			XINPUT_STATE tempState{ 0 };

			result = XInputGetState(0, &tempState);

			if (result == ERROR_SUCCESS) {
				_padConnected = true;
				if (tempState.dwPacketNumber != _padState.dwPacketNumber) {
					OnPadChange(_padState.Gamepad, tempState.Gamepad);
					_padState = tempState;
				}
			}
			else {
				_padConnected = false;
			}
		}

		const XINPUT_GAMEPAD& GetPadState() { return _padState.Gamepad; }

		virtual void OnPadChange(const XINPUT_GAMEPAD &prev, const XINPUT_GAMEPAD &curr) {}

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       Consult Functions        #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/

		int width() { return buffInf.width; }
		int height() { return buffInf.height; }

		bool PadConnected() { return _padConnected; }

		ui64 FrameCount() { return _frameCount; }

		Pixel GetPixel(int x, int y) {
			Pixel res;

			if (x < buffInf.width && x >= 0 && y < buffInf.height && y >= 0) {
				res = pixelBuffer[y * buffInf.width + x];
			}

			return res;
		}

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       Setting Functions        #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/

		void SetBackground(ui8 r, ui8 g, ui8 b) { clearColor = { r,g,b }; }
		void SetBackground(Pixel pixel) { clearColor = { pixel }; }

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       Drawing Functions        #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/

		/*sets entire screen to clear color*/
		void Clear() {
			//TODO: profile performance diference
			//std::fill(pixelBuffer, pixelBuffer + (buffInf.width * buffInf.height) - 1, clearColor);
			for (int i = 0; i < buffInf.width * buffInf.height; i++) {		
				pixelBuffer[i] = clearColor;
			}
		}
		/*sets the píxel color at coordinate x, y*/
		void SetPixel(ui32 x, ui32 y, ui8 r, ui8 g, ui8 b) {
			if (x < buffInf.width && y < buffInf.height) {
				pixelBuffer[y * buffInf.width + x].SetColor(r, g, b);
			}
		}

		/*writes a point in coordinates x, y*/
		void Point(int x, int y) {
			if (x < buffInf.width && x >= 0 && y < buffInf.height && y >= 0) {
				MapPixel* p = &(pixelBuffer[y * buffInf.width + x]);

				p->r = (p->r * 256 + (colorSet.r - p->r) * colorSet.a) >> 8;
				p->b = (p->b * 256 + (colorSet.b - p->b) * colorSet.a) >> 8;
				p->g = (p->g * 256 + (colorSet.g - p->g) * colorSet.a) >> 8;

				//pixelBuffer[y * buffInf.width + x].SetColor(colorSet.r, colorSet.g, colorSet.b);
			}
		}

		void Point(Vec2i& pos) {
			Point(pos.x, pos.y);
		}

		/*writes a line that goes from and to the given points*/
		void Line(int x1, int y1, int x2, int y2) {
			bool vert = false;

			if (abs(x2 - x1) < abs(y2 - y1)) {
				std::swap(x1, y1);
				std::swap(x2, y2);
				vert = true;
			}
			if (x2 < x1) {
				std::swap(x1, x2);
				std::swap(y1, y2);
			}

			int dx = x2 - x1;
			int dy = y2 - y1;
			int D = 2 * abs(dy) - abs(dx);
			int y = y1;

			for (int x = x1; x <= x2; x++) {
				if (vert) Point(y, x);
				else Point(x, y);

				if (D > 0) {
					y += (dy < 0) ? -1 : 1;
					D -= 2 * abs(dx);
				}
				D += 2 * abs(dy);
			}
		}
		void Line(const Vec2i& a, const Vec2i& b) {
			Line(a.x, a.y, b.x, b.y);
		}

		/*writes the triangle described by the given points*/
		void Triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
			Line(x1, y1, x2, y2);
			Line(x2, y2, x3, y3);
			Line(x3, y3, x1, y1);
		}
		void Triangle(const Vec2i& p1, const Vec2i& p2, const Vec2i& p3) {
			Triangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
		}

		/*writes a rectangle with his top-left corner at the given point, and with the given dimensions*/
		void Rect(int x, int y, int w, int h) {
			for (int hor = x; hor <= x + w; hor++) {
				Point(hor, y);
				Point(hor, y + h);
			}
			for (int ver = y + 1; ver < y + h; ver++) {
				Point(x, ver);
				Point(x + w, ver);
			}
		}
		void Rect(const Vec2i& pos, const Vec2i& size) {
			Rect(pos.x, pos.y, size.x, size.y);
		}

		/*writes a circle centered at the given point, and with the given radius*/
		void Circle(int x, int y, int r) {
			r = std::abs(r);
			int xc = 1, yc = r, d = 3 - (2 * r), yf = 0;
			auto octLine = [&]() {
				++yf;
				if (yf <= yc) {
					RightQuadrant(xc, yc, x, y);
				}
				if (yf + 1 <= yc) {
					LeftQuadrant(xc, yc, x, y);
				}
			};
			Point(x + r, y);
			Point(x - r, y);
			Point(x, y + r);
			Point(x, y - r);

			xc = 0;
			if (r < 21) {
				xc++;
				if (d < 0) d = d + 4 * xc + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine();

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 4 * xc + 6;
					else d = d + 8 * (xc - --yc) + 10;
					octLine();
				}
			}
			else if (r < 81) {
				xc++;
				if (d < 0) d = d + 2 * xc + xc + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine();

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 2 * xc + xc + 6;
					else d = d + 4 * (xc - --yc) + 10;
					octLine();
				}
			}
			else {
				xc++;
				if (d < 0) d = d + 2 * xc + xc + (xc >> 1) + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine();

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 2 * xc + xc + (xc >> 1) + 6;
					else d = d + 4 * (xc - --yc) + 10;
					octLine();
				}
			}
		}
		void Circle(const Vec2i& pos, int r) {
			Circle(pos.x, pos.y, r);
		}

		/*writes a triangle just as the triangle function, and fill it*/
		void FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
			Vec3i xComp(x1, x2, x3);
			Vec3i yComp(y1, y2, y3);

			Vec2i bbmin(xComp.vecMin(), yComp.vecMin());
			Vec2i bbmax(xComp.vecMax(), yComp.vecMax());

			Vec3f a(xComp.y - xComp.x, xComp.z - xComp.x, 0);
			Vec3f b(yComp.y - yComp.x, yComp.z - yComp.x, 0);

			Vec3f barycentric;

			for (int x = bbmin.x; x <= bbmax.x; x++) {
				for (int y = bbmin.y; y <= bbmax.y; y++) {
					a.z = xComp.x - x;
					b.z = yComp.x - y;
					Vec3f u = Vec3f::cross(a, b);

					if (abs(u.z) < 1) continue;
					float uZ = 1.f / u.z;
					barycentric = { 1 - (u.x + u.y) * uZ, u.y * uZ, u.x * uZ };

					if (barycentric.x >= 0 && barycentric.y >= 0 && barycentric.z >= 0)
						Point(x, y);
				}
			}
		}
		void FillTriangle(const Vec2i& p1, const Vec2i& p2, const Vec2i& p3) {
			FillTriangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
		}

		/*writes a rectangle just as the rectangle function and fill it*/
		void FillRect(int x, int y, int w, int h) {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					Point(x + j, y + i);
				}
			}
		}
		void FillRect(const Vec2i& pos, const Vec2i& size) {
			FillRect(pos.x, pos.y, size.x, size.y);
		}

		/*writes a circle just as the circle function, and fill it*/
		void FillCircle(int x, int y, int r) {
			r = std::abs(r);
			int xc = 1, yc = r, d = 3 - (2 * r), yf = 0;
			auto octLine = [&](int yi) {
				for (; yi <= yc; yi++) {
					RightQuadrant(xc, yi, x, y);
				}
				yi = yf + 1;
				for (; yi <= yc; yi++) {
					LeftQuadrant(xc, yi, x, y);
				}
			};

			Point(x, y);
			for (; xc <= r; xc++) {
				Point(x + xc, y);
				Point(x - xc, y);
				Point(x, y + xc);
				Point(x, y - xc);
			}

			xc = 0;
			if (r < 21) {
				xc++;
				if (d < 0) d = d + 4 * xc + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine(++yf);

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 4 * xc + 6;
					else d = d + 8 * (xc - --yc) + 10;
					octLine(++yf);
				}
			}
			else if (r < 81) {
				xc++;
				if (d < 0) d = d + 2 * xc + xc + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine(++yf);

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 2 * xc + xc + 6;
					else d = d + 4 * (xc - --yc) + 10;
					octLine(++yf);
				}
			}
			else {
				xc++;
				if (d < 0) d = d + 2 * xc + xc + (xc >> 1) + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine(++yf);

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 2 * xc + xc + (xc >> 1) + 6;
					else d = d + 4 * (xc - --yc) + 10;
					octLine(++yf);
				}
			}
		}
		void FillCircle(const Vec2i& pos, int r) {
			FillCircle(pos.x, pos.y, r);
		}

		void DrawImage(const Image& img, int x, int y) {
			for (int imgY = img.V5.height - 1; imgY >= 0; imgY--) {
				for (int imgX = 0; imgX < img.V5.width; imgX++) {
					colorSet.u = ((ui32*)img.data)[imgY * img.V5.width + imgX];
					Point(x + imgX, y + (img.V5.height - imgY) - 1);
				}
			}
		}

		void DrawImage(const Image& img, int x, int y, int w, int h) {
			float xFac = ((float)img.V5.width / (float)w);
			float yFac = ((float)img.V5.height / (float)h);
			int xOff = 0, yOff = 0;

			float imgY = img.V5.height - 0.0001f;
			float imgX;
			for (; imgY >= 0 && imgY < img.V5.height; imgY -= yFac) {
				imgX = 0;
				for (; imgX >= 0 && imgX < img.V5.width; imgX += xFac) {
					colorSet.u = ((ui32*)img.data)[(int)imgY * img.V5.width + (int)imgX];
					Point(x + xOff, y + yOff);
					xOff++;
				}
				xOff = 0;
				yOff++;
			}
		}

		void DrawPartialImage(const Image& img, int x, int y, int w, int h, ui32 u, ui32 v, int uW, int vH) {
			int imgV = (img.V5.height - v);
			int vD = imgV - vH;
			int uR = u + uW;

			if (u > img.V5.width || v > img.V5.height || vD > img.V5.height || uR < 0) return;

			if (vD < 0) {
				vD = 0; vH = imgV - vD;
			}
			if (uR > img.V5.width) {
				uR = img.V5.width; uW = uR - u;
			}

			float xFac = ((float)uW / (float)w);
			float yFac = ((float)vH / (float)h);

			int xOff = 0, yOff = 0;

			float imgY = imgV - 0.0001f;
			float imgX;

			for (; imgY < imgV && imgY >= vD; imgY -= yFac) {
				imgX = u;
				for (; imgX >= u && imgX < uR; imgX += xFac) {
					colorSet.u = ((ui32*)img.data)[(int)imgY * img.V5.width + (int)imgX];
					Point(x + xOff, y + yOff);
					xOff++;
				}

				xOff = 0;
				yOff++;
			}

		}


		//void DrawIntegerReducedImage(const Image& img, int x, int y, ui32 xFactor, ui32 yFactor) {
		//	int xOff = 0, yOff = 0;
		//	if ((!yFactor) || (!xFactor)) { xFactor = 1; yFactor = 1; }

		//	for (ui32 imgY = img.V5.height - 1; imgY < img.V5.height; imgY -= yFactor) {
		//		for (ui32 imgX = 0; imgX < img.V5.width; imgX += xFactor) {
		//			colorSet.u = ((ui32*)img.data)[imgY * img.V5.width + imgX];
		//			Point(x + xOff, y + yOff);
		//			xOff++;
		//		}
		//		xOff = 0;
		//		yOff++;
		//	}
		//}


		void DrawString(const char* str, int x, int y, int height,Pixel color = {0,0,0,0}) {
			int lineX = x;
			int width = (float)height * fontWHratio;

			color.a = 0;

			for (int i = 0; str[i] != '\0'; i++) {
				if (str[i] == '\n') {
					y += height;
					lineX = x;
				}
				else if(str[i] >= 32 && str[i] < 127) {

					DrawPartialMaskedImage(font, lineX, y, width, height,
						((str[i] - 32) % charsXWidth) * fontW,
						((str[i] - 32) / charsXWidth) * fontH + 1,
						fontW, fontH - 1,
						color
					);

					lineX += width;
				}
				else {
					lineX += width;
				}
			}
		}




	private:
		void RightQuadrant(int xc, int yc, int x, int y) {

			//down-right
			Point(xc + x, yc + y);
			//top-left
			Point(-xc + x, -yc + y);
			//right-up
			Point(yc + x, -xc + y);
			//left-down
			Point(-yc + x, xc + y);
		}
		void LeftQuadrant(int xc, int yc, int x, int y) {

			//down-left
			Point(-xc + x, yc + y);
			//top-right
			Point(xc + x, -yc + y);
			//right-down
			Point(yc + x, xc + y);
			//left-up
			Point(-yc + x, -xc + y);
		}

		void DrawPartialMaskedImage(const Image& img, int x, int y, int w, int h, ui32 u, ui32 v, int uW, int vH, Pixel color) {
			int imgV = (img.V5.height - v);
			int vD = imgV - vH;
			int uR = u + uW;

			if (u > img.V5.width || v > img.V5.height || vD > img.V5.height || uR < 0) return;

			if (vD < 0) {
				vD = 0; vH = imgV - vD;
			}
			if (uR > img.V5.width) {
				uR = img.V5.width; uW = uR - u;
			}

			float xFac = ((float)uW / (float)w);
			float yFac = ((float)vH / (float)h);

			int xOff = 0, yOff = 0;

			float imgY = imgV - 0.0001f;
			float imgX;

			for (; imgY < imgV && imgY >= vD; imgY -= yFac) {
				imgX = u;
				for (; imgX >= u && imgX < uR; imgX += xFac) {
					colorSet.u = ((ui32*)img.data)[(int)imgY * img.V5.width + (int)imgX] | color.u;
					Point(x + xOff, y + yOff);
					xOff++;
				}

				xOff = 0;
				yOff++;
			}

		}


	private:

		void Loop() {
			pixelBuffer = (MapPixel*)(buffInf.buffer);

			std::chrono::duration<float> elapsedTime;
			HDC context = GetDC(this->winHandle);

			auto ts1 = std::chrono::system_clock::now();
			auto ts2 = std::chrono::system_clock::now();

			PullPadState();

			OnCreate();
			this->UpdateScreen(context);

			_frameCount++;

			float deltaTime;
			while (this->render) {

				ts1 = std::chrono::system_clock::now();
				elapsedTime = ts1 - ts2;
				deltaTime = elapsedTime.count();
				ts2 = ts1;

				if (_padConnected || !(_frameCount & 0x3F)) {
					PullPadState();
				}

				OnUpdate(deltaTime);

				this->UpdateScreen(context);

				_frameCount++;
			};

			ReleaseDC(winHandle, context);
		}
	};

#endif




#ifndef THREADEDVOIENGINE

/*----------------------------------------------------------------------------------------------------------------------------------------------*/
/*#################################*/
/*#       NOT MULTI THREADED      #*/
/*#################################*/
/*----------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------------------------------*/
/*#################################*/
/*#         Render Window         #*/
/*#################################*/
/*----------------------------------------------------------------------------------------------------------------------------------------------*/

	class WindowHandler {

		WNDCLASS win{};

		bool run = true;

	protected:

		HWND winHandle;

		WinScreenBuffInf buffInf;

		int _width;
		int _height;
		int _xScale;
		int _yScale;

		ui8 keyState[254] = { 0 };
		MouseInf mouseState;

		static WindowHandler* ownHandle;

		WindowHandler() {}

		/*---------- Window class an Window Handle setup ------------*/
		bool Construct(HINSTANCE instance, const wchar_t* name, ui32 w, ui32 h, ui32 xScale, ui32 yScale) {

			win.style = CS_HREDRAW | CS_VREDRAW;
			win.hInstance = instance;
			win.lpszClassName = name;
			win.lpfnWndProc = WinProc;

			_width = w;
			_height = h;

			_xScale = xScale;
			_yScale = yScale;

			if (ownHandle) return false;
			ownHandle = this;

			if (RegisterClass(&win)) {

				RECT size{};
				size.right = w * xScale;
				size.bottom = h * yScale;
				AdjustWindowRect(&size, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_SIZEBOX, 0);

				winHandle = CreateWindowExW(
					0, win.lpszClassName, win.lpszClassName,
					WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
					CW_USEDEFAULT, CW_USEDEFAULT, size.right - size.left, size.bottom - size.top,
					0, 0, win.hInstance, 0
				);

				return !!winHandle;
			}
			return false;
		}

		/*---------- Window Messages loop and render loop initialization ------------*/
		template <typename T>
		void WindowLoopInit(typename LoopFunc<T>::loop engineLoop, typename LoopFunc<T>::first beginCycle, typename LoopFunc<T>::last endCycle, T* instance) {

			if (winHandle) {
				MSG msg;

				while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {

					if (msg.message == WM_QUIT) run = false;

					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				(instance->*beginCycle)();

				while (run) {
					while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {

						if (msg.message == WM_QUIT) run = false;

						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}

					(instance->*engineLoop)();
				}

				(instance->*endCycle)();
			}
		}

		/*---------- Updates the screen with the data of the rendering buffer ------------*/
		void UpdateScreen(HDC context) {
			Dimension d = GetClientDim();

			StretchDIBits(
				context,
				0, 0, d.w, d.h,
				0, 0, buffInf.width, buffInf.height,
				buffInf.buffer,
				&(buffInf.info),
				DIB_RGB_COLORS, SRCCOPY
			);
		}

		virtual void OnKeyUp(KeyAccess key) {};
		virtual void OnKeyDown(KeyAccess key) {};
		virtual void OnMouseUp(MouseAccess key) {};
		virtual void OnMouseDown(MouseAccess key) {};
		virtual void OnMouseMove(MouseInf inf) {};
		virtual void OnMouseWheel(MouseInf inf) {};
		virtual void OnMouseClick(MouseAccess key, bool state) {};

		bool MousePressedConsult(MouseAccess key) const {
			switch (key) {
			case RMB:
				return mouseState.rmb;
			case LMB:
				return mouseState.lmb;
			case MMB:
				return mouseState.mmb;
			case X1MB:
				return mouseState.x1mb;
			case X2MB:
				return mouseState.x2mb;
			}
		}

		Vec2i MousePosConsult() const {
			return mouseState.pos;
		}

		Vec2l MouseDPosConsut() {
			return mouseState.dPos;
		}

		i32 MouseXConsult() const{
			return mouseState.pos.x;
		}
		i32 MouseYConsult() const{
			return mouseState.pos.y;
		}

		i64 MouseDXConsult() const{
			return mouseState.dPos.x;
		}
		i64 MouseDYConsult() const{
			return mouseState.dPos.y;
		}

		i32 MouseWheelConsult() const {
			return mouseState.dWheel;
		}

		const MouseInf& MouseInfConsult() const{
			return mouseState;
		}

		bool KeyPressedConsult(KeyAccess key) const{
			bool result;

			result = (keyState[key] & 1);

			return result;
		};

	private:

		/*---------- Gets client dimension { width, height } of given window handle ------------*/
		static Dimension GetClientDim(HWND hwnd) {
			RECT cr;
			GetClientRect(hwnd, &cr);

			return { cr.right - cr.left, cr.bottom - cr.top };
		}

		/*---------- Gets client dimension { width, height } ------------*/
		Dimension GetClientDim() {
			RECT cr;
			GetClientRect(winHandle, &cr);

			return { cr.right - cr.left, cr.bottom - cr.top };
		}

		/*---------- Calls the KeyUp event function after locking the thread ------------*/
		void KeyUpCall(KeyAccess key) {
			keyState[key] &= 0xfe;
			OnKeyUp(key);
		}

		/*---------- Calls the KeyDown event function after locking the thread ------------*/
		void KeyDownCall(KeyAccess key) {
			keyState[key] |= 1;
			OnKeyDown(key);
		}

		/*---------- Calls the MouseUp event function after locking the thread ------------*/
		void MouseUpCall(MouseAccess key) {
			switch (key) {
			case RMB:
				mouseState.rmb = false;
				OnMouseUp(key);
				break;
			case LMB:
				mouseState.lmb = false;
				OnMouseUp(key);
				break;
			case MMB:
				mouseState.mmb = false;
				OnMouseUp(key);
				break;
			case X1MB:
				mouseState.x1mb = false;
				OnMouseUp(key);
				break;
			case X2MB:
				mouseState.x2mb = false;
				OnMouseUp(key);
				break;
			}
		}

		/*---------- Calls the MouseDown event function after locking the thread ------------*/
		void MouseDownCall(MouseAccess key) {
			switch (key) {
			case RMB:
				mouseState.rmb = true;
				OnMouseDown(key);
				break;
			case LMB:
				mouseState.lmb = true;
				OnMouseDown(key);
				break;
			case MMB:
				mouseState.mmb = true;
				OnMouseDown(key);
				break;
			case X1MB:
				mouseState.x1mb = true;
				OnMouseDown(key);
				break;
			case X2MB:
				mouseState.x2mb = true;
				OnMouseDown(key);
				break;
			}
		}

		void MouseMoveCall(i32 nX, i32 nY) {
			if (nX >= _width || nY >= _height) return;

			mouseState.dPos.x = nX - mouseState.pos.x;
			mouseState.dPos.y = nY - mouseState.pos.y;

			mouseState.pos.x = nX;
			mouseState.pos.y = nY;

			OnMouseMove(mouseState);
		}

		void MouseWheelCall(i32 dWheel) {
			mouseState.dWheel = dWheel;

			OnMouseWheel(mouseState);
		}

		/*---------- Sets and configurates buffer info with the given width and height ------------*/
		void SetBufferSize(int w, int h) {
			buffInf.width = w;
			buffInf.height = h;
			buffInf.pxBytes = 4;

			if (buffInf.buffer) {
				VirtualFree(buffInf.buffer, 0, MEM_RELEASE);
			}

			buffInf.info.bmiHeader = {
				sizeof(buffInf.info.bmiHeader),
				w, -h,
				1, 32,
				BI_RGB
			};
			buffInf.buffer = VirtualAlloc(0, (ui64)w * h * buffInf.pxBytes, MEM_COMMIT, PAGE_READWRITE);
		}

		/*---------- Window Messages handler procedure ------------*/
		static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			LRESULT result = 0;
			switch (msg) {
			case WM_DESTROY: {
				PostQuitMessage(0);
			}break;

			case WM_CLOSE: {
				DestroyWindow(hwnd);
			}break;

			case WM_SIZE: {
				if (!(ownHandle->buffInf.buffer)) {
					ownHandle->SetBufferSize(ownHandle->_width, ownHandle->_height);
				}
			}break;

			case WM_PAINT: {

				PAINTSTRUCT paint;
				HDC context = BeginPaint(hwnd, &paint);

				ownHandle->UpdateScreen(context);

				EndPaint(hwnd, &paint);
			}break;

				/*------------------- Mouse Messages Handle ------------------*/

			case WM_LBUTTONDOWN: {
				ownHandle->MouseDownCall(LMB);
				ownHandle->OnMouseClick(LMB, true);
			}break;
			case WM_LBUTTONUP: {
				ownHandle->MouseUpCall(LMB);
				ownHandle->OnMouseClick(LMB, false);
			}break;

			case WM_RBUTTONDOWN: {
				ownHandle->MouseDownCall(RMB);
				ownHandle->OnMouseClick(RMB, true);
			}break;
			case WM_RBUTTONUP: {
				ownHandle->MouseUpCall(RMB);
				ownHandle->OnMouseClick(RMB, false);
			}break;

			case WM_MBUTTONDOWN: {
				ownHandle->MouseDownCall(MMB);
				ownHandle->OnMouseClick(MMB, true);
			}break;
			case WM_MBUTTONUP: {
				ownHandle->MouseUpCall(MMB);
				ownHandle->OnMouseClick(MMB, false);
			}break;

			case WM_MOUSEMOVE: {
				ownHandle->MouseMoveCall(
					(0xFFFF & lParam) / ownHandle->_xScale,
					((lParam >> 16) & 0xFFFF) / ownHandle->_yScale
				);
			}break;
			case WM_MOUSEWHEEL: {
				short dw = (wParam >> 16) & 0xFFFF;

				ownHandle->MouseWheelCall(dw);
			}break;

				/*------------------- Key Messages Handle ------------------*/

			case WM_SYSKEYDOWN:
			case WM_KEYDOWN: {
				ui8 key = 0xFF & wParam;
				ownHandle->KeyDownCall((KeyAccess)key);
			}break;
			case WM_SYSKEYUP:
			case WM_KEYUP: {
				ui8 key = 0xFF & wParam;
				ownHandle->KeyUpCall((KeyAccess)key);
			}break;

				/*------------------- Default Messages Handle ------------------*/
			default: {
				result = DefWindowProc(hwnd, msg, wParam, lParam);
			}break;
			}
			return result;
		}
	};

	WindowHandler* WindowHandler::ownHandle;

	/*----------------------------------------------------------------------------------------------------------------------------------------------*/
		/*#################################*/
		/*#         Render Engine         #*/
		/*#################################*/
	/*----------------------------------------------------------------------------------------------------------------------------------------------*/

	class VoiEngine : WindowHandler {


		MapPixel* pixelBuffer;
		ui64 _frameCount = 0;

		XINPUT_STATE _padState{ 0 };
		bool _padConnected = false;

		Image font;
		int fontW, fontH, charsXWidth;
		float fontWHratio;

	public:

		Pixel colorSet{ 0xFF, 0xFF, 0xFF };
		MapPixel clearColor{};

		VoiEngine() : WindowHandler() {}

		/*---------- Starts engine and render loop ------------*/
		void Start() {
			this->WindowLoopInit<VoiEngine>(&VoiEngine::Loop, &VoiEngine::First, &VoiEngine::Last, this);
		}

		bool Construct(HINSTANCE instance, const wchar_t* name, ui32 w, ui32 h, ui32 wSize = 1, ui32 hSize = 1) {

			font = voi::Image::ReadDecodeImage("consolas_font_mid_res.bmp");
			Pixel black = { 0,0,0 };

			for (int y = 0; y < font.height(); y++) {
				if (font.data()[y * font.width() + (font.width() - 1)].u != black.u) {
					fontH = y;
					break;
				}
			}

			for (int x = font.width() - 1; x >= 0; x--) {
				if (font.data()[x].u != black.u) {
					fontW = font.width() - x - 1;
					break;
				}
			}

			charsXWidth = font.width() / fontW;
			fontWHratio = (float)fontW / (float)fontH;

			return this->WindowHandler::Construct(instance, name, w, h, wSize, hSize);
		}



		virtual void OnCreate() = 0;

		virtual void OnUpdate(float deltaTime) = 0;

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       Keyboard Functions       #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/


		bool IsKeyPressed(KeyAccess key) {
			return KeyPressedConsult(key);
		}

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#        Mouse Functions         #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/


		i32 IsMouseButtonPressed(MouseAccess key) {
			return MousePressedConsult(key);
		}

		Vec2l MousePos() {
			return MousePosConsult();
		}

		Vec2l MouseDPos() {
			return MouseDPosConsut();
		}

		i32 MouseX() {
			return MouseXConsult();
		}
		i32 MouseY() {
			return MouseYConsult();
		}

		i64 MouseDX() {
			return MouseDXConsult();
		}
		i64 MouseDY() {
			return MouseDYConsult();
		}

		i32 MouseWheel() {
			return MouseWheelConsult();
		}

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       GamePad Functions       #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/

		void PullPadState() {
			DWORD result;
			XINPUT_STATE tempState{ 0 };

			result = XInputGetState(0, &tempState);

			if (result == ERROR_SUCCESS) {
				_padConnected = true;
				if (tempState.dwPacketNumber != _padState.dwPacketNumber) {
					OnPadChange(_padState.Gamepad, tempState.Gamepad);
					_padState = tempState;
				}
			}
			else {
				_padConnected = false;
			}
		}

		const XINPUT_GAMEPAD& GetPadState() { return _padState.Gamepad; }

		virtual void OnPadChange(const XINPUT_GAMEPAD& prev, const XINPUT_GAMEPAD& curr) {}

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       Consult Functions        #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/

		int width() { return buffInf.width; }
		int height() { return buffInf.height; }

		int CharWidth(int height) { return int(height * fontWHratio); }
		int CharHeight(int width) { return round(width / fontWHratio); }

		int FontWidth() { return fontW; }
		int FontHeight() { return fontH; }

		float FontWHRatio() { return fontWHratio; }

		bool PadConnected() { return _padConnected; }

		ui64 FrameCount() { return _frameCount; }

		float TotalTime() { return totalTime; }

		Pixel GetPixel(int x, int y) {
			Pixel res;

			if (x < buffInf.width && x >= 0 && y < buffInf.height && y >= 0) {
				res = pixelBuffer[y * buffInf.width + x];
			}

			return res;
		}

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       Setting Functions        #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/

		void SetBackground(ui8 r, ui8 g, ui8 b) { clearColor = { r,g,b }; }
		void SetBackground(Pixel pixel) { clearColor = { pixel }; }

		/*-----------------------------------------------------------------------*/

						/*##################################*/
						/*#       Drawing Functions        #*/
						/*##################################*/

		/*-----------------------------------------------------------------------*/

		/*sets entire screen to clear color*/
		void Clear() {
			//TODO: profile performance diference
			//std::fill(pixelBuffer, pixelBuffer + (buffInf.width * buffInf.height) - 1, clearColor);
			for (int i = 0; i < buffInf.width * buffInf.height; i++) {
				pixelBuffer[i] = clearColor;
			}
		}
		/*sets the píxel color at coordinate x, y*/
		void SetPixel(int x, int y, ui8 r, ui8 g, ui8 b) {
			if (x < buffInf.width && x >= 0 && y < buffInf.height && y >= 0) {
				pixelBuffer[y * buffInf.width + x].SetColor(r, g, b);
			}
		}

		/*writes a point in coordinates x, y*/
		void Point(int x, int y) {
			if (x < buffInf.width && x >= 0 && y < buffInf.height && y >= 0) {
				MapPixel* p = &(pixelBuffer[y * buffInf.width + x]);

				p->r = (p->r * 256 + (colorSet.r - p->r) * colorSet.a) >> 8;
				p->b = (p->b * 256 + (colorSet.b - p->b) * colorSet.a) >> 8;
				p->g = (p->g * 256 + (colorSet.g - p->g) * colorSet.a) >> 8;

				//pixelBuffer[y * buffInf.width + x].SetColor(colorSet.r, colorSet.g, colorSet.b);
			}
		}

		void Point(Vec2i& pos) {
			Point(pos.x, pos.y);
		}

		/*writes a line that goes from and to the given points*/
		void Line(int x1, int y1, int x2, int y2) {
			bool vert = false;

			if (abs(x2 - x1) < abs(y2 - y1)) {
				std::swap(x1, y1);
				std::swap(x2, y2);
				vert = true;
			}
			if (x2 < x1) {
				std::swap(x1, x2);
				std::swap(y1, y2);
			}

			int dx = x2 - x1;
			int dy = y2 - y1;
			int D = 2 * abs(dy) - abs(dx);
			int y = y1;

			for (int x = x1; x <= x2; x++) {
				if (vert) Point(y, x);
				else Point(x, y);

				if (D > 0) {
					y += (dy < 0) ? -1 : 1;
					D -= 2 * abs(dx);
				}
				D += 2 * abs(dy);
			}
		}
		void Line(const Vec2i& a, const Vec2i& b) {
			Line(a.x, a.y, b.x, b.y);
		}

		/*writes the triangle described by the given points*/
		void Triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
			Line(x1, y1, x2, y2);
			Line(x2, y2, x3, y3);
			Line(x3, y3, x1, y1);
		}
		void Triangle(const Vec2i& p1, const Vec2i& p2, const Vec2i& p3) {
			Triangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
		}

		/*writes a rectangle with his top-left corner at the given point, and with the given dimensions*/
		void Rect(int x, int y, int w, int h) {
			for (int hor = x; hor <= x + w; hor++) {
				Point(hor, y);
				Point(hor, y + h);
			}
			for (int ver = y + 1; ver < y + h; ver++) {
				Point(x, ver);
				Point(x + w, ver);
			}
		}
		void Rect(const Vec2i& pos, const Vec2i& size) {
			Rect(pos.x, pos.y, size.x, size.y);
		}

		/*writes a circle centered at the given point, and with the given radius*/
		void Circle(int x, int y, int r) {
			r = std::abs(r);
			int xc = 1, yc = r, d = 3 - (2 * r), yf = 0;
			auto octLine = [&]() {
				++yf;
				if (yf <= yc) {
					RightQuadrant(xc, yc, x, y);
				}
				if (yf + 1 <= yc) {
					LeftQuadrant(xc, yc, x, y);
				}
			};
			Point(x + r, y);
			Point(x - r, y);
			Point(x, y + r);
			Point(x, y - r);

			xc = 0;
			if (r < 21) {
				xc++;
				if (d < 0) d = d + 4 * xc + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine();

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 4 * xc + 6;
					else d = d + 8 * (xc - --yc) + 10;
					octLine();
				}
			}
			else if (r < 81) {
				xc++;
				if (d < 0) d = d + 2 * xc + xc + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine();

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 2 * xc + xc + 6;
					else d = d + 4 * (xc - --yc) + 10;
					octLine();
				}
			}
			else {
				xc++;
				if (d < 0) d = d + 2 * xc + xc + (xc >> 1) + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine();

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 2 * xc + xc + (xc >> 1) + 6;
					else d = d + 4 * (xc - --yc) + 10;
					octLine();
				}
			}
		}
		void Circle(const Vec2i& pos, int r) {
			Circle(pos.x, pos.y, r);
		}

		void FillTriangle(float x0, float y0, float x1, float y1, float x2, float y2) {
			FillTriangle({ x0,y0 }, { x1,y1 }, { x2,y2 });
		}

		void FillTriangle(const voi::Vec2f& v0, const voi::Vec2f& v1, const voi::Vec2f& v2) {
			const voi::Vec2f* p0 = &v0;
			const voi::Vec2f* p1 = &v1;
			const voi::Vec2f* p2 = &v2;

			//sorting vertices
			if (p1->y < p0->y) std::swap(p0, p1);
			if (p2->y < p0->y) std::swap(p0, p2);
			if (p2->y < p1->y) std::swap(p1, p2);

			if (p0->y == p2->y) return;

			//top flat triangle
			if (p0->y == p1->y) {
				if (p1->x < p0->x) std::swap(p0, p1);
				FlatTopTri(*p0, *p1, *p2);
			}
			//bottom flat triangle
			else if (p1->y == p2->y) {
				if (p2->x < p1->x) std::swap(p1, p2);
				FlatBotTri(*p0, *p1, *p2);
			}
			//general triangle
			else {
				//get split vector by linear interpolation
				const float flatSplit = (p1->y - p0->y) / (p2->y - p0->y);
				const voi::Vec2f splitV = Vec2f::lerp(*p0, *p2, flatSplit);

				//render depending if split vector is at the rigth ot left of v1
				if (p1->x < splitV.x) {
					FlatBotTri(*p0, *p1, splitV);
					FlatTopTri(*p1, splitV, *p2);
				}
				else {
					FlatBotTri(*p0, splitV, *p1);
					FlatTopTri(splitV, *p1, *p2);
				}
			}

		}

		/*writes a rectangle just as the rectangle function and fill it*/
		void FillRect(int x, int y, int w, int h) {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					Point(x + j, y + i);
				}
			}
		}
		void FillRect(const Vec2i& pos, const Vec2i& size) {
			FillRect(pos.x, pos.y, size.x, size.y);
		}

		/*writes a circle just as the circle function, and fill it*/
		void FillCircle(int x, int y, int r) {
			r = std::abs(r);
			int xc = 1, yc = r, d = 3 - (2 * r), yf = 0;
			auto octLine = [&](int yi) {
				for (; yi <= yc; yi++) {
					RightQuadrant(xc, yi, x, y);
				}
				yi = yf + 1;
				for (; yi <= yc; yi++) {
					LeftQuadrant(xc, yi, x, y);
				}
			};

			Point(x, y);
			for (; xc <= r; xc++) {
				Point(x + xc, y);
				Point(x - xc, y);
				Point(x, y + xc);
				Point(x, y - xc);
			}

			xc = 0;
			if (r < 21) {
				xc++;
				if (d < 0) d = d + 4 * xc + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine(++yf);

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 4 * xc + 6;
					else d = d + 8 * (xc - --yc) + 10;
					octLine(++yf);
				}
			}
			else if (r < 81) {
				xc++;
				if (d < 0) d = d + 2 * xc + xc + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine(++yf);

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 2 * xc + xc + 6;
					else d = d + 4 * (xc - --yc) + 10;
					octLine(++yf);
				}
			}
			else {
				xc++;
				if (d < 0) d = d + 2 * xc + xc + (xc >> 1) + 6;
				else d = d + 4 * (xc - --yc) + 10;

				octLine(++yf);

				while (yc >= xc) {
					xc++;
					if (d < 0) d = d + 2 * xc + xc + (xc >> 1) + 6;
					else d = d + 4 * (xc - --yc) + 10;
					octLine(++yf);
				}
			}
		}
		void FillCircle(const Vec2i& pos, int r) {
			FillCircle(pos.x, pos.y, r);
		}

		void DrawImage(const Image& img, int x, int y) {
			for (int imgY = img.height() - 1; imgY >= 0; imgY--) {
				for (int imgX = 0; imgX < img.width(); imgX++) {
					colorSet = img.data()[imgY * img.width() + imgX];
					Point(x + imgX, y + (img.height() - imgY) - 1);
				}
			}
		}

		void DrawImage(const Image& img, int x, int y, int w, int h) {

			if (w == 0 || h == 0) return;

			float xFac = ((float)(img.width()) / (float)(w));
			float yFac = ((float)(img.height()) / (float)(h));

			float xInit = (w >= 0) ? 0.f : img.width();
			int yOff = 0;
			for (float yStep = (h > 0)? 0.f: img.height()
				; yStep <= img.height() && yStep >= 0.f; yStep += yFac) {

				int xOff = 0;
				for (float xStep = xInit; xStep <= img.width() && xStep >= 0.f; xStep += xFac) {

					float finalX = xStep >= img.width()? (img.width() -1): xStep;
					float finalY = yStep >= img.height()? (img.height() - 1): yStep;

					colorSet = img.data()[int(img.height() - 1 - finalY) * img.width() + int(finalX)];

					Point(x + xOff, y + yOff);

					xOff++;
				}

				yOff++;
			}
		}

		void DrawPartialImage(const Image& img, int x, int y, int w, int h, int s, int t, int tW, int tH) {
			if (w == 0 || h == 0 || tW == 0 || tH == 0) return;
			s = clamp(s, 0, img.width() - 1);
			t = clamp(t, 0, img.height() - 1);

			int tWMax = clamp(s + abs(tW), 0, img.width());
			int tHMax = clamp(t + abs(tH), 0, img.height());

			float xFac = ((float)(tWMax - s) / (float)(w));
			float yFac = ((float)(tHMax - t) / (float)(h));

			float xInit = (w >= 0) ? s : tWMax;

			int yOff = 0;
			for (float yStep = (h > 0) ? t : tHMax
				; yStep <= tHMax && yStep >= t; yStep += yFac) {

				int xOff = 0;
				for (float xStep = xInit; xStep <= tWMax && xStep >= s; xStep += xFac) {

					float finalX = xStep >= img.width() ? (img.width() - 1) : xStep;
					float finalY = yStep >= img.height() ? (img.height() - 1) : yStep;

					colorSet = img.data()[int(img.height() - 1 - finalY) * img.width() + int(finalX)];

					Point(x + xOff, y + yOff);

					xOff++;
				}

				yOff++;
			}
		}

		void DrawString(const char* str, int x, int y, int height, Pixel color = { 0,0,0,0 }) {
			int lineX = x;
			int width = (int)(height * fontWHratio);

			color.a = 0;

			for (int i = 0; str[i] != '\0'; i++) {
				if (str[i] == '\n') {
					y += height;
					lineX = x;
				}
				else if (str[i] >= 32 && str[i] < 127) {

					DrawPartialMaskedFontImage(font, lineX, y, width, height,
						((str[i] - 32) % charsXWidth) * fontW,
						((str[i] - 32) / charsXWidth) * fontH + 1, 
						fontW, fontH - 2,
						color
					);

					lineX += width;
				}
				else {
					lineX += width;
				}
			}
		}

		void DrawTexture(const voi::Image& img, int x, int y, int w, int h, float s, float t, float p, float q, voi::Pixel blanking = { 0,0,0 }, ui16 info = 0) {

			if (w == 0 || h == 0 || s - p == 0 || t - q == 0) return;

			if (w < 0.f) {
				std::swap(s, p);
				w = abs(w);
			}
			if (h < 0.f) {
				std::swap(t, q);
				h = abs(h);
			}

			float xFac = ((p - s) / (float)(w));
			float yFac = ((q - t) / (float)(h));

			float yStep = t;
			for (int yOff = 0; yOff < h; yOff++) {

				float xStep = s;
				for (int xOff = 0; xOff < w; xOff++) {

					colorSet = GetTexColor(img, xStep, 1.0f - yStep, info);

					Point(x + xOff, y + yOff);

					xStep += xFac;
				}

				yStep += yFac;
			}
		}


	private:

		template<typename T>
		T clamp(T a, T min, T max) {
			if (a < min) return min;
			if (a > max) return max;
			return a;
		}

		void FlatTopTri(const voi::Vec2f& v0, const voi::Vec2f& v1, const voi::Vec2f& v2) {
			const float slp02 = (v2.x - v0.x) / (v2.y - v0.y);
			const float slp12 = (v2.x - v1.x) / (v2.y - v1.y);

			//setting y top-left rule
			const int yStart = (int)ceil(v0.y - 0.5f);
			const int yEnd = (int)ceil(v2.y - 0.5f); //pixel AFTER the last drawn

			for (int y = yStart; y < yEnd; y++) {

				//calculate x positions based on y position using slopes
				const float px0 = slp02 * (y + 0.5f - v0.y) + v0.x;
				const float px1 = slp12 * (y + 0.5f - v1.y) + v1.x;

				//setting x top-left rule
				const int xStart = (int)ceil(px0 - 0.5f);
				const int xEnd = (int)ceil(px1 - 0.5f);

				for (int x = xStart; x < xEnd; x++) {
					Point(x, y);
				}
			}
		}
		void FlatBotTri(const voi::Vec2f& v0, const voi::Vec2f& v1, const voi::Vec2f& v2) {
			const float slp01 = (v1.x - v0.x) / (v1.y - v0.y);
			const float slp02 = (v2.x - v0.x) / (v2.y - v0.y);

			//setting y top-left rule
			const int yStart = (int)ceil(v0.y - 0.5f);
			const int yEnd = (int)ceil(v2.y - 0.5f); //pixel AFTER the last drawn

			for (int y = yStart; y < yEnd; y++) {

				//calculate x positions based on y position using slopes
				const float px0 = slp01 * (y + 0.5f - v0.y) + v0.x;
				const float px1 = slp02 * (y + 0.5f - v0.y) + v0.x;

				//setting x top-left rule
				const int xStart = (int)ceil(px0 - 0.5f);
				const int xEnd = (int)ceil(px1 - 0.5f);

				for (int x = xStart; x < xEnd; x++) {
					Point(x, y);
				}
			}
		}

		void RightQuadrant(int xc, int yc, int x, int y) {

			//down-right
			Point(xc + x, yc + y);
			//top-left
			Point(-xc + x, -yc + y);
			//right-up
			Point(yc + x, -xc + y);
			//left-down
			Point(-yc + x, xc + y);
		}
		void LeftQuadrant(int xc, int yc, int x, int y) {

			//down-left
			Point(-xc + x, yc + y);
			//top-right
			Point(xc + x, -yc + y);
			//right-down
			Point(yc + x, xc + y);
			//left-up
			Point(-yc + x, -xc + y);
		}

		void DrawPartialMaskedFontImage(const Image& img, int x, int y, int w, int h, int s, int t, int tW, int tH, Pixel color) {
			if (w == 0 || h == 0 || tW == 0 || tH == 0) return;
			s = clamp(s, 0, img.width() - 1);
			t = clamp(t, 0, img.height() - 1);

			int tWMax = clamp(s + abs(tW), 0, img.width());
			int tHMax = clamp(t + abs(tH), 0, img.height());

			float xFac = ((float)(tWMax - s) / (float)(w));
			float yFac = ((float)(tHMax - t) / (float)(h));

			float xInit = (w >= 0) ? s : tWMax;

			int yOff = 0;
			for (float yStep = (h > 0) ? t : tHMax
				; yStep <= tHMax && yStep >= t; yStep += yFac) {

				int xOff = 0;
				for (float xStep = xInit; xStep <= tWMax && xStep >= s; xStep += xFac) {

					float finalX = xStep >= img.width() ? (img.width() - 1) : xStep;
					float finalY = yStep >= img.height() ? (img.height() - 1) : yStep;

					colorSet.u = img.data()[int(img.height() - 1 - finalY) * img.width() + int(finalX)].u | color.u;

					Point(x + xOff, y + yOff);

					xOff++;
				}

				yOff++;
			}

			//colorSet.u = img.data()[(int)imgY * img.width() + (int)imgX].u | color.u;

		}

		voi::Pixel GetTexColor(const voi::Image& img, float x, float y, ui16 info) {

			switch (info & 0xFF) {
				//blank
			case 0: {
				float imgX = x * img.width(), imgY = y * img.height();

				if (imgX >= 0 && imgY >= 0 && imgX <= img.width() && imgY <= img.height()) {
					return GetInternalColor(img, imgX, imgY, info);
				}

				return { 0,0,0 };
			}
				  //clamp
			case 1: {
				float imgX = x * img.width(), imgY = y * img.height();

				imgX = clamp(imgX, (float)0, (float)img.width());
				imgY = clamp(imgY, (float)0, (float)img.height());

				return GetInternalColor(img, imgX, imgY, info);
			}
				  //repeat
			case 2: {
				float fX = floor(x), fY = floor(y);

				if (fX == x && x != 0) x += 1.f;
				if (fY == y && y != 0) y += 1.f;

				x = x - fX;
				y = y - fY;
				float imgX = x * img.width(), imgY = y * img.height();

				return GetInternalColor(img, imgX, imgY, info);
			}
				  //repeat flip
			case 3: {

				float prevX = x, prevY = y;
				float fX = floor(x), fY = floor(y);

				if (fX == x) x += 1.f;
				if (fY == y) y += 1.f;

				x = x - fX;
				y = y - fY;
				if (prevX < 0) x = 1.f - x;
				if (prevY < 0) y = 1.f - y;

				float imgX = x * img.width(), imgY = y * img.height();

				return GetInternalColor(img, imgX, imgY, info);
			}


			default:
				return { 0,0,0 };
			}
		}

		voi::Pixel GetInternalColor(const voi::Image& img, float x, float y, ui16 info) {

			switch ((info >> 8) & 0xFF) {
			case 0:
				if (x > (img.width() - 1)) x = img.width() - 1;
				if (y > (img.height() - 1)) y = img.height() - 1;

				return img.data()[int(y) * img.width() + int(x)];
			case 1: {

				x -= 0.5f;
				y -= 0.5f;

				if (x > (img.width() - 1)) x = img.width() - 1;
				else if (x < 0.f) x = 0.f;

				x = clamp(x, (float)0, (float)img.width() - 1);
				y = clamp(y, (float)0, (float)img.height() - 1);

				float xCeil = ceil(x), xFloor = floor(x);
				float yCeil = ceil(y), yFloor = floor(y);

				float xT = x - xFloor;
				float yT = y - yFloor;

				voi::Pixel yFloorPix = BLAlphaCorrectLerp(
					img.data()[int(yFloor) * img.width() + int(xFloor)],
					img.data()[int(yFloor) * img.width() + int(xCeil)],
					xT
				);

				voi::Pixel yCeilPix = BLAlphaCorrectLerp(
					img.data()[int(yCeil) * img.width() + int(xFloor)],
					img.data()[int(yCeil) * img.width() + int(xCeil)],
					xT
				);

				return  BLAlphaCorrectLerp(yFloorPix, yCeilPix, yT);
			}
			default:
				if (x > (img.width() - 1)) x = img.width() - 1;
				if (y > (img.height() - 1)) y = img.height() - 1;

				return img.data()[int(y) * img.width() + int(x)];
			}
		}

		voi::Pixel BLAlphaCorrectLerp(const voi::Pixel& a, const voi::Pixel& b, float alpha) {
			int intAlpha = alpha * 256;
			int colorIntAlpha;

			if (a.a == 0 && b.a == 0) return { 0 };

			int weightedAvg = ((float)b.a / float(a.a + b.a) * 256);

			if (b.a >= a.a) {
				colorIntAlpha = ((intAlpha << 8) + (255 - intAlpha) * ((weightedAvg - 128) * 2)) >> 8;
			}
			else {
				colorIntAlpha = ((intAlpha << 8) - intAlpha * ((128 - weightedAvg) * 2)) >> 8;
			}

			return {
					(ui8)(((a.r << 8) + (b.r - a.r) * colorIntAlpha) >> 8),
					(ui8)(((a.g << 8) + (b.g - a.g) * colorIntAlpha) >> 8),
					(ui8)(((a.b << 8) + (b.b - a.b) * colorIntAlpha) >> 8),
					(ui8)(((a.a << 8) + (b.a - a.a) * intAlpha) >> 8)
			};
		}


	private:

		HDC context;

		std::chrono::duration<float> elapsedTime;
		std::chrono::system_clock::time_point ts1;
		std::chrono::system_clock::time_point ts2;
		std::chrono::system_clock::time_point tStart;

		float deltaTime;
		float totalTime;

		void First() {
			pixelBuffer = (MapPixel*)(buffInf.buffer);
			context = GetDC(this->winHandle);

			ts1 = std::chrono::system_clock::now();
			ts2 = ts1;
			tStart = ts1;

			PullPadState();

			OnCreate();
			this->UpdateScreen(context);

			_frameCount++;
		}

		void Loop() {
				ts1 = std::chrono::system_clock::now();
				elapsedTime = ts1 - ts2;
				deltaTime = elapsedTime.count();
				ts2 = ts1;

				elapsedTime = tStart - ts1;
				totalTime = elapsedTime.count();

				if (_padConnected || !(_frameCount & 0x3F)) {
					PullPadState();
				}

				OnUpdate(deltaTime);

				this->UpdateScreen(context);

				_frameCount++;
		}

		void Last() {
			ReleaseDC(winHandle, context);
		}
	};

}
#endif
