#pragma once

#include "InteractTextBox.h"

namespace voi {
	class Button : public InteractTextBox {
	public:

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

						action(*this);
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
