#include <stdlib.h>
#include "page.h"
#include "redislite.h"
#include "page_index.h"

void *redislite_page_get(void* _db, void *_cs, int num, char* type) {
	changeset *cs = (changeset*)_cs;
	redislite *db = (redislite*)_db;
	if (cs) {
		int i;
		for (i = 0; i < cs->modified_pages_length; i++) {
			redislite_page *page = cs->modified_pages[i];
			*type = page->type->identifier;
			if (page->number == num) return page->data;
		}
	}

	unsigned char *data = redislite_read_page(db, _cs, num);
	if (data == NULL) return NULL;
	void *result = NULL;
	*type = data[0];
	redislite_page_type* page_type = redislite_page_get_type(db, data[0]);
	if (page_type) {
		result = page_type->read_function(db, data+1);
	}
	free(data);
	return result;
}

void *redislite_page_get_by_keyname(void *_db, void *_cs, char *key_name, int length, char *type) {
	int num = redislite_value_page_for_key(_db, _cs, key_name, length);
	return redislite_page_get(_db, _cs, num, type);
}

void redislite_page_register_type(void *_db, redislite_page_type* type) {
	redislite *db = (redislite*)_db;
	if (db->types == NULL) {
		db->types = malloc(sizeof(redislite_page_type*) * 256);
		int i;
		for (i = 0; i < 256; i++) {
			db->types[i] = NULL;
		}
	}
	db->types[type->identifier] = type;
}

redislite_page_type *redislite_page_get_type(void *_db, char identifier) {
	redislite *db = (redislite*)_db;
	if (db->types == NULL) {
		return NULL;
	}
	if (db->types[identifier] == NULL) {
		printf("Unknown identifier: '%c'\n", identifier);
	}
	return db->types[identifier];
}
