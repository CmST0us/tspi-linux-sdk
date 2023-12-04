#include "mpp_osd.h"
#include "mpi_enc.h"
#include "uvc_log.h"


#if MPP_ENC_OSD_ENABLE
/* argb(lower address -> high address) */
const uint32_t rgb888_palette_table[PALETTE_TABLE_LEN] = {
    0xffffffff, 0xd7ffffff, 0xafffffff, 0x87ffffff, 0x5fffffff, 0x00ffffff,
    0xffd7ffff, 0xd7d7ffff, 0xafd7ffff, 0x87d7ffff, 0x5fd7ffff, 0x00d7ffff,
    0xffafffff, 0xd7afffff, 0xafafffff, 0x87afffff, 0x5fafffff, 0x00afffff,
    0xff87ffff, 0xd787ffff, 0xaf87ffff, 0x8787ffff, 0x5f87ffff, 0x0087ffff,
    0xff5fffff, 0xd75fffff, 0xaf5fffff, 0x875fffff, 0x5f5fffff, 0x005fffff,
    0xff00ffff, 0xd700ffff, 0xaf00ffff, 0x8700ffff, 0x5f00ffff, 0x0000ffff,
    0xffffd7ff, 0xd7ffd7ff, 0xafffd7ff, 0x87ffd7ff, 0x5fffd7ff, 0x00ffd7ff,
    0xffd7d7ff, 0xd7d7d7ff, 0xafd7d7ff, 0x87d7d7ff, 0x5fd7d7ff, 0x00d7d7ff,
    0xffafd7ff, 0xd7afd7ff, 0xafafd7ff, 0x87afd7ff, 0x5fafd7ff, 0x00afd7ff,
    0xff87d7ff, 0xd787d7ff, 0xaf87d7ff, 0x8787d7ff, 0x5f87d7ff, 0x0087d7ff,
    0xff5fd7ff, 0xd75fd7ff, 0xaf5fd7ff, 0x875fd7ff, 0x5f5fd7ff, 0x005fd7ff,
    0xff00d7ff, 0xd700d7ff, 0xaf00d7ff, 0x8700d7ff, 0x5f00d7ff, 0x0000d7ff,
    0xffffafff, 0xd7ffafff, 0xafffafff, 0x87ffafff, 0x5fffafff, 0x00ffafff,
    0xffd7afff, 0xd7d7afff, 0xafd7afff, 0x87d7afff, 0x5fd7afff, 0x00d7afff,
    0xffafafff, 0xd7afafff, 0xafafafff, 0x87afafff, 0x5fafafff, 0x00afafff,
    0xff87afff, 0xd787afff, 0xaf87afff, 0x8787afff, 0x5f87afff, 0x0087afff,
    0xff5fafff, 0xd75fafff, 0xaf5fafff, 0x875fafff, 0x5f5fafff, 0x005fafff,
    0xff00afff, 0xd700afff, 0xaf00afff, 0x8700afff, 0x5f00afff, 0x0000afff,
    0xffff87ff, 0xd7ff87ff, 0xafff87ff, 0x87ff87ff, 0x5fff87ff, 0x00ff87ff,
    0xffd787ff, 0xd7d787ff, 0xafd787ff, 0x87d787ff, 0x5fd787ff, 0x00d787ff,
    0xffaf87ff, 0xd7af87ff, 0xafaf87ff, 0x87af87ff, 0x5faf87ff, 0x00af87ff,
    0xff8787ff, 0xd78787ff, 0xaf8787ff, 0x878787ff, 0x5f8787ff, 0x008787ff,
    0xff5f87ff, 0xd75f87ff, 0xaf5f87ff, 0x875f87ff, 0x5f5f87ff, 0x005f87ff,
    0xff0087ff, 0xd70087ff, 0xaf0087ff, 0x870087ff, 0x5f0087ff, 0x000087ff,
    0xffff5fff, 0xd7ff5fff, 0xafff5fff, 0x87ff5fff, 0x5fff5fff, 0x00ff5fff,
    0xffd75fff, 0xd7d75fff, 0xafd75fff, 0x87d75fff, 0x5fd75fff, 0x00d75fff,
    0xffaf5fff, 0xd7af5fff, 0xafaf5fff, 0x87af5fff, 0x5faf5fff, 0x00af5fff,
    0xff875fff, 0xd7875fff, 0xaf875fff, 0x87875fff, 0x5f875fff, 0x00875fff,
    0xff5f5fff, 0xd75f5fff, 0xaf5f5fff, 0x875f5fff, 0x5f5f5fff, 0x005f5fff,
    0xff005fff, 0xd7005fff, 0xaf005fff, 0x87005fff, 0x5f005fff, 0x00005fff,
    0xffff00ff, 0xd7ff00ff, 0xafff00ff, 0x87ff00ff, 0x5fff00ff, 0x00ff00ff,
    0xffd700ff, 0xd7d700ff, 0xafd700ff, 0x87d700ff, 0x5fd700ff, 0x00d700ff,
    0xffaf00ff, 0xd7af00ff, 0xafaf00ff, 0x87af00ff, 0x5faf00ff, 0x00af00ff,
    0xff8700ff, 0xd78700ff, 0xaf8700ff, 0x878700ff, 0x5f8700ff, 0x008700ff,
    0xff5f00ff, 0xd75f00ff, 0xaf5f00ff, 0x875f00ff, 0x5f5f00ff, 0x005f00ff,
    0xff0000ff, 0xd70000ff, 0xaf0000ff, 0x870000ff, 0x5f0000ff, 0x000000ff,
    0xeeeeeeff, 0xe4e4e4ff, 0xdadadaff, 0xd0d0d0ff, 0xc6c6c6ff, 0xbcbcbcff,
    0xb2b2b2ff, 0xa8a8a8ff, 0x9e9e9eff, 0x949494ff, 0x8a8a8aff, 0x767676ff,
    0x6c6c6cff, 0x626262ff, 0x585858ff, 0x4e4e4eff, 0x444444ff, 0x3a3a3aff,
    0x303030ff, 0x262626ff, 0x1c1c1cff, 0x121212ff, 0x080808ff, 0x000080ff,
    0x008000ff, 0x008080ff, 0x800000ff, 0x800080ff, 0x808000ff, 0xc0c0c0ff,
    0x808080ff, 0xffffccff, 0xccccccff, 0x99ccccff, 0x9999ccff, 0x3366ccff,
    0x0033ccff, 0x3300ccff, 0x00ffccff, 0x08000000
};

/* yuva(lower address -> high address) */
static const uint32_t yuv444_palette_table[PALETTE_TABLE_LEN] = {
    0xff8080eb, 0xff826ee9, 0xff835de6, 0xff854be4, 0xff863ae1, 0xff8a10db,
    0xff908ed2, 0xff927cd0, 0xff936acd, 0xff9559cb, 0xff9647c9, 0xff9a1ec3,
    0xffa09bba, 0xffa28ab7, 0xffa378b5, 0xffa566b2, 0xffa655b0, 0xffaa2baa,
    0xffb0a9a1, 0xffb1979f, 0xffb3859c, 0xffb5749a, 0xffb66297, 0xffba3991,
    0xffc0b689, 0xffc1a586, 0xffc39384, 0xffc58181, 0xffc6707f, 0xffca4679,
    0xffe6d64e, 0xffe7c54c, 0xffe9b349, 0xffeba247, 0xffec9044, 0xfff0663f,
    0xff6e84e4, 0xff7072e1, 0xff7261df, 0xff734fdc, 0xff753eda, 0xff7914d4,
    0xff7e92cb, 0xff8080c9, 0xff826ec6, 0xff835dc4, 0xff854bc1, 0xff8922bb,
    0xff8e9fb3, 0xff908eb0, 0xff927cae, 0xff936aab, 0xff9559a9, 0xff992fa3,
    0xff9ead9a, 0xffa09b98, 0xffa28a95, 0xffa37893, 0xffa56690, 0xffa93d8a,
    0xffaeba81, 0xffb0a97f, 0xffb1977c, 0xffb3857a, 0xffb57477, 0xffb94a72,
    0xffd4da47, 0xffd6c945, 0xffd7b742, 0xffd9a640, 0xffdb943d, 0xffde6a37,
    0xff5d88dc, 0xff5e76da, 0xff6065d7, 0xff6253d5, 0xff6342d2, 0xff6718cd,
    0xff6d96c4, 0xff6e84c1, 0xff7072bf, 0xff7261bc, 0xff734fba, 0xff7726b4,
    0xff7da3ab, 0xff7e92a9, 0xff8080a6, 0xff826ea4, 0xff835da1, 0xff87339b,
    0xff8db193, 0xff8e9f90, 0xff908e8e, 0xff927c8b, 0xff936a89, 0xff974183,
    0xff9dbe7a, 0xff9ead78, 0xffa09b75, 0xffa28a73, 0xffa37870, 0xffa74e6a,
    0xffc3de40, 0xffc4cd3d, 0xffc6bb3b, 0xffc7aa38, 0xffc99836, 0xffcd6e30,
    0xff4b8cd5, 0xff4d7bd3, 0xff4f69d0, 0xff5057ce, 0xff5246cb, 0xff561cc5,
    0xff5b9abd, 0xff5d88ba, 0xff5e76b8, 0xff6065b5, 0xff6253b3, 0xff662aad,
    0xff6ba7a4, 0xff6d96a1, 0xff6e849f, 0xff70729d, 0xff72619a, 0xff753794,
    0xff7bb58b, 0xff7da389, 0xff7e9286, 0xff808084, 0xff826e81, 0xff85457c,
    0xff8bc273, 0xff8db170, 0xff8e9f6e, 0xff908e6b, 0xff927c69, 0xff955263,
    0xffb1e238, 0xffb3d136, 0xffb4bf34, 0xffb6ae31, 0xffb79c2f, 0xffbb7229,
    0xff3a90ce, 0xff3b7fcb, 0xff3d6dc9, 0xff3f5bc6, 0xff404ac4, 0xff4420be,
    0xff4a9eb5, 0xff4b8cb3, 0xff4d7bb0, 0xff4f69ae, 0xff5057ab, 0xff542ea5,
    0xff5aab9d, 0xff5b9a9a, 0xff5d8898, 0xff5e7695, 0xff606593, 0xff643b8d,
    0xff6ab984, 0xff6ba782, 0xff6d967f, 0xff6e847d, 0xff70727a, 0xff744974,
    0xff7ac66c, 0xff7bb569, 0xff7da367, 0xff7e9264, 0xff808062, 0xff84565c,
    0xff9fe631, 0xffa1d52f, 0xffa3c32c, 0xffa4b22a, 0xffa6a027, 0xffaa7621,
    0xff109abc, 0xff1288ba, 0xff1377b7, 0xff1565b5, 0xff1653b3, 0xff1a2aad,
    0xff20a7a4, 0xff2296a1, 0xff23849f, 0xff25729c, 0xff26619a, 0xff2a3794,
    0xff30b58b, 0xff32a389, 0xff339286, 0xff358084, 0xff366e81, 0xff3a457b,
    0xff40c273, 0xff41b170, 0xff439f6e, 0xff458e6b, 0xff467c69, 0xff4a5263,
    0xff50d05a, 0xff51be58, 0xff53ad55, 0xff559b53, 0xff568a50, 0xff5a604a,
    0xff76f020, 0xff77de1d, 0xff79cd1b, 0xff7bbb18, 0xff7caa16, 0xff808010,
    0xff8080dc, 0xff8080d4, 0xff8080cb, 0xff8080c3, 0xff8080ba, 0xff8080b1,
    0xff8080a9, 0xff8080a0, 0xff808098, 0xff80808f, 0xff808087, 0xff808075,
    0xff80806d, 0xff808064, 0xff80805c, 0xff808053, 0xff80804a, 0xff808042,
    0xff808039, 0xff808031, 0xff808028, 0xff80801f, 0xff808017, 0xffb87327,
    0xff4d555f, 0xff854876, 0xff7bb818, 0xffb3ab2f, 0xff488d67, 0xff8080b5,
    0xff80807e, 0xff6a85e2, 0xff8080bf, 0xff826abc, 0xff967b9d, 0xffaf5f77,
    0xffc55a55, 0xffd88238, 0xff7415d2, 0x008080eb
};

/* PixFormat: ARGB => A:bit31~bit24 R:bit23~bit16 G:bit15~bit8 B:bit7~bit0 */
const RK_U32 u32DftARGB8888ColorTblUser[PALETTE_TABLE_LEN] = {
    0x00ffffff, 0xff5e6060, 0xffe9491e, 0xfff4bc1f, 0xff1ca2dd, 0xff87bd43,//logo--transparency/gray/red/earthy yellow /blue
    0xffff1f1f, 0xff0000af, 0xff0000d7, 0xff0000ff, 0xff005f00, 0xff005f5f,//mute red
    0xff005f87, 0xff005faf, 0xff005fd7, 0xff005fff, 0xff008000, 0xff008080,
    0xff008700, 0xff00875f, 0xff008787, 0xff0087af, 0xff0087d7, 0xff0087ff,
    0xff00af00, 0xff00af5f, 0xff00af87, 0xff00afaf, 0xff00afd7, 0xff00afff,
    0xff00d700, 0xff00d75f, 0xff00d787, 0xff00d7af, 0xff00d7d7, 0xff00d7ff,
    0xff00ff00, 0xff00ff28, 0xff00ff5f, 0xff00ff87, 0xff00ffaf, 0xff00ffd7,
    0xff00ffff, 0xff00ffff, 0xff080808, 0xff121212, 0xff1c1c1c, 0xff262626,
    0xff303030, 0xff3a3a3a, 0xff444444, 0xff4e4e4e, 0xff585858, 0xff5f0000,
    0xff5f005f, 0xff5f0087, 0xff5f00af, 0xff5f00d7, 0xff5f00ff, 0xff5f5f00, //10
    0xff5f5f5f, 0xff5f5f87, 0xff5f5faf, 0xff5f5fd7, 0xff5f5fff, 0xff5f8700,
    0xff5f875f, 0xff5f8787, 0xff5f87af, 0xff5f87d7, 0xff5f87ff, 0xff5faf00,
    0xff5faf5f, 0xff5faf87, 0xff5fafaf, 0xff5fafd7, 0xff5fafff, 0xff5fd700,
    0xff5fd75f, 0xff5fd787, 0xff5fd7af, 0xff5fd7d7, 0xff5fd7ff, 0xff5fff00,
    0xff5fff5f, 0xff5fff87, 0xff5fffaf, 0xff5fffd7, 0xff5fffff, 0xff626262,
    0xff6c6c6c, 0xff767676, 0xff800000, 0xff800080, 0xff808000, 0xff808080,
    0xff808080, 0xff870000, 0xff87005f, 0xff870087, 0xff8700af, 0xff8700d7,
    0xff8700ff, 0xff875f00, 0xff875f5f, 0xff875f87, 0xff875faf, 0xff875fd7,
    0xff875fff, 0xff878700, 0xff87875f, 0xff878787, 0xff8787af, 0xff8787d7,
    0xff8787ff, 0xff87af00, 0xff87af5f, 0xff87af87, 0xff87afaf, 0xff87afd7,//20
    0xff87afff, 0xff87d700, 0xff87d75f, 0xff87d787, 0xff87d7af, 0xff87d7d7,
    0xff87d7ff, 0xff87ff00, 0xff87ff5f, 0xff87ff87, 0xff87ffaf, 0xff87ffd7,
    0xff87ffff, 0xff8a8a8a, 0xff949494, 0xff9e9e9e, 0xffa8a8a8, 0xffaf0000,
    0xffaf005f, 0xffaf0087, 0xffaf00af, 0xffaf00d7, 0xffaf00ff, 0xffaf5f00,
    0xffaf5f5f, 0xffaf5f87, 0xffaf5faf, 0xffaf5fd7, 0xffaf5fff, 0xffaf8700,
    0xffaf875f, 0xffaf8787, 0xffaf87af, 0xffaf87d7, 0xffaf87ff, 0xffafaf00,
    0xffafaf5f, 0xffafaf87, 0xffafafaf, 0xffafafd7, 0xffafafff, 0xffafd700,
    0xffafd75f, 0xffafd787, 0xffafd7af, 0xffafd7d7, 0xffafd7ff, 0xffafff00,
    0xffafff5f, 0xffafff87, 0xffafffaf, 0xffafffd7, 0xffafffff, 0xffb2b2b2,
    0xffbcbcbc, 0xffc0c0c0, 0xffc6c6c6, 0xffd0d0d0, 0xffd70000, 0xffd7005f,
    0xffd70087, 0xffd700af, 0xffd700d7, 0xffd700ff, 0xffd75f00, 0xffd75f5f,
    0xffd75f87, 0xffd75faf, 0xffd75fd7, 0xffd75fff, 0xffd78700, 0xffd7875f,
    0xffd78787, 0xffd787af, 0xffd787d7, 0xffd787ff, 0xffd7af00, 0xffd7af5f,
    0xffd7af87, 0xffd7afaf, 0xffd7afd7, 0xffd7afff, 0xffd7d700, 0xffd7d75f,
    0xffd7d787, 0xffd7d7af, 0xffd7d7d7, 0xffd7d7ff, 0xffd7ff00, 0xffd7ff5f,
    0xffd7ff87, 0xffd7ffaf, 0xffd7ffd7, 0xffd7ffff, 0xffdadada, 0xffe4e4e4,
    0xffeeeeee, 0xffff0000, 0xffff0028, 0xffff005f, 0xffff0087, 0xffff00af,
    0xffff00d7, 0xffff00ff, 0xffff00ff, 0xffff5f00, 0xffff5f5f, 0xffff5f87,
    0xffff5faf, 0xffff5fd7, 0xffff5fff, 0xffff8700, 0xffff875f, 0xffff8787,
    0xffff87af, 0xffff87d7, 0xffff87ff, 0xffffaf00, 0xffffaf5f, 0xffffaf87,
    0xffffafaf, 0xffffafd7, 0xffffafff, 0xffffd700, 0xffffd75f, 0xffffd787,
    0xffffd7af, 0xffffd7d7, 0xffffd7ff, 0xffffff00, 0xffffff28, 0xffffff5f,
    0xffffff87, 0xffffffaf, 0xffffffd7, 0xffffffff,
};


/* PixFormat: ARGB => A:bit31~bit24 R:bit23~bit16 G:bit15~bit8 B:bit7~bit0 */
const RK_U32 u32DftARGB8888ColorTbl[PALETTE_TABLE_LEN] = {
    0x00000000, 0xff000000, 0xff000000, 0xff00005f, 0xff000080, 0xff000087,//0x00ffffff
    0xff0000af, 0xff0000d7, 0xff0000ff, 0xff0000ff, 0xff005f00, 0xff005f5f,
    0xff005f87, 0xff005faf, 0xff005fd7, 0xff005fff, 0xff008000, 0xff008080,
    0xff008700, 0xff00875f, 0xff008787, 0xff0087af, 0xff0087d7, 0xff0087ff,
    0xff00af00, 0xff00af5f, 0xff00af87, 0xff00afaf, 0xff00afd7, 0xff00afff,
    0xff00d700, 0xff00d75f, 0xff00d787, 0xff00d7af, 0xff00d7d7, 0xff00d7ff,
    0xff00ff00, 0xff00ff28, 0xff00ff5f, 0xff00ff87, 0xff00ffaf, 0xff00ffd7,
    0xff00ffff, 0xff00ffff, 0xff080808, 0xff121212, 0xff1c1c1c, 0xff262626,
    0xff303030, 0xff3a3a3a, 0xff444444, 0xff4e4e4e, 0xff585858, 0xff5f0000,
    0xff5f005f, 0xff5f0087, 0xff5f00af, 0xff5f00d7, 0xff5f00ff, 0xff5f5f00,
    0xff5f5f5f, 0xff5f5f87, 0xff5f5faf, 0xff5f5fd7, 0xff5f5fff, 0xff5f8700,
    0xff5f875f, 0xff5f8787, 0xff5f87af, 0xff5f87d7, 0xff5f87ff, 0xff5faf00,
    0xff5faf5f, 0xff5faf87, 0xff5fafaf, 0xff5fafd7, 0xff5fafff, 0xff5fd700,
    0xff5fd75f, 0xff5fd787, 0xff5fd7af, 0xff5fd7d7, 0xff5fd7ff, 0xff5fff00,
    0xff5fff5f, 0xff5fff87, 0xff5fffaf, 0xff5fffd7, 0xff5fffff, 0xff626262,
    0xff6c6c6c, 0xff767676, 0xff800000, 0xff800080, 0xff808000, 0xff808080,
    0xff808080, 0xff870000, 0xff87005f, 0xff870087, 0xff8700af, 0xff8700d7,
    0xff8700ff, 0xff875f00, 0xff875f5f, 0xff875f87, 0xff875faf, 0xff875fd7,
    0xff875fff, 0xff878700, 0xff87875f, 0xff878787, 0xff8787af, 0xff8787d7,
    0xff8787ff, 0xff87af00, 0xff87af5f, 0xff87af87, 0xff87afaf, 0xff87afd7,
    0xff87afff, 0xff87d700, 0xff87d75f, 0xff87d787, 0xff87d7af, 0xff87d7d7,
    0xff87d7ff, 0xff87ff00, 0xff87ff5f, 0xff87ff87, 0xff87ffaf, 0xff87ffd7,
    0xff87ffff, 0xff8a8a8a, 0xff949494, 0xff9e9e9e, 0xffa8a8a8, 0xffaf0000,
    0xffaf005f, 0xffaf0087, 0xffaf00af, 0xffaf00d7, 0xffaf00ff, 0xffaf5f00,
    0xffaf5f5f, 0xffaf5f87, 0xffaf5faf, 0xffaf5fd7, 0xffaf5fff, 0xffaf8700,
    0xffaf875f, 0xffaf8787, 0xffaf87af, 0xffaf87d7, 0xffaf87ff, 0xffafaf00,
    0xffafaf5f, 0xffafaf87, 0xffafafaf, 0xffafafd7, 0xffafafff, 0xffafd700,
    0xffafd75f, 0xffafd787, 0xffafd7af, 0xffafd7d7, 0xffafd7ff, 0xffafff00,
    0xffafff5f, 0xffafff87, 0xffafffaf, 0xffafffd7, 0xffafffff, 0xffb2b2b2,
    0xffbcbcbc, 0xffc0c0c0, 0xffc6c6c6, 0xffd0d0d0, 0xffd70000, 0xffd7005f,
    0xffd70087, 0xffd700af, 0xffd700d7, 0xffd700ff, 0xffd75f00, 0xffd75f5f,
    0xffd75f87, 0xffd75faf, 0xffd75fd7, 0xffd75fff, 0xffd78700, 0xffd7875f,
    0xffd78787, 0xffd787af, 0xffd787d7, 0xffd787ff, 0xffd7af00, 0xffd7af5f,
    0xffd7af87, 0xffd7afaf, 0xffd7afd7, 0xffd7afff, 0xffd7d700, 0xffd7d75f,
    0xffd7d787, 0xffd7d7af, 0xffd7d7d7, 0xffd7d7ff, 0xffd7ff00, 0xffd7ff5f,
    0xffd7ff87, 0xffd7ffaf, 0xffd7ffd7, 0xffd7ffff, 0xffdadada, 0xffe4e4e4,
    0xffeeeeee, 0xffff0000, 0xffff0028, 0xffff005f, 0xffff0087, 0xffff00af,
    0xffff00d7, 0xffff00ff, 0xffff00ff, 0xffff5f00, 0xffff5f5f, 0xffff5f87,
    0xffff5faf, 0xffff5fd7, 0xffff5fff, 0xffff8700, 0xffff875f, 0xffff8787,
    0xffff87af, 0xffff87d7, 0xffff87ff, 0xffffaf00, 0xffffaf5f, 0xffffaf87,
    0xffffafaf, 0xffffafd7, 0xffffafff, 0xffffd700, 0xffffd75f, 0xffffd787,
    0xffffd7af, 0xffffd7d7, 0xffffd7ff, 0xffffff00, 0xffffff28, 0xffffff5f,
    0xffffff87, 0xffffffaf, 0xffffffd7, 0xffffffff,
};

static void bmp_to_yuva_map(osd_data_s *data);
static bool osd_get_bmp_info(osd_data_s *draw_data);

void mpp_osd_region_id_enable_set(MpiEncTestData *p, int region_id, int enable)
{
    if (p)
        p->osd_data.region[region_id].enable = enable;
}

int mpp_osd_region_id_enable_get(MpiEncTestData *p, int region_id)
{
    if (p)
        return p->osd_data.region[region_id].enable;
    else
        return -1;
}

void mpp_osd_enable_set(MpiEncTestData *p, bool enable)
{
    int i = 0;
    if (p) {
        p->osd_enable = enable;
        if ((p->type == MPP_VIDEO_CodingAVC || p->type == MPP_VIDEO_CodingHEVC)) {
            if (!enable) {
                for (i = 0; i < OSD_REGIONS_CNT; i ++) {
#ifdef MPP_OSD_ENABLE_SET_HISTORY
                    p->osd_index_enable[i] = p->osd_data.region[i].enable;
#endif
                    mpp_osd_region_id_enable_set(p, i, false);
                }
            } else {
                for (i = 0; i < OSD_REGIONS_CNT; i ++) {
#ifdef MPP_OSD_ENABLE_SET_HISTORY
                    mpp_osd_region_id_enable_set(p, i, p->osd_index_enable[i]);
#else
                    mpp_osd_region_id_enable_set(p, i, true);
#endif
                }
            }
        }
    }
}

bool mpp_osd_enable_get(MpiEncTestData *p)
{
    if (p)
        return p->osd_enable;
    else
        return false;
}

void mpp_osd_run(MpiEncTestData *p, int fd, MppFrame frame)
{
#if 0 //test
    if (!access("/tmp/osd2", 0)) {
        mpp_osd_enable_set(p, !mpp_osd_enable_get(p));
        system("rm /tmp/osd2");
    }
#endif

    if (mpp_osd_enable_get(p)) {// && p->osd_data.num_region && p->osd_data.buf
        if (p->type == MPP_VIDEO_CodingMJPEG) {
#if MJPEG_RGA_OSD_ENABLE
            for (int i = 0; i < p->osd_count; i ++) {
                if (p->osd_cfg[i].set_ok == true) {
                   if (p->osd_cfg[i].type == OSD_REGION_TYPE_PICTURE) {
#if 0 //test
                       if (!access("/tmp/osd0", 0)) {
                           mpp_osd_region_id_enable_set(p, 0, !mpp_osd_region_id_enable_get(p, 0));
                           system("rm /tmp/osd0");
                       } //test
                       if (!access("/tmp/osd1", 0)) {
                           mpp_osd_region_id_enable_set(p, 1, !mpp_osd_region_id_enable_get(p, 1));
                           system("rm /tmp/osd1");
                       }
#endif
                       if (mpp_osd_region_id_enable_get(p, i))
                           mjpeg_rga_osd_process(p, i, fd);
                   } else {
                       LOG_WARN("ost no supprot this type:%d\n", p->osd_cfg[i].type);
                   }
                }
            }
#endif
        } else {
            MppMeta meta = NULL;
            meta = mpp_frame_get_meta(frame);
           // LOG_INFO("MPP Encoder: set osd data(%d regions) to frame\n", p->osd_data.num_region);
#if 0 //test
            if (!access("/tmp/osd0", 0)) {
                mpp_osd_region_id_enable_set(p, 0, !mpp_osd_region_id_enable_get(p, 0));
                system("rm /tmp/osd0");
            } //test
            if (!access("/tmp/osd1", 0)) {
                mpp_osd_region_id_enable_set(p, 1, !mpp_osd_region_id_enable_get(p, 1));
                system("rm /tmp/osd1");
            }
#endif
            mpp_meta_set_ptr(meta, KEY_OSD_DATA, (void*)&p->osd_data);
        }
    }

}

static int mpp_osd_update_region_info(MppEncOSDData *osd, OsdRegionData *region_data)
{
  uint32_t new_size = 0;
  uint32_t old_size = 0;
  uint8_t rid = region_data->region_id;
  uint8_t *region_src = NULL;
  uint8_t *region_dst = NULL;

  if (!region_data->enable) {
      osd->region[rid].enable = 0;
      osd->num_region = 0;
      for (int i = 0; i < OSD_REGIONS_CNT; i++) {
          if (osd->region[i].enable)
              osd->num_region = i + 1;
      }
      assert(osd->num_region <= 8);
      return 0;
  }

  // get buffer size to compare.
  new_size = region_data->width * region_data->height;
  // If there is enough space, reuse the previous buffer.
  // However, the current area must be active, so as to
  // avoid opening up too large a buffer at the beginning,
  // and it will not be reduced later.
  if (osd->region[rid].enable)
    old_size =
      osd->region[rid].num_mb_x * osd->region[rid].num_mb_y * 256;

  // update region info.
  osd->region[rid].enable = region_data->enable;//1;
  osd->region[rid].inverse = region_data->inverse;
  osd->region[rid].start_mb_x = region_data->pos_x / 16;
  osd->region[rid].start_mb_y = region_data->pos_y / 16;
  osd->region[rid].num_mb_x = region_data->width / 16;
  osd->region[rid].num_mb_y = region_data->height / 16;

  // 256 * 16 => 4096 is enough for osd.
  assert(osd->region[rid].start_mb_x <= 256);
  assert(osd->region[rid].start_mb_y <=  256);
  assert(osd->region[rid].num_mb_x <=  256);
  assert(osd->region[rid].num_mb_y <=  256);

  // region[rid] buffer size is enough, copy data directly.
  if (old_size >= new_size) {
      LOG_DEBUG("MPP Encoder: Region[%d] reuse old buff:%u, new_size:%u\n",
                 rid, old_size, new_size);
      region_src = region_data->buffer;
      region_dst = (uint8_t *)mpp_buffer_get_ptr(osd->buf);
      region_dst += osd->region[rid].buf_offset;
      memcpy(region_dst, region_src, new_size);

      return 0;
  }

  // region[rid] buffer size too small, resize buffer.
  MppBuffer new_buff = NULL;
  MppBuffer old_buff = NULL;
  uint32_t old_offset[OSD_REGIONS_CNT] = {0};
  uint32_t total_size = 0;
  uint32_t current_size = 0;

  osd->num_region = 0;
  for (int i = 0; i < OSD_REGIONS_CNT; i++) {
    if (osd->region[i].enable) {
      old_offset[i] = osd->region[i].buf_offset;
      osd->region[i].buf_offset = total_size;
      total_size +=
        osd->region[i].num_mb_x * osd->region[i].num_mb_y * 256;
      osd->num_region = i + 1;
    } else {
      osd->region[i].start_mb_x = 0;
      osd->region[i].start_mb_y = 0;
      osd->region[i].buf_offset = 0;
      osd->region[i].num_mb_x = 0;
      osd->region[i].num_mb_y = 0;
    }
  }

  old_buff = osd->buf;
  int ret = mpp_buffer_get(NULL, &new_buff, total_size);
  if (ret) {
      LOG_ERROR("ERROR: MPP Encoder: get osd %dBytes buffer failed(%d)\n",
                total_size, ret);
      // reset target region.
      osd->region[rid].enable = 0;
      osd->region[rid].start_mb_x = 0;
      osd->region[rid].start_mb_y = 0;
      osd->region[rid].buf_offset = 0;
      return -1;
  }

  for (int i = 0; i < OSD_REGIONS_CNT; i++) {
      if (!osd->region[i].enable)
          continue;

      if (i != rid) {
          // copy other region data to new buffer.
          region_src = (uint8_t *)mpp_buffer_get_ptr(old_buff);
          region_src += old_offset[i];
          region_dst = (uint8_t *)mpp_buffer_get_ptr(new_buff);
          region_dst += osd->region[i].buf_offset;
          current_size =
            osd->region[i].num_mb_x * osd->region[i].num_mb_y * 256;
      } else {
          // copy current region data to new buffer.
          region_src = region_data->buffer;
          region_dst = (uint8_t *)mpp_buffer_get_ptr(new_buff);
          region_dst += osd->region[i].buf_offset;
          current_size = new_size;
      }

      assert(region_src);
      assert(region_dst);
      memcpy(region_dst, region_src, current_size);
  }

  //replace old buff with new buff.
  osd->buf = new_buff;
  if (old_buff)
      mpp_buffer_put(old_buff);

  return 0;
}

static int mpp_osd_region_set(MpiEncTestData *p, OsdRegionData *rdata)
{
  if (!rdata)
    return -1;

  LOG_DEBUG("MPP Encoder: setting osd regions...\n");
  if ((rdata->region_id >= OSD_REGIONS_CNT)) {
      LOG_ERROR("ERROR: MPP Encoder: invalid region id(%d), should be [0, %d).\n",
                 rdata->region_id, OSD_REGIONS_CNT);
      return -1;
  }

  if (rdata->enable && !rdata->buffer) {
      LOG_ERROR("ERROR: MPP Encoder: invalid region data");
      return -1;
  }

  if ((rdata->width % 16) || (rdata->height % 16) ||
      (rdata->pos_x % 16) || (rdata->pos_y % 16)) {
      LOG_WARN("WARN: MPP Encoder: osd size must be 16 aligned,w:%d,h:%d,x:%d,y:%d\n",
                rdata->width, rdata->height, rdata->pos_x, rdata->pos_y);
      rdata->width = UPALIGNTO16(rdata->width);
      rdata->height = UPALIGNTO16(rdata->height);
      rdata->pos_x = UPALIGNTO16(rdata->pos_x);
      rdata->pos_y = UPALIGNTO16(rdata->pos_y);
  }

  int ret = mpp_osd_update_region_info(&p->osd_data, rdata);
  return ret;
}

int mpp_enc_gen_osd_plt(MpiEncTestData *p, uint32_t *ptl_data)
{
  if (!ptl_data)
      return -1;

  MppCtx ctx = p->ctx;
  MppApi *mpi = p->mpi;
  MppEncOSDPltCfg osd_plt_cfg;

  //TODO rgba plt to yuva plt.
  for (int k = 0; k < 256; k++)
      p->osd_plt.data[k].val = *(ptl_data + k);//ptl_data[k % 8];//

  p->osd_plt_cfg.change = MPP_ENC_OSD_PLT_CFG_CHANGE_ALL;
  p->osd_plt_cfg.type = MPP_ENC_OSD_PLT_TYPE_USERDEF;
  p->osd_plt_cfg.plt = &p->osd_plt;

  int ret = mpi->control(ctx, MPP_ENC_SET_OSD_PLT_CFG, &p->osd_plt_cfg);
  if (ret)
      LOG_ERROR("ERROR: MPP Encoder: set osd plt failed ret %d\n", ret);

  return ret;
}

int mpp_osd_bmp_to_ayuv_image(MpiEncTestData *p, osd_data_s *draw_data)
{
    int ret = 0;
    int read_len = 0, read_cout = 0, pitch = 0;
    OsdRegionData osd_region_data;

    draw_data->bmp_file = fopen(draw_data->image, "rb");
    if (draw_data->bmp_file == NULL) {
        LOG_ERROR("yuv file open:%s fail!\n", draw_data->image);
        return -1;
    }
    if(!osd_get_bmp_info(draw_data)) {
        LOG_ERROR("osd_get_bmp_info fail!\n");
        ret = -1;
        goto ERR;
    }
    draw_data->width = draw_data->bmp_info_head.biWidth;
    draw_data->height = draw_data->bmp_info_head.biHeight;
    pitch = WIDTHBYTES(draw_data->width * draw_data->bmp_info_head.biBitCount);
    draw_data->size = pitch * draw_data->height;

    LOG_DEBUG("w:%d h:%d d:%d size:%d bfOffBits:%d\n",
               draw_data->width, draw_data->height,
               draw_data->bmp_info_head.biBitCount, draw_data->size,
               draw_data->bmp_bit_head.bfOffBits); //test debug

    draw_data->bmp_data = (uint8_t *)malloc(draw_data->size);

    while (read_len < draw_data->size && read_cout < 10) {//read bmp data
        read_len += fread(&draw_data->bmp_data[read_len], 1, draw_data->size - read_len, draw_data->bmp_file);
        read_cout ++;
    };
    if (read_cout >= 10) {
        LOG_ERROR("yuv read open:%s fail, len=%d!\n", draw_data->image, read_len);
        ret = -1;
        goto ERR;
    }

    LOG_DEBUG("0:%d 1:%d 2:%d 3:%d 4:%d\n",
               draw_data->bmp_data[0],
               draw_data->bmp_data[1],
               draw_data->bmp_data[2],
               draw_data->bmp_data[3],
               draw_data->bmp_data[4]); //test debug

    draw_data->buffer = (uint8_t *)malloc(draw_data->width * draw_data->height);
    bmp_to_yuva_map(draw_data);

    osd_region_data.inverse = 0;
    osd_region_data.enable = draw_data->enable;
    osd_region_data.region_id = draw_data->region_id;
    osd_region_data.pos_x = draw_data->origin_x;
    osd_region_data.pos_y = draw_data->origin_y;
    osd_region_data.width = draw_data->width;
    osd_region_data.height = draw_data->height;
    osd_region_data.buffer = draw_data->buffer;

    LOG_DEBUG("UpdateImage enable %d x %d y %d w %d h %d\n", draw_data->enable,
              draw_data->origin_x, draw_data->origin_y, draw_data->width,
              draw_data->height);
    ret = mpp_osd_region_set(p, &osd_region_data);
ERR:
    if (draw_data->bmp_file)
        fclose(draw_data->bmp_file);
    if (draw_data->bmp_data)
        free(draw_data->bmp_data);
    if (draw_data->buffer)
        free(draw_data->buffer);
    return ret;
}
/* Match an RGB value to a particular palette index */
static bool osd_get_bmp_info(osd_data_s *draw_data)
{
    WORD fileType;
    char tmp[100];
    fread(&fileType, 1, sizeof(WORD), draw_data->bmp_file);
    if (fileType != 0x4d42) {
        LOG_ERROR("file is not .bmp file:%s!", draw_data->image);
        return false;
    }
    fread(&draw_data->bmp_bit_head, 1, sizeof(BITMAPFILEHEADER), draw_data->bmp_file);
    fread(&draw_data->bmp_info_head, 1, sizeof(BITMAPINFOHEADER), draw_data->bmp_file);
    if (draw_data->bmp_bit_head.bfOffBits > 54) {
        fread(tmp, 1, draw_data->bmp_bit_head.bfOffBits - 54, draw_data->bmp_file);
        draw_data->bmp_type = BMP_TYPE_BGRA_2;
    } else {
        draw_data->bmp_type = BMP_TYPE_BGRA_1;
    }

    if (draw_data->bmp_info_head.biBitCount < 24) {
        LOG_ERROR("bmp:%s must be 24/32 depth!", draw_data->image);
        return false;
    }
    return true;
}

RK_S32 color_tbl_argb_to_avuy(const RK_U32 *pu32RgbaTbl, RK_U32 *pu32AvuyTbl)
{
  unsigned char r, g, b, a;
  unsigned char y, u, v, alpha;

  for (int i = 0; i < PALETTE_TABLE_LEN; i++) {
    a = (pu32RgbaTbl[i] & 0xFF000000) >> 24;
    r = (pu32RgbaTbl[i] & 0x00FF0000) >> 16;
    g = (pu32RgbaTbl[i] & 0x0000FF00) >> 8;
    b = (pu32RgbaTbl[i] & 0x000000FF);

    // BT601
    y = 16 + 0.257 * r + 0.504 * g + 0.098 * b;
    u = 128 - 0.148 * r - 0.291 * g + 0.439 * b;
    v = 128 + 0.439 * r - 0.368 * g - 0.071 * b;
    alpha = a;

    pu32AvuyTbl[i] = (alpha << 24) | (v << 16) | (u << 8) | y;
  }

  return 0;
}

static uint8_t inline find_color(const uint32_t *pal, uint32_t len, uint8_t r,
                                  uint8_t g, uint8_t b)
{
    uint32_t i = 0;
    uint8_t pixel = 0;
    unsigned int smallest = 0;
    unsigned int distance = 0;
    int rd, gd, bd;
    uint8_t rp, gp, bp;

    smallest = ~0;

    for (i = 0; i < len; ++i) {
        bp = (pal[i] & 0xff000000) >> 24;
        gp = (pal[i] & 0x00ff0000) >> 16;
        rp = (pal[i] & 0x0000ff00) >> 8;
        rd = rp - r;
        gd = gp - g;
        bd = bp - b;
        distance = (rd * rd) + (gd * gd) + (bd * bd);
        if (distance < smallest) {
            pixel = i;
            /* Perfect match! */
            if (distance == 0)
              break;
            smallest = distance;
        }
    }

    return pixel;
}

/* Match an RGB value to a particular palette index */
RK_U8 find_argb_color_tbl_by_order(const RK_U32 *pal, RK_U32 len,
                                   RK_U32 u32ArgbColor)
{
  RK_U8 a, r, g, b;
  RK_U32 i = 0;
  RK_U8 pixel = 0;
  RK_U32 smallest = 0;
  RK_U32 distance = 0;
  RK_U8 ap, rp, gp, bp;

  a = (u32ArgbColor & 0xFF000000) >> 24;
  r = (u32ArgbColor & 0x00FF0000) >> 16;
  g = (u32ArgbColor & 0x0000FF00) >> 8;
  b = (u32ArgbColor & 0x000000FF);

  smallest = ~0;
  for (i = 0; i < len; ++i) {
    ap = (pal[i] & 0xFF000000) >> 24;
    rp = (pal[i] & 0x00FF0000) >> 16;
    gp = (pal[i] & 0x0000FF00) >> 8;
    bp = (pal[i] & 0x000000FF);

    distance = abs(ap - a) + abs(rp - r) + abs(gp - g) + abs(bp - b);
    if (distance < smallest) {
      pixel = i;

      /* Perfect match! */
      if (distance == 0)
        break;

      smallest = distance;
    }
  }

  return pixel;
}

static void bmp_to_yuva_map(osd_data_s *data)
{
    LOG_DEBUG("bmp_to_yuva_map depth:%d\n", data->bmp_info_head.biBitCount);
    int k, offset;
    int width = data->width;
    int height = data->height;
    int pitch = WIDTHBYTES(width * data->bmp_info_head.biBitCount);

    switch (data->bmp_info_head.biBitCount) {
        case 24:
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                k = (height - i - 1) * pitch + j * 3;
                offset = i * data->width + j;
                // printf("offset %d k %d\n", offset, k);
                data->buffer[offset] =
                    find_color(rgb888_palette_table, PALETTE_TABLE_LEN, data->bmp_data[k + 2],
                               data->bmp_data[k + 1], data->bmp_data[k]);//
            }
        }
        break;
        case 32: {
            RK_U32 TargetWidth, TargetHeight;
            BMP_DATA ColorValue;
            RK_U32 *BitmapLineStart;
            RK_U8 *CanvasLineStart;
            TargetWidth = data->width;
            TargetHeight = data->height;

            RK_U8 tb_data;
#if OSD_DATA_DEBUG
            FILE *fp_in = NULL, *fp_out = NULL;
            fp_in = fopen("/data/in.bmp","w+b");
            fp_out = fopen("/data/out.bmp","w+b");
#endif
            for (RK_U32 i = 0; i < TargetHeight; i++) {
                BitmapLineStart = (RK_U32 *)data->bmp_data + (TargetHeight - 1 - i) * TargetWidth;
                CanvasLineStart = data->buffer + i * TargetWidth;
                for (RK_U32 j = 0; j < TargetWidth; j++) {
                    ColorValue.data_32 = *(BitmapLineStart + j);
                    if (data->bmp_type == BMP_TYPE_BGRA_1 && ColorValue.data_32 != 0x00ffffff)
                        ColorValue.data_32 |= 0xff000000;
                    else if (data->bmp_type == BMP_TYPE_BGRA_2 && ColorValue.bgra_a == 0x00)
                        ColorValue.data_32 = 0x00ffffff;

                    tb_data = find_argb_color_tbl_by_order(
                          data->plt_table, PALETTE_TABLE_LEN, ColorValue.data_32);
#if FIX_MPP_WHITE_EDGE_VALUE
                    if (tb_data > FIX_MPP_WHITE_EDGE_VALUE) {
                        //LOG_ERROR("data:%d!", tb_data); // test
                        tb_data = 0;
                    }
#endif
                    *(CanvasLineStart + j) = tb_data;
#if OSD_DATA_DEBUG
                    fwrite(&ColorValue.data_32, 1, 4, fp_in);
                    fwrite(&tb_data, 1, 1, fp_out);
#endif
                }
            }
#if OSD_DATA_DEBUG
            fclose(fp_in);
            fclose(fp_out);
            LOG_ERROR("!");
#endif
/*
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    k = (height - i - 1) * pitch + j * 4;
                    offset = i * data->width + j;
                    // printf("offset %d k %d\n", offset, k);
                    data->buffer[offset] =
                        find_color(u32DftARGB8888ColorTbl, PALETTE_TABLE_LEN, data->bmp_data[k + 2],
                                   data->bmp_data[k + 1], data->bmp_data[k]);//rgb888_palette_table
                }
            }*/
        }
        break;
        default:
            LOG_ERROR("bmp must be 24/32 depth!");
        break;
    }
}

static MPP_RET mpp_enc_gen_osd_data(MppEncOSDData *osd_data, MppBuffer osd_buf, RK_U32 frame_cnt)
{
    RK_U32 k = 0;
    RK_U32 buf_size = 0;
    RK_U32 buf_offset = 0;
    RK_U8 *buf = mpp_buffer_get_ptr(osd_buf);

    osd_data->num_region = 8;
    osd_data->buf = osd_buf;

    for (k = 0; k < osd_data->num_region; k++) {
        MppEncOSDRegion *region = &osd_data->region[k];
        RK_U8 idx = k;

        region->enable = 1;
        region->inverse = 0;//frame_cnt & 1;
        region->start_mb_x = k * 3;
        region->start_mb_y = k * 2;
        region->num_mb_x = 2;
        region->num_mb_y = 2;

        buf_size = region->num_mb_x * region->num_mb_y * 256;
        buf_offset = k * buf_size;
        osd_data->region[k].buf_offset = buf_offset;

        memset(buf + buf_offset, idx, buf_size);
    }

    return MPP_OK;
}

MPP_RET mpp_osd_default_set(MpiEncTestData *p)
{
    MPP_RET ret = MPP_OK;
    if (p->osd_enable && (p->type == MPP_VIDEO_CodingAVC || p->type == MPP_VIDEO_CodingHEVC)) {
        const RK_U32 *pu32ArgbColorTbl;
        if (p->osd_plt_user)
            pu32ArgbColorTbl = u32DftARGB8888ColorTblUser;
        else
            pu32ArgbColorTbl = u32DftARGB8888ColorTbl;
        color_tbl_argb_to_avuy(pu32ArgbColorTbl, p->plt_table);
        p->osd_idx_size  = p->hor_stride * p->ver_stride / 8;
        ret = mpp_buffer_get(p->pkt_grp, &p->osd_idx_buf, p->osd_idx_size);
        if (ret) {
            LOG_ERROR("failed to get buffer for input osd index ret %d\n", ret);
            goto RET;
        }
#if 1
        /* gen and cfg osd plt */
        ret = mpp_enc_gen_osd_plt(p, p->plt_table);
        if (ret) {
            LOG_ERROR("mpp_enc_gen_osd_plt err:%d\n", ret);
            goto RET;
        }
        osd_data_s osd_data;
        osd_data.plt_table = pu32ArgbColorTbl;

        for (int i = 0; i < p->osd_count; i++) {
            if (p->osd_cfg[i].set_ok == true) {
                if (p->osd_cfg[i].type == OSD_REGION_TYPE_PICTURE) {
                    osd_data.enable = p->osd_cfg[i].enable;
                    osd_data.region_id = i;
                    osd_data.origin_x = p->osd_cfg[i].start_x;
                    osd_data.origin_y = p->osd_cfg[i].start_y;
                    osd_data.image = p->osd_cfg[i].image_path;
                    ret = mpp_osd_bmp_to_ayuv_image(p, &osd_data);
                    if (ret)
                        p->osd_cfg[i].set_ok = false;
                } else {
                    LOG_WARN("ost no supprot this type:%d\n", p->osd_cfg[i].type);
                }
            }
        }
#else // test //
        /* gen and cfg osd plt */
        ret = mpp_enc_gen_osd_plt(p, p->plt_table);
        if (ret) {
            LOG_ERROR("mpp_enc_gen_osd_plt err:%d\n", ret);
            goto RET;
        }
#if 1 //yuv image demo
        osd_data_s osd_data;

        osd_data.plt_table = pu32ArgbColorTbl;
        osd_data.enable = 1;
        osd_data.region_id = 0;
        osd_data.origin_x = 16;
        osd_data.origin_y = 32;
        osd_data.image = "/data/test-32.bmp";
        mpp_osd_bmp_to_ayuv_image(p, &osd_data);

        osd_data.enable = 1;
        osd_data.region_id = 1;
        osd_data.origin_x = 16;
        osd_data.origin_y = 320;
        osd_data.image = "/data/mute-32.bmp";
        mpp_osd_bmp_to_ayuv_image(p, &osd_data);
#else //simple demo
        mpp_enc_gen_osd_data(&p->osd_data, p->osd_idx_buf, p->frame_count);
#endif
#endif
    }
#if MJPEG_RGA_OSD_ENABLE || YUV_RGA_OSD_ENABLE
    else if (p->osd_enable && ((p->type == MPP_VIDEO_CodingMJPEG && MJPEG_RGA_OSD_ENABLE) ||
             (p->type == 0 && YUV_RGA_OSD_ENABLE))) {
        p->rga_osd_drm_fd = -1;
        osd_data_s osd_data;
        for (int i = 0; i < p->osd_count; i++) {
           if (p->osd_cfg[i].set_ok == true) {
               if (p->osd_cfg[i].type == OSD_REGION_TYPE_PICTURE) {
                   osd_data.enable = p->osd_cfg[i].enable;
                   mpp_osd_region_id_enable_set(p, i, osd_data.enable);
                   osd_data.region_id = i;
                   osd_data.origin_x = p->osd_cfg[i].start_x;
                   osd_data.origin_y = p->osd_cfg[i].start_y;
                   osd_data.image = p->osd_cfg[i].image_path;
                   ret = mjpeg_rga_osd_bmp_to_buff(p, &osd_data);
                   if (ret)
                       p->osd_cfg[i].set_ok = false;
               } else {
                   LOG_WARN("ost no supprot this type:%d\n", p->osd_cfg[i].type);
               }
           }
       }
    }
#endif
RET:
    return ret;
}
#endif

#if MJPEG_RGA_OSD_ENABLE || YUV_RGA_OSD_ENABLE
#include <rga/im2d.h>
#include <rga/rga.h>
#include "drm.h"

int mjpeg_rga_osd_bmp_to_buff(MpiEncTestData *p, osd_data_s *draw_data)
{
    int ret = 0;
    int read_len = 0, read_cout = 0, pitch = 0;
    OsdRegionData osd_region_data;
    uint32_t id = draw_data->region_id;

    draw_data->bmp_file = fopen(draw_data->image, "rb");
    if (draw_data->bmp_file == NULL) {
        LOG_ERROR("yuv file open:%s fail!\n", draw_data->image);
        return -1;
    }
    if(!osd_get_bmp_info(draw_data)) {
        LOG_ERROR("osd_get_bmp_info fail!\n");
        ret = -1;
        goto ERR;
    }
    draw_data->width = draw_data->bmp_info_head.biWidth;
    draw_data->height = draw_data->bmp_info_head.biHeight;
    pitch = WIDTHBYTES(draw_data->width * draw_data->bmp_info_head.biBitCount);
    draw_data->size = pitch * draw_data->height;
    p->osd_cfg[id].width = draw_data->width;
    p->osd_cfg[id].height = draw_data->height;
    p->osd_cfg[id].drm_size = draw_data->size;
    LOG_DEBUG("w:%d h:%d d:%d size:%d bfOffBits:%d bmp_type:%d\n",
               draw_data->width, draw_data->height,
               draw_data->bmp_info_head.biBitCount, draw_data->size,
               draw_data->bmp_bit_head.bfOffBits,
               draw_data->bmp_type); //test debug

    if (draw_data->bmp_info_head.biBitCount != 32) {
        LOG_ERROR("only support 32 bit argb bmp!this biBitCount:%d not support now!\n",
                   draw_data->bmp_info_head.biBitCount);
        ret = -1;
        goto ERR;
    }

    if (p->rga_osd_drm_fd == -1)
        p->rga_osd_drm_fd = drm_open();
    if (p->rga_osd_drm_fd < 0) {
        LOG_ERROR("rga_osd_drm_fd=%d!\n", p->rga_osd_drm_fd);
        ret = -1;
        goto ERR;
    }
    ret = drm_alloc(p->rga_osd_drm_fd, draw_data->size, 16, &p->osd_cfg[id].handle, 0);
    if (ret)
    {
        LOG_ERROR("drm_alloc fail\n");
        goto ERR;
    }
    ret = drm_handle_to_fd(p->rga_osd_drm_fd, p->osd_cfg[id].handle, &p->osd_cfg[id].rga_osd_fd, 0);
    if (ret)
    {
        LOG_ERROR("drm_handle_to_fd fail\n");
        goto ERR;
    }
    p->osd_cfg[id].buffer = (uint8_t *)drm_map_buffer(p->rga_osd_drm_fd, p->osd_cfg[id].handle, p->osd_cfg[id].drm_size);
    LOG_DEBUG("rga_osd_drm_fd=%d,buffer->handle=%d,size=%d\n",
               p->rga_osd_drm_fd, p->osd_cfg[id].handle, p->osd_cfg[id].drm_size);

    draw_data->bmp_data = (uint8_t *)malloc(draw_data->size);

    while (read_len < draw_data->size && read_cout < 10) {//read bmp data
        read_len += fread(&draw_data->bmp_data[read_len], 1, draw_data->size - read_len, draw_data->bmp_file);
        read_cout ++;
    };

    if (read_cout >= 10) {
        LOG_ERROR("yuv read open:%s fail, len=%d!\n", draw_data->image, read_len);
        ret = -1;
        goto ERR;
    }

#if OSD_DATA_DEBUG
    RK_U8 tb_data;
    FILE *fp_out = NULL;
    fp_out = fopen("/data/out.bmp","w+b");
#endif

    RK_U32 TargetWidth, TargetHeight;
    BMP_DATA ColorValue;
    RK_U32 *BitmapLineStart;
    RK_U32 *CanvasLineStart;
    RK_U8 CanvasValue;
    TargetWidth = draw_data->width;
    TargetHeight = draw_data->height;

    if (draw_data->bmp_type == BMP_TYPE_BGRA_1) {
        for (RK_U32 i = 0; i < TargetHeight; i++) {
            BitmapLineStart = (RK_U32 *)draw_data->bmp_data + (TargetHeight - 1 - i) * TargetWidth;
            CanvasLineStart = (RK_U32 *)(p->osd_cfg[id].buffer + i * TargetWidth * 4);
            for (RK_U32 j = 0; j < TargetWidth; j++) {
                ColorValue.data_32 = *(BitmapLineStart + j); //bgra1 format
                if (ColorValue.data_32 == 0x00ffffff)
                    ColorValue.data_32 = 0x00000000; //0x00000000 mean transparency
                else if (ColorValue.data_32 > 0) {
#if FIX_RGA_WHITE_EDGE_VALUE
                    if (ColorValue.bgra_r > FIX_RGA_WHITE_EDGE_VALUE &&
                        ColorValue.bgra_g > FIX_RGA_WHITE_EDGE_VALUE &&
                        ColorValue.bgra_b > FIX_RGA_WHITE_EDGE_VALUE) //fix white edge
                        ColorValue.data_32 = 0x00000000; //0x00000000 mean transparency
                    else
#endif
                        ColorValue.data_32 |= 0xff000000; //0xffxxxxxx mean not transparency
                }
                *(CanvasLineStart + j) = ColorValue.data_32;// bgra -> bgra
            }
        }

    } else if (draw_data->bmp_type == BMP_TYPE_BGRA_2) {
        for (RK_U32 i = 0; i < TargetHeight; i++) {
            BitmapLineStart = (RK_U32 *)draw_data->bmp_data + (TargetHeight - 1 - i) * TargetWidth;
            CanvasLineStart = (RK_U32 *)(p->osd_cfg[id].buffer + i * TargetWidth * 4);
            for (RK_U32 j = 0; j < TargetWidth; j++) {
                ColorValue.data_32 = *(BitmapLineStart + j); //bgra2 format
                if (ColorValue.bgra_a == 0x00)
                    ColorValue.data_32 = 0x00000000; //0x00000000 mean transparency
                else if (ColorValue.data_32 > 0) {
#if FIX_RGA_WHITE_EDGE_VALUE
                    if (ColorValue.bgra_r > FIX_RGA_WHITE_EDGE_VALUE &&
                        ColorValue.bgra_g > FIX_RGA_WHITE_EDGE_VALUE &&
                        ColorValue.bgra_b > FIX_RGA_WHITE_EDGE_VALUE) //fix white edge
                        ColorValue.data_32 = 0x00000000; //0x00000000 mean transparency
                    else
#endif
                        ColorValue.data_32 |= 0xff000000; //0xffxxxxxx mean not transparency
                }
                *(CanvasLineStart + j) = ColorValue.data_32;// bgra -> bgra
            }
        }
    } else {
        LOG_ERROR("no support such bmp type=%d!\n", draw_data->bmp_type);
        ret = -1;
        goto ERR;
    }

#if OSD_DATA_DEBUG
    for (RK_U32 i = 0; i < draw_data->size; i++)
        fwrite(&p->osd_cfg[id].buffer[i], 1, 1, fp_out);
    fclose(fp_out);
    LOG_ERROR("debug osd data ok!");
#endif

    LOG_DEBUG("TargetWidth=%d,TargetHeight=%d,0:%x 1:%x 2:%x 3:%x 4:%x\n",
               TargetWidth, TargetHeight,
               p->osd_cfg[id].buffer[0],
               p->osd_cfg[id].buffer[1],
               p->osd_cfg[id].buffer[2],
               p->osd_cfg[id].buffer[3],
               p->osd_cfg[id].buffer[4]); //test debug

ERR:
    if (draw_data->bmp_file)
        fclose(draw_data->bmp_file);
    if (draw_data->bmp_data)
        free(draw_data->bmp_data);
    if (ret) {
       drm_unmap_buffer(p->osd_cfg[id].buffer, p->osd_cfg[id].drm_size);
       if (p->osd_cfg[id].rga_osd_fd)
           close(p->osd_cfg[id].rga_osd_fd);
       if (p->rga_osd_drm_fd && p->osd_cfg[id].handle)
           drm_free(p->rga_osd_drm_fd, p->osd_cfg[id].handle);
       drm_close(p->rga_osd_drm_fd);
    }
    return ret;
}

void mjpeg_rga_osd_process(MpiEncTestData *p, int id, int src_fd)
{
    rga_buffer_t pat;
    rga_buffer_t src;
    IM_STATUS STATUS;
    src = wrapbuffer_fd(src_fd, p->width, p->height, RK_FORMAT_YCbCr_420_SP);
    pat = wrapbuffer_fd(p->osd_cfg[id].rga_osd_fd, p->osd_cfg[id].width, p->osd_cfg[id].height, RK_FORMAT_BGRA_8888);
    //RK_FORMAT_RGBA_8888 // RK_FORMAT_BGRA_8888

    im_rect pat_rect = {0, 0, p->osd_cfg[id].width, p->osd_cfg[id].height};
    im_rect src_rect = {p->osd_cfg[id].start_x, p->osd_cfg[id].start_y, p->osd_cfg[id].width, p->osd_cfg[id].height};
    STATUS = improcess(src,  src,  pat, src_rect, src_rect, pat_rect, IM_ALPHA_BLEND_DST_OVER);
    //STATUS = imblend(src,  pat, IM_ALPHA_BLEND_DST_OVER, 1);
    //STATUS = imcomposite(src, src, pat, IM_ALPHA_BLEND_DST_OVER, 1);
}
#endif
