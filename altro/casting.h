#ifndef __MIO_CASTING_H__
#define __MIO_CASTING_H__

namespace mio{

#define static_cast_void_ptr(ptr_type, ptr_var) static_cast<ptr_type *>(static_cast<void *>(ptr_var))

// variable1_t *var1;
// variable2_t *var2;
// variable2_t *var = mio::StaticCastPtr<variable2_t>(var1);
template <typename TYPE_OUT, typename PTR_TYPE_IN>
inline TYPE_OUT *StaticCastPtr(PTR_TYPE_IN ptr_in){
  void *void_ptr = static_cast<void*>(ptr_in);
  return static_cast<TYPE_OUT*>(void_ptr);
}

// variable1_t *var1;
// variable2_t *var2;
// variable2_t *var = mio::StaticCastPtr<variable2_t*>(var1);
template <typename TYPE_OUT, typename PTR_TYPE_IN>
inline TYPE_OUT StaticCastPtr2(PTR_TYPE_IN var_in){
  void *void_cast = static_cast<void*>(var_in);
  return static_cast<TYPE_OUT>(void_cast);
}

template <typename TYPE_OUT, typename PTR_TYPE_IN>
inline const TYPE_OUT *StaticCastConstPtr(const PTR_TYPE_IN ptr_in){
  const void *void_ptr = static_cast<const void*>(ptr_in);
  return static_cast<const TYPE_OUT*>(void_ptr);
}

}

#endif //__MIO_CASTING_H__

