#pragma once
#define __QOR_CHAR_PRERENDER_SRC__

#include "../include/text_builder.h"
#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>


struct __FreetypeContext
{
    FT_Library library;
    FT_Face face;
    FT_GlyphSlot slot;
};