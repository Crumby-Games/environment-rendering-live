#include "helpers.h"
#include "layout.h"
#include <imgui.h>

namespace GUI {
	void ImportData::nextState() {
		switch (state) {
		case State::Pending:
			Loading();
			state = State::Loading;
			break;
		case State::Loading:
			try {
				loader(pendingPath);
				path = pendingPath;
			}
			catch (std::exception& e) {
				errorOut = e;
				pendingPath = path;
			}
			state = State::Idle;
			break;
		}
	};

	std::string truncateTextToFit(const std::string& text, float maxWidth) noexcept {
		float width = ImGui::CalcTextSize(text.c_str()).x;
		if (width <= maxWidth) return text;

		float charWidth = width / text.length();
		int maxChars = (int)(maxWidth / charWidth);

		if (text.length() > maxChars)
			return maxChars <= 3 ? "..." : text.substr(0, maxChars - 3) + "...";

		return text;
	}

	bool StaticSidebarWindow(const char* label, float* width) {
		ImVec2 display = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowPos(ImVec2(display.x - *width, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(*width + SIDEBAR_EDGE_BUFFER, display.y + SIDEBAR_EDGE_BUFFER), ImGuiCond_Always);

		ImGui::SetNextWindowSizeConstraints(
			ImVec2(scale(SIDEBAR_WIDTH_MIN), display.y + SIDEBAR_EDGE_BUFFER),
			ImVec2(std::max(scale(SIDEBAR_WIDTH_MIN), display.x * SIDEBAR_WIDTH_MAX_RATIO), display.y + SIDEBAR_EDGE_BUFFER)
		);

		ImGui::Begin(label, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

		if (ImGui::GetWindowSize().x - SIDEBAR_EDGE_BUFFER != *width) {
			*width = ImGui::GetWindowSize().x - SIDEBAR_EDGE_BUFFER;
			return true;
		}
		return false;
	}

	void Label(const char* name) {
		ImGui::AlignTextToFramePadding();
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + LabelColumnWidth() - scale(TEXT_WRAPPING_MARGIN));
		ImGui::TextWrapped("%s",name);
		ImGui::PopTextWrapPos();
		ImGui::SameLine(ImGui::GetCursorPosX() + LabelColumnWidth());
	}

	void WideLabel(const char* name) {
		ImGui::AlignTextToFramePadding();
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + LabelColumnWidth() + WidgetColumnWidth() + ResetColumnWidth() - scale(TEXT_WRAPPING_MARGIN));
		ImGui::TextWrapped("* %s *",name);
		ImGui::PopTextWrapPos();
	}

	void Tooltip(const char* text) {
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			float wrapWidth;
			ImVec2 size = ImGui::CalcTextSize(text);
			wrapWidth = ceil(sqrt(size.x * size.y) * 3);
			wrapWidth = std::min(scale(MAX_TOOLTIP_WIDTH), wrapWidth);
			ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + wrapWidth);
			ImGui::TextUnformatted(text);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	bool WideButton(const char* label) {
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + LabelColumnWidth());
		return ImGui::Button(label, ImVec2(WidgetColumnWidth(), 0));
	}

	void Vec3Display(glm::vec3 data) {
		ImGui::TextWrapped("(%4.1f %4.1f %4.1f)", data.x, data.y, data.z);
	}

	void Mat4Table(const char* id, glm::mat4 data) {
		if (ImGui::BeginTable(id, 4)) {
			for (int row = 0; row < 4; row++) {
				ImGui::TableNextRow();
				for (int column = 0; column < 4; column++) {
					ImGui::TableNextColumn();
					ImGui::Text("%.1f", data[column][row]);
				}
			}
			ImGui::EndTable();
		}
	}

	void Loading() {
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings;

		ImGui::Begin("Loading##env", nullptr, flags);
		ImGui::TextUnformatted("Loading...");
		ImGui::End();
	}
}
