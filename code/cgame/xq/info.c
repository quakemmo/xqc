/* DESCRIPTION

	This code deals with 3 things:
		- item info
		- spell info
		- generic JSON info (aka InfoInfo)

	A queue of stuff that we need to ask the server about is made and eventually sent to the server.
	Responses from the server will be cached here if it's item or spell info.
	InfoInfo isn't cached (up to the caller to cache the results as needed).

	This needs to be largely redone - remove the legacy (but working) ItemInfo and SpellInfo code,
	and channel everything through InfoInfo, which in turn needs to be made a lot more robust.
	Since InfoInfo doesn't implement any caching mechanism, Item/Spell caches can be left in there.

	Code in cl_input.c gets stuff from the queues here and sends requests to the server.
	It only sends one request of each type per client snapshot. It should be able to send a lot more.
*/

#include "../cg_local.h"

#include "cJSON.h"



#define XQ_ITEMINFO_RETRY_SECONDS			2		// How many seconds to wait before asking the server for an item info again if no reply
#define XQ_ITEMINFO_RETRY_TIMES				3		// How many times to retry getting item info (once per XQ_ITEMINFO_RETRY_SECONDS)
#define XQ_ITEMINFO_QUEUE_MAX				50		// Size of queue of items we need to send the server a request to get item info
													// This must be fairly high since inventory can have big bags full of stuff + bank etc.

#define XQ_SPELLINFO_RETRY_SECONDS			2
#define XQ_SPELLINFO_RETRY_TIMES			3
#define XQ_SPELLINFO_QUEUE_MAX				50

#define XQ_INFOINFO_RETRY_SECONDS			2
#define XQ_INFOINFO_RETRY_TIMES				3
#define XQ_INFOINFO_QUEUE_MAX				10





typedef struct xq_iteminfo_asked_item_s {
	int64_t	item;
	int		asked_ts;
	int		asked_times;
} xq_iteminfo_asked_item_t;
typedef struct xq_spellinfo_asked_item_s {
	int		spell;
	int		asked_ts;
	int		asked_times;
} xq_spellinfo_asked_spell_t;
typedef struct xq_infoinfo_asked_info_s {
	int		id1;
	int		id2;
	int		id3;
	int		id4;
	int		asked_ts;
	int		asked_times;
} xq_infoinfo_asked_info_t;

typedef struct xq_iteminfo_asked_s {
	#define	xq_iteminfo_asked_t_MAGIC		0x19dde038
	int							magics;
	xq_iteminfo_asked_item_t 	items[XQ_ITEMINFO_QUEUE_MAX];
	int							count;
	int							magice;
} xq_iteminfo_asked_t;
typedef struct xq_spellinfo_asked_s {
	xq_spellinfo_asked_spell_t 	spells[XQ_SPELLINFO_QUEUE_MAX];
	int							count;
} xq_spellinfo_asked_t;
typedef struct xq_infoinfo_asked_s {
	xq_infoinfo_asked_info_t 	infos[XQ_INFOINFO_QUEUE_MAX];
	int							count;
} xq_infoinfo_asked_t;

typedef struct xq_itemcache_s {
	#define xq_itemcache_t_MAGIC			0x9fc93fa3
	int				magics;

	int				atime;
	xq_item_t 	item;


	int				magice;
} xq_itemcache_t;
typedef struct xq_spellcache_s {
	int				atime;
	xq_spell_t 		spell;
} xq_spellcache_t;
typedef struct xq_infocache_s {
	int				atime;
	xq_info_t 		info;
} xq_infocache_t;

xq_itemcache_t			*xq_itemcache;
xq_iteminfo_asked_t		xq_iteminfo_asked;
xq_spellcache_t			*xq_spellcache;
xq_spellinfo_asked_t	xq_spellinfo_asked;
xq_infocache_t			*xq_infocache;
xq_infoinfo_asked_t		xq_infoinfo_asked;

static void setItemCacheMagics(void) {
	for (int i = 0;  i < XQ_ITEM_MAX_CACHE;  i++) {
		xq_itemcache[i].magics = xq_itemcache[i].magice = xq_itemcache_t_MAGIC;
	}
}
xq_item_t *xq_ItemInfo(int64_t itemid) {
	return (xq_item_t *)xq_GetCache(XQ_ITEM, itemid, 0, 0, 0);
}
xq_spell_t *xq_SpellInfo(int spellid) {
	return (xq_spell_t *)xq_GetCache(XQ_SPELL, (int64_t)spellid, 0, 0, 0);
}
xq_info_t *xq_InfoInfo(int info_id1, int info_id2, int info_id3, int info_id4) {
	xq_infocache_t *cache =
		(xq_infocache_t *)xq_GetCache(XQ_INFO, (int64_t)info_id1, (int64_t)info_id2, (int64_t)info_id3, (int64_t)info_id4);
	static xq_info_t inforet = {0};


	if (cache != NULL) {
		xq_info_t *info = &cache->info;
		char *json = (char *)(info->data);
		if (xq_debugInfoInfo.integer) {
			if (json) {
				xq_clog(COLOR_WHITE, "Going to parse [%s]", json);
			}
		}

		cJSON *j = cJSON_Parse(json);
		cJSON *item;
		cJSON *data;

		if (j == NULL) {
			xq_clog(COLOR_RED, "xq_InfoInfo:  cJSON_Parse() returned NULL: [%s]\n", json);
			return NULL;
		}

		if (!cJSON_IsObject(j)) {
			xq_clog(COLOR_RED, "xq_InfoInfo:  JSON not an object: %s", json);
			cJSON_Delete(j);
			return NULL;
		}


		item = cJSON_GetObjectItemCaseSensitive(j, "err");
		if (item == NULL) {
			xq_clog(COLOR_RED, "xq_InfoInfo:  cJSON_GetObjectItemCaseSensitive(err) returned NULL: %s", json);
			cJSON_Delete(j);
			return NULL;
		}

		if (!cJSON_IsNumber(item)) {
			xq_clog(COLOR_RED, "xq_InfoInfo:  err is not a number: %s", json);
			cJSON_Delete(j);
			return NULL;
		}

		if (item->valuedouble != 0) {
			// XXX on error we shouldn't query again until some time has elapsed
			xq_clog(COLOR_RED, "xq_InfoInfo:  err is not 0 (%f): %s", item->valuedouble, json);
			cJSON_Delete(j);
			return NULL;
		}

		if (xq_debugInfoInfo.integer) {
			xq_clog(COLOR_RED, "xq_InfoInfo:  err is 0: %s", json);
		}


		data = cJSON_GetObjectItemCaseSensitive(j, "data");
		if (data == NULL) {
			xq_clog(COLOR_RED, "xq_InfoInfo:  cJSON_GetObjectItemCaseSensitive(data) returned NULL: %s", json);
			cJSON_Delete(j);
			return NULL;
		}


		if (!cJSON_IsObject(data)) {
			xq_clog(COLOR_RED, "xq_InfoInfo:  data not an object: %s", json);
			return NULL;
		}
		if (xq_debugInfoInfo.integer) {
			xq_clog(COLOR_RED, "xq_InfoInfo:  data (%li) ok: %s", (int64_t)data, json);
		}



		// At this point, we have a valid "data" object, and the "err" value is 0
		// We can process the data object, depending on the type of info we received (held in info->id1, and supposedly in info_id1).
		// Processing means parsing the JSON, replacing it with the parsed data, and returning the pointer to the whole info struct (new one), after clearing the cache entry

		inforet.id1 = info->id1;
		inforet.id2 = info->id2;
		inforet.id3 = info->id3;
		inforet.id4 = info->id4;
		switch (info->id1) {
			case XQ_INFO_MODELS:;

				static xq_model_t mods[XQ_MAX_MODELS];
				memset((void *)&mods, 0, sizeof(mods));
				cJSON *ar = cJSON_GetObjectItem(data, "models");
				cJSON *iterator = NULL;
				if (cJSON_IsArray(ar)) {
					int num_models = 0;
					cJSON_ArrayForEach(iterator, ar) {
						if (cJSON_IsObject(iterator)) {
							if (num_models >= XQ_MAX_MODELS) {
								xq_clog(COLOR_RED, "XQ_MAX_MODELS reached");
							} else {

								item = cJSON_GetObjectItemCaseSensitive(iterator, "id");
								if (item) {
									xq_cmodels[num_models].id = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "type");
								if (item) {
									xq_cmodels[num_models].type= item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "path");
								if (item) {
									Q_strncpyz(xq_cmodels[num_models].path, item->valuestring, XQ_MODEL_MAX_PATH);
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "scale");
								if (item) {
									xq_cmodels[num_models].default_scale= item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "faces");
								if (item) {
									xq_cmodels[num_models].numfaces = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "skins");
								if (item) {
									xq_cmodels[num_models].numskins = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "crawler");
								if (item) {
									xq_cmodels[num_models].crawler = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "full_armor");
								if (item) {
									xq_cmodels[num_models].full_armor = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "mins0");
								if (item) {
									xq_cmodels[num_models].npc_mins[0] = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "mins1");
								if (item) {
									xq_cmodels[num_models].npc_mins[1] = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "mins2");
								if (item) {
									xq_cmodels[num_models].npc_mins[2] = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "maxs0");
								if (item) {
									xq_cmodels[num_models].npc_maxs[0] = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "maxs1");
								if (item) {
									xq_cmodels[num_models].npc_maxs[1] = item->valueint;
								}

								item = cJSON_GetObjectItemCaseSensitive(iterator, "maxs2");
								if (item) {
									xq_cmodels[num_models].npc_maxs[2] = item->valueint;
								}
							}

							// Force re-registering animodel with the potentially updated info
							xq_animodels[xq_cmodels[num_models].id].registered = 0;


							num_models++;
						} else {
							if (xq_debugInfoInfo.integer) {
								xq_clog(COLOR_RED, "MODELS: iterator failed");
							}
						}
					}
				}
				memcpy((void *)&inforet.data, (void *)&mods, MIN(sizeof(inforet.data), sizeof(mods)));
				inforet.len = 1; // This means success.  0 here means error
				break;
			default:
				xq_clog(COLOR_RED, "xq_InfoInfo:  unknown id1: %i", info->id1);
				break;
		}
		cJSON_Delete(j);
		inforet.id1 = info_id1;
		inforet.id2 = info_id2;
		inforet.id3 = info_id3;
		inforet.id4 = info_id4;
		// Unlike SpellInfo or ItemInfo, we immediately clear InfoInfo cache, upon first
		// successfull retrieval.
		// It's up to the caller to store the results somewhere.  Failing to do so will
		// result in additional network requests.
		xq_CacheClear(XQ_ITEM, info->id1, info->id2, info->id3, info->id4);
		return (xq_info_t *)&inforet;

	} else {
		if (xq_debugInfoInfo.integer) {
			xq_clog(COLOR_YELLOW, "xq_InfoInfo:  No data (yet)");
		}
		return NULL;
	}
}
void *xq_GetCache(int type, int64_t what1, int64_t what2, int64_t what3, int64_t what4) {
	int maxcnt, maxaskedcnt;

	if (type == XQ_ITEM) {
		maxcnt = XQ_ITEM_MAX_CACHE;
		maxaskedcnt = xq_iteminfo_asked.count;
	} else if (type == XQ_SPELL) {
		maxcnt = XQ_SPELL_MAX_NUM;
		maxaskedcnt = xq_spellinfo_asked.count;
	} else if (type == XQ_INFO) {
		maxcnt = XQ_INFO_MAX_CACHE;
		maxaskedcnt = xq_infoinfo_asked.count;
	} else {
		xq_clog(COLOR_RED, "xq_GetCache:  Unknown type (%i)", type);
		return NULL;
	}


	// Make sure what1 is valid
	if (what1 == 0) {
		xq_clog(COLOR_RED, "xq_GetCache: Got what1 id %li out of range (type: %i)", what1, type);
		return NULL;
	}


	// Look for the cached info
	int i;
	for (i = 0;  i < maxcnt;  i++) {
		if (type == XQ_ITEM && xq_itemcache[i].item.id == what1) {
			if (xq_debugInfo.integer) {
				xq_clog(COLOR_WHITE, "xq_ItemInfo: Returning cached data for item %s", xq_itemcache[i].item.name);
			}
			xq_itemcache[i].atime = time(NULL);
			if (xq_itemcache[i].item.id == 0) break;
			return (void *)&xq_itemcache[i].item;
		} else if (type == XQ_SPELL && xq_spellcache[i].spell.id == (int)what1) {
			xq_spellcache[i].atime = time(NULL);
			if (xq_spellcache[i].spell.id == 0) break;
			return (void *)&xq_spellcache[i].spell;
		} else if (	type == XQ_INFO &&
					xq_infocache[i].info.id1 == (int)what1 &&
					xq_infocache[i].info.id2 == (int)what2 &&
					xq_infocache[i].info.id3 == (int)what3 &&
					xq_infocache[i].info.id4 == (int)what4
					) {
			if (xq_infocache[i].info.id1 == 0) break;
			xq_infocache[i].atime = time(NULL);
			return (void *)&xq_infocache[i];// We are returning the cache item, not just the info slot (like we do for other cache types).
											// This is due to the fact that xq_InfoInfo will further process the info
											// by parsing the JSON it contains and replacing the JSON with the parsed data.
											// It will also set the cache->parsed to 1 so it's only parsed once.
		}
	} 


	// Cache lookup failed - add to Ask queue. That queue will be eventually sent to the server.
	for (i = 0;  i < maxaskedcnt;  i++) {
		if (type == XQ_ITEM && xq_iteminfo_asked.items[i].item == what1) {
			if (xq_debugInfo.integer) {
				xq_clog(COLOR_WHITE, "Item %li already asked at %i.  Asked count is %i", what1, i, xq_iteminfo_asked.count);
			}
			return NULL;
		} else if (type == XQ_SPELL && xq_spellinfo_asked.spells[i].spell == (int)what1) {
			if (xq_debugInfo.integer) {
				xq_clog(COLOR_WHITE, "Spell %li already asked at %i.  Asked count is %i", what1, i, xq_spellinfo_asked.count);
			}
			return NULL;
		} else if (	type == XQ_INFO &&
					xq_infoinfo_asked.infos[i].id1 == (int)what1 &&
					xq_infoinfo_asked.infos[i].id2 == (int)what2 &&
					xq_infoinfo_asked.infos[i].id3 == (int)what3 &&
					xq_infoinfo_asked.infos[i].id4 == (int)what4) {
			if (xq_debugInfo.integer) {
				xq_clog(COLOR_WHITE, "Info %i %i %i %i already asked at %i.  Asked count is %i", (int)what1, (int)what2, (int)what3, (int)what4, i, xq_infoinfo_asked.count);
			}
			return NULL;
		}
	}

	if (xq_debugInfo.integer) {
		xq_clog(COLOR_WHITE, "Cache element %li %li %li %li not in asked list %i", what1, what2, what3, what4, type);
	}


	if (type == XQ_ITEM && maxaskedcnt >= XQ_ITEMINFO_QUEUE_MAX) {
		xq_clog(COLOR_RED, "xq_GetCache: xq_item_t queue full item id %li", what1);
		return NULL;
	} if (type == XQ_SPELL && maxaskedcnt >= XQ_SPELLINFO_QUEUE_MAX) {
		xq_clog(COLOR_RED, "xq_GetCache: xq_spellinfo_t queue full spell id %li", what1);
		return NULL;
	} if (type == XQ_INFO && maxaskedcnt >= XQ_INFOINFO_QUEUE_MAX) {
		xq_clog(COLOR_RED, "xq_GetCache: xq_infoinfo_t queue full info id1 %li, id2 %li, id3 %li, id4 %li, maxaskedcnt %i", what1, what2, what3, what4, maxaskedcnt);
		return NULL;
	}

	if (type == XQ_ITEM) {
		if (xq_debugInfo.integer) {
			xq_clog(COLOR_WHITE, "%i xq_InfoInfo: Adding item %li to xq_iteminfo_asked.items[%i]", cg.time, what1, xq_iteminfo_asked.count);
		}
		xq_iteminfo_asked.items[xq_iteminfo_asked.count].asked_ts = 0;
		xq_iteminfo_asked.items[xq_iteminfo_asked.count].asked_times = 0;
		xq_iteminfo_asked.items[xq_iteminfo_asked.count++].item = what1;

	} else if (type == XQ_SPELL) {
		xq_spellinfo_asked.spells[xq_spellinfo_asked.count].asked_ts = 0;
		xq_spellinfo_asked.spells[xq_spellinfo_asked.count].asked_times = 0;
		xq_spellinfo_asked.spells[xq_spellinfo_asked.count++].spell = (int)what1;
	} else if (type == XQ_INFO) {
		if (xq_debugInfo.integer) {
			xq_clog(COLOR_WHITE, "%i xq_InfoInfo: Adding info %li / %li / %li / %li to xq_infoinfo_asked.infos[%i]", cg.time, what1, what2, what3, what4, xq_infoinfo_asked.count);
		}
		xq_infoinfo_asked.infos[xq_infoinfo_asked.count].asked_ts = 0;
		xq_infoinfo_asked.infos[xq_infoinfo_asked.count].asked_times = 0;
		xq_infoinfo_asked.infos[xq_infoinfo_asked.count].id1 = (int)what1;
		xq_infoinfo_asked.infos[xq_infoinfo_asked.count].id2 = (int)what2;
		xq_infoinfo_asked.infos[xq_infoinfo_asked.count].id3 = (int)what3;
		xq_infoinfo_asked.infos[xq_infoinfo_asked.count++].id4 = (int)what4;
	}
	return NULL;

}
void xq_SetCache(uint64_t ptr, int type, int action) {
//	action = 0: add/replace, action = 1: delete


	int i;
	int oldest_atime = time(NULL) + 100;
	int maxcnt = 0;
	int *maxaskedcnt = NULL;

	xq_item_t *item = (xq_item_t *)ptr;
	xq_spell_t *spell = (xq_spell_t *)ptr;
	xq_info_t *info = (xq_info_t *)ptr;

	switch (type) {
		case XQ_ITEM:
			if (item->id == 0) {
				xq_clog(COLOR_RED, "xq_SetCache: got item id %li out of range", item->id);
				return;
			}
			maxcnt = XQ_ITEM_MAX_CACHE;
			maxaskedcnt = &xq_iteminfo_asked.count;
			xq_CacheClear(XQ_ITEM, item->id, 0, 0, 0);
			break;
		case XQ_SPELL:
			if (spell->id < 1) {
				xq_clog(COLOR_RED, "xq_SetCache: got spell id %i out of range", spell->id);
				return;
			}
			maxcnt = XQ_SPELL_MAX_NUM;
			maxaskedcnt = &xq_spellinfo_asked.count;
			xq_CacheClear(XQ_SPELL, spell->id, 0, 0, 0);
			break;
		case XQ_INFO:
			if (info->id1 < 1) {
				xq_clog(COLOR_RED, "xq_SetCache: Got info id1 %i", info->id1);
				return;
			}
			maxcnt = XQ_INFO_MAX_CACHE;
			maxaskedcnt = &xq_infoinfo_asked.count;
			xq_CacheClear(XQ_INFO, info->id1, info->id2, info->id3, info->id4);
			break;
		default:
			xq_clog(COLOR_RED, "xq_SetCache:  Unknown type (%i)", type);
			return;
	}

	if (action == 0) {
		int oldest_atime_index = -1; 
		// Find the LRU cache entry and replace it
		for (i = 0;  i < maxcnt;  i++) {
			if (type == XQ_ITEM && xq_itemcache[i].atime < oldest_atime) {
				oldest_atime = xq_itemcache[i].atime;
				oldest_atime_index = i;
			} else if (type == XQ_SPELL && xq_spellcache[i].atime < oldest_atime) {
				oldest_atime = xq_spellcache[i].atime;
				oldest_atime_index = i;
			} else if (type == XQ_INFO && xq_infocache[i].atime < oldest_atime) {
				oldest_atime = xq_infocache[i].atime;
				oldest_atime_index = i;
			}
		}

		if (oldest_atime_index == -1) {
			// Should be unreachable
			xq_clog(COLOR_RED, "xq_SetCache: Can't find LRU entry.  Not good.");
			return;
		}

		if (type == XQ_ITEM) {
			memcpy(&xq_itemcache[oldest_atime_index].item, (void *)ptr, sizeof(xq_item_t));
			xq_itemcache[oldest_atime_index].atime = time(NULL);
			if (xq_debugInfo.integer) {
				xq_clog(COLOR_YELLOW, "xq_SetCache: Set to index %i: item id %li, name %s",
					oldest_atime_index, xq_itemcache[oldest_atime_index].item.id, xq_itemcache[oldest_atime_index].item.name);
			}
		} else if (type == XQ_SPELL) {
			memcpy(&xq_spellcache[oldest_atime_index].spell, (void *)ptr, sizeof(xq_spell_t));
			xq_spellcache[oldest_atime_index].atime = time(NULL);
		} else if (type == XQ_INFO) {
			memcpy(&xq_infocache[oldest_atime_index].info, (void *)ptr, sizeof(xq_info_t));
			xq_infocache[oldest_atime_index].atime = time(NULL);
		}
	}


	// Delete the entry (should be 1 or 0) from the asked queue
	if (*maxaskedcnt > 0) {
		for (i = 0;  i < *maxaskedcnt;  i++) {
			if (
				(type == XQ_ITEM && xq_iteminfo_asked.items[i].item == item->id) ||
				(type == XQ_SPELL && xq_spellinfo_asked.spells[i].spell == spell->id) ||
				(	type == XQ_INFO &&
					xq_infoinfo_asked.infos[i].id1 == info->id1 &&
					xq_infoinfo_asked.infos[i].id2 == info->id2 &&
					xq_infoinfo_asked.infos[i].id3 == info->id3 &&
					xq_infoinfo_asked.infos[i].id4 == info->id4
				)
			) {
				if (i < (*maxaskedcnt - 1)) {
					if (type == XQ_ITEM) {
						memcpy(&xq_iteminfo_asked.items[i], &xq_iteminfo_asked.items[i+1],
							sizeof(xq_iteminfo_asked_item_t) * ((XQ_ITEMINFO_QUEUE_MAX - i) - 1));
					} else if (type == XQ_SPELL) {
						memcpy(&xq_spellinfo_asked.spells[i], &xq_spellinfo_asked.spells[i+1],
							sizeof(xq_spellinfo_asked_spell_t) * ((XQ_SPELLINFO_QUEUE_MAX - i) - 1));
					} else if (type == XQ_INFO) {
						memcpy(&xq_infoinfo_asked.infos[i], &xq_infoinfo_asked.infos[i+1],
							sizeof(xq_infoinfo_asked_info_t) * ((XQ_INFOINFO_QUEUE_MAX - i) - 1));
					}
				}
				(*maxaskedcnt)--;
				if (type == XQ_ITEM) {
					memset(&xq_iteminfo_asked.items[XQ_ITEMINFO_QUEUE_MAX-1], 0, sizeof(xq_iteminfo_asked_item_t));
				} else if (type == XQ_SPELL) {
					memset(&xq_spellinfo_asked.spells[XQ_SPELLINFO_QUEUE_MAX-1], 0, sizeof(xq_spellinfo_asked_spell_t));
				} else if (type == XQ_INFO) {
					memset(&xq_infoinfo_asked.infos[XQ_INFOINFO_QUEUE_MAX-1], 0, sizeof(xq_infoinfo_asked_info_t));
				}
				break;
			}
		}
	}
}

uint64_t xq_ItemInfoAsk(void) {
	// Find a queued item that has been queried more than XQ_ITEMINFO_RETRY_SECONDS seconds ago.
	// this tries to find an item that has been asked the least times
	if (xq_iteminfo_asked.count < 1) {
		return 0;
	}
	if (xq_debugInfo.integer) {
		xq_clog(COLOR_YELLOW, "xq_ItemInfoAsk queue not empty: %i", xq_iteminfo_asked.count);
	}

	xq_iteminfo_asked_item_t *slot;
	int lowest_asked_times_idx = -1;
	int lowest_asked_times = MAX_QINT;
	for (int i = 0;  i < xq_iteminfo_asked.count;  i++) {
		slot = &xq_iteminfo_asked.items[i];
		if (slot->item == 0) {
			return 0;
		}

		if ((time(NULL) - slot->asked_ts) > XQ_ITEMINFO_RETRY_SECONDS) {
			if (slot->asked_times > XQ_ITEMINFO_RETRY_TIMES) {
				xq_item_t tmpitem;
				tmpitem.id = slot->item;
				if (xq_debugInfo.integer) {
					xq_clog(COLOR_GREEN, "%i Asking to delete item %li from cache: asked_ts %i, asked_times %i",
						cg.time, slot->item, slot->asked_ts, slot->asked_times);
				}
				xq_SetCache((uint64_t)&tmpitem, XQ_ITEM, 1);
			} else {
				if (slot->asked_times < lowest_asked_times) {
					lowest_asked_times = slot->asked_times;
					lowest_asked_times_idx = i;
				}
			}
		}
	}

	if (lowest_asked_times_idx > -1) {
		slot = &xq_iteminfo_asked.items[lowest_asked_times_idx];

		slot->asked_ts = time(NULL);
		if (xq_debugInfo.integer) {
			xq_clog(COLOR_YELLOW, "xq_ItemInfoAsk feeding item %li to cl_input.c (current asked_times: %i)",
				slot->item, slot->asked_times);
		}
		slot->asked_times++;
		return (uint64_t)&slot->item;
	}
	return 0;
}
uint64_t xq_SpellInfoAsk(void) {
	// Find a queued spell that has been queried more than XQ_SPELLINFO_RETRY_SECONDS seconds ago.
	// this tries to find a spell that has been asked the least times
	if (xq_spellinfo_asked.count < 1) {
		return 0;
	}
	if (xq_debugInfo.integer) {
		xq_clog(COLOR_YELLOW, "xq_SpellInfoAsk queue not empty: %i", xq_spellinfo_asked.count);
	}

	xq_spellinfo_asked_spell_t *slot;
	int lowest_asked_times_idx = -1;
	int lowest_asked_times = MAX_QINT;
	for (int i = 0;  i < xq_spellinfo_asked.count;  i++) {
		slot = &xq_spellinfo_asked.spells[i];
		if (slot->spell == 0) {
			return 0;
		}

		if ((time(NULL) - slot->asked_ts) > XQ_SPELLINFO_RETRY_SECONDS) {
			if (slot->asked_times > XQ_SPELLINFO_RETRY_TIMES) {
				xq_spell_t tmpspell;
				tmpspell.id = slot->spell;
				if (xq_debugInfo.integer) {
					xq_clog(COLOR_GREEN, "%i Asking to delete spell %i from cache: asked_ts %i, asked_times %i",
						cg.time, slot->spell, slot->asked_ts, slot->asked_times);
				}
				xq_SetCache((uint64_t)&tmpspell, XQ_SPELL, 1);
			} else {
				if (slot->asked_times < lowest_asked_times) {
					lowest_asked_times = slot->asked_times;
					lowest_asked_times_idx = i;
				}
			}
		} else {
			if (xq_debugInfo.integer) {
				xq_clog(COLOR_WHITE, "retry time not reached for spell %i (asked_ts: %i, asked_times: %i)", slot->spell, slot->asked_ts, slot->asked_times);
			}
		}
	}

	if (lowest_asked_times_idx > -1) {
		slot = &xq_spellinfo_asked.spells[lowest_asked_times_idx];

		slot->asked_ts = time(NULL);
		if (xq_debugInfo.integer) {
			xq_clog(COLOR_YELLOW, "xq_SpellInfoAsk feeding spell %i to cl_input.c (current asked_times: %i)",
				slot->spell, slot->asked_times);
		}
		slot->asked_times++;
		return (uint64_t)&slot->spell;
	}
	return 0;
}
uint64_t xq_InfoInfoAsk(void) {
	// Find a queued info that has been queried more than XQ_INFOINFO_RETRY_SECONDS seconds ago.
	// This doesn't try to find the slot with the least asked_times (it should).
	if (xq_infoinfo_asked.count < 1) {
		return 0;
	}
	if (xq_debugInfo.integer) {
		xq_clog(COLOR_YELLOW, "xq_InfoInfoAsk queue not empty: %i", xq_infoinfo_asked.count);
	}
	

	for (int i = 0;  i < xq_infoinfo_asked.count;  i++) {
		xq_infoinfo_asked_info_t *slot = &xq_infoinfo_asked.infos[i];
		if (slot->id1 == 0) {
			return 0;
		}

		if ((time(NULL) - slot->asked_ts) > XQ_INFOINFO_RETRY_SECONDS) {
			if (slot->asked_times > XQ_INFOINFO_RETRY_TIMES) {
				xq_info_t tmpinfo;
				tmpinfo.id1 = slot->id1;
				tmpinfo.id2 = slot->id2;
				tmpinfo.id3 = slot->id3;
				tmpinfo.id4 = slot->id4;
				xq_SetCache((uint64_t)&tmpinfo, XQ_INFO, 1);
			} else {
				xq_infoinfo_asked.infos[i].asked_ts = time(NULL);
				if (xq_debugInfo.integer) {
					xq_clog(COLOR_YELLOW, "xq_InfoInfoAsk feeding info %i %i %i %i to cl_input.c",
						slot->id1, slot->id2, slot->id3, slot->id4);
				}
				slot->asked_times++;

				static int64_t args[4];
				args[0] = slot->id1;
				args[1] = slot->id2;
				args[2] = slot->id3;
				args[3] = slot->id4;

				return (uint64_t)args;
			}
		}
	}
	return 0;
}
void xq_CacheClear(int type, int64_t what1, int64_t what2, int64_t what3, int64_t what4) {
	int i;
	int maxcount = 0;

	switch (type) {
		case XQ_ITEM:
			maxcount = XQ_ITEM_MAX_CACHE;
			break;
		case XQ_SPELL:
			maxcount = XQ_SPELL_MAX_NUM;
			break;
		case XQ_INFO:
			maxcount = XQ_INFO_MAX_CACHE;
			break;
	}

	if (xq_debugInfo.integer) {
		xq_clog(COLOR_WHITE, "xq_CacheClear(%i, %li, %li, %li, %li])", type, what1, what2, what3, what4);
	}

	for (i = 0;  i < maxcount;  i++) {
		if (type == XQ_ITEM) {
			if (xq_itemcache[i].item.id == what1) {
				if (xq_debugInfo.integer) {
					xq_clog(COLOR_WHITE, "xq_CacheClear(XQ_ITEM, %li, [%s]): at index %i",
						what1, xq_itemcache[i].item.name, i);
				}

				memcpy(&xq_itemcache[i], &xq_itemcache[i + 1], sizeof(xq_itemcache_t) * (maxcount - (i + 1)));

				memset((void *)&xq_itemcache[maxcount - 1].item, 0, sizeof(xq_item_t));
				xq_itemcache[maxcount - 1].atime = 0;
				return;
			}
		} else if (type == XQ_SPELL) {
			if (xq_spellcache[i].spell.id == (int)what1) {
				if (xq_debugInfo.integer) {
					xq_clog(COLOR_WHITE, "xq_CacheClear(XQ_SPELL, %li): at index %i", what1, i);
				}

				memcpy((void *)&xq_spellcache[i], (void *)&xq_spellcache[i + 1], sizeof(xq_spellcache_t) * (maxcount - (i + 1)));
				memset((void *)&xq_spellcache[maxcount - 1].spell, 0, sizeof(xq_spell_t));
				xq_spellcache[maxcount - 1].atime = 0;
				return;
			}
		} else if (type == XQ_INFO) {
			if (
					xq_infocache[i].info.id1 == (int)what1 &&
					xq_infocache[i].info.id2 == (int)what2 &&
					xq_infocache[i].info.id3 == (int)what3 &&
					xq_infocache[i].info.id4 == (int)what4
				) {

				memcpy((void *)&xq_infocache[i], (void *)&xq_infocache[i + 1], sizeof(xq_infocache_t) * (maxcount - (i + 1)));
				memset((void *)&xq_infocache[maxcount - 1].info, 0, sizeof(xq_info_t));
				xq_infocache[maxcount - 1].atime = 0;

				if (xq_debugInfo.integer) {
					xq_clog(COLOR_WHITE, "xq_CacheClear(XQ_INFO, %li, %li, %li, %li): at index %i", what1, what2, what3, what4, i);
				}
				return;
			}
		}
	}
}
void xq_CacheDelete(int type) {
	// Deletes all cache of a given type
	switch (type) {
		case XQ_ITEM:		memset(xq_itemcache, 0, sizeof(xq_itemcache_t)*XQ_ITEM_MAX_CACHE);			setItemCacheMagics();	break;
		case XQ_SPELL:		memset(xq_spellcache, 0, sizeof(xq_spellcache_t)*XQ_SPELL_MAX_NUM);									break;
		case XQ_INFO:		memset(xq_infocache, 0, sizeof(xq_infocache_t)*XQ_INFO_MAX_CACHE);									break;
	}

	memset(&xq_iteminfo_asked, 0, sizeof(xq_iteminfo_asked));
	xq_iteminfo_asked.magics = xq_iteminfo_asked.magice = xq_iteminfo_asked_t_MAGIC;
	memset(&xq_spellinfo_asked, 0, sizeof(xq_spellinfo_asked));
	memset(&xq_infoinfo_asked, 0, sizeof(xq_infoinfo_asked));
}




void xq_localcmd_cacheclear(int argc, char *param1, char *param2, char *param3, char *param4, char *param5) {

	if (argc == 0) {
		xq_clog(COLOR_YELLOW, "Usage: /cacheclear [item|spell|info] [itemid|spellid|info_id1 info_id2 info_id3 info_id4] (no obj id = clear whole cache)");
	} else {
		if (xq_seq(param1, "item")) {
			if (argc == 1) {
				xq_clog(COLOR_WHITE, "Clearing all item cache");
				while (xq_itemcache[0].item.id != 0) {
					xq_CacheClear(XQ_ITEM, xq_itemcache[0].item.id, 0, 0, 0);
				}
			} else if (argc == 2) {
				xq_clog(COLOR_WHITE, "Going to clear item %i", atoi(param2));
				xq_CacheClear(XQ_ITEM, atoi(param2), 0, 0, 0);
			}
		} else if (xq_seq(param1, "spell")) {
			if (argc == 1) {
				xq_clog(COLOR_WHITE, "Clearing all spell cache");
				while (xq_spellcache[0].spell.id != 0) {
					xq_CacheClear(XQ_SPELL, xq_spellcache[0].spell.id, 0, 0, 0);
				}
			} else if (argc == 2) {
				xq_clog(COLOR_WHITE, "Going to clear spell %i", atoi(param2));
				xq_CacheClear(XQ_SPELL, atoi(param2), 0, 0, 0);
			}
		} else if (xq_seq(param1, "info")) {
			if (argc == 1) {
				xq_clog(COLOR_WHITE, "Clearing all info cache");
				while (xq_infocache[0].info.id1 != 0) {
					xq_CacheClear(XQ_INFO, xq_infocache[0].info.id1, xq_infocache[0].info.id2, xq_infocache[0].info.id3, xq_infocache[0].info.id4);
				}
			} else if (argc == 5) {
				xq_clog(COLOR_WHITE, "Going to clear info %i %i %i %i", atoi(param2), atoi(param3), atoi(param4), atoi(param5));
				xq_CacheClear(XQ_INFO, atoi(param2), atoi(param3), atoi(param4), atoi(param5));
			}
		}
	}
}
void xq_localcmd_cacheinfo(int type) {

	int i;

	if (type == 0 || type == 100) {
		xq_clog(COLOR_YELLOW, "Usage: /cacheinfo [item|spell|info]");
	}
	if (type == 0 || type == 1) {
		for (i = 0;  i < XQ_ITEM_MAX_CACHE;  i++) {
			xq_item_t *item = &xq_itemcache[i].item;
			if (!item->id) {
				break;
			} else if (type == 1) {
				xq_clog(COLOR_WHITE, "ItemInfo Cache: cache slot %i, item %li, name %s", i, item->id, item->name);
			}
		}
		xq_clog(COLOR_WHITE, "ItemInfo Cache: used slots %i", i);
	}

	if (type == 0 || type == 2) {
		for (i = 0;  i < XQ_SPELL_MAX_NUM;  i++) {
			xq_spell_t *spell = &xq_spellcache[i].spell;
			if (!spell->id) {
				break;
			} else if (type == 2) {
				xq_clog(COLOR_WHITE, "SpellInfo Cache: cache slot %i, spell %i, name %s", i, spell->id, spell->name);
			}
		}
		xq_clog(COLOR_WHITE, "SpellInfo Cache: used slots %i", i);
	}

	if (type == 0 || type == 3) {
		for (i = 0;  i < XQ_INFO_MAX_CACHE;  i++) {
			xq_info_t *info = &xq_infocache[i].info;
			if (!info->id1) {
				break;
			} else if (type == 3) {
				xq_clog(COLOR_WHITE, "InfoInfo Cache: cache slot %i, id's: %i, %i, %i, %i", i, info->id1, info->id2, info->id3, info->id4);
			}
		}
		xq_clog(COLOR_WHITE, "InfoInfo Cache: used slots %i", i);
	}
}
void xq_Info_CheckMagics() {
	assert(xq_iteminfo_asked.magics == xq_iteminfo_asked_t_MAGIC);
	assert(xq_iteminfo_asked.magice == xq_iteminfo_asked_t_MAGIC);

	int i;
	for (i = 0;  i < XQ_ITEM_MAX_CACHE;  i++) {
		assert(xq_itemcache[i].magics == xq_itemcache_t_MAGIC);
		assert(xq_itemcache[i].magice == xq_itemcache_t_MAGIC);
	}

	for (i = 0;  i < XQ_MAX_MODELS;  i++) {
		assert(xq_animodels[i].magics == xq_cgame_animodel_t_MAGIC);
		assert(xq_animodels[i].magice == xq_cgame_animodel_t_MAGIC);
	}
}


void xq_Info_Init() {
	//  xq_itemcache
	xq_itemcache = calloc(XQ_ITEM_MAX_CACHE, sizeof(xq_itemcache_t));

	//  xq_spellcache
	xq_spellcache = calloc(XQ_SPELL_MAX_NUM, sizeof(xq_spellcache_t));

	//  xq_infocache
	xq_infocache = calloc(XQ_INFO_MAX_CACHE, sizeof(xq_infocache_t));

	xq_CacheDelete(XQ_ITEM);
	xq_CacheDelete(XQ_SPELL);
	xq_CacheDelete(XQ_INFO);
}
void xq_Info_Deinit() {
	free(xq_itemcache);
	free(xq_spellcache);
	free(xq_infocache);
}
