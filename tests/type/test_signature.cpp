/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/

#ifdef _MSC_VER
   // C4503 decorated name length exceeded, name was truncated
   // The only workaround is to make structs to hide the template complexity
   // We don't want to have to do that
#  pragma warning( disable: 4503 )
#endif

#include <gtest/gtest.h>
#define __QI_SIGNATURE_UNKNOWN_INSTEAD_OF_ASSERT

#include <qi/signature.hpp>
#include <qi/anyvalue.hpp>
#include <qi/jsoncodec.hpp>

#include <vector>
#include <map>

namespace qi
{
  // OLD API compat layer for this test.
  template<typename T> struct signatureFromType
  {
    static std::string value()
    {
      return typeOf<T>()->signature().toString();
    }
  };
  struct signatureFromObject
  {
    template<typename T> static std::string value(const T& ptr)
    {
      return typeOf(ptr)->signature().toString();
    }
  };
}
class noSigForThis;
using MapInt = std::map<int, int>;

TEST(TestSignature, BasicTypeSignature) {
  EXPECT_EQ("b",    qi::signatureFromType<bool>::value());
  EXPECT_EQ("c",    qi::signatureFromType<char>::value());
  EXPECT_EQ("C",    qi::signatureFromType<unsigned char>::value());
  EXPECT_EQ("w",    qi::signatureFromType<short>::value());
  EXPECT_EQ("W",    qi::signatureFromType<unsigned short>::value());
  EXPECT_EQ("i",    qi::signatureFromType<int>::value());
  EXPECT_EQ("I",    qi::signatureFromType<unsigned int>::value());

  EXPECT_EQ("c",    qi::signatureFromType<qi::int8_t>::value());
  EXPECT_EQ("C",    qi::signatureFromType<qi::uint8_t>::value());
  EXPECT_EQ("w",    qi::signatureFromType<qi::int16_t>::value());
  EXPECT_EQ("W",    qi::signatureFromType<qi::uint16_t>::value());
  EXPECT_EQ("i",    qi::signatureFromType<qi::int32_t>::value());
  EXPECT_EQ("I",    qi::signatureFromType<qi::uint32_t>::value());
  EXPECT_EQ("l",    qi::signatureFromType<qi::int64_t>::value());
  EXPECT_EQ("L",    qi::signatureFromType<qi::uint64_t>::value());

  // long is platform-dependant and can't be tested.
  EXPECT_EQ("l",    qi::signatureFromType<long long>::value());
  EXPECT_EQ("L",    qi::signatureFromType<unsigned long long>::value());

  EXPECT_EQ("f",    qi::signatureFromType<float>::value());
  EXPECT_EQ("d",    qi::signatureFromType<double>::value());
  EXPECT_EQ("s",    qi::signatureFromType<std::string>::value());
  EXPECT_EQ("[i]",  qi::signatureFromType< std::vector<int> >::value());
  EXPECT_EQ("{ii}", qi::signatureFromType< MapInt >::value() );
}

TEST(TestSignature, SignatureSize) {
  qi::Signature s("(iiii)");
  EXPECT_EQ(4u, s.children().size());
  EXPECT_EQ(0u, s.children().at(0).children().size());

  s = qi::Signature("[i]");
  EXPECT_EQ(1u, s.children().size());

  s = qi::Signature("{ii}");
  EXPECT_EQ(2u, s.children().size());
}

TEST(TestSignature, TypeConstRefPointerMix) {

  EXPECT_EQ("b",    qi::signatureFromType<bool>::value());
  EXPECT_EQ("c",    qi::signatureFromType<char>::value());
  EXPECT_EQ("i",    qi::signatureFromType<int>::value());
  EXPECT_EQ("f",    qi::signatureFromType<float>::value());
  EXPECT_EQ("d",    qi::signatureFromType<double>::value());
  EXPECT_EQ("s",    qi::signatureFromType<std::string>::value());
  EXPECT_EQ("[i]",  qi::signatureFromType< std::vector<int> >::value());
  EXPECT_EQ("{ii}", qi::signatureFromType< MapInt >::value() );

  EXPECT_EQ("b",    qi::signatureFromType<const bool>::value());
  EXPECT_EQ("c",    qi::signatureFromType<const char>::value());
  EXPECT_EQ("i",    qi::signatureFromType<const int>::value());
  EXPECT_EQ("f",    qi::signatureFromType<const float>::value());
  EXPECT_EQ("d",    qi::signatureFromType<const double>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const std::string>::value());
  EXPECT_EQ("[i]",  qi::signatureFromType<const std::vector< int > >::value());
  EXPECT_EQ("{ii}", qi::signatureFromType<const MapInt >::value());

  EXPECT_EQ("b",    qi::signatureFromType<const bool&>::value());
  EXPECT_EQ("c",    qi::signatureFromType<const char&>::value());
  EXPECT_EQ("i",    qi::signatureFromType<const int&>::value());
  EXPECT_EQ("f",    qi::signatureFromType<const float&>::value());
  EXPECT_EQ("d",    qi::signatureFromType<const double&>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const std::string&>::value());
  EXPECT_EQ("[i]",  qi::signatureFromType<const std::vector< int >& >::value());
  EXPECT_EQ("{ii}", qi::signatureFromType<const MapInt& >::value());
}

TEST(TestSignature, Bools) {
  EXPECT_EQ("b",    qi::signatureFromType<bool>::value());
  EXPECT_EQ("b",    qi::signatureFromType<bool&>::value());
  EXPECT_EQ("b",    qi::signatureFromType<const bool>::value());
  EXPECT_EQ("b",    qi::signatureFromType<const bool&>::value());
}

TEST(TestSignature, Strings) {
  EXPECT_EQ("s",    qi::signatureFromType<std::string>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const std::string>::value());

  EXPECT_EQ("s",    qi::signatureFromType<char *>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const char *>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const char * const>::value());
  EXPECT_EQ("s",    qi::signatureFromType<char const * const>::value());
  EXPECT_EQ("s",    qi::signatureFromType<char *&>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const char *&>::value());
  EXPECT_EQ("s",    qi::signatureFromType<const char * const&>::value());
  EXPECT_EQ("s",    qi::signatureFromType<char const * const&>::value());
}


TEST(TestSignature, VArgs) {
  EXPECT_EQ("#m",    qi::signatureFromType<qi::AnyVarArguments>::value());

  //vargs
  EXPECT_TRUE(qi::Signature("#b").isValid());
  EXPECT_TRUE(qi::Signature("#b<titi,toto>").isValid());

  //kwargs
  EXPECT_TRUE(qi::Signature("~b").isValid());
  EXPECT_TRUE(qi::Signature("~b<titi,toto>").isValid());
}

struct MPoint {
  MPoint(int x=0, int y=0)
    : x(x)
    , y(y)
  {}
  int x;
  int y;
};
QI_TYPE_STRUCT(MPoint, x, y);

TEST(TestSignature, NamedTuple) {
  EXPECT_EQ("(ii)<MPoint,x,y>", qi::typeOf<MPoint>()->signature().toString());
}

TEST(TestSignature, ComplexTypeSignature) {
  //{ii}
  using MapInt = std::map<int, int>;
  //{{ii}{ii}}
  using MapInt2 = std::map<MapInt, MapInt>;
  using VectorMapInt2 = std::vector<MapInt2>;

  // MapInt2& Does not works
  using MapInt2Ref = MapInt2;

  using VectMapInt2Ref = std::vector<MapInt2Ref>;

  // const VectMapInt2Ref does not works
  using VectMapInt2RefConst = VectMapInt2Ref;

  using VectVectMapInt2ConstRef = std::vector<VectMapInt2RefConst>;

  using FuckinMap = std::map<VectorMapInt2, VectVectMapInt2ConstRef>;

  //{[{{ii}{ii}}][[{{ii}{ii}}&]#]}
  //and obama said: Yes We Can!
  EXPECT_EQ("{ii}"                        , qi::typeOf<MapInt>()->signature().toString());
  EXPECT_EQ("{{ii}{ii}}"                  , qi::typeOf<MapInt2>()->signature().toString());
  EXPECT_EQ("[{{ii}{ii}}]"                , qi::typeOf<VectorMapInt2>()->signature().toString());
  EXPECT_EQ("{{ii}{ii}}"                  , qi::typeOf<MapInt2Ref>()->signature().toString());
  EXPECT_EQ("[{{ii}{ii}}]"                , qi::typeOf<VectMapInt2Ref>()->signature().toString());
  EXPECT_EQ("[{{ii}{ii}}]"                , qi::typeOf<VectMapInt2RefConst>()->signature().toString());
  EXPECT_EQ("[[{{ii}{ii}}]]"              , qi::typeOf<VectVectMapInt2ConstRef>()->signature().toString());
  EXPECT_EQ("{[{{ii}{ii}}][[{{ii}{ii}}]]}", qi::typeOf<FuckinMap>()->signature().toString());
}

TEST(TestSignature, FromObject) {
  int myint = 42;
  EXPECT_EQ("i", qi::signatureFromObject::value(myint));
}

TEST(TestSignature, ComplexConstRefPtr) {
  EXPECT_EQ("f",    qi::signatureFromType<float>::value());
  EXPECT_EQ("f",    qi::signatureFromType<float&>::value());
  EXPECT_EQ("f",    qi::signatureFromType<const float &>::value());
  EXPECT_EQ("f",    qi::signatureFromType<const float>::value());
}

TEST(TestSignature, Equal) {
  EXPECT_EQ(qi::signatureFromType<float&>::value(), qi::signatureFromType<float>::value());
  EXPECT_TRUE(qi::Signature("[s]") == qi::Signature("[s]"));
  EXPECT_TRUE(qi::Signature("(ss)<Point,x,y>") == qi::Signature("(ss)")); // really?

  EXPECT_TRUE(qi::Signature("(mm)") != "(mmm)");
  EXPECT_TRUE(qi::Signature("(mm)") != "(m)");
}

TEST(TestSignature, InvalidSignature) {


  //empty signature are invalid
  EXPECT_THROW(qi::Signature(""), std::runtime_error);
  EXPECT_THROW(qi::Signature("("), std::runtime_error);
  EXPECT_THROW(qi::Signature("{"), std::runtime_error);
  EXPECT_THROW(qi::Signature("["), std::runtime_error);
  EXPECT_THROW(qi::Signature("plafbim"), std::runtime_error);
  EXPECT_THROW(qi::Signature("(m)(sib)"), std::runtime_error);
  EXPECT_THROW(qi::Signature("ddd"), std::runtime_error);
  EXPECT_THROW(qi::Signature("(mm"), std::runtime_error);
}

TEST(TestSignature, InvalidNumberOfArgs) {
  //empty signature are invalid
  EXPECT_THROW(qi::Signature("[iii]"), std::runtime_error);
  EXPECT_THROW(qi::Signature("{iii}"), std::runtime_error);
  EXPECT_THROW(qi::Signature("{i}"), std::runtime_error);
}


TEST(TestSignature, FromString) {
  qi::Signature *sig;

  sig = new qi::Signature();
  EXPECT_FALSE(sig->isValid());
  delete sig;


  sig = new qi::Signature("(s)");
  EXPECT_TRUE(sig->isValid());
  delete sig;

  sig = new qi::Signature("()");
  EXPECT_TRUE(sig->isValid());
  delete sig;

  sig = new qi::Signature("()<>");
  EXPECT_TRUE(sig->isValid());
  delete sig;

  ASSERT_NO_THROW(sig = new qi::Signature("(((sIsI[(sssW)]s)))"));
  EXPECT_TRUE(sig->isValid());
  delete sig;

  ASSERT_NO_THROW(sig = new qi::Signature("({I(Isss[(ss)]s)}{I(Is)}{I(Iss)}s)"));
  EXPECT_TRUE(sig->isValid());
  delete sig;
}

TEST(TestSignature, SignatureSplitError) {
  std::vector<std::string> sigInfo;


  sigInfo = qi::signatureSplit("reply");
  EXPECT_EQ("", sigInfo[0]);
  EXPECT_EQ("reply", sigInfo[1]);
  EXPECT_EQ("", sigInfo[2]);

  EXPECT_ANY_THROW(qi::signatureSplit("reply::"));
  EXPECT_ANY_THROW(qi::signatureSplit("reply::("));
  EXPECT_ANY_THROW(qi::signatureSplit("reply::e"));

  EXPECT_ANY_THROW(qi::signatureSplit("reply::(RRRR)"));
  EXPECT_NO_THROW(qi::signatureSplit("reply::(sssss)"));

  EXPECT_ANY_THROW(qi::signatureSplit("reply::([sss])"));
  EXPECT_NO_THROW(qi::signatureSplit("reply::([s])"));

  EXPECT_ANY_THROW(qi::signatureSplit("titi::s([[xxx]])"));
  EXPECT_ANY_THROW(qi::signatureSplit("titi::s([[sss]])"));
  EXPECT_NO_THROW(qi::signatureSplit("titi::s([[s]])"));

  EXPECT_ANY_THROW(qi::signatureSplit("titi::s([({ii}s[ss])])"));
  EXPECT_NO_THROW(qi::signatureSplit("titi::s([({ii}s[s])])"));

  EXPECT_NO_THROW(qi::signatureSplit("titi::s({i{is}})"));
}

TEST(TestSignature, IsCompatible) {
  qi::Signature s("(s[m])");
  EXPECT_EQ(s.isConvertibleTo("(si)"), 0.);
  EXPECT_EQ(s.isConvertibleTo("(sf)"), 0.);
  EXPECT_EQ(s.isConvertibleTo("(ss)"), 0.);
  EXPECT_GT(s.isConvertibleTo("(sm)"), 0.);

  qi::Signature s2("(s)");
  EXPECT_EQ(s2.isConvertibleTo("(si)"), 0.);
  EXPECT_GT(s2.isConvertibleTo("(m)"), 0.);

  qi::Signature s3("([s])");
  EXPECT_GT(s3.isConvertibleTo("([m])"), s3.isConvertibleTo("(m)"));
}

TEST(TestSignature, SignatureSplit) {
  std::vector<std::string> sigInfo;

  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("reply::(s)"));
  EXPECT_EQ("", sigInfo[0]);
  EXPECT_EQ("reply", sigInfo[1]);
  EXPECT_EQ("(s)", sigInfo[2]);

  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("reply::s(s)"));
  EXPECT_EQ("s", sigInfo[0]);
  EXPECT_EQ("reply", sigInfo[1]);
  EXPECT_EQ("(s)", sigInfo[2]);


  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("info::(m)(sib)"));
  EXPECT_EQ("(m)", sigInfo[0]);
  EXPECT_EQ("info", sigInfo[1]);
  EXPECT_EQ("(sib)", sigInfo[2]);

  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("toto::((([{ii}])))({[{{ii}{ii}}][[{{ii}{ii}}]]})"));
  EXPECT_EQ("((([{ii}])))", sigInfo[0]);
  EXPECT_EQ("toto", sigInfo[1]);
  EXPECT_EQ("({[{{ii}{ii}}][[{{ii}{ii}}]]})", sigInfo[2]);

  ASSERT_NO_THROW(sigInfo = qi::signatureSplit("toto"));
  EXPECT_EQ("", sigInfo[0]);
  EXPECT_EQ("toto", sigInfo[1]);
  EXPECT_EQ("", sigInfo[2]);
}

TEST(TestSignature, ItAnnotation) {
  std::string orig("((m)<Plouf,x>(scf)<Point3d,x,y,z>)");
  qi::Signature sig(orig);
  qi::SignatureVector subsig = sig.children();

  EXPECT_EQ(orig, sig.toString());
  EXPECT_EQ("(m)<Plouf,x>", subsig[0].toString());
  EXPECT_EQ("(scf)<Point3d,x,y,z>", subsig[1].toString());
}

TEST(TestSignature, TestToSTLType)
{
  qi::Signature sig("(s)");
  EXPECT_EQ("(String)", sig.toPrettySignature());

  qi::Signature sig2("(slb)");
  EXPECT_EQ("(String,Int64,Bool)", sig2.toPrettySignature());

  qi::Signature sig3("{is}");
  EXPECT_EQ("Map<Int32,String>", sig3.toPrettySignature());

  qi::Signature sig4("((m)(scf))");
  EXPECT_EQ("((Value),(String,Int8,Float))", sig4.toPrettySignature());

  qi::Signature sig5("((m)<Plouf,x>(scf)<Point3d,x,y,z>)");
  EXPECT_EQ("(Plouf,Point3d)", sig5.toPrettySignature());
}

//expect that the following test to do not build. (static assert)
static int gGlobalResult = 0;

void vfun0()                                                                                      { gGlobalResult = 0; }
void vfun1(const int &p0)                                                                         { gGlobalResult = p0; }
void vfun2(const int &p0,const int &p1)                                                           { gGlobalResult = p0 + p1; }
void vfun3(const int &p0,const int &p1,const int &p2)                                             { gGlobalResult = p0 + p1 + p2; }
void vfun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { gGlobalResult = p0 + p1 + p2 + p3; }
void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { gGlobalResult = p0 + p1 + p2 + p3 + p4; }
void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }

int fun0()                                                                                      { return 0; }
int fun1(const int &p0)                                                                         { return p0; }
int fun2(const int &p0,const int &p1)                                                           { return p0 + p1; }
int fun3(const int &p0,const int &p1,const int &p2)                                             { return p0 + p1 + p2; }
int fun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { return p0 + p1 + p2 + p3; }
int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { return p0 + p1 + p2 + p3 + p4; }
int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { return p0 + p1 + p2 + p3 + p4 + p5; }


struct Foo {
  void voidCall()                                          { return; }
  int intStringCall(const std::string &plouf)              { return plouf.size(); }

  int fun0()                                                                                      { return 0; }
  int fun1(const int &p0)                                                                         { return p0; }
  int fun2(const int &p0,const int &p1)                                                           { return p0 + p1; }
  int fun3(const int &p0,const int &p1,const int &p2)                                             { return p0 + p1 + p2; }
  int fun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { return p0 + p1 + p2 + p3; }
  int fun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { return p0 + p1 + p2 + p3 + p4; }
  int fun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { return p0 + p1 + p2 + p3 + p4 + p5; }

  void vfun0()                                                                                      { gGlobalResult = 0; }
  void vfun1(const int &p0)                                                                         { gGlobalResult = p0; }
  void vfun2(const int &p0,const int &p1)                                                           { gGlobalResult = p0 + p1; }
  void vfun3(const int &p0,const int &p1,const int &p2)                                             { gGlobalResult = p0 + p1 + p2; }
  void vfun4(const int &p0,const int &p1,const int &p2,const int &p3)                               { gGlobalResult = p0 + p1 + p2 + p3; }
  void vfun5(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4)                { gGlobalResult = p0 + p1 + p2 + p3 + p4; }
  void vfun6(const int &p0,const int &p1,const int &p2,const int &p3,const  int &p4,const  int &p5) { gGlobalResult = p0 + p1 + p2 + p3 + p4 + p5; }
};



TEST(TestSignature, WeirdPointerTypes) {
  EXPECT_EQ("X", qi::signatureFromType<bool*>::value());
  EXPECT_EQ("X", qi::signatureFromType<bool*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const bool*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const bool*&>::value());

  EXPECT_EQ("X", qi::signatureFromType<float *>::value());
  EXPECT_EQ("X", qi::signatureFromType<float const *>::value());
  EXPECT_EQ("X", qi::signatureFromType<float * const>::value());
  EXPECT_EQ("X", qi::signatureFromType<float const * const>::value());
}
//not instanciable
//TEST(TestSignature, noSignature) {
//  EXPECT_EQ("X", qi::signatureFromType<noSigForThis>::value());
//  EXPECT_EQ("X", qi::signatureFromType<void (int, int)>::value());
//}

TEST(TestSignature, WeirdBasicFunctionSignature) {
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(&fun6));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(fun6));
}

TEST(TestSignature, WeirdPointers) {
  EXPECT_EQ("X", qi::signatureFromType<const bool*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const int*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const float*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const double*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const std::string*>::value());
  EXPECT_EQ("X", qi::signatureFromType<const std::vector< int >* >::value());
  EXPECT_EQ("X", qi::signatureFromType<const MapInt* >::value());

  EXPECT_EQ("X", qi::signatureFromType<const bool*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const int*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const float*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const double*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const std::string*&>::value());
  EXPECT_EQ("X", qi::signatureFromType<const std::vector< int >*& >::value());
  EXPECT_EQ("X", qi::signatureFromType<const MapInt*& >::value());
}


TEST(TestSignature, FunctionType) {
  EXPECT_EQ("X", qi::signatureFromType<void (*)(int, int)>::value());
}

TEST(TestSignature, BasicVoidFunctionSignature) {
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(&vfun6));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(vfun6));

}

//dont ask me why, but member function evaluates to bool :(
TEST(TestSignature, WeirdBasicVoidMemberSignature) {
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::vfun6));
}

TEST(TestSignature, WeirdBasicMemberSignature) {
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun0));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun1));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun2));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun3));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun4));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun5));
  EXPECT_EQ("X", qi::signatureFromObject::value(&Foo::fun6));
}

//yes shared_ptr evaluate to bool
TEST(TestSignature, WeirdSharedPtr) {
  EXPECT_EQ("X", qi::signatureFromType< boost::shared_ptr<int> >::value());
  EXPECT_EQ("X", qi::signatureFromType< boost::shared_ptr<int> &>::value());
}

qiLogCategory("test.signature()");

void stackIt(std::vector<qi::Signature>& stack, const qi::Signature& sig) {
  qiLogInfo() << "Pushing:" << sig.toString() << ", annot:" << sig.annotation();
  qi::SignatureVector child = sig.children();
  for (qi::SignatureVector::const_reverse_iterator it = child.rbegin(); it != child.rend(); ++it) {
    stackIt(stack, *it);
  }
  stack.push_back(sig);
}

class SignatureValidator
{
public:

  SignatureValidator(qi::Signature s)
  : good(true), count(0)
  {
    stackIt(stack, s);
  }

  SignatureValidator& operator()(char t, const char* annotation)
  {
    if (!good)
      return *this;
    if (stack.empty())
    {
      qiLogError() << "No more elements after " << count;
      good = false;
      return *this;
    }
    qi::Signature& schi = stack.back();
    if (schi.type() != t)
    {
      qiLogError() << "Type mismatch " << t << " " << schi.type();
      good = false;
      return *this;
    }
    if (schi.annotation() != annotation)
    {
      qiLogError() << "Annotation mismatch " << annotation << " vs " << schi.annotation();
      good = false;
      return *this;
    }
    stack.pop_back();
    return *this;
  }

  operator bool() const
  {
    if (!stack.empty())
    {
      qiLogError() << "Remaining elements at the end";
      good = false;
    }
    return good;
  }

  std::vector<qi::Signature> stack;
  mutable bool good;
  int count;
};


qi::Signature tuple(const std::string& str)
{
  return qi::Signature("(" + str + ")");
}

TEST(TestSignature, Annotation) {
  using qi::Signature;
  Signature s("i<foo>");
  EXPECT_EQ("foo", s.annotation());
  EXPECT_EQ('i', s.type());

  s = Signature("i<foo<bar><<baz@5'\"[]><>a>>");
  EXPECT_EQ('i', s.type());
  EXPECT_EQ("foo<bar><<baz@5'\"[]><>a>", s.annotation());

  s = Signature("(ii<foo>i<bar>ii<bam<baz>>)");
  qiLogInfo() << "sig: " << s.toString();
  EXPECT_EQ(5u, s.children().size());
  EXPECT_TRUE(
    SignatureValidator(s)
    ('(', "")
    ('i', "")
    ('i', "foo")
    ('i', "bar")
    ('i', "")
    ('i', "bam<baz>")
    );
EXPECT_TRUE(SignatureValidator(tuple("i<foo>[i<bar>]<baz>{i<bim>I<bam>}<boum>(i<pif>I<paf>)<pouf>"))
  ('(', "")
  ('i', "foo")
  ('[', "baz")
  ('i', "bar")
  ('{', "boum")
  ('i', "bim")
  ('I', "bam")
  ('(', "pouf")
  ('i', "pif")
  ('I', "paf")
  );
 EXPECT_TRUE(SignatureValidator("[[[[i<a>]<b>]<c>]<d>]<e>")
   ('[', "e")
   ('[', "d")
   ('[', "c")
   ('[', "b")
   ('i', "a")
 );
 // Test the test-suite tool
 EXPECT_FALSE(SignatureValidator("i")('I', ""));
 EXPECT_FALSE(SignatureValidator("i"));
 EXPECT_FALSE(SignatureValidator(tuple("ii"))('i', ""));
 EXPECT_FALSE(SignatureValidator("i")('i', "coin"));
 EXPECT_FALSE(SignatureValidator("i<coin>")('i', ""));
}

TEST(TestSignature, AnnotationInvalid) {
  using qi::Signature;
  EXPECT_THROW(Signature("i<foo")  , std::runtime_error);
  EXPECT_THROW(Signature("ifoo>")  , std::runtime_error);
  EXPECT_THROW(Signature("[ifoo>]"), std::runtime_error);
  EXPECT_THROW(Signature("[i]>")   , std::runtime_error);
  EXPECT_THROW(Signature("<>")     , std::runtime_error);
  EXPECT_THROW(Signature(">")      , std::runtime_error);
  EXPECT_THROW(Signature("<")      , std::runtime_error);
}

struct Point
{
  double x;
  double y;
  std::string name;
};
QI_TYPE_STRUCT(Point, x, y, name);

TEST(TestSignature, AnnotationStruct)
{
  EXPECT_EQ("(dds)<Point,x,y,name>", qi::typeOf<Point>()->signature().toString());
}

TEST(TestSignature, AnnotatedStructCompatibleWithStringStringMap)
{
  qi::Signature s("(s)<Phrase,text>");
  EXPECT_EQ(0., s.isConvertibleTo("{ss}"));
}

TEST(TestSignature, AnnotatedStructCompatibleWithStringIntMap)
{
  qi::Signature s("(ii)<Locale,language,region>");
  EXPECT_EQ(0., s.isConvertibleTo("{si}"));
}

TEST(TestSignature, SeveralAnnotatedStructsCompatibleWithSeveralMaps)
{
  qi::Signature s("((s)<Phrase,text>(ii)<Locale,language,region>)");
  EXPECT_EQ(0., s.isConvertibleTo("({ss}{si})"));
}

TEST(TestSignature, ObjectAndSeveralAnnotatedStructsCompatibleWithObjectAndSeveralMaps)
{
  qi::Signature s("(o(s)<Phrase,text>(ii)<Locale,language,region>)");
  EXPECT_EQ(0., s.isConvertibleTo("(o{ss}{si})"));
}

std::string trimall(const std::string& s)
{
  std::string res;
  for (unsigned i=0; i<s.size(); ++i)
    if (s[i] != ' ')
      res += s[i];
  return res;
}

std::string unarmor(const std::string& s)
{
  return trimall(s.substr(1, s.size()-2));
}

TEST(TestSignature, ToData)
{
  #define j(a) trimall(qi::encodeJSON(qi::Signature(a).toData()))
  #define S(a) unarmor(#a)
  EXPECT_EQ(j("i"),             S(([ "i", [], "" ])));
  EXPECT_EQ(j("i<foo>"),        S(([ "i", [], "foo" ])));
  EXPECT_EQ(j("[i<foo>]<bar>"), S((["[", [[ "i", [], "foo" ]], "bar"])));
  EXPECT_EQ(j("(i[f]{if})"),
    S(( ["(", [["i", [], ""],["[", [["f", [], ""]], ""],["{", [["i", [], ""], ["f", [], ""]], "" ]], "" ])));
}


#define verif_iter(_it, _sig, _type, _hasChildren) \
{\
  EXPECT_STREQ(_sig, (_it).toString().c_str());\
  EXPECT_TRUE(_hasChildren == (_it).hasChildren()); \
  EXPECT_EQ(qi::Signature::_type, (_it).type());\
}


TEST(TestSignatureIterator, Simple) {

  qi::SignatureVector::iterator it;

  qi::Signature signature("(is)");
  qi::SignatureVector sig = signature.children();

  EXPECT_TRUE(signature.isValid());
  EXPECT_TRUE(sig.size() == 2);
  it = sig.begin();
  verif_iter(*it, "i", Type_Int32, false);
  ++it;
  verif_iter(*it, "s", Type_String, false);
}

TEST(TestSignatureIterator, Empty) {
  qi::Signature sig3;

  EXPECT_TRUE(sig3.children().size() == 0);
  verif_iter(sig3, "", Type_None, false);
  EXPECT_STREQ("", sig3.toString().c_str());
}



//#endif
