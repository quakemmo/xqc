#include "client.h"

#define XQ_PERS_MAX_KEYLEN			20
#define XQ_PERS_NUM					100
typedef struct xq_pers_s {
	char		key[XQ_PERS_MAX_KEYLEN+1];
	int64_t		ptr;
} xq_pers_t;

xq_pers_t xq_pers[XQ_PERS_NUM];

void xq_write_to_logfile(char *msg, char *filename) {

	static int newline = 1;

	if (msg[0] == '\n') msg++;

    char *hp = Cvar_VariableString("fs_homepath");
    char *bg = Cvar_VariableString("com_basegame");
	char fn[100];
	char buf[10000];

	snprintf(fn, 99, "%s/%s/%s", hp, bg, filename);
	FILE *fd = fopen(fn, "a+");
	if (fd) {

		struct tm lt;
		time_t now = time(NULL);
		int msec = xq_msec() - (now * 1000);
		char formatted_time[100];
		#ifdef XQ_UNIX
			localtime_r(&now, &lt);
		#else
			struct tm *lt_nr;
			lt_nr = localtime(&now);
			memcpy(&lt, lt_nr, sizeof(lt));
		#endif
		if (strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", &lt)) {
			if (newline) {
				snprintf(buf, 9999, "%s.%03d: %s", formatted_time, msec, msg);
				newline = 0;
			} else {
				snprintf(buf, 999, "%s", msg);
			}
			fputs(buf, fd);
			if (msg[strlen(msg)-1] == '\n') {
				newline = 1;
			}
		}
		fclose(fd);
	}
}
uint64_t xq_getkeyid() {
	cvar_t *kid = Cvar_Get("cl_crypto_keyid", "", CVAR_TEMP);
	if (kid) {
		return (uint64_t)kid->int64;
	}
	return 0;
}
char *xq_getkeybyid(uint64_t keyid, netadr_t *from) {
	cvar_t *key = Cvar_Get("cl_crypto_key", "", CVAR_TEMP);
	if (key) {
		return key->string;
	}
	return NULL;
}


/*
  Persistant int64_t values stored in client's memory that can be get or set
  by the cgame to keep important stuff over cgame reloads / zoning

  Calling from CGame:

	To get:
	int64_t val;
	int ret = trap_XQ_Pers_Get_I64("somekey", &val);
	if (ret == 0) {
		// we don't have the value in pers memory
	} else {
		// value has been found and now is in val
	}


	To set:
	int64_t val = 12345; // (or = malloc(...)
	int ret = trap_XQ_Pers_Set_I64("somekey", val);
	if (ret == 0) {
		// we failed to set the value as it's not in pers memory already and we're out of available slots
	} else {
		// we successfully set the value
	}


*/
void xq_pers_init() {
	memset(&xq_pers, 0, sizeof(xq_pers));
}
int xq_pers_get_i64(char *name, int64_t *ret) {
	int i;
	xq_pers_t *p;
	for (i = 0;  i < XQ_PERS_NUM;  i++) {
		p = &xq_pers[i];
		if (xq_seq(p->key, name)) {
			*ret = p->ptr;
			return 1;
		}
	}
	return 0;
}
int xq_pers_set_i64(char *name, int64_t val) {
	int i;
	xq_pers_t *p;
	int found = -1;
	for (i = 0;  i < XQ_PERS_NUM;  i++) {
		p = &xq_pers[i];
		if (xq_seq(p->key, name)) {
			found = i;
			break;
		}
		if (p->key[0] == 0) break;
	}
	if (found == -1 && i == XQ_PERS_NUM) {
		// we don't have the element and we're out of available slots to create one
		return 0;
	}

	// create new slot at idx i
	Q_strncpyz(p->key, name, sizeof(p->key));
	p->ptr = val;
	return 1;
}
