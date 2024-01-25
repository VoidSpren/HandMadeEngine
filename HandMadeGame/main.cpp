#include <string>
#include <Windows.h>
#include <Xinput.h>

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "utilDefs.h"
#include "Image.h"
#include "voiengine.h"
#include "PixelDefs.h"
#include "SerialPort.h"
#include "TextBox.h"
#include "Box.h"
#include "Button.h"
#include "TextInput.h"

class Testing : public voi::VoiEngine {

	/*---- RUNNING CODE ----*/

	int charW;
	int charH = 20;
	voi::TextInput Input;

	voi::Button button;

	voi::TextBox text;

	void OnCreate() override {
		clearColor = { 255,255,255 };

		charW = CharWidth(charH);

		Input.attach(this);
		Input.setBox(20, 20, 550, 90);
		Input.setBorderColor({ 128,128,128 });
		Input.setBackOverColor({ 240,240,240 });
		Input.setBorderOverColor({ 128,128,128 });
		Input.setBackClickColor({ 240,240,240 });

		button.attach(this);
		button.setBox(590, 20, 250, 90);
		button.setText("set Input to default");

		text.attach(this);
		text.setBox(20, 130, 550, 360);
	}

	void OnUpdate(float deltaTime) override {
		Clear();

		Input.Draw();
		button.Draw();
		text.Draw();
	}

	void OnMouseMove(voi::MouseInf inf) override {
		Input.ifOnOver(inf);
		button.ifOnOver(inf);
	}

	void OnMouseClick(voi::MouseAccess key, bool state) override {
		Input.onClick(state);
		button.onClick(state, [&](){ Input.setText(""); });
	}

	void OnKeyDown(voi::KeyAccess key) override {
		Input.onKeyDown(key);
		Input.onEnter(key, [&]() { text.setText(Input.getText()); });
	}

};

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode) {

	Testing test;
	if (test.Construct(instance, L"Testing", 1300,900)) test.Start();

	return 0;
}



/*----------------------ARDUINO INTERFACE------------------------------*/

/*-----keydown------*/

		//switch (mode) {

		//case 1:
		//	{
		//		WORD inChar;
		//		BYTE kbState[256];

		//		switch (key) {
		//		case voi::BACK:
		//			if (str.length() > 0) {
		//				if (str.back() == '\n') {
		//					lineY--;
		//				}
		//				else {
		//					lineX[lineY]--;
		//				}
		//				str.pop_back();
		//			}
		//			break;

		//		case voi::RETURN:
		//			//str.push_back('\n');
		//			//lineY++;
		//			//lineX[lineY] = 0;

		//			DrawString("waiting for connection...", 5, 30, charH);
		//			mode = 2;
		//			break;

		//		default:
		//			GetKeyboardState(kbState);
		//			int err = ToAscii(
		//				key, MapVirtualKeyA(key, MAPVK_VK_TO_VSC),
		//				kbState,
		//				&inChar,
		//				0
		//			);

		//			if (err < 1) errStr = "No Translation for key";
		//			else {
		//				str.push_back((char)(inChar & 0xFF));
		//				lineX[lineY]++;

		//				errStr = "";
		//				errStr.push_back(key);
		//			}

		//			break;
		//		}
		//	}
		//	break;

		//case 3:
		//	if (key == voi::RETURN) mode = 1;

		//default:
		//	if (key == voi::ESC) mode = 1;

		//	break;
		//}

/*----update--------*/

		//switch (mode) {

		//case 1:

		//	if (waitBarVisible) {
		//		colorSet = { 0,0,0 };
		//		FillRect(5 + charW * lineX[lineY], 5 + charH * lineY, 5, charH);
		//	}

		//	break;
		//case 2:
		//	arduino.Disconnect();
		//	arduino.Connect(str.c_str(), 2000);
		//	if (arduino.isConnected()) mode = 0;
		//	else mode = 3;

		//	break;

		//case 3:
		//	DrawString(arduino.ErrorMsg().c_str(), 5, 30, charH, { 255,0,0 });
		//	DrawString("Press \"Enter\" to restart and resubmit portName...", 5, 50, charH, { 255,0,0 });

		//	if (waitBarVisible) {
		//		colorSet = { 255,0,0 };
		//		FillRect(5 + charW * 49, 35 + charH * 1, 5, charH);
		//	}
		//	break;
		//default:
		//	{
		//		/*-------------Serial printing--------------*/
		//		
		//		char buffer[1024];

		//		int reads = arduino.ReadSerialPort(buffer, sizeof(buffer) - 1);
		//		buffer[reads] = 0;

		//		if (reads > 0) {
		//			serialBuffer.append(buffer);
		//			size_t pos = serialBuffer.find_last_of('\n');

		//			if (pos != std::string::npos) {
		//				serialDisplay.append(serialBuffer.substr(0, pos + 1));
		//				serialBuffer = serialBuffer.substr(pos + 1);
		//			}

		//			if (serialDisplay.length() >= 340) {
		//				serialDisplay.erase(0, serialDisplay.length() - 340);

		//				size_t pos = serialDisplay.find_first_of('\n');
		//				if (pos != std::string::npos) {
		//					serialDisplay.erase(0, pos + 1);
		//				}
		//			}

		//		}

		//		std::stringstream strStream;
		//		strStream << reads << ' ' << serialDisplay.length();
		//		DrawString(strStream.str().c_str(), width() - 5 - strStream.str().length() * charW, 5, charH, { 0,255,0 });

		//		DrawString(serialDisplay.c_str(), 5, 30, charH, { 0,0,255 });
		//		/*-----------------------------------------------*/

		//		
		//	}

		//	break;
		//}
