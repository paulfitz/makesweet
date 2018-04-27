#ifndef MS_DBG_INC
#define MS_DBG_INC

extern int __ms_verbose;
#define dbg_printf if (__ms_verbose) printf

#endif
