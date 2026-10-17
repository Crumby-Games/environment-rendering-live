#include "layout.h"

#include <utility>
#include <imgui.h>

static constexpr float LABEL_WIDTH_MIN = 100.f;
static constexpr float LABEL_WIDTH_RATIO = 0.2f;
static constexpr float RESET_BUTTON_WIDTH = 60.0f;

float GUI::LabelColumnWidth() {
	return std::max(scale(LABEL_WIDTH_MIN), LABEL_WIDTH_RATIO * ImGui::GetWindowWidth());
}

float GUI::ResetColumnWidth() {
	return scale(RESET_BUTTON_WIDTH);
}

float GUI::WidgetColumnWidth() {
	return ImGui::GetWindowContentRegionMax().x - SIDEBAR_EDGE_BUFFER - ResetColumnWidth() - ImGui::GetCursorPosX();
}
