#pragma once

#include "voiengine.h"
#include "TextBox.h"

namespace voi {
	class InteractTextBox : public TextBox {
	protected:

		bool movedIn = false;
		bool isClicked = false;
		bool isActive = false;
		MouseInf lastInfo;


		Pixel backOverColor;
		Pixel borderOverColor;

		Pixel backClickColor;
		Pixel borderClickColor;

		Pixel backActiveColor;
		Pixel borderActiveColor;

	public:
		InteractTextBox() : TextBox() {
			backColorMemory = backColor;
			borderColorMemory = borderColor;
			backOverColor = backColor;
			borderOverColor = borderColor;
			backClickColor = backColor;
			borderClickColor = borderColor;
			backActiveColor = backColor;
			borderActiveColor = borderColor;
		}

		virtual void restoreStyle() {
			backColor = backColorMemory;
			borderColor = borderColorMemory;
		}

		void setBackOverColor(ui8 r, ui8 g, ui8 b, ui8 a = 255) { setBackOverColor({ r,g,b,a }); }
		Pixel getBackOverColor() { return backOverColor; }
		virtual void setBackOverColor(Pixel color) { backOverColor = color; }

		void setBorderOverColor(ui8 r, ui8 g, ui8 b, ui8 a = 255) { setBorderOverColor({ r,g,b,a }); }
		Pixel getBorderOverColor() { return borderOverColor; }
		virtual void setBorderOverColor(Pixel color) { borderOverColor = color; }

		void setBackClickColor(ui8 r, ui8 g, ui8 b, ui8 a = 255) { setBackClickColor({ r,g,b,a }); }
		Pixel getBackClickColor() { return backClickColor; }
		virtual void setBackClickColor(Pixel color) { backClickColor = color; }

		void setBorderClickColor(ui8 r, ui8 g, ui8 b, ui8 a = 255) { setBorderClickColor({ r,g,b,a }); }
		Pixel getBorderClickColor() { return borderClickColor; }
		virtual void setBorderClickColor(Pixel color) { borderClickColor = color; }


		void setBox(int x, int y, int w, int h) { setBox({ x,y,w,h }); }
		void setPos(Vec2i vec) { setPos({ vec.x,vec.y }); }
		void setSize(Vec2i vec) { setSize(vec.x, vec.y); }

		virtual void setBox(Vec4i vec) override {
			TextBox::setBox(vec);
			if (!Box::ifOnOver(lastInfo)) {
				restoreStyle();
			}
		}

		virtual void setPos(int x, int y) override {
			TextBox::setPos(x, y);
			if (!Box::ifOnOver(lastInfo)) {
				restoreStyle();
			}
		}

		virtual void setSize(int x, int y) override {
			TextBox::setSize(x, y);
			if (!Box::ifOnOver(lastInfo)) {
				restoreStyle();
			}
		}

		virtual bool ifOnOver(const MouseInf& info) {

			lastInfo = info;
			if (isClicked || isActive)
			{
				return Box::ifOnOver(info);
			}

			if (Box::ifOnOver(info)) {
				if (!movedIn) {
					movedIn = true;

					backColor = backOverColor;
					borderColor = borderOverColor;
				}
			}
			else if (movedIn) {
				movedIn = false;
				restoreStyle();
			}

			return over;
		}
	};
}
