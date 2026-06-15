#include "pr_vtext.h"
#include "pr_game_context.h"
#include "../d3d11_renderer.h"
#include "../resource_manager.h"
#include "../logger.h"

// Minimal bitmap font for debug/fallback text rendering
// Each glyph is 5x7 pixels stored as 7 bytes (bit per pixel, MSB-left)
static const uint8_t s_font5x7[][7] = {
    // ' ' (space)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // '!'
    {0x20,0x20,0x20,0x20,0x00,0x20,0x00},
    // '"'
    {0x50,0x50,0x00,0x00,0x00,0x00,0x00},
    // '#'
    {0x50,0xF8,0x50,0xF8,0x50,0x00,0x00},
    // '$'
    {0x20,0x78,0xA0,0x70,0x28,0xF0,0x20},
    // '%'
    {0xC8,0xD0,0x20,0x40,0x58,0x98,0x00},
    // '&'
    {0x40,0xA0,0x40,0xA8,0x90,0x68,0x00},
    // '\''
    {0x20,0x20,0x00,0x00,0x00,0x00,0x00},
    // '('
    {0x10,0x20,0x20,0x20,0x20,0x10,0x00},
    // ')'
    {0x40,0x20,0x20,0x20,0x20,0x40,0x00},
    // '*'
    {0x00,0x50,0x20,0x70,0x20,0x50,0x00},
    // '+'
    {0x00,0x20,0x20,0xF8,0x20,0x20,0x00},
    // ','
    {0x00,0x00,0x00,0x00,0x20,0x20,0x40},
    // '-'
    {0x00,0x00,0x00,0xF8,0x00,0x00,0x00},
    // '.'
    {0x00,0x00,0x00,0x00,0x00,0x20,0x00},
    // '/'
    {0x08,0x10,0x20,0x40,0x80,0x00,0x00},
    // '0'
    {0x70,0x88,0x98,0xA8,0xC8,0x70,0x00},
    // '1'
    {0x20,0x60,0x20,0x20,0x20,0x70,0x00},
    // '2'
    {0x70,0x88,0x08,0x30,0x40,0xF8,0x00},
    // '3'
    {0x70,0x88,0x30,0x08,0x88,0x70,0x00},
    // '4'
    {0x10,0x30,0x50,0x90,0xF8,0x10,0x00},
    // '5'
    {0xF8,0x80,0xF0,0x08,0x88,0x70,0x00},
    // '6'
    {0x30,0x40,0xF0,0x88,0x88,0x70,0x00},
    // '7'
    {0xF8,0x08,0x10,0x20,0x40,0x40,0x00},
    // '8'
    {0x70,0x88,0x70,0x88,0x88,0x70,0x00},
    // '9'
    {0x70,0x88,0x78,0x08,0x10,0x60,0x00},
    // ':'
    {0x00,0x20,0x00,0x00,0x20,0x00,0x00},
    // ';'
    {0x00,0x20,0x00,0x00,0x20,0x20,0x40},
    // '<'
    {0x08,0x10,0x20,0x10,0x08,0x00,0x00},
    // '='
    {0x00,0x00,0xF8,0x00,0xF8,0x00,0x00},
    // '>'
    {0x80,0x40,0x20,0x40,0x80,0x00,0x00},
    // '?'
    {0x70,0x88,0x10,0x20,0x00,0x20,0x00},
    // '@'
    {0x70,0x88,0xB8,0xB8,0x80,0x70,0x00},
    // 'A'
    {0x70,0x88,0x88,0xF8,0x88,0x88,0x00},
    // 'B'
    {0xF0,0x88,0xF0,0x88,0x88,0xF0,0x00},
    // 'C'
    {0x70,0x88,0x80,0x80,0x88,0x70,0x00},
    // 'D'
    {0xF0,0x88,0x88,0x88,0x88,0xF0,0x00},
    // 'E'
    {0xF8,0x80,0xF0,0x80,0x80,0xF8,0x00},
    // 'F'
    {0xF8,0x80,0xF0,0x80,0x80,0x80,0x00},
    // 'G'
    {0x70,0x88,0x80,0xB8,0x88,0x70,0x00},
    // 'H'
    {0x88,0x88,0xF8,0x88,0x88,0x88,0x00},
    // 'I'
    {0x70,0x20,0x20,0x20,0x20,0x70,0x00},
    // 'J'
    {0x08,0x08,0x08,0x08,0x88,0x70,0x00},
    // 'K'
    {0x88,0x90,0xA0,0xC0,0xA0,0x90,0x88},
    // 'L'
    {0x80,0x80,0x80,0x80,0x80,0xF8,0x00},
    // 'M'
    {0x88,0xD8,0xA8,0x88,0x88,0x88,0x00},
    // 'N'
    {0x88,0xC8,0xA8,0x98,0x88,0x88,0x00},
    // 'O'
    {0x70,0x88,0x88,0x88,0x88,0x70,0x00},
    // 'P'
    {0xF0,0x88,0xF0,0x80,0x80,0x80,0x00},
    // 'Q'
    {0x70,0x88,0x88,0xA8,0x90,0x68,0x00},
    // 'R'
    {0xF0,0x88,0xF0,0xA0,0x90,0x88,0x00},
    // 'S'
    {0x70,0x80,0x70,0x08,0x88,0x70,0x00},
    // 'T'
    {0xF8,0x20,0x20,0x20,0x20,0x20,0x00},
    // 'U'
    {0x88,0x88,0x88,0x88,0x88,0x70,0x00},
    // 'V'
    {0x88,0x88,0x88,0x50,0x50,0x20,0x00},
    // 'W'
    {0x88,0x88,0xA8,0xA8,0xD8,0x88,0x00},
    // 'X'
    {0x88,0x50,0x20,0x20,0x50,0x88,0x00},
    // 'Y'
    {0x88,0x50,0x20,0x20,0x20,0x20,0x00},
    // 'Z'
    {0xF8,0x10,0x20,0x40,0x80,0xF8,0x00},
};

static int GlyphIndex(char c) {
    if (c >= ' ' && c <= 'Z') return c - ' ';
    if (c >= 'a' && c <= 'z') return c - 'a' + ('A' - ' ');
    return 0; // space for unknown
}

static const char* MapGlyphTextureName(char c) {
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');

    if (c >= '0' && c <= '9') {
        switch (c) {
            case '0': return "g_nam_01";
            case '1': return "g_nam_11";
            case '2': return "g_nam_21";
            case '3': return "g_nam_31";
            case '4': return "g_nam_41";
            case '5': return "g_nam_51";
            case '6': return "g_nam_61";
            case '7': return "g_nam_71";
            case '8': return "g_nam_81";
            case '9': return "g_nam_91";
        }
    }

    if (c >= 'A' && c <= 'Z') {
        switch (c) {
            case 'A': return "g_nam_a1";
            case 'B': return "g_nam_b1";
            case 'C': return "g_nam_c1";
            case 'D': return "g_nam_d1";
            case 'E': return "g_nam_e1";
            case 'F': return "g_nam_f1";
            case 'G': return "g_nam_g1";
            case 'H': return "g_nam_h1";
            case 'I': return "g_nam_i1";
            case 'J': return "g_nam_j1";
            case 'K': return "g_nam_k1";
            case 'L': return "g_nam_l1";
            case 'M': return "g_nam_m1";
            case 'N': return "g_nam_n1";
            case 'O': return "g_nam_o1";
            case 'P': return "g_nam_p1";
            case 'Q': return "g_nam_q1";
            case 'R': return "g_nam_r1";
            case 'S': return "g_nam_s1";
            case 'T': return "g_nam_t1";
            case 'U': return "g_nam_u1";
            case 'V': return "g_nam_v1";
            case 'W': return "g_nam_w1";
            case 'X': return "g_nam_x1";
            case 'Y': return "g_nam_y1";
            case 'Z': return "g_nam_z1";
        }
    }

    if (c == '_') return "g_nam__1";
    if (c == '!') return "g_nam_ex";
    if (c == '-') return "g_nam_mi";
    if (c == '.') return "g_nam_pr";
    if (c == ',') return "g_nam_cm";
    if (c == '?') return "g_nam_qu";
    if (c == '\'') return "g_nam_fp";
    if (c == '"') return "g_nam_mk";
    if (c == ':') return "g_nam_cc";
    if (c == '+') return "g_nam_pl";
    if (c == '=') return "g_nam_eq";
    if (c == '#') return "g_nam_sh";
    if (c == '$') return "g_nam_dl";
    if (c == '%') return "g_nam_pe";
    if (c == '&') return "g_nam_an";
    if (c == '*') return "g_nam_as";
    if (c == '@') return "g_nam_at";
    return nullptr;
}

static bool MeasureSpriteGlyphAdvance(PrGameContext& ctx, char c, float scale, float& outAdvance) {
    outAdvance = 0.0f;
    if (!ctx.resources) {
        return false;
    }

    if (c == ' ') {
        outAdvance = 12.0f * scale;
        return true;
    }

    const char* name = MapGlyphTextureName(c);
    if (!name) {
        return false;
    }

    if (!ctx.resources->GetTextureView(name)) {
        return false;
    }

    float tw = 24.0f;
    if (TextureResource* tr = ctx.resources->GetTexture(name)) {
        if (tr->tim.width > 0) {
            tw = (float)tr->tim.width;
        }
    }

    constexpr float kGlyphScaleFactor = 0.5f;
    outAdvance = tw * kGlyphScaleFactor * scale;
    return true;
}

static bool DrawSpriteGlyph(PrGameContext& ctx, float x, float y, float scale,
                            char c, float r, float g, float b, float a, float& outAdvance) {
    outAdvance = 0.0f;
    if (!ctx.renderer || !ctx.resources) {
        return false;
    }

    if (c == ' ') {
        outAdvance = 12.0f * scale;
        return true;
    }

    const char* name = MapGlyphTextureName(c);
    if (!name) {
        return false;
    }

    ID3D11ShaderResourceView* srv = ctx.resources->GetTextureView(name);
    if (!srv) {
        return false;
    }

    float tw = 24.0f;
    float th = 28.0f;
    if (TextureResource* tr = ctx.resources->GetTexture(name)) {
        if (tr->tim.width > 0 && tr->tim.height > 0) {
            tw = (float)tr->tim.width;
            th = (float)tr->tim.height;
        }
    }

    constexpr float kGlyphScaleFactor = 0.5f;
    const float w = tw * kGlyphScaleFactor * scale;
    const float h = th * kGlyphScaleFactor * scale;

    D3D11Renderer::SpriteCmd cmd;
    cmd.texture = srv;
    cmd.x = x;
    cmd.y = y;
    cmd.w = w;
    cmd.h = h;
    cmd.u0 = 0.0f; cmd.v0 = 0.0f; cmd.u1 = 1.0f; cmd.v1 = 1.0f;
    cmd.r = r; cmd.g = g; cmd.b = b; cmd.a = a;
    cmd.blend = D3D11Renderer::BlendMode::Alpha;
    cmd.layer = 900;
    cmd.order = 0;
    ctx.renderer->SubmitSprite(cmd);
    outAdvance = w;
    return true;
}

void PrVText::Init(PrGameContext& ctx) {
    (void)ctx;
}

void PrVText::Shutdown(PrGameContext& ctx) {
    (void)ctx;
}

float PrVText::MeasureString(PrGameContext& ctx, const char* text, float scale, bool allowSpriteGlyphs) {
    if (!text) {
        return 0.0f;
    }

    if (allowSpriteGlyphs) {
        {
            bool drewAny = false;
            float width = 0.0f;
            for (const char* p = text; *p; ++p) {
                float adv = 0.0f;
                if (MeasureSpriteGlyphAdvance(ctx, *p, scale, adv)) {
                    drewAny = true;
                    width += adv;
                    continue;
                }
                if (drewAny) {
                    width += 12.0f * scale;
                }
            }
            if (drewAny) {
                return width;
            }
        }
    }

    const float pixW = 2.0f * scale;
    const float glyphW = 5.0f * pixW;
    const float spacing = 1.0f * pixW;

    float width = 0.0f;
    for (const char* p = text; *p; ++p) {
        (void)GlyphIndex(*p);
        width += glyphW + spacing;
    }
    return width;
}

void PrVText::DrawString(PrGameContext& ctx, int x, int y, const char* text, uint32_t color, float scale, bool allowSpriteGlyphs) {
    if (!ctx.renderer || !text) return;

    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;
    const float a = ((color >> 24) & 0xFF) / 255.0f;

    if (allowSpriteGlyphs) {
        {
            bool drewAny = false;
            float cx = (float)x;
            for (const char* p = text; *p; ++p) {
                float adv = 0.0f;
                if (DrawSpriteGlyph(ctx, cx, (float)y, scale, *p, r, g, b, a, adv)) {
                    drewAny = true;
                    cx += adv;
                    continue;
                }

                if (drewAny) {
                    cx += 12.0f * scale;
                }
            }
            if (drewAny) {
                return;
            }
        }
    }

    const float pixW = 2.0f * scale;
    const float pixH = 2.0f * scale;
    const float glyphW = 5.0f * pixW;
    const float spacing = 1.0f * pixW;

    float cx = (float)x;
    for (const char* p = text; *p; ++p) {
        int gi = GlyphIndex(*p);
        if (gi < 0 || gi >= (int)(sizeof(s_font5x7) / sizeof(s_font5x7[0]))) {
            cx += glyphW + spacing;
            continue;
        }
        const uint8_t* glyph = s_font5x7[gi];
        for (int row = 0; row < 7; row++) {
            uint8_t bits = glyph[row];
            for (int col = 0; col < 5; col++) {
                if (bits & (0x80 >> col)) {
                    ctx.renderer->DrawRect(cx + col * pixW, (float)y + row * pixH,
                                           pixW, pixH, r, g, b, a);
                }
            }
        }
        cx += glyphW + spacing;
    }
}
