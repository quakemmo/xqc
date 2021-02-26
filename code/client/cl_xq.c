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
	return cl_crypto_keyid->int64;
}
char *xq_getkeybyid(uint64_t keyid, netadr_t *from) {
	return cl_crypto_key->string;
}


/*
  Persistent int64_t values stored in client's memory that can be get or set
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

void CL_XQ_DevAuth() {
	// When we are not given cl_crypto_key and cl_crypto_keyid on the command line,
	// this is called. We hopefully have cl_devpassword, cl_patcherpath and cl_devauthurl set to something.

	// We are going to try to obtain a keyid and a key from the auth server
	// using cl_acctname, cl_devpassword and cl_devauthurl cl_patcherpath

	// If successful, we'll set the cl_crypto_key and cl_crypto_keyid cvars to the obtained
	// credentials. If it fails, we will display a message and exit the game.


	Com_Printf("No cl_crypto_keyid and cl_crypto_key as command line arguments - are we trying dev auth?\n");


	int cantry = 0;
	if (!(cl_patcherpath && cl_patcherpath->string[0])) {
		Com_Printf("No cl_patcherpath specified...it must be the full path of the patcher program.\n");
	} else if (!(cl_devauthpath && cl_devauthpath->string[0])) {
		Com_Printf("No cl_devauthpath specified...it must he the part of the API URL after the ://\n");
	} else if (!(cl_devauthscheme && cl_devauthscheme->string[0])) {
		Com_Printf("No cl_devauthscheme specified...it must either http or https\n");
	} else if (!(cl_acctname && cl_acctname->string[0])) {
		Com_Printf("No cl_acctname specified...it must be your game account name.\n");
	} else if (!(cl_devpassword && cl_devpassword->string[0])) {
		Com_Printf("No cl_devpassword specified...it must be the password of your game account.\n");
	} else {
		cantry = 1;
	}

	if (!cantry) {
		Com_Printf("Doesn't look like we have everything we need to try to obtain a session from the auth server.\n");
		exit(1);
	}

	Com_Printf("Going to try to obtain a session.\n");

	char path[1024] = {0};
	char logpath[1024] = {0};

	snprintf(path, 1024, "%s -X %s -Y %s -Z %s://%s",
		cl_patcherpath->string,
		cl_acctname->string,
		cl_devpassword->string,
		cl_devauthscheme->string,
		cl_devauthpath->string
	);

	snprintf(logpath, 1024, "%s -X %s -Y %s -Z %s://%s",
		cl_patcherpath->string,
		cl_acctname->string,
		"<snipped>",
		cl_devauthscheme->string,
		cl_devauthpath->string
	);

	FILE *fp = popen(path, "r");
	if (fp == NULL) {
		Com_Printf("We failed running: [%s]\n", logpath);
		exit(1);
	} else {
		Com_Printf("[%s] seems to launch properly.\n", logpath);
	}

	char line[200] = {0};
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strlen(line) > 2) {
			// strip trailing CR
			line[strlen(line)-1] = 0;
		}
		if (!strncmp(line, "session:", 8)) {
			Cvar_Set("cl_crypto_key", line+8);
		} else if (!strncmp(line, "hash:", 5)) {
			Cvar_Set("cl_crypto_keyid", line+5);
		} else if (!strncmp(line, "newlogin:", 9)) {
			Cvar_Set("cl_acctname", line+9);
		}
	}

	Com_Printf("Patcher got us the following values: key [%s], keyid [%s], newlogin [%s]",
		cl_crypto_key->string,
		cl_crypto_keyid->string,
		cl_acctname->string
	);


}
