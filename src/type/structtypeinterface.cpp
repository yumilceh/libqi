/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qi/type/typeinterface.hpp>
#include <qi/anyvalue.hpp>
#include <qi/numeric.hpp>

namespace qi
{
  namespace detail {
    std::string normalizeClassName(const std::string &name) {
      //::qi::Foo -> Foo
      size_t id = name.rfind("::");
      if (id != std::string::npos)
        return name.substr(id + 2);
      else return name;
    }
  }

  AnyReferenceVector StructTypeInterface::values(void* storage)
  {
    std::vector<TypeInterface*> types = memberTypes();
    std::vector<void*> values = get(storage);
    AnyReferenceVector result;
    for (unsigned i=0; i<types.size(); ++i)
      result.push_back(AnyReference(types[i], values[i]));
    return result;
  }

  std::vector<void*> StructTypeInterface::get(void* storage)
  {
    std::vector<void*> result;
    const auto count = qi::numericConvert<unsigned int>(memberTypes().size());
    for (auto i = 0u; i < count; ++i)
      result.push_back(get(storage, i));
    return result;
  }

  void StructTypeInterface::set(void** storage, const std::vector<void*>& values)
  {
    for (unsigned i=0; i<values.size(); ++i)
      set(storage, i, values[i]);
  }
}
