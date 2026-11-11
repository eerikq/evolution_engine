#pragma once

namespace config {
    inline constexpr int MAX_FRAMES_IN_FLIGHT = 2;
#ifdef EVO_DEBUG
    inline constexpr bool VALIDATION_LAYERS_ENABLED = true;
#else
    inline constexpr bool VALIDATION_LAYERS_ENABLED = false;
#endif
} // namespace config
