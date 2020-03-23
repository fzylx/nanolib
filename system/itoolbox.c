#include "itoolbox.h"





//=====================================================================
// Posix IPV4/IPV6 Compatible Socket Address
//=====================================================================

/* setup address */
void iposix_addr_init(iPosixAddress *addr, int family)
{
	if (family == AF_INET) {
		memset(&(addr->sin4), 0, sizeof(struct sockaddr_in));
		addr->sin4.sin_family = family;
	}
#ifdef AF_INET6
	else if (family == AF_INET6) {
		memset(&(addr->sin6), 0, sizeof(struct sockaddr_in6));
		addr->sin6.sin6_family = family;
	}
#endif
	else {
		memset(addr, 0, sizeof(iPosixAddress));
		addr->sin4.sin_family = family;
	}
}


void iposix_addr_set_ip(iPosixAddress *addr, const void *ip)
{
	if (addr->sin4.sin_family == AF_INET) {
		memcpy(iposix_addr_v4_u8(addr), ip, 4);
	}
#ifdef AF_INET6
	else if (addr->sin6.sin6_family == AF_INET6) {
		memcpy(iposix_addr_v6_u8(addr), ip, 4);
	}
#endif
}

void iposix_addr_set_port(iPosixAddress *addr, int port)
{
	if (addr->sin4.sin_family == AF_INET) {
		addr->sin4.sin_port = htons(port);
	}
#ifdef AF_INET6
	else if (addr->sin6.sin6_family == AF_INET6) {
		addr->sin6.sin6_port = htons(port);
	}
#endif
}

int iposix_addr_get_family(const iPosixAddress *addr)
{
	return iposix_addr_family(addr);
}

int iposix_addr_get_ip(const iPosixAddress *addr, void *ip)
{
	int size = 4;
	if (addr->sin4.sin_family == AF_INET) {
		if (ip) {
			memcpy(ip, iposix_addr_v4_cu8(addr), 4);
		}
	}
#ifdef AF_INET6
	else if (addr->sin6.sin6_family == AF_INET6) {
		size = 16;
		if (ip) {
			memcpy(ip, iposix_addr_v6_cu8(addr), 16);
		}
	}
#endif
	return size;
}

int iposix_addr_get_port(const iPosixAddress *addr)
{
	int port = 0;
	if (addr->sin4.sin_family == AF_INET) {
		port = ntohs(addr->sin4.sin_port);
	}
#ifdef AF_INET6
	else if (addr->sin4.sin_family == AF_INET6) {
		port = ntohs(addr->sin6.sin6_port);
	}
#endif
	return port;
}

int iposix_addr_get_size(const iPosixAddress *addr)
{
	return iposix_addr_size(addr);
}

int iposix_addr_set_ip_text(iPosixAddress *addr, const char *text)
{
	if (addr->sin4.sin_family == AF_INET) {
		return isockaddr_set_ip_text(&(addr->sa), text);
	}
#ifdef AF_INET6
	else if (addr->sin6.sin6_family == AF_INET6) {
		int isname = 1, i;
		for (i = 0; text[i]; i++) {
			if (text[i] == ':') {
				isname = 0;
				break;
			}
		}
		if (isname == 0) {
			return isockaddr_pton(AF_INET6, text, &(addr->sin6.sin6_addr));
		}
		else {
			iPosixRes *res = iposix_res_get(text, 6);
			int hr = 0;
			if (res == NULL) return -1;
			if (res->size < 1) {
				hr = -2;
			}	else {
				memcpy(iposix_addr_v6_u8(addr), res->address[0], 16);
			}
			iposix_res_free(res);
			return hr;
		}
	}
#endif
	return -1;
}

char *iposix_addr_get_ip_text(const iPosixAddress *addr, char *text)
{
	if (addr->sin4.sin_family == AF_INET) {
		isockaddr_ntop(AF_INET, &(addr->sin4.sin_addr), text, 32);
	}
#ifdef AF_INET6
	else if (addr->sin6.sin6_family == AF_INET6) {
		isockaddr_ntop(AF_INET6, &(addr->sin6.sin6_addr), text, 256);
	}
#endif
	return text;
}


int iposix_addr_make(iPosixAddress *addr, int family, const char *t, int p)
{
	int af_inet6 = -2;
#ifdef AF_INET6
	af_inet6 = (int)AF_INET6;
#endif
	if (family < 0 || (family != AF_INET && family != af_inet6)) {
		int ipv6 = 0, i;
		for (i = 0; t[i]; i++) {
			if (t[i] == ':') ipv6 = 1;
		}
		family = AF_INET;
		if (ipv6) {
		#ifdef AF_INET6
			family = AF_INET6;
		#else
			return -1;
		#endif
		}
	}
#ifndef AF_INET6
	if (family != AF_INET) {
		if (family != PF_INET) {
			return -1;
		}
	}
#endif
	iposix_addr_init(addr, family);
	iposix_addr_set_ip_text(addr, t);
	iposix_addr_set_port(addr, p);
	return 0;
}

char *iposix_addr_str(const iPosixAddress *addr, char *text)
{
	static char buffer[256 + 10];
	int family = iposix_addr_get_family(addr);
	if (text == NULL) text = buffer;
	if (family == AF_INET) {
		return isockaddr_str(&addr->sa, text);
	}
#ifdef AF_INET6
	else if (family == AF_INET6) {
		char *ptr = text;
		int port;
		*ptr++ = '[';
		iposix_addr_get_ip_text(addr, ptr);
		ptr += (int)strlen(ptr);
		*ptr++ = ']';
		*ptr++ = ':';
		port = iposix_addr_get_port(addr);
		sprintf(ptr, "%d", port);
		return text;
	}
#endif
	return NULL;
}


int iposix_addr_compare(const iPosixAddress *a1, const iPosixAddress *a2)
{
	int f1 = iposix_addr_family(a1);
	int f2 = iposix_addr_family(a2);
	if (f1 < f2) return -3;
	else if (f1 > f2) return 3;
	if (f1 == AF_INET) {
		int hr = memcmp(&(a1->sin4.sin_addr), &(a2->sin4.sin_addr), 
				sizeof(a1->sin4.sin_addr));
		if (hr < 0) return -2;
		if (hr > 0) return 2;
	}
#ifdef AF_INET6
	else if (f1 == AF_INET6) {
		int hr = memcmp(&(a1->sin6.sin6_addr), &(a2->sin6.sin6_addr),
				sizeof(a1->sin6.sin6_addr));
		if (hr < 0) return -2;
		if (hr > 0) return 2;
	}
#endif
	int p1 = iposix_addr_get_port(a1);
	int p2 = iposix_addr_get_port(a2);
	if (p1 < p2) return -1;
	if (p1 > p2) return 1;
	return 0;
}


//=====================================================================
// DNS Resolve
//=====================================================================

// create new iPosixRes
iPosixRes *iposix_res_new(int size)
{
	iPosixRes *res = NULL;
	int required = (16 + sizeof(void*) + sizeof(int)) * size;
	char *ptr;
	int i;
	ptr = (char*)malloc(sizeof(iPosixRes) + required + 16);
	res = (iPosixRes*)ptr;
	if (res == NULL) {
		return NULL;
	}
	ptr += sizeof(iPosixRes);
	res->address = (unsigned char**)ptr;
	ptr += sizeof(void*) * size;
	res->family = (int*)ptr;
	ptr += sizeof(int) * size;
	res->size = size;
	ptr = (char*)((((size_t)ptr) + 15) & (~((size_t)15)));
	for (i = 0; i < size; i++) {
		res->address[i] = (unsigned char*)ptr;
		ptr += 16;
	}
	return res;
}


// delete res
void iposix_res_free(iPosixRes *res)
{
	assert(res);
	res->size = 0;
	res->family = NULL;
	res->address = NULL;
	free(res);
}

// omit duplications
void iposix_res_unique(iPosixRes *res)
{
	int avail = 0, i;
	for (i = 0; i < res->size; i++) {
		IUINT32 *pi = (IUINT32*)res->address[i];
		int dup = 0, j;
		if (res->family[i] != AF_INET) {
#ifdef AF_INET6
			if (res->family[i] != AF_INET6) 
#endif
				dup++;
		}
		for (j = 0; j < avail && dup == 0; j++) {
			IUINT32 *pj = (IUINT32*)res->address[j];
			if (res->family[j] == res->family[i]) {
				if (res->family[j] == AF_INET) {
					if (pi[0] == pj[0]) {
						dup++;	
					}
				}
#ifdef AF_INET6
				if (res->family[j] == AF_INET6) {
					if (pi[0] == pj[0] && pi[1] == pj[1] &&
						pi[2] == pj[2] && pi[3] == pj[3]) {
						dup++;
					}
				}
#endif
			}
		}
		if (dup == 0) {
			if (avail != i) {
				res->family[avail] = res->family[i];
				memcpy(res->address[avail], res->address[i], 16);
			}
			avail++;
		}
	}
	res->size = avail;
}


// ipv = 0/any, 4/ipv4, 6/ipv6
iPosixRes *iposix_res_get(const char *hostname, int ipv)
{
	iPosixRes *res = NULL;
	struct addrinfo hints, *r, *p;
	int status, count;
	memset(&hints, 0, sizeof(hints));
	if (ipv == 4) {
		hints.ai_family = AF_INET;
	}
	else if (ipv == 6) {
	#ifdef AF_INET6
		hints.ai_family = AF_INET6;	
	#else
		return NULL;
	#endif
	}
	else if (ipv != 4 && ipv != 6) {
	#ifdef AF_UNSPEC
		hints.ai_family = AF_UNSPEC;
	#else
		return NULL;
	#endif
	}
	hints.ai_socktype = SOCK_STREAM;
	status = getaddrinfo(hostname, NULL, &hints, &r);
	if (status != 0) return NULL;
	for (p = r, count = 0; p != NULL; p = p->ai_next) {
		if (ipv == 4) {
			if (p->ai_family == AF_INET) count++;
		}
		else if (ipv == 6) {
		#ifdef AF_INET6
			if (p->ai_family == AF_INET6) count++;
		#endif
		}
		else {
			if (p->ai_family == AF_INET) count++;
		#ifdef AF_INET6
			else if (p->ai_family == AF_INET6) count++;
		#endif
		}
	}
	res = iposix_res_new(count);
	if (res == NULL) {
		freeaddrinfo(r);
		return NULL;
	}
	for (p = r, count = 0; p != NULL; p = p->ai_next) {
		int skip = 1;
		if (ipv == 4) {
			if (p->ai_family == AF_INET) skip = 0;
		}
		else if (ipv == 6) {
		#ifdef AF_INET6
			if (p->ai_family == AF_INET6) skip = 0;
		#endif
		}
		else {
			if (p->ai_family == AF_INET) skip = 0;
		#ifdef AF_INET6
			else if (p->ai_family == AF_INET6) skip = 0;
		#endif
		}
		if (skip) {
			continue;
		}
		res->family[count] = p->ai_family;
		if (p->ai_family == AF_INET) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			memcpy(res->address[count], &(ipv4->sin_addr), 4);
		}
		else {
		#ifdef AF_INET6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			memcpy(res->address[count], &(ipv6->sin6_addr), 16);
		#else
			memset(res->address[count], 0, 16);
		#endif
		}
		count++;
	}
	freeaddrinfo(r);
	return res;
}


int iposix_addr_version(const char *text)
{
	int i;
	for (i = 0; text[i]; i++) {
		if (text[i] == ':') return 6;
	}
	return 4;
}


//=====================================================================
// Protocol Reader
//=====================================================================
struct CAsyncReader
{
	int mode;
	int complete;
	ilong need;
	unsigned char spliter;
	struct IMSTREAM cache;
	struct IMSTREAM input;
};


CAsyncReader *async_reader_new(ib_memnode *fnode)
{
	CAsyncReader *reader;
	reader = (CAsyncReader*)ikmem_malloc(sizeof(CAsyncReader));
	if (reader == NULL) return NULL;
	ims_init(&reader->input, fnode, 0, 0);
	ims_init(&reader->cache, fnode, 0, 0);
	reader->spliter = (unsigned char)'\n';
	reader->mode = ISTREAM_READ_BYTE;
	reader->need = 0;
	reader->complete = 0;
	return reader;
}

void async_reader_delete(CAsyncReader *reader)
{
	if (reader != NULL) {
		ims_destroy(&reader->input);
		ims_destroy(&reader->cache);
		memset(reader, 0, sizeof(CAsyncReader));
		ikmem_free(reader);
	}
}

static void async_reader_redirect(struct IMSTREAM *dst, struct IMSTREAM *src)
{
	while (ims_dsize(src) > 0) {
		ilong size;
		void *ptr;
		size = ims_flat(src, &ptr);
		if (size > 0) {
			ims_write(dst, ptr, size);
			ims_drop(src, size);
		}
	}
}

static void async_reader_reset(CAsyncReader *reader)
{
	if (ims_dsize(&reader->cache) > 0) {
		struct IMSTREAM tmp;
		ims_init(&tmp, reader->cache.fixed_pages, 0, 0);
		async_reader_redirect(&tmp, &reader->input);
		async_reader_redirect(&reader->input, &reader->cache);
		async_reader_redirect(&reader->input, &tmp);
		ims_destroy(&tmp);
		reader->complete = 0;
		assert(ims_dsize(&reader->cache) == 0);
	}
}

void async_reader_mode(CAsyncReader *reader, int mode, ilong what)
{
	if (mode == ISTREAM_READ_LINE) {
		if (reader->mode == mode && 
			reader->spliter == (unsigned char)what) 
			return;
		reader->spliter = (unsigned char)what;
	}
	else if (mode == ISTREAM_READ_BLOCK) {
		reader->need = what;
		if (reader->mode == mode) return;
	}
	else {
		assert(mode == ISTREAM_READ_BYTE);
		if (reader->mode == mode) return;
	}
	reader->mode = mode;
	async_reader_reset(reader);
}

long async_reader_read(CAsyncReader *reader, void *data, long maxsize)
{
	unsigned char *out = (unsigned char*)data;
	ilong size = 0;
	ilong remain = 0;
	if (reader->mode == ISTREAM_READ_BYTE) {
		void *pointer;
		remain = ims_flat(&reader->input, &pointer);
		if (remain == 0) return -1;
		if (data == NULL) return 1;
		if (maxsize < 1) return -2;
		out[0] = *((unsigned char*)pointer);
		ims_drop(&reader->input, 1);
		return 1;
	}
	else if (reader->mode == ISTREAM_READ_LINE) {
		if (reader->complete) {
			remain = ims_dsize(&reader->cache);
			if (data == NULL) return (long)remain;
			if (maxsize < remain) return -2;
			ims_read(&reader->cache, data, remain);
			reader->complete = 0;
			return (long)remain;
		}	else {
			unsigned char spliter = reader->spliter;
			while (1) {
				void *pointer;
				unsigned char *src;
				ilong i;
				remain = ims_flat(&reader->input, &pointer);
				if (remain == 0) return -1;
				src = (unsigned char*)pointer;
				for (i = 0; i < remain; i++) {
					if (src[i] == spliter) break;
				}
				if (i >= remain) {
					ims_write(&reader->cache, src, remain);
					ims_drop(&reader->input, remain);
				}	else {
					ims_write(&reader->cache, src, i + 1);
					ims_drop(&reader->input, i + 1);
					size = ims_dsize(&reader->cache);
					if (data == NULL) {
						reader->complete = 1;
						return (long)size;
					}
					if (maxsize < size) {
						reader->complete = 1;
						return -2;
					}
					ims_read(&reader->cache, data, size);
					reader->complete = 0;
					return (long)size;
				}
			}
		}
	}
	else if (reader->mode == ISTREAM_READ_BLOCK) {
		remain = ims_dsize(&reader->input);
		size = reader->need;
		if (remain < size) return -1;
		if (data == NULL) return (long)size;
		if (maxsize < size) return -2;
		ims_read(&reader->input, data, size);
		return (long)size;
	}
	return -1;
}

void async_reader_feed(CAsyncReader *reader, const void *data, long len)
{
	if (len > 0 && data != NULL) {
		ims_write(&reader->input, data, len);
	}
}


void async_reader_clear(CAsyncReader *reader)
{
	reader->mode = ISTREAM_READ_BYTE;
	reader->need = 0;
	reader->complete = 0;
	reader->spliter = (unsigned char)'\n';
	ims_clear(&reader->input);
	ims_clear(&reader->cache);
}


//=====================================================================
// utils
//=====================================================================

int isocket_pair_ex(int *pair)
{
	int fds[2] = { -1, -1 };
	int hr = 0;
#ifdef __unix
	#ifndef __AVM2__
	if (pipe(fds) == 0) hr = 0;
	else hr = errno;
	#endif
#else
	if (isocket_pair(fds, 1) != 0) {
		int ok = 0, i;
		for (i = 0; i < 15; i++) {
			isleep(10);
			if (isocket_pair(fds, 1) == 0) {
				ok = 1;
				break;
			}
		}
		hr = ok? 0 : -1;
		if (ok) {
			ikeepalive(fds[0], 50, 300, 10);
			ikeepalive(fds[1], 50, 300, 10);
		}
	}
#endif
	if (hr != 0) {
		if (pair) {
			pair[0] = -1;
			pair[1] = -1;
		}
		return hr;
	}
	if (pair) {
		pair[0] = fds[0];
		pair[1] = fds[1];
	}
	return 0;
}


//=====================================================================
// can be used to wakeup select
//=====================================================================
struct CSelectNotify
{
	int fds[2];
	int event;
	IMUTEX_TYPE lock_pipe;
	IMUTEX_TYPE lock_select;
	char *buffer;
	int capacity;
};


CSelectNotify* select_notify_new(void)
{
	CSelectNotify *sn = (CSelectNotify*)ikmem_malloc(sizeof(CSelectNotify));
	if (!sn) {
		return NULL;
	}
	if (isocket_pair_ex(sn->fds) != 0) {
		ikmem_free(sn);
		return NULL;
	}
	sn->event = 0;
	sn->buffer = NULL;
	sn->capacity = 0;
	IMUTEX_INIT(&sn->lock_pipe);
	IMUTEX_INIT(&sn->lock_select);
	return sn;
}

void select_notify_delete(CSelectNotify *sn)
{
	if (sn) {
		IMUTEX_LOCK(&sn->lock_pipe);
		IMUTEX_LOCK(&sn->lock_select);
		if (sn->fds[0]) iclose(sn->fds[0]);
		if (sn->fds[1]) iclose(sn->fds[1]);
		sn->fds[0] = -1;
		sn->fds[1] = -1;
		if (sn->buffer) ikmem_free(sn->buffer);
		sn->buffer = NULL;
		sn->capacity = 0;
		IMUTEX_UNLOCK(&sn->lock_select);
		IMUTEX_UNLOCK(&sn->lock_pipe);
		IMUTEX_DESTROY(&sn->lock_pipe);
		IMUTEX_DESTROY(&sn->lock_select);
		ikmem_free(sn);
	}
}

int select_notify_wait(CSelectNotify *sn, const int *fds, 
	const int *event, int *revent, int count, long millisec)
{
	int hr = 0, n = 0, i;
	int need, unit, require;
	int *new_fds;
	int *new_event;
	int *new_revent;
	char *ptr;
	IMUTEX_LOCK(&sn->lock_select);
	n = (count + 1 + 31) & (~31);	
	need = iselect(NULL, NULL, NULL, n, 0, NULL);
	unit = (sizeof(int) * n + 31) & (~31);
	require = need + unit * 3;
	if (require > sn->capacity) {
		if (sn->buffer) ikmem_free(sn->buffer);
		sn->buffer = (char*)ikmem_malloc(require);
		if (sn->buffer == NULL) {
			sn->capacity = 0;
			IMUTEX_UNLOCK(&sn->lock_select);
			return -1000;
		}
		sn->capacity = require;
	}
	ptr = sn->buffer;
	new_fds = (int*)ptr;
	ptr += unit;
	new_event = (int*)ptr;
	ptr += unit;
	new_revent = (int*)ptr;
	ptr += unit;
	for (i = 0; i < count; i++) {
		new_fds[i] = fds[i];
		new_event[i] = event[i];
	}
	new_fds[count] = sn->fds[0];
	new_event[count] = IPOLL_IN;
	hr = iselect(new_fds, new_event, new_revent, count + 1, millisec, ptr);
	if (revent) {
		for (i = 0; i < count; i++) {
			revent[i] = new_revent[i];
		}
	}
	IMUTEX_LOCK(&sn->lock_pipe);
	if (sn->event) {
		char dummy[10];
		int fd = sn->fds[0];
	#ifdef __unix
		read(fd, dummy, 8);
	#else
		irecv(fd, dummy, 8, 0);
	#endif
		sn->event = 0;
	}
	IMUTEX_UNLOCK(&sn->lock_pipe);
	IMUTEX_UNLOCK(&sn->lock_select);
	return hr;
}


int select_notify_wake(CSelectNotify *sn)
{
	int fd = sn->fds[1];
	int hr = 0;
	IMUTEX_LOCK(&sn->lock_pipe);
	if (sn->event == 0) {
		char dummy = 1;
	#ifdef __unix
		#ifndef __AVM2__
		hr = write(fd, &dummy, 1);
		#endif
	#else
		hr = send(fd, &dummy, 1, 0);
	#endif
		if (hr == 1) {
			sn->event = 1;
			hr = 0;
		}
	}
	IMUTEX_UNLOCK(&sn->lock_pipe);
	return hr;
}


//=====================================================================
// Terminal Colors
//=====================================================================

// 设置颜色：低4位是文字颜色，高4位是背景颜色
// 具体编码可以搜索 ansi color或者 
// http://en.wikipedia.org/wiki/ANSI_escape_code
void console_set_color(int color)
{
	#ifdef _WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD result = 0;
	if (color & 1) result |= FOREGROUND_RED;
	if (color & 2) result |= FOREGROUND_GREEN;
	if (color & 4) result |= FOREGROUND_BLUE;
	if (color & 8) result |= FOREGROUND_INTENSITY;
	if (color & 16) result |= BACKGROUND_RED;
	if (color & 32) result |= BACKGROUND_GREEN;
	if (color & 64) result |= BACKGROUND_BLUE;
	if (color & 128) result |= BACKGROUND_INTENSITY;
	SetConsoleTextAttribute(hConsole, (WORD)result);
	#else
	int foreground = color & 7;
	int background = (color >> 4) & 7;
	int bold = color & 8;
	printf("\033[%s3%d;4%dm", bold? "01;" : "", foreground, background);
	#endif
}

// 设置光标位置左上角是，行与列都是从1开始计数的
void console_cursor(int row, int col)
{
	#ifdef _WIN32
	COORD point; 
	point.X = col - 1;
	point.Y = row - 1;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), point); 
	#else
	printf("\033[%d;%dH", row, col);
	#endif
}

// 恢复屏幕颜色
void console_reset(void)
{
	#ifdef _WIN32
	console_set_color(7);
	#else
	printf("\033[0m");
	#endif
}

// 清屏
void console_clear(int color)
{
	#ifdef _WIN32
	COORD coordScreen = { 0, 0 };
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwConSize;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(hConsole, TEXT(' '),
								dwConSize,
								coordScreen,
								&cCharsWritten);
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	FillConsoleOutputAttribute(hConsole,
								csbi.wAttributes,
								dwConSize,
								coordScreen,
								&cCharsWritten);
	SetConsoleCursorPosition(hConsole, coordScreen); 
	#else
	printf("\033[2J");
	#endif
}


//=====================================================================
// utilities
//=====================================================================


char* hash_signature_md5(
		char *out,             // output string with size above 64 bytes
		const void *in,        // input data
		int in_size,           // input size
		const char *secret,    // secret token
		int secret_size,       // secret size
		IUINT32 timestamp)     // time stamp in unix epoch seconds
{
	HASH_MD5_CTX md5;
	unsigned char buffer[32];
	if (secret_size < 0) {
		secret_size = (int)strlen(secret);
	}
	iencode32u_lsb((char*)buffer, timestamp);
	HASH_MD5_Init(&md5, 0);
	HASH_MD5_Update(&md5, "SIGNATURE", 9);
	HASH_MD5_Update(&md5, in, (unsigned int)in_size);
	HASH_MD5_Update(&md5, secret, (unsigned int)secret_size);
	HASH_MD5_Update(&md5, buffer, 4);
	HASH_MD5_Final(&md5, buffer + 4);
	hash_digest_to_string(buffer, 20, out);
	return out;
}


// extract timestamp from signature
IUINT32 hash_signature_time(const char *signature)
{
	unsigned char head[4];
	IUINT32 timestamp;
	int i;
	for (i = 0; i < 4; i++) {
		char ch = signature[i];
		int index = 0;
		if (ch >= '0' && ch <= '9') {
			index = (int)(ch - '0');
		}
		else if (ch >= 'a' && ch <= 'f') {
			index = (int)(ch - 'a') + 10;
		}
		else if (ch >= 'A' && ch <= 'F') {
			index = (int)(ch - 'A') + 10;
		}
		head[i] = (unsigned char)(index & 15);
	}
	idecode32u_lsb((char*)head, &timestamp);
	return (IUINT32)timestamp;
}



