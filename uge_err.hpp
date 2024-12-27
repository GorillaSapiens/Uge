#ifndef _INCLUDE_UGE_ERR_H_
#define _INCLUDE_UGE_ERR_H_

#define UGE_STRINGIFY(x) #x
#define UGE_TOSTRING(x) UGE_STRINGIFY(x)
#define UGE_AT __FILE__ ":" UGE_TOSTRING(__LINE__)

#define UGE_ERR(x) std::string(x " at " UGE_AT)

#endif // _INCLUDE_UGE_ERR_H_

// vim: expandtab:noai:ts=3:sw=3
