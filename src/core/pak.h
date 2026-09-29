/*

 * These codes were written by AI (Doubao)

*/

// ============================================================================

/* ============================================================================
 * pak.h — 极简 .pak 归档库（单头文件 / header-only，C11）
 * ----------------------------------------------------------------------------
 * 特点
 *   - 打包方：    #include "pak.h"  → 调用 pak_pack()
 *   - 解包方：    #include "pak.h"  → 调用 pak_open() / pak_extract_all() 等，
 *                 全部 API 均为 static inline，引入本头文件即可，无需额外
 *                 链接任何 .c / .lib，不会产生重复符号（多个 .c 同时 include 也安全）。
 *   - 纯 C11、无第三方依赖，MinGW / MSVC / GCC / Clang 均可编译。
 *   - 小端字节序，结构字段一律通过显式读写函数编解码，不做 packed struct，
 *     避免未对齐访问与编译器相关的内存布局差异。
 *   - 每个条目带 CRC32 校验，解包时逐字节核对，防静默损坏。
 *   - 解包内置路径穿越防护（zip-slip 防护）：拒绝绝对路径、盘符、".." 段。
 *
 * 文件格式（小端，4 字节对齐的数据区）
 *   +0   4B  魔数 "PAK" + 0x01（格式版本）
 *   +4   4B  uint32 条目数 count
 *   +8   4B  uint32 目录区字节数 dir_size（不含 16 字节头）
 *   +12  4B  uint32 数据区起始偏移 data_offset = align4(16 + dir_size)
 *   +16  ... 目录区：count 个条目，条目布局见下
 *   +data_offset ... 数据区：各文件数据按条目顺序依次存放
 *
 *   目录条目（共 12 + name_len 字节）
 *   +0   4B  uint32 name_len（UTF-8 字节数，不含结尾 '\0'）
 *   +4   ...  name_len 字节的文件名 / 包内路径（UTF-8）
 *   +?   4B  uint32 数据偏移（相对文件头，必须 >= data_offset）
 *   +?   4B  uint32 数据长度（字节）
 *   +?   4B  uint32 CRC32（标准 IEEE 802.3，初值/终值 0xFFFFFFFF，多项式 0xEDB88320）
 *
 * 限制
 *   - 单文件与单包体积上限 4 GiB（字段为 uint32；32 位 long 平台上受 2 GiB 限制）。
 *   - 文件名长度上限 1 MiB，条目数上限 100 万。
 * ========================================================================== */
#ifndef PAK_H_INCLUDED
#define PAK_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
  #include <direct.h>     /* _mkdir */
  #include <windows.h>    /* CreateFileMapping / MapViewOfFile */
  #define PAK_MKDIR(p) _mkdir(p)
#else
  #include <sys/stat.h>
  #include <sys/mman.h>   /* mmap / munmap */
  #include <fcntl.h>      /* open */
  #include <unistd.h>     /* close / fstat */
  #define PAK_MKDIR(p) mkdir(p, 0755)
#endif

#if defined(_MSC_VER)
  #define PAK_INLINE static __inline
#else
  #define PAK_INLINE static inline
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * 格式常量
 * ------------------------------------------------------------------------ */
#define PAK_MAGIC0   'P'
#define PAK_MAGIC1   'A'
#define PAK_MAGIC2   'K'
#define PAK_MAGIC3   0x01        /* 格式版本 1 */

#define PAK_MAX_ENTRIES  (1000000u)
#define PAK_MAX_NAME_LEN (1u << 20)  /* 1 MiB */
#define PAK_IO_BUF       (1u << 16)  /* 64 KiB 流式 IO 缓冲 */

/* ---------------------------------------------------------------------------
 * 错误码
 * ------------------------------------------------------------------------ */
typedef enum {
    PAK_OK = 0,
    PAK_ERR_ARG,        /* 参数非法 */
    PAK_ERR_OPEN,       /* 打开/创建文件失败 */
    PAK_ERR_READ,       /* 读取失败 */
    PAK_ERR_WRITE,      /* 写入失败 */
    PAK_ERR_MAGIC,      /* 魔数不对，不是 pak 文件 */
    PAK_ERR_VERSION,    /* 格式版本不支持 */
    PAK_ERR_TRUNCATED,  /* 文件被截断或目录自相矛盾 */
    PAK_ERR_TOO_BIG,    /* 超过 4 GiB 限制 */
    PAK_ERR_NAME,       /* 文件名非法（不安全或过长） */
    PAK_ERR_NOTFOUND,   /* 条目不存在 */
    PAK_ERR_CRC,        /* CRC32 校验失败 */
    PAK_ERR_MEMORY      /* 内存不足 */
} pak_err_t;

/* 目录条目（pak_open 后由库填充，name 以 '\0' 结尾） */
typedef struct {
    uint32_t name_len;   /* 名字字节数，不含 '\0' */
    char    *name;       /* 包内路径，UTF-8 */
    uint32_t offset;     /* 数据在文件中的绝对偏移 */
    uint32_t size;       /* 数据字节数 */
    uint32_t crc32;      /* 数据 CRC32 */
} pak_entry_t;

/* 已打开的归档句柄（支持两种后端：流式 fread / 系统级内存映射 mmap） */
typedef struct {
    FILE        *f;          /* 流模式句柄；mmap 模式下为 NULL */
    uint32_t     count;
    uint32_t     data_offset;
    pak_entry_t *entries;
    /* mmap 模式字段（pak_open_mm 使用） */
    int                   mmap_mode;   /* 1 = 内存映射后端 */
    const unsigned char  *map_base;    /* 映射基址 */
    size_t                map_size;
#ifdef _WIN32
    void *map_handle_file;             /* HANDLE: CreateFile */
    void *map_handle_view;             /* HANDLE: CreateFileMapping */
#else
    int   map_fd;                      /* open() 的文件描述符 */
#endif
} pak_t;

/* 打包源文件描述：disk_path 为磁盘真实路径，archive_name 为包内路径
 * （为 NULL 时自动取 disk_path 的文件名 basename） */
typedef struct {
    const char *disk_path;
    const char *archive_name;
} pak_src_t;

/* ---------------------------------------------------------------------------
 * 内部工具（static inline，多 TU 引入各自实例化，无链接冲突）
 * ------------------------------------------------------------------------ */

/* 显式读写小端 uint32 —— 不做 packed struct，天然避免未对齐访问与 ABI 差异 */
PAK_INLINE uint32_t pak_get_u32(const unsigned char *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

PAK_INLINE void pak_put_u32(unsigned char *p, uint32_t v)
{
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
}

PAK_INLINE uint32_t pak_align4(uint32_t v) { return (v + 3u) & ~3u; }

/* 64 位定位原语。
 * Windows 用 _fseeki64/_ftelli64（无需特性宏，MinGW/MSVC 均提供）；
 * POSIX 用 fseek/ftell：x86_64 下 long 本身 64 位，覆盖 4 GiB 上限；
 * 32 位 POSIX 平台受 2 GiB 限制（可用 -D_FILE_OFFSET_BITS=64 解除）。 */
#ifdef _WIN32
  #define PAK_SEEK(f,o)     _fseeki64((f),(__int64)(o),SEEK_SET)
  #define PAK_SEEK_END(f)   _fseeki64((f),0,SEEK_END)
  #define PAK_TELL(f)       _ftelli64(f)
#else
  #define PAK_SEEK(f,o)     fseek((f),(long)(o),SEEK_SET)
  #define PAK_SEEK_END(f)   fseek((f),0,SEEK_END)
  #define PAK_TELL(f)       ftell(f)
#endif

/* CRC32（IEEE 802.3）：初值 0xFFFFFFFF，结束时异或 0xFFFFFFFF。
 * 查表法，表用 volatile 双重检查一次性构建。
 * 线程说明：多个线程并发触发首次构建时会各自构建一次，写入内容相同，
 * 最终结果确定；若需要严格的一次初始化语义，请在首次调用前自行加锁。 */
static uint32_t      pak_crc_table[256];
static volatile int  pak_crc_ready = 0;

PAK_INLINE void pak_crc_build_table(void)
{
    int i, j;
    if (pak_crc_ready)
        return;
    for (i = 0; i < 256; ++i) {
        uint32_t c = (uint32_t)i;
        for (j = 0; j < 8; ++j)
            c = (c & 1u) ? (c >> 1) ^ 0xEDB88320u : (c >> 1);
        pak_crc_table[i] = c;
    }
    pak_crc_ready = 1;
}

/* 增量 CRC：crc 为上一次返回值，首次调用传 0xFFFFFFFFu */
PAK_INLINE uint32_t pak_crc_update(uint32_t crc, const void *data, size_t len)
{
    const unsigned char *p = (const unsigned char *)data;
    size_t i;
    pak_crc_build_table();
    for (i = 0; i < len; ++i)
        crc = pak_crc_table[(crc ^ p[i]) & 0xFFu] ^ (crc >> 8);
    return crc;
}

/* 取路径的文件名部分（兼容 '/' 与 '\\'） */
PAK_INLINE const char *pak_basename(const char *path)
{
    const char *s, *last = path;
    for (s = path; *s; ++s)
        if (*s == '/' || *s == '\\')
            last = s + 1;
    return last;
}

/* 包内路径安全检查（zip-slip 防护）：
 *   拒绝绝对路径、盘符、"." 与 ".." 段；空名/以分隔符结尾也拒绝。
 * 打包与解包都经过此检查。 */
PAK_INLINE int pak_is_safe_name(const char *name)
{
    const char *seg_start, *p;
    if (!name || !*name)
        return 0;
    for (seg_start = name, p = name; ; ++p) {
        if (*p == '/' || *p == '\\' || *p == '\0') {
            size_t seg_len = (size_t)(p - seg_start);
            if (p == name) return 0;                        /* 以分隔符开头 */
            if (seg_len == 1 && seg_start[0] == '.')  return 0;  /* "." */
            if (seg_len == 2 && seg_start[0] == '.' && seg_start[1] == '.')
                return 0;                                   /* ".." */
            if (*p == '\0') {
                if (p == seg_start) return 0;               /* 以分隔符结尾 */
                break;
            }
            seg_start = p + 1;
        } else if (*p == ':') {
            return 0;                                       /* 盘符 */
        }
    }
    return 1;
}

/* 字符串小工具 */
PAK_INLINE char *pak_strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *d = (char *)malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

/* 拼接两个路径段（自动补分隔符） */
PAK_INLINE char *pak_join(const char *a, const char *b)
{
    size_t la = strlen(a), lb = strlen(b);
    char *s = (char *)malloc(la + lb + 2);
    if (!s) return NULL;
    memcpy(s, a, la);
    if (la && s[la-1] != '/' && s[la-1] != '\\')
        s[la++] = '/';
    memcpy(s + la, b, lb);
    s[la + lb] = '\0';
    return s;
}

/* 取目录部分（无分隔符时返回 "."） */
PAK_INLINE char *pak_dirname(const char *path)
{
    const char *p, *slash = NULL;
    char *s;
    for (p = path; *p; ++p)
        if (*p == '/' || *p == '\\')
            slash = p;
    if (!slash)
        return pak_strdup(".");
    s = (char *)malloc((size_t)(slash - path) + 1);
    if (!s) return NULL;
    memcpy(s, path, (size_t)(slash - path));
    s[slash - path] = '\0';
    return s;
}

PAK_INLINE pak_err_t pak_mkdir_one(const char *path)
{
    if (PAK_MKDIR(path) == 0)
        return PAK_OK;
    return (errno == EEXIST) ? PAK_OK : PAK_ERR_OPEN;
}

/* 递归创建目录；跳过盘符根与连续分隔符产生的空段 */
PAK_INLINE pak_err_t pak_mkdirs(const char *path)
{
    char *tmp, *p;
    pak_err_t err = PAK_OK;
    if (!path) return PAK_ERR_ARG;
    tmp = pak_strdup(path);
    if (!tmp) return PAK_ERR_MEMORY;
    for (p = tmp + 1; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            char save = *p;
            int drive_root, ends_sep;
            *p = '\0';
            drive_root = (tmp[0] && tmp[1] == ':' && tmp[2] == '\0');
            ends_sep    = (p > tmp && (p[-1] == '/' || p[-1] == '\\'));
            if (tmp[0] && !drive_root && !ends_sep) {
                err = pak_mkdir_one(tmp);
                if (err != PAK_OK) { free(tmp); return err; }
            }
            *p = save;
        }
    }
    if (tmp[0] && !(tmp[1] == ':' && tmp[2] == '\0')) {
        err = pak_mkdir_one(tmp);
    }
    free(tmp);
    return err;
}

/* ---------------------------------------------------------------------------
 * 错误信息
 * ------------------------------------------------------------------------ */
PAK_INLINE const char *pak_strerror(pak_err_t e)
{
    switch (e) {
    case PAK_OK:         return "ok";
    case PAK_ERR_ARG:    return "invalid argument";
    case PAK_ERR_OPEN:   return "cannot open file";
    case PAK_ERR_READ:   return "read error / unexpected eof";
    case PAK_ERR_WRITE:  return "write error";
    case PAK_ERR_MAGIC:  return "not a pak file (bad magic)";
    case PAK_ERR_VERSION:return "unsupported pak version";
    case PAK_ERR_TRUNCATED: return "corrupted or truncated archive";
    case PAK_ERR_TOO_BIG:return "file or archive exceeds 4 GiB limit";
    case PAK_ERR_NAME:   return "invalid or unsafe entry name";
    case PAK_ERR_NOTFOUND:return "entry not found";
    case PAK_ERR_CRC:    return "crc32 mismatch";
    case PAK_ERR_MEMORY: return "out of memory";
    default:             return "unknown error";
    }
}

/* ---------------------------------------------------------------------------
 * 打包：把 files[0..n) 打成 out_path 指定的 .pak 文件。
 * archive_name 为 NULL 时使用 disk_path 的 basename。
 * 失败时清理已写出的不完整文件，并返回错误码。
 * ------------------------------------------------------------------------ */
PAK_INLINE pak_err_t pak_pack(const char *out_path, const pak_src_t *files, int n)
{
    typedef struct {
        char    *name;
        uint32_t name_len;
        uint32_t size;
        uint32_t crc_pos;       /* 输出文件中该条目 crc 字段的偏移，用于回填 */
    } item_t;
    item_t   *items = NULL;
    FILE     *out = NULL;
    uint64_t  dir_size = 0, data_offset;
    uint32_t  data_cursor = 0;
    int       out_created = 0;
    pak_err_t err = PAK_OK;
    int       i;

    if (!out_path || !files || n <= 0)
        return PAK_ERR_ARG;

    /* ---- 阶段 1：收集元数据（名字、大小），不碰输出文件 ---- */
    items = (item_t *)calloc((size_t)n, sizeof(item_t));
    if (!items) { err = PAK_ERR_MEMORY; goto fail; }

    for (i = 0; i < n; ++i) {
        FILE   *in;
        long    sz;
        const char *disk = files[i].disk_path;
        const char *base = (files[i].archive_name && files[i].archive_name[0])
                           ? files[i].archive_name : pak_basename(disk);

        if (!disk || !*disk) { err = PAK_ERR_ARG; goto fail; }
        in = fopen(disk, "rb");
        if (!in) { err = PAK_ERR_OPEN; goto fail; }
        if (fseek(in, 0, SEEK_END) != 0 || (sz = ftell(in)) < 0) {
            fclose(in); err = PAK_ERR_READ; goto fail;
        }
        fclose(in);

        if ((uint64_t)sz > 0xFFFFFFFFull) { err = PAK_ERR_TOO_BIG; goto fail; }
        if (!pak_is_safe_name(base))       { err = PAK_ERR_NAME;  goto fail; }

        items[i].size = (uint32_t)sz;
        items[i].name_len = (uint32_t)strlen(base);
        items[i].name = (char *)malloc((size_t)items[i].name_len + 1);
        if (!items[i].name) { err = PAK_ERR_MEMORY; goto fail; }
        memcpy(items[i].name, base, (size_t)items[i].name_len + 1);

        dir_size += (uint64_t)items[i].name_len + 16u;  /* 4(name_len) + name + 4*3(offset/size/crc) */
        if (dir_size > 0xFFFFFFFFull) { err = PAK_ERR_TOO_BIG; goto fail; }
    }

    /* ---- 阶段 2：写入 ---- */
    data_offset = pak_align4(16u + (uint32_t)dir_size);
    if (data_offset > 0xFFFFFFFFu) { err = PAK_ERR_TOO_BIG; goto fail; }

    out = fopen(out_path, "wb");
    if (!out) { err = PAK_ERR_OPEN; goto fail; }
    out_created = 1;

    /* 2.1 预留 16 字节头，末尾回填 */
    {
        unsigned char hdr[16] = {0};
        if (fwrite(hdr, 1, sizeof(hdr), out) != sizeof(hdr)) { err = PAK_ERR_WRITE; goto fail; }
    }

    /* 2.2 目录区：offset 此时已可确定（数据区游标），crc 先占位 */
    for (i = 0; i < n; ++i) {
        unsigned char le[4];
        pak_put_u32(le, items[i].name_len);
        if (fwrite(le, 1, 4, out) != 4) { err = PAK_ERR_WRITE; goto fail; }
        if (items[i].name_len &&
            fwrite(items[i].name, 1, items[i].name_len, out) != items[i].name_len) {
            err = PAK_ERR_WRITE; goto fail;
        }
        if ((uint64_t)data_offset + data_cursor > 0xFFFFFFFFu) { err = PAK_ERR_TOO_BIG; goto fail; }
        pak_put_u32(le, (uint32_t)data_offset + data_cursor);  /* 绝对偏移（相对文件头） */
        if (fwrite(le, 1, 4, out) != 4) { err = PAK_ERR_WRITE; goto fail; }
        pak_put_u32(le, items[i].size);
        if (fwrite(le, 1, 4, out) != 4) { err = PAK_ERR_WRITE; goto fail; }
        pak_put_u32(le, 0u);   /* crc 占位 */
        if (fwrite(le, 1, 4, out) != 4) { err = PAK_ERR_WRITE; goto fail; }
        items[i].crc_pos = (uint32_t)PAK_TELL(out) - 4u;

        data_cursor += items[i].size;
        if (data_cursor < items[i].size) { err = PAK_ERR_TOO_BIG; goto fail; } /* 溢出 */
    }

    /* 2.3 数据区 4 字节对齐填充 */
    {
        unsigned char zeros[64] = {0};
        uint32_t pad = (uint32_t)(data_offset - 16u - dir_size);
        uint32_t left = pad;
        while (left) {
            size_t w = (left < sizeof(zeros)) ? left : sizeof(zeros);
            if (fwrite(zeros, 1, w, out) != w) { err = PAK_ERR_WRITE; goto fail; }
            left -= (uint32_t)w;
        }
    }

    /* 2.4 数据区：边写边算 CRC，之后回填各条目 crc 字段 */
    for (i = 0; i < n; ++i) {
        FILE *in = fopen(files[i].disk_path, "rb");
        unsigned char buf[PAK_IO_BUF];
        uint32_t crc = 0xFFFFFFFFu;
        uint64_t remain = items[i].size;
        if (!in) { err = PAK_ERR_OPEN; goto fail; }
        while (remain) {
            size_t want = (remain < sizeof(buf)) ? (size_t)remain : sizeof(buf);
            size_t got = fread(buf, 1, want, in);
            if (got == 0) { fclose(in); err = PAK_ERR_READ; goto fail; }
            crc = pak_crc_update(crc, buf, got);
            if (fwrite(buf, 1, got, out) != got) { fclose(in); err = PAK_ERR_WRITE; goto fail; }
            remain -= got;
        }
        fclose(in);
        if (PAK_SEEK(out, items[i].crc_pos) != 0) { err = PAK_ERR_WRITE; goto fail; }
        {
            unsigned char le[4];
            pak_put_u32(le, crc ^ 0xFFFFFFFFu);
            if (fwrite(le, 1, 4, out) != 4) { err = PAK_ERR_WRITE; goto fail; }
        }
        if (PAK_SEEK_END(out) != 0) { err = PAK_ERR_WRITE; goto fail; }  /* 回到文件尾继续追加 */
    }

    /* 2.5 回填文件头 */
    if (PAK_SEEK(out, 0) != 0) { err = PAK_ERR_WRITE; goto fail; }
    {
        unsigned char hdr[16];
        hdr[0] = PAK_MAGIC0; hdr[1] = PAK_MAGIC1;
        hdr[2] = PAK_MAGIC2; hdr[3] = PAK_MAGIC3;
        pak_put_u32(hdr + 4,  (uint32_t)n);
        pak_put_u32(hdr + 8,  (uint32_t)dir_size);
        pak_put_u32(hdr + 12, (uint32_t)data_offset);
        if (fwrite(hdr, 1, 16, out) != 16) { err = PAK_ERR_WRITE; goto fail; }
    }
    if (fclose(out) != 0) { out = NULL; err = PAK_ERR_WRITE; goto fail; }
    out = NULL;

    for (i = 0; i < n; ++i) free(items[i].name);
    free(items);
    return PAK_OK;

fail:
    if (out) fclose(out);
    if (items) {
        for (i = 0; i < n; ++i) free(items[i].name);
        free(items);
    }
    if (out_created) remove(out_path);   /* 只清理本次创建的不完整输出 */
    return err;
}

/* ---------------------------------------------------------------------------
 * 打开归档。校验魔数、版本与目录自洽性，读入全部目录项。
 * 成功返回非 NULL；失败返回 NULL 并通过 err 给出原因（err 可为 NULL）。
 * ------------------------------------------------------------------------ */
PAK_INLINE void pak_close(pak_t *p);   /* 前向声明（解析失败清理时用到） */

/* 从内存块解析 pak 头 + 目录。
 *   mem        头+目录所在内存（mmap 为整个文件，流式为 16+dir_size 的堆块）
 *   mem_size   mem 字节数（仅需覆盖头+目录）
 *   file_size  整个归档文件字节数（校验数据区越界用；mmap 下等于 mem_size）
 * 条目 name 会拷贝到堆（与映射生命周期解耦）；数据本身不解引用。 */
PAK_INLINE pak_t *pak_parse(const unsigned char *mem, size_t mem_size,
                            uint64_t file_size, pak_err_t *err)
{
    pak_t *p = NULL;
    uint32_t count, dir_size, data_offset, dir_pos, i;
    pak_err_t e = PAK_OK;

    if (err) *err = PAK_OK;
    if (!mem || mem_size < 16u) { if (err) *err = PAK_ERR_TRUNCATED; return NULL; }

    if (mem[0] != PAK_MAGIC0 || mem[1] != PAK_MAGIC1 || mem[2] != PAK_MAGIC2) {
        e = PAK_ERR_MAGIC; goto fail;
    }
    if (mem[3] != PAK_MAGIC3) { e = PAK_ERR_VERSION; goto fail; }
    count       = pak_get_u32(mem + 4);
    dir_size    = pak_get_u32(mem + 8);
    data_offset = pak_get_u32(mem + 12);

    /* 目录自洽性校验：全部用 64 位中间量，防 uint32 溢出被绕过 */
    if (count > PAK_MAX_ENTRIES)                           { e = PAK_ERR_TRUNCATED; goto fail; }
    if (data_offset < 16u)                                 { e = PAK_ERR_TRUNCATED; goto fail; }
    if ((uint64_t)dir_size > (uint64_t)data_offset - 16u)  { e = PAK_ERR_TRUNCATED; goto fail; }
    if (data_offset > file_size)                           { e = PAK_ERR_TRUNCATED; goto fail; }
    if (count && (uint64_t)dir_size < (uint64_t)count * 16u){ e = PAK_ERR_TRUNCATED; goto fail; }

    p = (pak_t *)calloc(1, sizeof(pak_t));
    if (!p) { e = PAK_ERR_MEMORY; goto fail; }
    p->count = count;
    p->data_offset = data_offset;
    if (count == 0) return p;   /* 空包合法 */

    p->entries = (pak_entry_t *)calloc((size_t)count, sizeof(pak_entry_t));
    if (!p->entries) { e = PAK_ERR_MEMORY; goto fail; }

    dir_pos = 16;
    for (i = 0; i < count; ++i) {
        pak_entry_t *ent = &p->entries[i];
        uint64_t ent_size;
        uint32_t off, sz;

        if ((size_t)dir_pos + 4u > mem_size) { e = PAK_ERR_TRUNCATED; goto fail; }
        ent->name_len = pak_get_u32(mem + dir_pos);
        if (ent->name_len > PAK_MAX_NAME_LEN) { e = PAK_ERR_NAME; goto fail; }

        ent_size = 4u + (uint64_t)ent->name_len + 12u;
        if ((uint64_t)dir_pos + ent_size > 16u + (uint64_t)dir_size) {
            e = PAK_ERR_TRUNCATED; goto fail;
        }
        if ((size_t)dir_pos + ent_size > mem_size) { e = PAK_ERR_TRUNCATED; goto fail; }

        ent->name = (char *)malloc((size_t)ent->name_len + 1);
        if (!ent->name) { e = PAK_ERR_MEMORY; goto fail; }
        memcpy(ent->name, mem + dir_pos + 4, ent->name_len);
        ent->name[ent->name_len] = '\0';

        off = pak_get_u32(mem + dir_pos + 4 + ent->name_len);
        sz  = pak_get_u32(mem + dir_pos + 8 + ent->name_len);
        ent->crc32 = pak_get_u32(mem + dir_pos + 12 + ent->name_len);

        /* 数据必须位于数据区内且不越过文件尾 */
        if (off < data_offset)                           { e = PAK_ERR_TRUNCATED; goto fail; }
        if ((uint64_t)off + sz > file_size)              { e = PAK_ERR_TRUNCATED; goto fail; }
        ent->offset = off;
        ent->size   = sz;

        dir_pos = (uint32_t)((uint64_t)dir_pos + ent_size);
    }
    return p;

fail:
    pak_close(p);
    if (err) *err = e;
    return NULL;
}

/* 流式打开：只把 16 字节头 + 目录读入内存解析，数据仍按需 fseek/fread。
 * 适合偶尔使用、不想长占地址空间的场景。 */
PAK_INLINE pak_t *pak_open(const char *path, pak_err_t *err)
{
    pak_t *p = NULL;
    FILE *f = NULL;
    unsigned char hdr[16];
    unsigned char *block = NULL;
    uint32_t dir_size;
    uint64_t file_size;
    pak_err_t e = PAK_OK;

    if (err) *err = PAK_OK;
    if (!path) { if (err) *err = PAK_ERR_ARG; return NULL; }

    f = fopen(path, "rb");
    if (!f) { e = PAK_ERR_OPEN; goto fail; }

    if (fread(hdr, 1, 16, f) != 16) { e = PAK_ERR_TRUNCATED; goto fail; }
    if (hdr[0] != PAK_MAGIC0 || hdr[1] != PAK_MAGIC1 || hdr[2] != PAK_MAGIC2) {
        e = PAK_ERR_MAGIC; goto fail;
    }
    if (hdr[3] != PAK_MAGIC3) { e = PAK_ERR_VERSION; goto fail; }
    dir_size = pak_get_u32(hdr + 8);

    /* 求文件大小 */
    if (PAK_SEEK_END(f) != 0) { e = PAK_ERR_READ; goto fail; }
    {
        long long sz = PAK_TELL(f);
        if (sz < 0) { e = PAK_ERR_READ; goto fail; }
        file_size = (uint64_t)sz;
    }

    /* 头 + 目录读入堆，交给共享解析 */
    if ((uint64_t)dir_size + 16u > file_size) { e = PAK_ERR_TRUNCATED; goto fail; }
    block = (unsigned char *)malloc((size_t)dir_size + 16u);
    if (!block) { e = PAK_ERR_MEMORY; goto fail; }
    memcpy(block, hdr, 16);
    if (dir_size) {
        if (PAK_SEEK(f, 16) != 0) { e = PAK_ERR_READ; goto fail; }
        if (fread(block + 16, 1, dir_size, f) != dir_size) { e = PAK_ERR_TRUNCATED; goto fail; }
    }

    p = pak_parse(block, (size_t)dir_size + 16u, file_size, &e);
    if (!p) goto fail;
    p->f = f;
    f = NULL;

    free(block);
    return p;

fail:
    if (block) free(block);
    if (f) fclose(f);
    if (err) *err = e;
    return NULL;
}

/* ---------------------------------------------------------------------------
 * mmap 打开：用系统级内存映射（POSIX mmap / Windows MapViewOfFile）把整个
 * pak 文件映射进进程地址空间，目录从映射内存解析。
 * 之后 pak_data() 直接返回映射内的数据指针——零拷贝、零系统调用，
 * 适合运行时高频资源加载（纹理/音频/关卡数据）。
 * 映射在 pak_close() 时解除；其间返回的所有指针保持有效。
 * ------------------------------------------------------------------------ */
PAK_INLINE pak_t *pak_open_mm(const char *path, pak_err_t *err)
{
    pak_t *p = NULL;
    pak_err_t e = PAK_OK;

    if (err) *err = PAK_OK;
    if (!path) { if (err) *err = PAK_ERR_ARG; return NULL; }

#ifdef _WIN32
    {
        HANDLE hFile, hMap;
        LARGE_INTEGER sz;
        const unsigned char *base;
        hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) { e = PAK_ERR_OPEN; goto fail; }
        if (!GetFileSizeEx(hFile, &sz) || sz.QuadPart <= 0) {
            CloseHandle(hFile); e = PAK_ERR_TRUNCATED; goto fail;
        }
        hMap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if (!hMap) { CloseHandle(hFile); e = PAK_ERR_OPEN; goto fail; }
        base = (const unsigned char *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
        if (!base) { CloseHandle(hMap); CloseHandle(hFile); e = PAK_ERR_OPEN; goto fail; }

        p = pak_parse(base, (size_t)sz.QuadPart, (uint64_t)sz.QuadPart, &e);
        if (!p) { UnmapViewOfFile(base); CloseHandle(hMap); CloseHandle(hFile); goto fail; }
        p->mmap_mode = 1;
        p->map_base = base;
        p->map_size = (size_t)sz.QuadPart;
        p->map_handle_file = hFile;
        p->map_handle_view = hMap;
        return p;
    }
#else
    {
        int fd = open(path, O_RDONLY);
        struct stat st;
        const unsigned char *base;
        if (fd < 0) { e = PAK_ERR_OPEN; goto fail; }
        if (fstat(fd, &st) != 0 || st.st_size <= 0) { close(fd); e = PAK_ERR_TRUNCATED; goto fail; }
        base = (const unsigned char *)mmap(NULL, (size_t)st.st_size, PROT_READ,
                                           MAP_PRIVATE, fd, 0);
        if (base == MAP_FAILED) { close(fd); e = PAK_ERR_OPEN; goto fail; }

        p = pak_parse(base, (size_t)st.st_size, (uint64_t)st.st_size, &e);
        if (!p) { munmap((void *)base, (size_t)st.st_size); close(fd); goto fail; }
        p->mmap_mode = 1;
        p->map_base = base;
        p->map_size = (size_t)st.st_size;
        p->map_fd = fd;
        return p;
    }
#endif

fail:
    if (err) *err = e;
    return NULL;
}

/* mmap 模式：直接返回条目数据在映射内的指针（零拷贝，不校验 CRC）。
 * 流模式返回 NULL。指针在 pak_close 前始终有效。 */
PAK_INLINE const void *pak_data(const pak_t *p, const pak_entry_t *e,
                                uint32_t *out_size)
{
    if (!p || !e) return NULL;
    if (out_size) *out_size = 0;
    if (!p->mmap_mode || !p->map_base) return NULL;
    if (out_size) *out_size = e->size;
    return p->map_base + e->offset;
}

/* 关闭归档，释放全部资源（NULL 安全）。mmap 模式在此解除映射并释放句柄。 */
PAK_INLINE void pak_close(pak_t *p)
{
    uint32_t i;
    if (!p) return;
    if (p->entries) {
        for (i = 0; i < p->count; ++i) free(p->entries[i].name);
        free(p->entries);
    }
    if (p->mmap_mode) {
#ifdef _WIN32
        if (p->map_base)         UnmapViewOfFile((LPCVOID)p->map_base);
        if (p->map_handle_view)  CloseHandle((HANDLE)p->map_handle_view);
        if (p->map_handle_file)  CloseHandle((HANDLE)p->map_handle_file);
#else
        if (p->map_base)         munmap((void *)p->map_base, p->map_size);
        if (p->map_fd >= 0)      close(p->map_fd);
#endif
    }
    if (p->f) fclose(p->f);
    free(p);
}

PAK_INLINE uint32_t pak_count(const pak_t *p) { return p ? p->count : 0; }

PAK_INLINE const pak_entry_t *pak_entry(const pak_t *p, uint32_t i)
{
    if (!p || i >= p->count) return NULL;
    return &p->entries[i];
}

/* 按名字查找，返回第一个匹配条目（O(n) 线性扫描，适合小型目录） */
PAK_INLINE const pak_entry_t *pak_find(const pak_t *p, const char *name)
{
    uint32_t i;
    if (!p || !name) return NULL;
    for (i = 0; i < p->count; ++i)
        if (strcmp(p->entries[i].name, name) == 0)
            return &p->entries[i];
    return NULL;
}

/* 读取条目数据的一个窗口（不校验 CRC，适合按需读取） */
PAK_INLINE pak_err_t pak_read(const pak_t *p, const pak_entry_t *e,
                              void *buf, uint32_t off, uint32_t len)
{
    if (!p || !e || !buf) return PAK_ERR_ARG;
    if ((uint64_t)off + len > (uint64_t)e->size) return PAK_ERR_ARG;
    if (len == 0) return PAK_OK;
    if (p->mmap_mode) {
        memcpy(buf, p->map_base + e->offset + off, len);   /* 映射内 memcpy，零系统调用 */
        return PAK_OK;
    }
    if (PAK_SEEK(p->f, (int64_t)e->offset + off) != 0) return PAK_ERR_READ;
    return (fread(buf, 1, len, p->f) == len) ? PAK_OK : PAK_ERR_READ;
}

/* ---------------------------------------------------------------------------
 * 内存加载：把整个条目一次读入堆内存，返回 malloc 分配的缓冲，调用方用
 * free() 释放。读取全程逐块校验 CRC，损坏返回 PAK_ERR_CRC 且不产出缓冲。
 * 这是"运行时直接使用"的核心接口——不需要解包落盘：
 *
 *     void *data; uint32_t n;
 *     if (pak_load_name(p, "textures/tex.png", &data, &n) == PAK_OK) {
 *         ... 直接使用 data[0..n)，用完 free(data) ...
 *     }
 * 空文件（size==0）会返回 1 字节占位缓冲，size 为 0，free 依然安全。
 * 需要跳过 CRC 校验的场合（性能敏感且信任来源）用 pak_read 分块自读。
 * ------------------------------------------------------------------------ */
PAK_INLINE pak_err_t pak_load(const pak_t *p, const pak_entry_t *e,
                              void **out_buf, uint32_t *out_size)
{
    unsigned char *buf;
    uint32_t crc = 0xFFFFFFFFu;
    pak_err_t err = PAK_OK;

    if (!p || !e || !out_buf) return PAK_ERR_ARG;
    *out_buf = NULL;
    if (out_size) *out_size = 0;
    if (!p->f && !p->mmap_mode) return PAK_ERR_OPEN;

    buf = (unsigned char *)malloc(e->size ? e->size : 1u);
    if (!buf) return PAK_ERR_MEMORY;

    if (p->mmap_mode) {
        /* 映射内整块拷贝 + 一次 CRC 校验 */
        if (e->size) memcpy(buf, p->map_base + e->offset, e->size);
    } else {
        uint32_t off = 0;
        if (e->size && PAK_SEEK(p->f, (int64_t)e->offset) != 0) { err = PAK_ERR_READ; goto done; }
        while (off < e->size) {
            size_t want = (e->size - off < PAK_IO_BUF) ? (e->size - off) : PAK_IO_BUF;
            size_t got = fread(buf + off, 1, want, p->f);
            if (got == 0) { err = PAK_ERR_READ; goto done; }
            crc = pak_crc_update(crc, buf + off, got);
            off += (uint32_t)got;
        }
    }
    if (e->size) crc = pak_crc_update(crc, buf, e->size);
    if (e->crc32 != (crc ^ 0xFFFFFFFFu)) { err = PAK_ERR_CRC; goto done; }

    *out_buf = buf;
    if (out_size) *out_size = e->size;
    return PAK_OK;

done:
    free(buf);
    return err;
}

/* 按名字加载到内存（pak_find + pak_load 一步到位） */
PAK_INLINE pak_err_t pak_load_name(const pak_t *p, const char *name,
                                   void **out_buf, uint32_t *out_size)
{
    const pak_entry_t *e = pak_find(p, name);
    if (!e) return PAK_ERR_NOTFOUND;
    return pak_load(p, e, out_buf, out_size);
}

/* 解包单个条目到磁盘文件：流式复制并核对 CRC，校验失败删除半成品 */
PAK_INLINE pak_err_t pak_extract_one(const pak_t *p, const pak_entry_t *e,
                                     const char *out_path)
{
    FILE *out = NULL;
    unsigned char *buf = NULL;
    uint64_t remain;
    uint32_t crc = 0xFFFFFFFFu;
    pak_err_t err = PAK_OK;

    if (!p || !e || !out_path) return PAK_ERR_ARG;
    if (!p->f && !p->mmap_mode) return PAK_ERR_OPEN;

    out = fopen(out_path, "wb");
    if (!out) return PAK_ERR_OPEN;

    if (p->mmap_mode) {
        const unsigned char *src = p->map_base + e->offset;
        remain = e->size;
        while (remain) {
            size_t w = (remain < PAK_IO_BUF) ? (size_t)remain : (size_t)PAK_IO_BUF;
            if (fwrite(src + (e->size - remain), 1, w, out) != w) { err = PAK_ERR_WRITE; goto done; }
            remain -= w;
        }
        crc = pak_crc_update(crc, src, e->size);
    } else {
        buf = (unsigned char *)malloc(PAK_IO_BUF);
        if (!buf) { fclose(out); return PAK_ERR_MEMORY; }
        if (PAK_SEEK(p->f, (int64_t)e->offset) != 0) { err = PAK_ERR_READ; goto done; }
        remain = e->size;
        while (remain) {
            size_t want = (remain < PAK_IO_BUF) ? (size_t)remain : (size_t)PAK_IO_BUF;
            size_t got = fread(buf, 1, want, p->f);
            if (got == 0) { err = PAK_ERR_READ; goto done; }
            crc = pak_crc_update(crc, buf, got);
            if (fwrite(buf, 1, got, out) != got) { err = PAK_ERR_WRITE; goto done; }
            remain -= got;
        }
    }
    if (fclose(out) != 0) { out = NULL; err = PAK_ERR_WRITE; goto done; }
    out = NULL;
    if (e->crc32 != (crc ^ 0xFFFFFFFFu)) { err = PAK_ERR_CRC; goto done; }

done:
    if (out) fclose(out);
    if (err != PAK_OK && out_path) remove(out_path);
    if (buf) free(buf);
    return err;
}

/* 解包全部条目到 out_dir：自动递归建目录，逐条 CRC 校验。
 * 任一条失败即中止并返回错误码（前面已写出的文件保留）。 */
PAK_INLINE pak_err_t pak_extract_all(const pak_t *p, const char *out_dir)
{
    uint32_t i;
    if (!p || !out_dir) return PAK_ERR_ARG;
    for (i = 0; i < p->count; ++i) {
        const pak_entry_t *e = &p->entries[i];
        char *full, *dirpart;
        pak_err_t err;

        if (!pak_is_safe_name(e->name)) return PAK_ERR_NAME;  /* 防穿越 */
        full = pak_join(out_dir, e->name);
        if (!full) return PAK_ERR_MEMORY;
        dirpart = pak_dirname(full);
        if (!dirpart) { free(full); return PAK_ERR_MEMORY; }
        err = pak_mkdirs(dirpart);
        free(dirpart);
        if (err == PAK_OK) err = pak_extract_one(p, e, full);
        free(full);
        if (err != PAK_OK) return err;
    }
    return PAK_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* PAK_H_INCLUDED */
