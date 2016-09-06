#pragma once


#include <glm/glm.hpp>

#include <cstddef>


#ifdef _WIN32
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE inline
#endif


#ifndef SKIP_TYPEDEFS
	typedef float					f32;
	typedef	double					f64;
	typedef signed		char		i8 ;
	typedef unsigned	char		u8 ;
	typedef signed		short		i16;
	typedef unsigned	short		u16;
	typedef signed		int			i32;
	typedef unsigned	int			u32;
#	ifdef _WIN32
	typedef signed		__int64		i64;
	typedef unsigned	__int64		u64;
#	else // WIN32
	typedef signed		long long	i64;
	typedef unsigned	long long	u64;
#	endif // WIN32
#endif // SKIP_TYPEDEFS

namespace types
{

  enum DataType {
          TYPE_F16,	TYPE_F32,	TYPE_F64,
          TYPE_I8,	TYPE_U8,
          TYPE_I16,	TYPE_U16,
          TYPE_I32,	TYPE_U32,
          TYPE_I64,	TYPE_U64,
          TYPE_UNKNOWN
  };

}

std::size_t TypeSize(types::DataType type);
void TypeCast(u8 *target, types::DataType tType, u8 *source, types::DataType sType);

template<typename T> types::DataType TypeOf() {return TypeOf<typename T::StorageType>();}
#	define TYPE_OPERATION(C_TYPE, DATA_TYPE, SIZE, GL_DATA_TYPE) \
template<> FORCE_INLINE types::DataType TypeOf<C_TYPE>() {return DATA_TYPE;}
#	include "Types.inl"
#	undef TYPE_OPERATION
template<> FORCE_INLINE types::DataType TypeOf<glm::vec2>() {return TypeOf<float>();}
template<> FORCE_INLINE types::DataType TypeOf<glm::vec3>() {return TypeOf<float>();}
template<> FORCE_INLINE types::DataType TypeOf<glm::vec4>() {return TypeOf<float>();}
template<> FORCE_INLINE types::DataType TypeOf<glm::ivec2>() {return TypeOf<int>();}
template<> FORCE_INLINE types::DataType TypeOf<glm::ivec3>() {return TypeOf<int>();}
template<> FORCE_INLINE types::DataType TypeOf<glm::ivec4>() {return TypeOf<int>();}

template<typename T> int Elements() {return T::Elements();}
#	define TYPE_OPERATION(C_TYPE, DATA_TYPE, SIZE, GL_DATA_TYPE) \
template<> FORCE_INLINE int Elements<C_TYPE>() {return 1;}
#	include "Types.inl"
#	undef TYPE_OPERATION
template<> FORCE_INLINE int Elements<glm::vec2>() {return 2;}
template<> FORCE_INLINE int Elements<glm::vec3>() {return 3;}
template<> FORCE_INLINE int Elements<glm::vec4>() {return 4;}
template<> FORCE_INLINE int Elements<glm::ivec2>() {return 2;}
template<> FORCE_INLINE int Elements<glm::ivec3>() {return 3;}
template<> FORCE_INLINE int Elements<glm::ivec4>() {return 4;}
