#ifndef _INCLUDE_ERR_H_
#define _INCLUDE_ERR_H_

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#define ERR(x) std::string(x " at " AT)

#endif // _INCLUDE_ERR_H_
