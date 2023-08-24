#pragma once

#include "utilDefs.h"
#include "LinearAlg.h"

namespace voi {

	struct Pixel;

	/*---------- Memory plain RGB pixel representation ------------*/

	struct MapPixel {
		MapPixel() : u(0) {}
		MapPixel(ui8 r, ui8 g, ui8 b) : b(b), g(g), r(r), x(0) {}
		MapPixel(const MapPixel& other) { u = other.u; }
		void operator= (const MapPixel& other) { u = other.u; }

		MapPixel(const Pixel& other);

		union {
			ui32 u;
			struct {
				ui8 b;
				ui8 g;
				ui8 r;
				ui8 x;
			};
		};

		void SetColor(ui8 _r, ui8 _g, ui8 _b) {
			r = _r;
			g = _g;
			b = _b;
			x = 0;
		}
	};

	/*---------- RGBA pixel representation ------------*/

	struct Pixel {
		Pixel() : u(0xFF000000) {}
		Pixel(ui32 u): u(u) {}
		Pixel(ui8 r, ui8 g, ui8 b) : b(b), g(g), r(r), a(255) {}
		Pixel(ui8 r, ui8 g, ui8 b, ui8 a) : b(b), g(g), r(r), a(a) {}
		Pixel(const Pixel& other) { u = other.u; }
		template<typename T>
		Pixel(const Vec4<T>& other): b(other.z), g(other.y), r(other.x), a(other.w) {}
		void operator= (const Pixel& other) { u = other.u; }

		Pixel(const MapPixel& other) {
			u = other.u;
			a = 255;
		}

		union {
			ui32 u;
			struct {
				ui8 b;
				ui8 g;
				ui8 r;
				ui8 a;
			};
		};

		static Pixel lerp(const Pixel& a, const Pixel& b, const float alpha) {
			const int intAlpha = (int)(alpha * 256.f);

			return {
				(ui8)((a.r * 256 + (b.r - a.r) * intAlpha) >> 8),
				(ui8)((a.g * 256 + (b.g - a.g) * intAlpha) >> 8),
				(ui8)((a.b * 256 + (b.b - a.b) * intAlpha) >> 8),
				(ui8)((a.a * 256 + (b.a - a.a) * intAlpha) >> 8)
			};
		}
	};

	MapPixel::MapPixel(const Pixel& other) { u = other.u; }

	template<typename T> Vec4<T>::Vec4(const Pixel& other){
		x = other.r;
		y = other.g;
		z = other.b;
		w = other.a;
	}
}
