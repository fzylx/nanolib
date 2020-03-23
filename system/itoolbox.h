//=====================================================================
//
// itoolbox.h - 
//
// Created by skywind on 2019/06/12
// Last Modified: 2019/06/12 20:18:52
//
//=====================================================================
#ifndef _ITOOLBOX_H_
#define _ITOOLBOX_H_

#include "imembase.h"
#include "imemdata.h"
#include "inetbase.h"
#include "inetcode.h"
#include "isecure.h"


#ifdef __cplusplus
extern "C" {
#endif

//=====================================================================
// Posix IPV4/IPV6 Compatible Socket Address
//=====================================================================
typedef union _iPosixAddress {
	struct sockaddr sa;
	struct sockaddr_in sin4;
#ifdef AF_INET6
	struct sockaddr_in6 sin6;
#endif
}	iPosixAddress;


// get family from generic sockaddr
#define iposix_addr_family(pa) ((pa)->sa.sa_family)

#define iposix_addr_v4_ptr(pa) (&((pa)->sin4.sin_addr.s_addr))
#define iposix_addr_v4_vec(type, pa) ((type*)iposix_addr_v4_ptr(pa))
#define iposix_addr_v4_port(pa) ((pa)->sin4.sin_port)
#define iposix_addr_v4_u8(pa) iposix_addr_v4_vec(unsigned char, pa)
#define iposix_addr_v4_cu8(pa) iposix_addr_v4_vec(const unsigned char, pa)

#ifdef AF_INET6
#define iposix_addr_v6_ptr(pa) (((pa)->sin6.sin6_addr.s6_addr))
#define iposix_addr_v6_vec(type, pa) ((type*)iposix_addr_v6_ptr(pa))
#define iposix_addr_v6_port(pa) ((pa)->sin6.sin6_port)
#define iposix_addr_v6_u8(pa) iposix_addr_v6_vec(unsigned char, pa)
#define iposix_addr_v6_cu8(pa) iposix_addr_v6_vec(const unsigned char, pa)

#define iposix_addr_size(pa) \
	((iposix_addr_family(pa) == AF_INET6)? \
	 sizeof(struct sockaddr_in6) : sizeof(struct sockaddr))
#else
#define iposix_addr_size(pa) (sizeof(struct sockaddr))
#endif


void iposix_addr_init(iPosixAddress *addr, int family);
void iposix_addr_set_ip(iPosixAddress *addr, const void *ip);
void iposix_addr_set_port(iPosixAddress *addr, int port);

int iposix_addr_get_family(const iPosixAddress *addr);
int iposix_addr_get_ip(const iPosixAddress *addr, void *ip);
int iposix_addr_get_port(const iPosixAddress *addr);
int iposix_addr_get_size(const iPosixAddress *addr);

int iposix_addr_set_ip_text(iPosixAddress *addr, const char *text);
char *iposix_addr_get_ip_text(const iPosixAddress *addr, char *text);

int iposix_addr_make(iPosixAddress *addr, int family, const char *t, int p);
char *iposix_addr_str(const iPosixAddress *addr, char *text);

int iposix_addr_compare(const iPosixAddress *a1, const iPosixAddress *a2);
int iposix_addr_version(const char *text);


//=====================================================================
// DNS Resolve
//=====================================================================
typedef struct _iPosixRes
{
	int size;
	int *family;
	unsigned char **address;
}	iPosixRes;

// create new iPosixRes
iPosixRes *iposix_res_new(int size);

// remove res
void iposix_res_free(iPosixRes *res);

// omit duplications
void iposix_res_unique(iPosixRes *res);


// ipv = 0/any, 4/ipv4, 6/ipv6
iPosixRes *iposix_res_get(const char *hostname, int ipv);


//=====================================================================
// Protocol Reader
//=====================================================================
struct CAsyncReader;
typedef struct CAsyncReader CAsyncReader;

CAsyncReader *async_reader_new(ib_memnode *fnode);

void async_reader_delete(CAsyncReader *reader);


#define ISTREAM_READ_BYTE		0
#define ISTREAM_READ_LINE		1
#define ISTREAM_READ_BLOCK		2

void async_reader_mode(CAsyncReader *reader, int mode, ilong what);

long async_reader_read(CAsyncReader *reader, void *data, long maxsize);

void async_reader_feed(CAsyncReader *reader, const void *data, long len);

void async_reader_clear(CAsyncReader *reader);


//=====================================================================
// utils
//=====================================================================

int isocket_pair_ex(int *pair);

// can be used to wakeup select
struct CSelectNotify;
typedef struct CSelectNotify CSelectNotify;

CSelectNotify* select_notify_new(void);

void select_notify_delete(CSelectNotify *sn);

int select_notify_wait(CSelectNotify *sn, const int *fds, 
	const int *event, int *revent, int count, long millisec);

int select_notify_wake(CSelectNotify *sn);


//=====================================================================
// Terminal Control
//=====================================================================

// colors
#define CTEXT_BLACK			0
#define CTEXT_RED			1
#define CTEXT_GREEN			2
#define CTEXT_YELLOW		3
#define CTEXT_BLUE			4
#define CTEXT_MAGENTA		5
#define CTEXT_CYAN			6
#define CTEXT_WHITE			7
#define CTEXT_BOLD			8
#define CTEXT_BOLD_RED		9
#define CTEXT_BOLD_GREEN	10
#define CTEXT_BOLD_YELLO	11
#define CTEXT_BOLD_BLUE		12
#define CTEXT_BOLD_MAGENTA	13
#define CTEXT_BOLD_CYAN		14
#define CTEXT_BOLD_WHITE	15

// 背景颜色定义
#define CBG_BLACK			0
#define CBG_RED				(1 << 4)
#define CBG_GREEN			(2 << 4)
#define CBG_YELLO			(3 << 4)
#define CBG_BLUE			(4 << 4)
#define CBG_MAGENTA			(5 << 4)
#define CBG_CYAN			(6 << 4)
#define CBG_WHITE			(7 << 4)


// 设置颜色：低4位是文字颜色，高4位是背景颜色
// 具体编码可以搜索 ansi color或者 
// http://en.wikipedia.org/wiki/ANSI_escape_code
void console_set_color(int color);


// 设置光标位置左上角是，行与列都是从1开始计数的
void console_cursor(int row, int col);

// 恢复屏幕颜色
void console_reset(void);

// 清屏
void console_clear(int color);


//=====================================================================
// utilities
//=====================================================================

// generate a 40 bytes signature
char* hash_signature_md5(
		char *out,             // output string with size above 41 bytes
		const void *in,        // input data
		int size,              // input size
		const char *secret,    // secret token
		int secret_size,       // secret size
		IUINT32 timestamp);    // time stamp in unix epoch seconds

// extract timestamp from signature
IUINT32 hash_signature_time(const char *signature);



#ifdef __cplusplus
}
#endif

#endif


