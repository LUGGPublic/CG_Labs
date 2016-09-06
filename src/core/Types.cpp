#include "Types.h"

size_t TypeSize(types::DataType type)
{
#	define TYPE_OPERATION(C_TYPE, DATA_TYPE, SIZE, GL_DATA_TYPE) \
		case DATA_TYPE: return SIZE;
	switch (type) {
#		include "Types.inl"
		default: break;
	}
	return 0;
#	undef TYPE_OPERATION
}

// LOWPRIO: Better solution?
template<class SRC_TYPE>
void TypeCast_(u8 *target, types::DataType toType, SRC_TYPE val)
{
#	define TYPE_OPERATION(C_TYPE, DATA_TYPE, SIZE, GL_DATA_TYPE) \
		case DATA_TYPE: (*((C_TYPE *) target)) = (C_TYPE(val)); return;
	switch (toType) {
#		include "Types.inl"
		default: break;
	}
#	undef TYPE_OPERATION
}

void TypeCast(u8 *target, types::DataType tType, u8 *source, types::DataType sType)
{
#	define TYPE_OPERATION(C_TYPE, DATA_TYPE, SIZE, GL_DATA_TYPE) \
		case DATA_TYPE: TypeCast_<C_TYPE>(target, tType, *((C_TYPE*) source)); return;
	switch (sType) {
#		include "Types.inl"
		default: break;
	}
#	undef TYPE_OPERATION
}
