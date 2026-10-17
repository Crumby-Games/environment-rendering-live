#pragma once

// Types of mouse actions
enum class MouseButton {
	None,
	Left,
	Middle,
	Right
};

struct Input {
	struct Mouse {
		// Current active MouseButton
		MouseButton activeButton = MouseButton::None;

		// Current x/y coordinates of cursor, relative to top-left of window
		double x = 0.0, y = 0.0;
		// Total x/y change in cursor since last frame / button action start, whichever was sooner
		double dx = 0.0, dy = 0.0;
		// Total mouse wheel change since last frame / button action start, whichever was sooner
		double dw = 0.0;
	};

	// Current mouse state data
	Mouse mouse;
};
