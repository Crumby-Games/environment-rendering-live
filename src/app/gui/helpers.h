#pragma once

#include <string>
#include <glm/glm.hpp>
#include <optional>
#include <functional>

namespace GUI {
	// Abstraction of data required to handle GUI feedback when assets are loaded on the main thread
	struct ImportData {
		ImportData(std::optional<std::exception>& errDisplay, std::function<void(const std::string&)> loader, const std::optional<std::string> defaultFilePath = std::nullopt)
			: errorOut(errDisplay), loader(loader), defaultPath(defaultFilePath), path(defaultFilePath.value_or("")), pendingPath(path) {
		}
		enum class State { Idle, Pending, Loading };
		std::optional<std::exception>& errorOut;
		std::function<void(const std::string&)> loader;
		const std::optional<std::string> defaultPath;
		std::string path;
		std::string pendingPath;
		State state = State::Idle;

		void load() { state = State::Pending; }
		void nextState();
	};

	// An angle (in radians) formatted to be in degrees and between -360 and 360.
	inline float prettyAngle(float radians) noexcept { return fmod(glm::degrees(radians), 360.0f); }
	// Limits length of string to maxWidth (in pixels). If the text is too long, its end is replaced with "...". Will never be shorter than "...".
	std::string truncateTextToFit(const std::string& text, float maxWidth) noexcept;
	// Sidebar window that is only resizable on the left edge.
	bool StaticSidebarWindow(const char* label, float* width);
	// Simple inline label which works within the sidebar's columns.
	void Label(const char* name);
	void WideLabel(const char* name);
	// Simple plaintext tooltip which is shown when hovering over the last element
	void Tooltip(const char* text);
	// Simple button that takes the space of a widget. If no Label is specified before, it will take both the label and widget columns.
	bool WideButton(const char* label);
	// Shorthand for displaying a vector as a string.
	void Vec3Display(glm::vec3 data);
	// Displays a 4x4 matrix as a 4x4 table, taking up full horizontal space.
	void Mat4Table(const char* id, glm::mat4 data);
	// Notification when a load on the main thread is required
	void Loading();
}
