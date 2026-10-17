#pragma once

#include <vector>
#include <string>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include "ImGuiFileDialog.h"
#include "layout.h"
#include "core/type_utils.h"

namespace GUI {
	// Template class for layout and controls of widgets that are bound to a variable's value
	template<typename T>
	class Widget {
	public:
		Widget(const char* id, T& value) : id_(id), value_(value)
		{
			ImGui::PushID(id_);
			width_ = WidgetColumnWidth();
			ImGui::SetNextItemWidth(width_);
		}

		~Widget() {
			ImGui::PopID();
		}

		// Regardless of data type, provides a "Reset" button which sets the value when pressed.
		// Only visible when the value is not the default.
		bool DefaultValue(T defaultValue) {
			bool clicked = false;
			if (value_ != defaultValue) {
				float resetWidth = ResetColumnWidth();
				ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - resetWidth);
				clicked = ImGui::Button("Reset", ImVec2(resetWidth, 0));
				if (clicked) value_ = defaultValue;
			}
			return clicked;
		}

		/* WIDGET TYPES */

		// Box that can be ticked or unticked.
		// Requires boolean value.
		// See ImGui::Checkbox.
		// @returns Whether value changed.

		bool Checkbox() {
			bool* v = valuePtr<bool>(value_);
#ifdef _DEBUG
			if (v == nullptr) throw std::runtime_error(std::format("{} widget, {}, requires {} value type", "Checkbox", id_, "boolean"));
#endif
			return ImGui::Checkbox("##value", v);
		}

		// Solid slider that can be dragged with the cursor to change value between min and max.
		// Requires float value type.
		// See ImGui::SliderFloat.
		// @returns Whether value changed.
		bool Slider(float min, float max, bool logarithmic = false, const char* fmt = "%.2f") {
			float* v = valuePtr<float>(value_);
#ifdef _DEBUG
			if (v == nullptr) throw std::runtime_error(std::format("{} widget, {}, requires {} value type", "Slider (float)", id_, "float"));
#endif
			ImGuiSliderFlags flags = 0;
			if (logarithmic) flags |= ImGuiSliderFlags_Logarithmic;
			return ImGui::SliderFloat("##value", v, min, max, fmt, flags);
		}

		// Solid slider that can be dragged with the cursor to change value between min and max.
		// Requires int value type.
		// See ImGui::SliderInt.
		// @returns Whether value changed.
		bool Slider(int min, int max, bool logarithmic = false) {
			int* v = valuePtr<int>(value_);
#ifdef _DEBUG
			if (v == nullptr) throw std::runtime_error(std::format("{} widget, {}, requires {} value type", "Slider (int)", id_, "integer"));
#endif
			ImGuiSliderFlags flags = 0;
			if (logarithmic) flags |= ImGuiSliderFlags_Logarithmic;
			return ImGui::SliderInt("##value", v, min, max, "%d", flags);
		}

		// Color picker with 3 channels: red, green and blue.
		// Requires glm::vec3 value type.
		// See ImGui::ColorEdit3.
		// @returns Whether value changed.
		bool ColorRGB() {
			glm::vec3* v = valuePtr<glm::vec3>(value_);
#ifdef _DEBUG
			if (v == nullptr) throw std::runtime_error(std::format("{} widget, {}, requires {} value type", "ColorRGB", id_, "glm::vec3"));
#endif
			return ImGui::ColorEdit3("##value", glm::value_ptr(*v));
		}

		// Color picker with 4 channels: red, green and blue.
		// Requires glm::vec4 value type.
		// See ImGui::ColorEdit4.
		// @returns Whether value changed.
		bool ColorRGBA() {
			glm::vec4* v = valuePtr<glm::vec4>(value_);
#ifdef _DEBUG
			if (v == nullptr) throw std::runtime_error(std::format("{} widget, {}, requires {} value type", "ColorRGBA", id_, "glm::vec4"));
#endif
			return ImGui::ColorEdit4("##value", glm::value_ptr(*v));
		}

		// Clickable dropdown for any valid option. For each item, provide a value and a prompt.
		// Does not require a specific value type, assuming the value type has behaviour for an equality operator.
		// See ImGui::Combo.
		// @returns Whether value changed.
		bool Dropdown(const std::vector<std::pair<T, const char*>>& items) {
			int currentIndex = 0;
			bool changed = false;

			for (int i = 0; i < items.size(); i++) {
				if (items[i].first == value_) {
					currentIndex = i;
					break;
				}
			}

			if (ImGui::BeginCombo("##value", items[currentIndex].second)) {
				for (int i = 0; i < items.size(); i++) {
					bool selected = (value_ == items[i].first);
					if (ImGui::Selectable(items[i].second, selected)) {
						if (items[i].first != value_) {
							value_ = items[i].first;
							changed = true;
						}
						break;
					}

					if (selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			return changed;
		}

		// Full file import menu using ImGuiFileDialog.
		// Requires std::string value type for path.
		// See ImGuiFileDialog.
		// @param filter: Must provide file extensions separated by space.
		// @param startPath: Absolute path to directory for starting navigation from.
		// @returns True when a file has been selected (even if repeated)
		bool FilePicker(const char* filter, const char* startPath) {
			static_assert(std::is_same_v<T, std::string>, "FilePicker requires std::string value type."); // Uses a static assert because a filepath will never be a variant like UniformValue

			auto& pathOut = *static_cast<std::string*>(&value_);

			bool changed = false;
			auto dialogKey = std::string("choose_") + id_;

			if (ImGui::Button("Import", ImVec2(scale(IMPORT_BUTTON_WIDTH), 0)))
			{
				IGFD::FileDialogConfig config;
				config.path = startPath;

				ImGuiFileDialog::Instance()->OpenDialog(
					dialogKey,
					id_,
					filter,
					config
				);
			}

			ImGui::SameLine();

			if (!pathOut.empty()) {
				std::string name = std::filesystem::path(pathOut).filename().string();
				ImGui::TextUnformatted(truncateTextToFit(name, width_ - scale(IMPORT_BUTTON_WIDTH) - scale(TEXT_WRAPPING_MARGIN)).c_str());
			}
			else {
				ImGui::TextDisabled("None");
			}

			if (ImGuiFileDialog::Instance()->Display(dialogKey))
			{
				if (ImGuiFileDialog::Instance()->IsOk())
				{
					pathOut = ImGuiFileDialog::Instance()->GetFilePathName();
					changed = true;
				}

				ImGuiFileDialog::Instance()->Close();
			}

			return changed;
		}
	private:
		const char* id_;
		float width_;
		float paddingRight_ = 0.0f;
		T& value_;
	};
}
