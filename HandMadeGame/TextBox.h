#pragma once
#include <string>

#include "Box.h"

namespace voi {
	class TextBox: public Box{
	protected:

		int charH;
		int charW;

		int textH;
		int textW;

		int lineY;
		int lineX;

		int returnN;
		int maxLine;

		float textHPercentage;
		float textWPercentage;

		std::string text;

	public:
		TextBox() : Box({ 0,0,0,0 }) ,charH(0), charW(0), lineX(0), lineY(0), text("") {
			textHPercentage = 0.75f;
			textWPercentage = 0.9f;
		}

		void calcTextProperties() {
			textH = box.w * textHPercentage;
			charH = round((float)textH / (returnN + 1));

			charW = engine->CharWidth(charH);
			textW = charW * maxLine;
			float MaxTextW = box.z * textWPercentage;
			 
			if (textW > MaxTextW) {
				charW = round(MaxTextW / maxLine);
				charH = engine->CharHeight(charW);
				charW = engine->CharWidth(charH);

				textH = charH * (returnN + 1);
				textW = charW * maxLine;
			}

			lineY = round(box.w / 2.0f - textH / 2.0f + box.y);
			lineX = round(box.z / 2.0f - textW / 2.0f + box.x);
		}

		int getCharHeight() { return charH; }
		int getcharWidth() { return charW; }

		float getTextHPercentage() { return textHPercentage; }
		float getTextWPercentage() { return textWPercentage; }

		void calcLinesProperties() {
			returnN = 0;
			maxLine = 0;
			int count = 0;
			for (auto c : text) {
				if (c == '\n') {
					returnN++;
					if (count > maxLine) maxLine = count;
					count = 0;
				}
				else {
					count++;
				}
			}
			if (returnN == 0) maxLine = count;
			calcTextProperties();
		}

		void setBox(int x, int y, int w, int h) { setBox({ x,y,w,h }); }
		void setPos(Vec2i vec) { setPos({ vec.x,vec.y }); }
		void setSize(Vec2i vec) { setSize(vec.x, vec.y); }

		virtual void setBox(Vec4i vec) override {
			box = vec;
			calcTextProperties();
		}

		virtual void setPos(int x, int y) override {
			lineY += y - box.y;
			lineX += x - box.x;

			box.x = x; box.y = y;
		}

		virtual void setSize(int x, int y) override {
			box.z = x; box.w = y;
			calcTextProperties();
		}


		virtual void setText(std::string nText) {
			text = nText;
			calcLinesProperties();
		}
		
		void setText(const char* nText) { setText(std::string(nText)); }

		std::string getText() const { return text; }

		virtual void Draw() override {
			this->Box::Draw();

			engine->DrawString(text.c_str(), lineX, lineY, charH);
		}
	};
}
