#include "AttrCacheTable.h"
#include <bits/stdc++.h>
#include <cstring>

AttrCacheEntry* AttrCacheTable::attrCache[MAX_OPEN];

/* returns the attrOffset-th attribute for the relation corresponding to relId
NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
*/
int AttrCacheTable::getAttrCatEntry(int relId, int attrOffset, AttrCatEntry* attrCatBuf) {
  // check if 0 <= relId < MAX_OPEN and return E_OUTOFBOUND otherwise
   if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  // if there's no entry at the rel-id
  if (attrCache[relId] == nullptr) {
    return E_RELNOTOPEN;
  }
  // check if attrCache[relId] == nullptr and return E_RELNOTOPEN if true

  // traverse the linked list of attribute cache entries
  for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
    if (entry->attrCatEntry.offset == attrOffset) {
      // copy entry->attrCatEntry to *attrCatBuf and return SUCCESS;
      *attrCatBuf = entry->attrCatEntry;
      return SUCCESS;
    }
  }

  // there is no attribute at this offset
  return E_ATTRNOTEXIST;
}

/* Converts a attribute catalog record to AttrCatEntry struct
    We get the record as Attribute[] from the BlockBuffer.getRecord() function.
    This function will convert that to a struct AttrCatEntry type.
*/
void AttrCacheTable::recordToAttrCatEntry(union Attribute record[ATTRCAT_NO_ATTRS],
                                          AttrCatEntry* attrCatEntry) {
  strcpy(attrCatEntry->relName, record[ATTRCAT_REL_NAME_INDEX].sVal);
  strcpy(attrCatEntry->attrName, record[ATTRCAT_ATTR_NAME_INDEX].sVal);
  attrCatEntry->attrType = (int)record[ATTRCAT_ATTR_TYPE_INDEX].nVal;
  attrCatEntry->primaryFlag=(bool)record[ATTRCAT_PRIMARY_FLAG_INDEX].nVal;
  attrCatEntry->rootBlock= (int)record[ATTRCAT_ROOT_BLOCK_INDEX].nVal;
  attrCatEntry->offset = (int)record[ATTRCAT_OFFSET_INDEX].nVal;
  // copy the rest of the fields in the record to the attrCacheEntry struct
}

/* returns the attribute with name `attrName` for the relation corresponding to relId
NOTE: this function expects the caller to allocate memory for `*attrCatBuf`
*/
int AttrCacheTable::getAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry* attrCatBuf) {

  // check that relId is valid and corresponds to an open relation
 if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  // if there's no entry at the rel-id
  if (attrCache[relId] == nullptr) {
    return E_RELNOTOPEN;
   }
  // iterate over the entries in the attribute cache and set attrCatBuf to the entry that
  //    matches attrName
for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
    if (strcmp(entry->attrCatEntry.attrName, attrName)==0) {
      *attrCatBuf = entry->attrCatEntry;
      return SUCCESS;
      }
    }
  // no attribute with name attrName for the relation
  return E_ATTRNOTEXIST;
}

int AttrCacheTable::getSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId* searchIndex) {
	/*if relId is outside the range [0, MAX_OPEN-1]*/
	if (relId < 0 || relId >= MAX_OPEN) {
		return E_OUTOFBOUND;
	}
       /*if entry corresponding to the relId in the Attribute Cache Table is free*/
	if (attrCache[relId] == nullptr) {
		return E_RELNOTOPEN;
	}
	
		/*for each attribute corresponding to relation with relId */
	for (AttrCacheEntry* cacheEntry = attrCache[relId]; cacheEntry != nullptr; cacheEntry = cacheEntry->next) {
		/* if attrName/offset field of the AttrCatEntry
			is equal to the input attrName/attrOffset */
		if (strcmp(attrName, cacheEntry->attrCatEntry.attrName) == 0) {
			//copy the searchIndex field of the corresponding Attribute Cache entry
			//in the Attribute Cache Table to input searchIndex variable.
			*searchIndex = cacheEntry->searchIndex;
			return SUCCESS;
		}
	}
	return E_ATTRNOTEXIST;
}
int AttrCacheTable::getSearchIndex(int relId, int attrOffset, IndexId* searchIndex) {
	/*if relId is outside the range [0, MAX_OPEN-1]*/
	if (relId < 0 || relId >= MAX_OPEN) {
		return E_OUTOFBOUND;
	}
	
	/*if entry corresponding to the relId in the Attribute Cache Table is free*/
	if (attrCache[relId] == nullptr) {
		return E_RELNOTOPEN;
	}
	
	/*for each attribute corresponding to relation with relId */
	for (AttrCacheEntry* cacheEntry = attrCache[relId]; cacheEntry != nullptr; cacheEntry = cacheEntry->next) {
		/* if attrName/offset field of the AttrCatEntry
			is equal to the input attrName/attrOffset */
		if (attrOffset == cacheEntry->attrCatEntry.offset) {

			//copy the searchIndex field of the corresponding Attribute Cache entry
			//in the Attribute Cache Table to input searchIndex variable.
			*searchIndex = cacheEntry->searchIndex;
			return SUCCESS;
		}
	}
	return E_ATTRNOTEXIST;
}
int AttrCacheTable::setSearchIndex(int relId, char attrName[ATTR_SIZE], IndexId* searchIndex) {
	/*if relId is outside the range [0, MAX_OPEN-1]*/
	if (relId < 0 || relId >= MAX_OPEN) {
		return E_OUTOFBOUND;
	}

	/*entry corresponding to the relId in the Attribute Cache Table is free*/
	if (attrCache[relId] == nullptr) {
		return E_RELNOTOPEN;
	}

	/* each attribute corresponding to relation with relId */
	for (AttrCacheEntry* cacheEntry = attrCache[relId]; cacheEntry != nullptr; cacheEntry = cacheEntry->next) {
		/* attrName/offset field of the AttrCatEntry
		is equal to the input attrName/attrOffset */
		if (strcmp(attrName, cacheEntry->attrCatEntry.attrName) == 0) {

			// copy the input searchIndex variable to the searchIndex field of the
			//corresponding Attribute Cache entry in the Attribute Cache Table.
			cacheEntry->searchIndex = *searchIndex;
			return SUCCESS;
		}
	}
	return E_ATTRNOTEXIST;
}
int AttrCacheTable::setSearchIndex(int relId, int attrOffset, IndexId* searchIndex) {
	/*if relId is outside the range [0, MAX_OPEN-1]*/
	if (relId < 0 || relId >= MAX_OPEN) {
		return E_OUTOFBOUND;
	}

	/*entry corresponding to the relId in the Attribute Cache Table is free*/
	if (attrCache[relId] == nullptr) {
		return E_RELNOTOPEN;
	}

	/* each attribute corresponding to relation with relId */
	for (AttrCacheEntry* cacheEntry = attrCache[relId]; cacheEntry != nullptr; cacheEntry = cacheEntry->next) {
		/* attrName/offset field of the AttrCatEntry
		is equal to the input attrName/attrOffset */
		if (attrOffset == cacheEntry->attrCatEntry.offset) {

			// copy the input searchIndex variable to the searchIndex field of the
			//corresponding Attribute Cache entry in the Attribute Cache Table.
			cacheEntry->searchIndex = *searchIndex;
			return SUCCESS;
		}
	}
	return E_ATTRNOTEXIST;
}
int AttrCacheTable::resetSearchIndex(int relId, char attrName[ATTR_SIZE]) {
  // declare an IndexId having value {-1, -1}
  // set the search index to {-1, -1} using AttrCacheTable::setSearchIndex
  // return the value returned by setSearchIndex
	IndexId resetIndex = {-1, -1};
	return AttrCacheTable::setSearchIndex(relId, attrName, &resetIndex);
}
int AttrCacheTable::resetSearchIndex(int relId, int attrOffset) {
  // declare an IndexId having value {-1, -1}
  // set the search index to {-1, -1} using AttrCacheTable::setSearchIndex
  // return the value returned by setSearchIndex
	IndexId resetIndex = {-1, -1};
	return setSearchIndex(relId, attrOffset, &resetIndex);
}

// Sets the Attribute Catalog entry corresponding to the given attribute of the specified relation in the Attribute Cache Table.
int AttrCacheTable::setAttrCatEntry(int relId, char attrName[ATTR_SIZE], AttrCatEntry* attrCatBuf) {
	/*relId is outside the range [0, MAX_OPEN-1]*/
	if (relId < 0 || relId >= MAX_OPEN) {
		return E_OUTOFBOUND;
	}
	/*entry corresponding to the relId in the Attribute Cache Table is free*/
	if (attrCache[relId] == nullptr) {
		return E_RELNOTOPEN;
	}
	/* each attribute corresponding to relation with relId */
	for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
		/* the attrName field of the AttrCatEntry
		   is equal to the input attrName */
		if (strcmp(entry->attrCatEntry.attrName, attrName) == 0) {
			// copy the attrCatBuf to the corresponding Attribute Catalog entry in
			// the Attribute Cache Table.
			entry->attrCatEntry = *attrCatBuf;
			// set the dirty flag of the corresponding Attribute Cache entry in the
			// Attribute Cache Table.
			entry->dirty = true;
			return SUCCESS;
		}
	}
	return E_ATTRNOTEXIST;
}
// Sets the Attribute Catalog entry corresponding to the given attribute of the specified relation in the Attribute Cache Table.
int AttrCacheTable::setAttrCatEntry(int relId, int attrOffset, AttrCatEntry* attrCatBuf) {
	/*relId is outside the range [0, MAX_OPEN-1]*/
	if (relId < 0 || relId >= MAX_OPEN) {
		return E_OUTOFBOUND;
	}
	/*entry corresponding to the relId in the Attribute Cache Table is free*/
	if (attrCache[relId] == nullptr) {
		return E_RELNOTOPEN;
	}
	/* each attribute corresponding to relation with relId */
	for (AttrCacheEntry* entry = attrCache[relId]; entry != nullptr; entry = entry->next) {
		/* the offset field of the AttrCatEntry
		   is equal to the input attroffset */
		if (entry->attrCatEntry.offset == attrOffset) {
			// copy the attrCatBuf to the corresponding Attribute Catalog entry in
			// the Attribute Cache Table.
			entry->attrCatEntry = *attrCatBuf;
			// set the dirty flag of the corresponding Attribute Cache entry in the
			// Attribute Cache Table.
			entry->dirty = true;
			return SUCCESS;
		}
	}
	return E_ATTRNOTEXIST;
}
/* A function that converts AttrCatEntry structure to a record, implemented as an array of union Attribute. */
void AttrCacheTable::attrCatEntryToRecord(AttrCatEntry* attrCatEntry, union Attribute record[ATTRCAT_NO_ATTRS]) {
	// left to you
	strcpy(record[RELCAT_REL_NAME_INDEX].sVal, attrCatEntry->relName);
	strcpy(record[RELCAT_NO_ATTRIBUTES_INDEX].sVal, attrCatEntry->attrName);
	record[RELCAT_NO_RECORDS_INDEX].nVal = (double)attrCatEntry->attrType;
	record[RELCAT_FIRST_BLOCK_INDEX].nVal = (double)attrCatEntry->primaryFlag;
	record[RELCAT_LAST_BLOCK_INDEX].nVal = (double)attrCatEntry->rootBlock;
	record[RELCAT_NO_SLOTS_PER_BLOCK_INDEX].nVal = (double)attrCatEntry->offset;
}

