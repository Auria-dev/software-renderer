#define SWAP_F(a, b) do { float t = a; a = b; b = t; } while (0)
#define SWAP_U32(a, b) do { u32 t = a; a = b; b = t; } while (0)

void RASTERIZER_NAME(
        render_context *ctx,
        float x0, float y0, float w0, float u0, float v0, u32 c0,
        float x1, float y1, float w1, float u1, float v1, u32 c1,
        float x2, float y2, float w2, float u2, float v2, u32 c2) {

    float area = ((x2 - x0) * (y1 - y0)) - ((y2 - y0) * (x1 - x0));
    if (area < 0) {
        area = -area;
        SWAP_F(x1,x2);
        SWAP_F(y1,y2);
        SWAP_F(w1,w2);
        SWAP_F(u1,u2);
        SWAP_F(v1,v2);
        SWAP_U32(c1,c2);
    }
    if (fabsf(area) < 1e-6f) return;

    const int win_width = ctx->framebuffer.width;
    const int win_height = ctx->framebuffer.height;

    const int min_x_f = (int) floorf(fmin(fmin(x0, x1), x2));
    const int min_y_f = (int) floorf(fmin(fmin(y0, y1), y2));
    const int max_x_f = (int)  ceilf(fmax(fmax(x0, x1), x2));
    const int max_y_f = (int)  ceilf(fmax(fmax(y0, y1), y2));

    const int clamped_min_x = (min_x_f < 0) ? 0 : min_x_f;
    const int clamped_min_y = (min_y_f < 0) ? 0 : min_y_f;
    const int clamped_max_x = (max_x_f >= win_width) ? (win_width - 1) : max_x_f;
    const int clamped_max_y = (max_y_f >= win_height) ? (win_height - 1) : max_y_f;

    if (clamped_min_x > clamped_max_x || clamped_min_y > clamped_max_y) return;
    
    const float rcp_area = 1.0f / area;
    const float rcp_w0 = 1.0f / w0;
    const float rcp_w1 = 1.0f / w1;
    const float rcp_w2 = 1.0f / w2;
    
    const float dx0 = y2 - y1;
    const float dy0 = x1 - x2;
    const float dx1 = y0 - y2;
    const float dy1 = x2 - x0;
    const float dx2 = y1 - y0;
    const float dy2 = x0 - x1;

#if RASTER_GOURAUD == 1
    const float r0 = (float)((c0 >> 16) & 0xFF);
    const float g0 = (float)((c0 >>  8) & 0xFF);
    const float b0 = (float)( c0        & 0xFF);
    const float r1 = (float)((c1 >> 16) & 0xFF);
    const float g1 = (float)((c1 >>  8) & 0xFF);
    const float b1 = (float)( c1        & 0xFF);
    const float r2 = (float)((c2 >> 16) & 0xFF);
    const float g2 = (float)((c2 >>  8) & 0xFF);
    const float b2 = (float)( c2        & 0xFF);

    const float r0_persp = r0 * rcp_w0;
    const float g0_persp = g0 * rcp_w0;
    const float b0_persp = b0 * rcp_w0;
    const float r1_persp = r1 * rcp_w1;
    const float g1_persp = g1 * rcp_w1;
    const float b1_persp = b1 * rcp_w1;
    const float r2_persp = r2 * rcp_w2;
    const float g2_persp = g2 * rcp_w2;
    const float b2_persp = b2 * rcp_w2;
    
    const float r_dx = rcp_area * (r0_persp * dx0 + r1_persp * dx1 + r2_persp * dx2);
    const float g_dx = rcp_area * (g0_persp * dx0 + g1_persp * dx1 + g2_persp * dx2);
    const float b_dx = rcp_area * (b0_persp * dx0 + b1_persp * dx1 + b2_persp * dx2);
    const float r_dy = rcp_area * (r0_persp * dy0 + r1_persp * dy1 + r2_persp * dy2);
    const float g_dy = rcp_area * (g0_persp * dy0 + g1_persp * dy1 + g2_persp * dy2);
    const float b_dy = rcp_area * (b0_persp * dy0 + b1_persp * dy1 + b2_persp * dy2);
#endif // RASTER_GOURAUD

#if RASTER_TEXTURE == 1
    const float u0_persp = u0 * rcp_w0;
    const float u1_persp = u1 * rcp_w1;
    const float u2_persp = u2 * rcp_w2;
    const float v0_persp = v0 * rcp_w0;
    const float v1_persp = v1 * rcp_w1;
    const float v2_persp = v2 * rcp_w2;

    const float u_dx = rcp_area * (u0_persp * dx0 + u1_persp * dx1 + u2_persp * dx2);
    const float u_dy = rcp_area * (u0_persp * dy0 + u1_persp * dy1 + u2_persp * dy2);
    const float v_dx = rcp_area * (v0_persp * dx0 + v1_persp * dx1 + v2_persp * dx2);
    const float v_dy = rcp_area * (v0_persp * dy0 + v1_persp * dy1 + v2_persp * dy2);
#endif // RASTER_TEXTURE

    const float depth_dx = rcp_area * (rcp_w0 * dx0 + rcp_w1 * dx1 + rcp_w2 * dx2);
    const float depth_dy = rcp_area * (rcp_w0 * dy0 + rcp_w1 * dy1 + rcp_w2 * dy2);
    
    const float psx = clamped_min_x+0.5f;
    const float psy = clamped_min_y+0.5f;
    float w0_row = (psx - x1) * (y2 - y1) - (psy - y1) * (x2 - x1);
    float w1_row = (psx - x2) * (y0 - y2) - (psy - y2) * (x0 - x2);
    float w2_row = (psx - x0) * (y1 - y0) - (psy - y0) * (x1 - x0);
    
    float depth_recip_row = rcp_area * (rcp_w0 * w0_row + rcp_w1 * w1_row + rcp_w2 * w2_row);
#if RASTER_GOURAUD == 1
    float r_row = rcp_area * (r0_persp * w0_row + r1_persp * w1_row + r2_persp * w2_row);
    float g_row = rcp_area * (g0_persp * w0_row + g1_persp * w1_row + g2_persp * w2_row);
    float b_row = rcp_area * (b0_persp * w0_row + b1_persp * w1_row + b2_persp * w2_row);
#endif // RASTER_GOURAUD
#if RASTER_TEXTURE == 1
    float u_row = rcp_area * (u0_persp * w0_row + u1_persp * w1_row + u2_persp * w2_row);
    float v_row = rcp_area * (v0_persp * w0_row + v1_persp * w1_row + v2_persp * w2_row);
#endif // RASTER_TEXTURE

#if RASTER_TEXTURE == 1

    texture_t *texture = m_get_texture(ctx->material_manager, ctx->material_id);
    if (!texture) {
        return;
    }
    const int tex_width = texture->width;
    const int tex_height = texture->height;
    const int tex_width_mask = tex_width - 1;
    const int tex_height_mask = tex_height - 1;
#endif
#if RASTER_GOURAUD == 0 // flat shading
    const u8 flat_r = (u8)((c0 >> 16) & 0xFF);
    const u8 flat_g = (u8)((c0 >>  8) & 0xFF);
    const u8 flat_b = (u8)( c0        & 0xFF);
#endif // RASTER_GOURAUD

    intptr_t row_offset = (intptr_t)clamped_min_y * win_width;
    for (int y = clamped_min_y; y <= clamped_max_y; ++y) {
        float* z_ptr = ctx->framebuffer.depth_buffer + row_offset + clamped_min_x;
        u32* color_ptr = ctx->framebuffer.color_buffer + row_offset + clamped_min_x;

        float w0_start = w0_row;
        float w1_start = w1_row;
        float w2_start = w2_row;
        float depth    = depth_recip_row;
#if RASTER_GOURAUD == 1
        float r_start = r_row;
        float g_start = g_row;
        float b_start = b_row;
#endif // RASTER_GOURAUD
#if RASTER_TEXTURE == 1
        float u_start = u_row;
        float v_start = v_row;
#endif // RASTER_TEXTURE
        
        for (int x = clamped_min_x; x <= clamped_max_x; x++) {
            if (w0_start >= 0 && w1_start >= 0 && w2_start >= 0 && depth > *z_ptr) {
                
#if RASTER_GOURAUD == 1 && RASTER_TEXTURE == 1 // SGT (Scalar, Gouraud, Textured)
                {
                    const float inv_w = 1.0f / depth;
                    const float u = u_start * inv_w;
                    const float v = v_start * inv_w;
                    const int  vr = r_start * inv_w;
                    const int  vg = g_start * inv_w;
                    const int  vb = b_start * inv_w;

#ifdef SAMPLE_BILINEAR // bilinear sampling
                    const float tex_u = u * tex_width;
                    const float tex_v = v * tex_height;
                    const int tex_x0 = ((int)tex_u) & tex_width_mask;
                    const int tex_y0 = ((int)tex_v) & tex_height_mask;
                    const int tex_x1 = (tex_x0 + 1) & tex_width_mask;
                    const int tex_y1 = (tex_y0 + 1) & tex_height_mask;
                    const float frac_u = tex_u - floorf(tex_u);
                    const float frac_v = tex_v - floorf(tex_v);
                    const u8* texel00 = texture->data + (tex_y0 * tex_width + tex_x0) * 4;
                    const u8* texel10 = texture->data + (tex_y0 * tex_width + tex_x1) * 4;
                    const u8* texel01 = texture->data + (tex_y1 * tex_width + tex_x0) * 4;
                    const u8* texel11 = texture->data + (tex_y1 * tex_width + tex_x1) * 4;
                    u8 texel[4];
                    for (int i = 0; i < 4; ++i) {
                        float c0 = texel00[i] * (1.0f - frac_u) + texel10[i] * frac_u;
                        float c1 = texel01[i] * (1.0f - frac_u) + texel11[i] * frac_u;
                        float c = c0 * (1.0f - frac_v) + c1 * frac_v;
                        texel[i] = (u8)(c + 0.5f);
                    }
#else // nearest-neighbor sampling
                    const int tex_x = (int)(u * tex_width) & tex_width_mask;
                    const int tex_y = (int)(v * tex_height) & tex_height_mask;
                    const u8* texel = texture->data + (tex_y * tex_width + tex_x) * 4;
#endif // SAMPLE MODE

                    if (texel[3] != 0x00) { // TODO: blending
                        const u8 tr = texel[0];
                        const u8 tg = texel[1];
                        const u8 tb = texel[2];

                        int mod_r = (tr * vr) >> 8;
                        int mod_g = (tg * vg) >> 8;
                        int mod_b = (tb * vb) >> 8;
                        
                        mod_r = (mod_r < 0) ? 0 : (mod_r > 255) ? 255 : mod_r;
                        mod_g = (mod_g < 0) ? 0 : (mod_g > 255) ? 255 : mod_g;
                        mod_b = (mod_b < 0) ? 0 : (mod_b > 255) ? 255 : mod_b;
                        
                        *z_ptr = depth;
                        *color_ptr = 0xffu << 24 | mod_r << 16 | mod_g << 8 | mod_b;
                    }
                }
#elif RASTER_GOURAUD == 1 && RASTER_TEXTURE == 0 // SGC (Scalar, Gouraud, Colored)
                {
                    const float inv_w = 1.0f / depth;
                    int vr = (int)(r_start * inv_w);
                    int vg = (int)(g_start * inv_w);
                    int vb = (int)(b_start * inv_w);

                    vr = (vr < 0) ? 0 : (vr > 255) ? 255 : vr;
                    vg = (vg < 0) ? 0 : (vg > 255) ? 255 : vg;
                    vb = (vb < 0) ? 0 : (vb > 255) ? 255 : vb;
                    
                    *z_ptr = depth;
                    *color_ptr = 0xffu << 24 | vr << 16 | vg << 8 | vb;
                }
#elif RASTER_GOURAUD == 0 && RASTER_TEXTURE == 1 // SFT (Scalar, Flat, Textured)
                {
                    const float inv_w = 1.0f / depth;
                    const float u = u_start * inv_w;
                    const float v = v_start * inv_w;

#ifdef SAMPLE_BILINEAR // bilinear sampling
                    const float tex_u = u * tex_width;
                    const float tex_v = v * tex_height;
                    const int tex_x0 = ((int)tex_u) & tex_width_mask;
                    const int tex_y0 = ((int)tex_v) & tex_height_mask;
                    const int tex_x1 = (tex_x0 + 1) & tex_width_mask;
                    const int tex_y1 = (tex_y0 + 1) & tex_height_mask;
                    const float frac_u = tex_u - floorf(tex_u);
                    const float frac_v = tex_v - floorf(tex_v);
                    const u8* texel00 = texture->data + (tex_y0 * tex_width + tex_x0) * 4;
                    const u8* texel10 = texture->data + (tex_y0 * tex_width + tex_x1) * 4;
                    const u8* texel01 = texture->data + (tex_y1 * tex_width + tex_x0) * 4;
                    const u8* texel11 = texture->data + (tex_y1 * tex_width + tex_x1) * 4;
                    u8 texel[4];
                    for (int i = 0; i < 4; ++i) {
                        float c0 = texel00[i] * (1.0f - frac_u) + texel10[i] * frac_u;
                        float c1 = texel01[i] * (1.0f - frac_u) + texel11[i] * frac_u;
                        float c = c0 * (1.0f - frac_v) + c1 * frac_v;
                        texel[i] = (u8)(c + 0.5f);
                    }
#else // nearest-neighbor sampling
                    const int tex_x = (int)(u * tex_width) & tex_width_mask;
                    const int tex_y = (int)(v * tex_height) & tex_height_mask;
                    const u8* texel = texture->data + (tex_y * tex_width + tex_x) * 4;
#endif // SAMPLE MODE
                    
                    if (texel[3] != 0x00) {
                        int mod_r = (texel[0] * flat_r) >> 8;
                        int mod_g = (texel[1] * flat_g) >> 8;
                        int mod_b = (texel[2] * flat_b) >> 8;
                        
                        *z_ptr = depth;
                        *color_ptr = 0xffu << 24 | mod_r << 16 | mod_g << 8 | mod_b;
                    }
                }
#elif RASTER_GOURAUD == 0 && RASTER_TEXTURE == 0 // SFC (Scalar, Flat, Colored)
                {
                    *z_ptr = depth;
                    *color_ptr = 0xffu << 24 | flat_r << 16 | flat_g << 8 | flat_b;
                }
#endif // end of shader types
            }
            
            w0_start += dx0;
            w1_start += dx1;
            w2_start += dx2;
            depth    += depth_dx;
#if RASTER_GOURAUD == 1
            r_start += r_dx;
            g_start += g_dx;
            b_start += b_dx;
#endif // RASTER_GOURAUD
#if RASTER_TEXTURE == 1
            u_start += u_dx;
            v_start += v_dx;
#endif // RASTER_TEXTURE
            z_ptr++;
            color_ptr++;
        }

        w0_row += dy0;
        w1_row += dy1;
        w2_row += dy2;
        depth_recip_row += depth_dy;
#if RASTER_GOURAUD == 1
        r_row += r_dy;
        g_row += g_dy;
        b_row += b_dy;
#endif // RASTER_GOURAUD
#if RASTER_TEXTURE == 1
        u_row += u_dy;
        v_row += v_dy;
#endif // RASTER_TEXTURE
        row_offset += win_width;
    }
}

#undef SWAP_F
#undef SWAP_U32
#undef RASTERIZER_NAME
#undef RASTER_GOURAUD
#undef RASTER_TEXTURE
#undef SAMPLE_BILINEAR