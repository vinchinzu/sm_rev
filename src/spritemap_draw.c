// Generic spritemap draw helpers extracted from Bank 81.

#include "sm_rtl.h"
#include "ida_types.h"
#include "variables.h"
#include "funcs.h"
#include "samus_asset_bridge.h"

#define g_off_82C569 ((uint16*)RomFixedPtr(0x82c569))
#define g_off_93A1A1 ((uint16*)RomFixedPtr(0x93a1a1))

static void DrawGrappleOrProjectileSpritemap(const uint8 *pp, uint16 x_r20, uint16 y_r18) {  // 0x818A5F
  int idx = oam_next_ptr;
  int n = GET_WORD(pp);
  pp += 2;
  for (; n != 0; n--) {
    uint16 x = x_r20 + GET_WORD(pp);
    OamEnt *v4 = gOamEnt(idx);
    v4->xcoord = x;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    v4->ycoord = y_r18 + pp[2];
    *(uint16 *)&v4->charnum = GET_WORD(pp + 3);
    pp += 5;
    idx = (idx + 4) & 0x1FF;
  }
  oam_next_ptr = idx;
}

void DrawSpritemap(uint8 db, uint16 j, uint16 x_r20, uint16 y_r18, uint16 chr_r22) {  // 0x81879F
  const uint8 *pp = RomPtrWithBank(db, j);
  int n = GET_WORD(pp);
  pp += 2;
  int idx = oam_next_ptr;
  for (; (n != 0) && (idx < 0x200); n--) {
    OamEnt *oam = gOamEnt(idx);
    int x = x_r20 + GET_WORD(pp);
    int y = (uint8)y_r18 + pp[2];
    if (!sign8(pp[2]) ? (y >= 0xe0) : (y & 0x100) ? ((uint8)y >= 0xe0) : ((uint8)y < 0xe0))
      x = 0x180, y = 0xe0;
    oam->xcoord = x;
    oam->ycoord = y;
    *(uint16 *)&oam->charnum = chr_r22 | GET_WORD(pp + 3) & 0xF1FF;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    idx += 4;
    pp += 5;
  }
  oam_next_ptr = idx;
}

void DrawSpritemapOffScreen(uint16 j, uint16 x_r20, uint16 y_r18, uint16 chr_r22) {  // 0x818853
  const uint8 *pp = RomPtr_8C(j);
  int n = GET_WORD(pp);
  pp += 2;
  int idx = oam_next_ptr;
  for (; (n != 0) && (idx < 0x200); n--) {
    OamEnt *oam = gOamEnt(idx);
    int x = x_r20 + GET_WORD(pp);
    int y = (uint8)y_r18 + pp[2];
    if (!sign8(pp[2]) ? (y < 0xe0) : (y & 0x100) ? ((uint8)y < 0xe0) : ((uint8)y >= 0xe0))
      x = 0x180, y = 0xe0;
    oam->xcoord = x;
    oam->ycoord = y;
    *(uint16 *)&oam->charnum = chr_r22 | *(uint16 *)(pp + 3) & 0xF1FF;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    idx += 4;
    pp += 5;
  }
  oam_next_ptr = idx;
}

void DrawMenuSpritemap(uint16 a, uint16 k, uint16 j, uint16 chr_r3) {  // 0x81891F
  const uint8 *pp = RomPtr_82(g_off_82C569[a]);
  int n = GET_WORD(pp);
  pp += 2;
  int idx = oam_next_ptr;
  for (; n != 0; n--) {
    OamEnt *oam = gOamEnt(idx);
    uint16 x = k + GET_WORD(pp);
    oam->xcoord = x;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    oam->ycoord = j + pp[2];
    *(uint16 *)&oam->charnum = chr_r3 | GET_WORD(pp + 3) & 0xF1FF;
    pp += 5;
    idx = (idx + 4) & 0x1FF;
  }
  oam_next_ptr = idx;
}

void DrawSamusSpritemap(uint16 a, uint16 x_pos, uint16 y_pos) {  // 0x8189AE
  const uint8 *spritemap_table = SamusAssetBridge_GetBank92(0x808d);
  if (spritemap_table == NULL)
    return;
  uint16 spritemap_ptr = GET_WORD(spritemap_table + a * 2);
  if (spritemap_ptr == 0)
    return;
  const uint8 *pp = SamusAssetBridge_GetBank92(spritemap_ptr);
  if (pp == NULL)
    return;
  int idx = oam_next_ptr;
  int n = GET_WORD(pp);
  pp += 2;
  for (; n != 0; n--) {
    uint16 x = x_pos + GET_WORD(pp);
    OamEnt *v9 = gOamEnt(idx);
    v9->xcoord = x;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    v9->ycoord = y_pos + pp[2];
    *(uint16 *)&v9->charnum = GET_WORD(pp + 3);
    pp += 5;
    idx = (idx + 4) & 0x1FF;
  }
  oam_next_ptr = idx;
}

void DrawBeamGrappleSpritemap(uint16 a, uint16 x_r20, uint16 y_r18) {  // 0x818A37
  DrawGrappleOrProjectileSpritemap(RomPtr_93(g_off_93A1A1[a]), x_r20, y_r18);
}

void DrawProjectileSpritemap(uint16 k, uint16 x_r20, uint16 y_r18) {  // 0x818A4B
  DrawGrappleOrProjectileSpritemap(RomPtr_93(projectile_spritemap_pointers[k >> 1]), x_r20, y_r18);
}

void DrawSpritemapWithBaseTile(uint8 db, uint16 j, uint16 r20_x, uint16 r18_y, uint16 r3, uint16 r0) {  // 0x818AB8
  if (j == 0)
    return; // bug fix
  uint8 *pp = (uint8 *)RomPtrWithBank(db, j);
  int idx = oam_next_ptr;
  int n = GET_WORD(pp);
  pp += 2;
  for (; n != 0; n--) {
    OamEnt *oam = gOamEnt(idx);
    uint16 x = r20_x + GET_WORD(pp + 0);
    oam->xcoord = x;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    oam->ycoord = r18_y + pp[2];
    *(uint16 *)&oam->charnum = r3 | (r0 + GET_WORD(pp + 3));
    pp += 5;
    idx = (idx + 4) & 0x1FF;
  }
  oam_next_ptr = idx;
}

void DrawSpritemapWithBaseTile2(uint8 db, uint16 j, uint16 r20_x, uint16 r18_y, uint16 r3, uint16 r0) {  // 0x818B22
  const uint8 *pp = RomPtrWithBank(db, j);
  int idx = oam_next_ptr;
  int n = GET_WORD(pp);
  pp += 2;
  for (; n != 0; n--) {
    OamEnt *oam = gOamEnt(idx);
    uint16 x = r20_x + GET_WORD(pp);
    oam->xcoord = x;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    int y = pp[2] + (uint8)r18_y;
    oam->ycoord = (!(y & 0x100) == !sign8(pp[2])) ? y : 0xf0;
    *(uint16 *)&oam->charnum = r3 | (r0 + GET_WORD(pp + 3));
    pp += 5;
    idx = (idx + 4) & 0x1FF;
  }
  oam_next_ptr = idx;
}

void DrawSpritemapWithBaseTileOffscreen(uint8 db, uint16 j, uint16 r20_x, uint16 r18_y, uint16 r3, uint16 r0) {  // 0x818B96
  const uint8 *pp = RomPtrWithBank(db, j);
  int idx = oam_next_ptr;
  int n = GET_WORD(pp);
  pp += 2;
  for (; n != 0; n--) {
    OamEnt *oam = gOamEnt(idx);
    int x = r20_x + GET_WORD(pp);
    oam->xcoord = x;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    int y = pp[2] + (uint8)r18_y;
    oam->ycoord = (!(y & 0x100) != !sign8(pp[2])) ? y : 0xf0;
    *(uint16 *)&oam->charnum = r3 | (r0 + GET_WORD(pp + 3));
    pp += 5;
    idx = (idx + 4) & 0x1FF;
  }
  oam_next_ptr = idx;
}

void DrawEprojSpritemapWithBaseTile(uint8 db, uint16 j, uint16 x_r20, uint16 y_r18, uint16 chr_r26, uint16 chr_r28) {  // 0x818C0A
  const uint8 *pp = RomPtrWithBank(db, j);
  int idx = oam_next_ptr;
  int n = GET_WORD(pp);
  pp += 2;
  for (; n != 0; n--) {
    OamEnt *oam = gOamEnt(idx);
    uint16 x = x_r20 + GET_WORD(pp);
    oam->xcoord = x;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    int y = pp[2] + (uint8)y_r18;
    oam->ycoord = (!(y & 0x100) == !sign8(pp[2])) ? y : 0xf0;
    *(uint16 *)&oam->charnum = chr_r28 | (chr_r26 + GET_WORD(pp + 3));
    idx = (idx + 4) & 0x1FF;
    pp += 5;
  }
  oam_next_ptr = idx;
}

void DrawEprojSpritemapWithBaseTileOffscreen(uint8 db, uint16 j, uint16 x_r20, uint16 y_r18, uint16 chr_r26, uint16 chr_r28) {  // 0x818C7F
  const uint8 *pp = RomPtrWithBank(db, j);
  int idx = oam_next_ptr;
  int n = GET_WORD(pp);
  pp += 2;
  for (; n != 0; n--) {
    OamEnt *oam = gOamEnt(idx);
    uint16 x = x_r20 + GET_WORD(pp);
    oam->xcoord = x;
    oam_ext[idx >> 5] |= (((x & 0x100) >> 8) | (*(int16 *)pp < 0) * 2) << (2 * ((idx >> 2) & 7));
    int y = pp[2] + (uint8)y_r18;
    oam->ycoord = (!(y & 0x100) != !sign8(pp[2])) ? y : 0xf0;
    *(uint16 *)&oam->charnum = chr_r28 | (chr_r26 + GET_WORD(pp + 3));
    idx = (idx + 4) & 0x1FF;
    pp += 5;
  }
  oam_next_ptr = idx;
}
