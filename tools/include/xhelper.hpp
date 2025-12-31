#ifndef XHELPER_HPP
#define XHELPER_HPP 1

#include <memory>
#include <wglobal.hpp>
#include <xatomic.hpp>

#define FUNC_SIGNATURE __PRETTY_FUNCTION__

#define GET_STR(args) #args

#define CCMRAM __attribute__((section(".ccmram")))
#define RAM  __attribute__((section(".ram")))
#define PACK __attribute__((packed));
#define INLINE __attribute__((__always_inline__))

#define CHECK_EMPTY(x,...) do { if(!x){__VA_ARGS__;} }while(false)
#define CHECK_ERR(x,...) CHECK_EMPTY(x,__VA_ARGS__)

enum class NonConst{};
enum class Const{};

#endif
