#pragma once

#include "InteractTextBox.h"

namespace voi {
	class Button : public InteractTextBox {
	public:

		Button() {
			backOverColor = { 200,200,200 };
			backClickColor = { 150,150,150 };
		}

		template<typename _OnClick>
		void onClick(bool state, _OnClick action) {
			if (state && over) {
					isClicked = true;

					backColor = backClickColor;
					borderColor = borderClickColor;
			}
			else {

				if (Box::ifOnOver(lastInfo)) {
					if (isClicked) {
						backColor = backOverColor;
						borderColor = borderOverColor;

						action();
					}
				}
				else {
					movedIn = false;
					restoreStyle();
				}
				isClicked = false;
			}
		}
	};
}
