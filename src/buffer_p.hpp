#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_BUFFER_P_HPP_
#define _SRC_BUFFER_P_HPP_

#define STATIC_BLOCK 768
#define BLOCK   4096

#include <vector>
#include <boost/optional.hpp>
#include <qi/atomic.hpp>
#include <qi/types.hpp>

namespace qi
{
  class BufferPrivate
  {
  public:
    BufferPrivate();
    BufferPrivate(const BufferPrivate&);
    ~BufferPrivate();
    void* operator new(size_t);
    void operator delete(void*);
    BufferPrivate& operator=(const BufferPrivate&);
    unsigned char* data();
    const unsigned char* data() const;
    bool            resize(size_t size = 0x100000);
    boost::optional<size_t> indexOfSubBuffer(size_t offset) const;
    friend bool operator==(const BufferPrivate& a, const BufferPrivate& b);

  public:
    unsigned char*  _bigdata = nullptr;
    unsigned char   _data[STATIC_BLOCK] = {};
    size_t          _cachedSubBufferTotalSize = 0u;
    size_t          used = 0u; // size used
    size_t          available = std::extent<decltype(_data)>::value; // total size of buffer

    std::vector<std::pair<size_t, Buffer> > _subBuffers;
  };
}

#endif  // _SRC_BUFFER_P_HPP_
