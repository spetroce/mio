//#ifndef __MIO_MACROS_H__
//#define __MIO_MACROS_H__

//checks if a pointer is valid (not nullptr), dynmaically creates pointer data (if necessary), and
//sets a flag indicating if memory was or was not allocated.
#define CHK_PTR_MAKE_REF(type, pointer, reference)\
  const bool pointer ## _equals_nullptr = pointer == nullptr;\
  if(pointer ## _equals_nullptr)\
    pointer = new type;\
  type &reference = *pointer;

#define CHK_PTR_MAKE_REF_CLEAN_UP(pointer) if(pointer ## _equals_nullptr) delete pointer;

//less than - smaller values first
#define COMPARE_VAL_LESS_THAN(class_name, var_name)\
    static inline bool var_name ## _less_than(const class_name &var_a, const class_name &var_b){\
      return(var_a.var_name < var_b.var_name);\
    }\

//greater than - greater values first
#define COMPARE_VAL_GREATER_THAN(class_name, var_name)\
    static inline bool var_name ## _greater_than(const class_name &var_a, const class_name &var_b){\
      return(var_a.var_name > var_b.var_name);\
    }\

//less than - smaller values first (for variables that end in an underscore)
#define COMPARE_VAL_LESS_THAN_(class_name, var_name)\
    static inline bool var_name ## less_than(const class_name &var_a, const class_name &var_b){\
      return(var_a.var_name < var_b.var_name);\
    }\

//greater than - greater values first (for variables that end in an underscore)
#define COMPARE_VAL_GREATER_THAN_(class_name, var_name)\
    static inline bool var_name ## greater_than(const class_name &var_a, const class_name &var_b){\
      return(var_a.var_name > var_b.var_name);\
    }\

//#endif //__MIO_MACROS_H__

