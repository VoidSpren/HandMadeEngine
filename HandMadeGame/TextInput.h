#pragma once

#include <string>

#include "voiengine.h"
#include "InteractTextBox.h"

namespace voi {
	class TextInput : public InteractTextBox {
	protected:
		int displayCharCount = 0;
		int displayOffset = 0;
		int editPos = 0;
		std::string backingTxt = "";


	public:

		int calcDisplayProperties() {
			displayCharCount = (getWidth() * getTextWPercentage()) / (getHeight() * getTextHPercentage() * engine->FontWHRatio()) + 2;
			text = "";
			text.insert(0, displayCharCount, ' ');

			if (editPos < text.size()) {
				text.replace(0, backingTxt.size(), backingTxt);
			}
			else {
				text.replace(0, text.size(), backingTxt, backingTxt.size() - text.size());
			}

			calcLinesProperties();
			return displayCharCount;
		}

		void setBox(int x, int y, int w, int h) { setBox({ x,y,w,h }); }
		void setSize(Vec2i vec) { setSize(vec.x, vec.y); }

		virtual void setBox(Vec4i vec) override {
			box = vec;

			calcDisplayProperties();
		}

		virtual void setSize(int x, int y) override {
			box.z = x; box.w = y;

			calcDisplayProperties();
		}

		void onKeyDown(KeyAccess key) {
			if (isActive) {
				WORD inChar;
				BYTE kbState[256];

				switch (key) {

				case voi::LEFT:
					if (editPos > 0) {
						editPos--;

						if (editPos < displayOffset && displayOffset > 0) {
							displayOffset--;
							text.replace(0, text.size(), backingTxt, displayOffset, text.size());
						}
					}

					break;
				case voi::RIGHT:
					if (editPos < backingTxt.size()) {
						editPos++;
						int displayEndOffset = displayOffset + text.size();

						if (editPos > displayEndOffset && displayEndOffset < backingTxt.size()) {
							displayOffset++;
							text.replace(0, text.size(), backingTxt, displayOffset, text.size());
						}
					}
					break;

				case voi::BACK:
					if (editPos > 0) {

						if (backingTxt.size() <= text.size()) {
							backingTxt.erase(editPos - 1, 1);
							text.erase(editPos - 1, 1);
							text.push_back(' ');
						}
						else {

							backingTxt.erase(editPos - 1, 1);


							if (displayOffset + text.size() > backingTxt.size()) {
								displayOffset--;
							}
							text.replace(0, text.size(), backingTxt, displayOffset, text.size());
						}

						editPos--;
					}
					break;

				case voi::RETURN:
					break;

				default:
					GetKeyboardState(kbState);
					int err = ToAscii(
						key, MapVirtualKeyA(key, MAPVK_VK_TO_VSC),
						kbState,
						&inChar,
						0
					);

					if (err == 1) {
						char c = (char)(inChar & 0xFF);
						backingTxt.insert(editPos, 1, c);

						if (backingTxt.size() <= text.size()) {
							text.insert(editPos, 1, c);
							text.pop_back();
						}
						else {
							if ((editPos - displayOffset) >= text.size()) {
								displayOffset++;
							}

							text.replace(0, text.size(), backingTxt, displayOffset, text.size());
						}


						editPos++;
					}

					break;
				}
			}
		}

		void onClick(bool state) {
			if (state && over) {
				isClicked = true;

				backColor = backClickColor;
				borderColor = borderClickColor;
			}
			else {

				if (Box::ifOnOver(lastInfo)) {
					if (isClicked) {
						isActive = true;
					}
				}
				else {
					movedIn = false;
					isActive = false;
				}


				restoreStyle();
				isClicked = false;
			}
		}	

		virtual void restoreStyle() override {
			if (isActive) {
				backColor = backActiveColor;
				borderColor = borderActiveColor;
			}
			else {
				backColor = backColorMemory;
				borderColor = borderColorMemory;
			}
		}

		virtual void Draw() override {
			InteractTextBox::Draw();

			if (isActive) {
				engine->colorSet = { 255,255,255 };
				if ((displayOffset + displayCharCount) < backingTxt.size()) {
					engine->FillRect(lineX + charW * displayCharCount - charW / 3, lineY, charW / 3, charH + 1);
				}
				if (displayOffset > 0) {
					engine->FillRect(lineX, lineY, charW / 3, charH + 1);
				}

				if ((int(engine->TotalTime()) - engine->TotalTime()) > 0.5f) {
					engine->colorSet = { 0,0,0 };

					if ((editPos - displayOffset) >= displayCharCount) engine->FillRect(lineX + charW * displayCharCount - charW / 3, lineY, charW / 3, charH + 1);
					else engine->FillRect(lineX + charW * (editPos - displayOffset), lineY + charH * 0.9, charW, charH * 0.1);
				}
			}

		}
	};
}
