#pragma once

namespace eureka
{
    inline constexpr int DEFAULT_WINDOW_WIDTH = 1024;
    inline constexpr int DEFAULT_WINDOW_HEIGHT = 768;
    inline constexpr int DEFAULT_WINDOW_POS_X = 100;
    inline constexpr int DEFAULT_WINDOW_POS_Y = 100;

    struct WindowPosition
    {
        int x{ 0 };
        int y{ 0 };
    };
    struct WindowConfig
    {
        WindowPosition position{ DEFAULT_WINDOW_POS_X , DEFAULT_WINDOW_POS_Y };
        uint32_t width = DEFAULT_WINDOW_WIDTH;
        uint32_t height = DEFAULT_WINDOW_HEIGHT;
    };
}

