#pragma once

namespace Lumen::GUI {

// Draws the actual Lumen window — kept deliberately simple: one section
// each for Shadows, Reflections, and Clouds, no raw-struct dump. Called
// every frame from the Present hook while the menu is open.
void DrawLumenMenu(bool *open);

} // namespace Lumen::GUI
