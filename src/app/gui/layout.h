#pragma once

namespace GUI {
	// Font size before scaling
	constexpr float BASE_FONT_SIZE = 14.f;
	// The number of pixels that the sidebar is extended on the right and bottom side.
	// Used to push right and bottom edge off the screen to make them undraggable.
	constexpr float SIDEBAR_EDGE_BUFFER = 5.f;
	// Before scaling, starting width of sidebar in pixels
	constexpr float SIDEBAR_INITIAL_WIDTH = 400.f;
	// Absolute minimum width of sidebar in pixels
	constexpr float SIDEBAR_WIDTH_MIN = 350.f;
	// Maximum width of sidebar relative to full application window
	constexpr float SIDEBAR_WIDTH_MAX_RATIO = 0.75f;
	// Maximum width of tooltips in pixels
	constexpr float MAX_TOOLTIP_WIDTH = 500.f;
	// Number of pixels of additional space provided on the right side of some wrapped text.
	constexpr float TEXT_WRAPPING_MARGIN = 5.f;
	// Width of "import" button used in FilePicker widget. 
	constexpr float IMPORT_BUTTON_WIDTH = 60.0f;

	/* GUI Scaling */
	inline float scaleBase = 1.0;
	inline float scaleFactor = 1.0;
	inline float scale(float x) noexcept { return x * scaleBase * scaleFactor; }

	// Space (in pixels) between left margin (after indentation) and widget column. Dynamic.
	float LabelColumnWidth();
	// Space (in pixels) between label and reset columns. Derived entirely from remaining space after the label and reset columns.
	float WidgetColumnWidth();
	// Space (in pixels) of reset column. Constant.
	float ResetColumnWidth();
}
