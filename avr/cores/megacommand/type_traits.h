#pragma once

// copied from x64 g++-7 for Linux

// Compare for equality of types.
template<typename, typename>
  struct __are_same
  {
    enum { __value = 0 };
  };

template<typename _Tp>
  struct __are_same<_Tp, _Tp>
  {
    enum { __value = 1 };
  };


