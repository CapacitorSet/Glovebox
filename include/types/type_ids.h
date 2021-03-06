#ifndef GLOVEBOX_TYPE_IDS_H
#define GLOVEBOX_TYPE_IDS_H

// The enum is not actually used, it just ensures that type IDs do not overlap
enum {
	UNINITIALIZED_TYPE_ID = 0,
	INT_TYPE_ID,
	FIXED_TYPE_ID,
	ARRAY_TYPE_ID,
	STRING_TYPE_ID,
};

#endif // GLOVEBOX_TYPE_IDS_H
