#pragma once

#include "utilDefs.h"

#include "LinearAlg.h"
#include "voiengine.h"
#include "PixelDefs.h"

namespace voi {
	class Box {
	protected:
		VoiEngine* engine;
		Vec4i box;

		Pixel backColorMemory;
		Pixel borderColorMemory;

		Pixel backColor;
		Pixel borderColor;

		bool over = false;
	public:
		Box(): engine(NULL), box{0,0,0,0}, backColor{ 255,255,255 }, borderColor{ 0,0,0 } {}

		Box(Vec4i _vec)
			:engine(NULL), box(_vec), backColor{255,255,255}, borderColor{0,0,0} {}
		Box(Vec4i _vec, Pixel _backColor)
			:engine(NULL), box(_vec), backColor(_backColor), borderColor{ 0,0,0 } {}
		Box(Vec4i _vec, Pixel _backColor, Pixel _borderColor)
			:engine(NULL), box(_vec), backColor(_backColor), borderColor(_borderColor) {}

		void attach(VoiEngine* engineParam) { engine = engineParam; }

		void setBox(int x, int y, int w, int h) { setBox({ x,y,w,h }); }
		virtual void setBox(Vec4i vec) { box = vec; }
		Vec4i getBox() const { return box; }

		int getX() { return box.x; }
		int getY() { return box.y; }
		int getWidth() { return box.z; }
		int getHeight() { return box.w; }

		void setPos(Vec2i vec) { setPos(vec.x,vec.y); }
		virtual void setPos(int x, int y) { box.x = x; box.y = y; }
		Vec2i getPos() const { return { box.x, box.y }; }

		void setSize(Vec2i vec) { setSize(vec.x, vec.y); }
		virtual void setSize(int x, int y) { box.z = x; box.w = y; }
		Vec2i getSize() const { return { box.z, box.w }; }

		void setBackColor(ui8 r, ui8 g, ui8 b, ui8 a = 255) { setBackColor({ r,g,b,a }); }
		Pixel getBackColor() const { return backColorMemory; }
		virtual void setBackColor(Pixel color) { backColor = color; backColorMemory = color; }

		void setBorderColor(ui8 r, ui8 g, ui8 b, ui8 a = 255) { setBorderColor({ r,g,b,a }); }
		Pixel getBorderColor() const { return borderColorMemory; }
		virtual void setBorderColor(Pixel color) { borderColor = color; borderColorMemory = color; }

		void setBackDisplayColor(ui8 r, ui8 g, ui8 b, ui8 a = 255) { setBackDisplayColor({ r,g,b,a }); }
		Pixel getBackDisplayColor() const { return backColor; }
		virtual void setBackDisplayColor(Pixel color) { backColor = color; }

		void setBorderDisplayColor(ui8 r, ui8 g, ui8 b, ui8 a = 255) { setBorderDisplayColor({ r,g,b,a }); }
		Pixel getBorderDisplayColor() const { return borderColor; }
		virtual void setBorderDisplayColor(Pixel color) { borderColor = color; }

		void restoreBackDisplayColor() { backColor = backColorMemory; }
		void restoreBorderDisplayColor() { borderColor = borderColorMemory; }

		virtual bool ifOnOver(const MouseInf& info) {
			over = (
				info.pos.x >= box.x && info.pos.x <= (box.x + box.z) &&
				info.pos.y >= box.y && info.pos.y <= (box.y + box.w)
				);

			return over;
		}

		virtual void Draw() {
			engine->colorSet = backColor;
			engine->FillRect(box.x, box.y, box.z, box.w);

			engine->colorSet = borderColor;
			engine->Rect(box.x, box.y, box.z, box.w);

		}
	};
}