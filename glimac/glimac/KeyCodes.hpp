#pragma once

#include <vector>

#include "common.hpp"
#include "glm.hpp"

#define MIN_ZOOM 1

using namespace glm;

namespace glimac
{

    enum movKeys
    {
        keyUp       = (1u << 0),
        keyDown     = (1u << 1),
        keyLeft     = (1u << 2),
        keyRight    = (1u << 3),
        scrollUp    = (1u << 4),
        scrollDown  = (1u << 5),
        shiftKey    = (1u << 6),
        switchMode  = (1u << 7),
        rotateLeft  = (1u << 8),
        rotateRight = (1u << 9),
        rotateUp    = (1u << 10),
        rotateDown  = (1u << 11),
        rightClick  = (1u << 12),
        ctrlKey     = (1u << 13),
        leftClick   = (1u << 14),
        spacebar    = (1u << 15),
        keySun      = (1u << 16),
        keyDebug    = (1u << 17)
    };

}