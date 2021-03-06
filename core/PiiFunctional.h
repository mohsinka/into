/* This file is part of Into.
 * Copyright (C) Intopii 2013.
 * All rights reserved.
 *
 * Licensees holding a commercial Into license may use this file in
 * accordance with the commercial license agreement. Please see
 * LICENSE.commercial for commercial licensing terms.
 *
 * Alternatively, this file may be used under the terms of the GNU
 * Affero General Public License version 3 as published by the Free
 * Software Foundation. In addition, Intopii gives you special rights
 * to use Into as a part of open source software projects. Please
 * refer to LICENSE.AGPL3 for details.
 */

#ifndef _PIIFUNCTIONAL_H
#define _PIIFUNCTIONAL_H

#include "PiiMetaTemplate.h"
#include "PiiTypeTraits.h"
#ifdef PII_CXX11
#  include <utility>
#  include <functional>
#endif

namespace Pii
{
  /**
   * @group functional Function Objects
   *
   * Useful function objects missing from the standard library. The
   * `typedefs` required in adaptable functions are provided both in
   * STL and in Intopii/Qt style. The function adaptors are modeled
   * after SGI extensions to STL.
   */

  /**
   * An stl-compatible model for a generator function that takes no
   * arguments.
   */
  template <class Result> struct Generator
  {
    /**
     * Typedef for the result type.
     */
    typedef Result ResultType;
    /**
     * Stl-style typedef for the result type.
     */
    typedef Result result_type;
  };

  /**
   * An stl-compatible model for an adaptable unary function.
   */
  template <class Arg, class Result = Arg> struct UnaryFunction
  {
    /**
     * Typedef for the argument type.
     */
    typedef Arg ArgumentType;
    /**
     * Stl-style typedef for the argument type.
     */
    typedef Arg argument_type;
    /**
     * Typedef for the result type.
     */
    typedef Result ResultType;
    /**
     * Stl-style typedef for the result type.
     */
    typedef Result result_type;
  };

  /**
   * An stl-compatible model for an adaptable binary function.
   */
  template <class Arg1, class Arg2 = Arg1, class Result = Arg1> struct BinaryFunction
  {
    /**
     * Typedef for the first argument type.
     */
    typedef Arg1 FirstArgumentType;
    /**
     * Stl-style typedef for the first argument type.
     */
    typedef Arg1 first_argument_type;
    /**
     * Typedef for the second argument type.
     */
    typedef Arg2 SecondArgumentType;
    /**
     * Stl-style typedef for the second argument type.
     */
    typedef Arg2 second_argument_type;
    /**
     * Typedef for the result type.
     */
    typedef Result ResultType;
    /**
     * Stl-style typedef for the result type.
     */
    typedef Result result_type;
  };

  /**
   * A model for an adaptable ternary function.
   */
  template <class Arg1, class Arg2 = Arg1, class Arg3 = Arg1, class Result = Arg1> struct TernaryFunction
  {
    /**
     * Typedef for the first argument type.
     */
    typedef Arg1 FirstArgumentType;
    /**
     * Stl-style typedef for the first argument type.
     */
    typedef Arg1 first_argument_type;
    /**
     * Typedef for the second argument type.
     */
    typedef Arg2 SecondArgumentType;
    /**
     * Stl-style typedef for the second argument type.
     */
    typedef Arg2 second_argument_type;
    /**
     * Typedef for the third argument type.
     */
    typedef Arg3 ThirdArgumentType;
    /**
     * Stl-style typedef for the third argument type.
     */
    typedef Arg3 third_argument_type;
    /**
     * Typedef for the result type.
     */
    typedef Result ResultType;
    /**
     * Stl-style typedef for the result type.
     */
    typedef Result result_type;
  };

  /**
   * An adaptable binary functor that performs the binary OR operation.
   */
  template <class T> struct BinaryOr : public BinaryFunction<T>
  {
    T operator() (const T& a, const T& b) const { return a | b; }
  };
  /**
   * An adaptable binary functor that performs the binary AND
   * operation.
   */
  template <class T> struct BinaryAnd : public BinaryFunction<T>
  {
    T operator() (const T& a, const T& b) const { return a & b; }
  };
  /**
   * An adaptable binary functor that performs the binary XOR
   * operation.
   */
  template <class T> struct BinaryXor : public BinaryFunction<T>
  {
    T operator() (const T& a, const T& b) const { return a ^ b; }
  };
  /**
   * An adaptable binary unary functor that performs the binary NOT
   * operation.
   */
  template <class T> struct BinaryNot : public BinaryFunction<T>
  {
    T operator() (const T& a) const { return ~a; }
  };
  /**
   * An adaptable binary binary functor that shifts the left operand
   * left as many times as indicated by the right operand.
   */
  template <class T> struct BinaryShiftLeft : public BinaryFunction<T,int>
  {
    T operator() (const T& a, int b) const { return a << b; }
  };
  /**
   * An adaptable binary binary functor that shifts the left operand
   * right as many times as indicated by the right operand.
   */
  template <class T> struct BinaryShiftRight : public BinaryFunction<T,int>
  {
    T operator() (const T& a, int b) const { return a >> b; }
  };

  /**
   * An adaptable unary function that casts its argument from type
   * `From` to type `To`.
   */
  template <class From, class To> struct Cast : public UnaryFunction<From,To>
  {
    To operator() (const From& value) const { return To(value); }
  };

  template <class T> struct ArrayLookup :
  public UnaryFunction<int, T>
  {
    ArrayLookup(T* array) : array(array) {}
    T& operator() (int i) const { return array[i]; }
    T* array;
  };

  template <class T>
  ArrayLookup<T> arrayLookup(T* array)
  {
    return ArrayLookup<T>(array);
  }

  /**
   * A unary function adaptor that makes the result of one unary
   * function the argument of another. If function 1 is f(x) and
   * function 2 is g(x), the composition returns f(g(x)).
   *
   * ~~~(c++)
   * PiiMatrix<double> mat(1,5, 1.0, 2.0, 3.0, 4.0, 5.0);
   * // Calculate -sin(x)
   * mat.map(Pii::unaryCompose(std::negate(), std::ptr_fun(sin)));
   * ~~~
   *
   * @see unaryCompose()
   * @see BinaryCompose
   */
  template <class AdaptableUnaryFunction1, class AdaptableUnaryFunction2>
  class UnaryCompose : public UnaryFunction<typename AdaptableUnaryFunction2::argument_type,
                                            typename AdaptableUnaryFunction1::result_type>
  {
  public:
    /**
     * Construct a UnaryCompose object. It is seldom necessary to use
     * the constructor directly. Use the [unaryCompose()] function
     * instead.
     */
    UnaryCompose(const AdaptableUnaryFunction1& op1,
                 const AdaptableUnaryFunction2& op2) :
      firstOp(op1), secondOp(op2)
    {}

    typename AdaptableUnaryFunction1::result_type operator()(const typename AdaptableUnaryFunction2::argument_type& value) const
    {
      return firstOp(secondOp(value));
    }

    /**
     * The first function.
     */
    AdaptableUnaryFunction1 firstOp;
    /**
     * The second function.
     */
    AdaptableUnaryFunction2 secondOp;
  };

  /**
   * Construct a composed unary function (UnaryCompose) out of two
   * adaptable unary functions.
   *
   * @relates UnaryCompose
   */
  template <class AdaptableUnaryFunction1, class AdaptableUnaryFunction2>
  inline UnaryCompose<AdaptableUnaryFunction1, AdaptableUnaryFunction2>
  unaryCompose(const AdaptableUnaryFunction1& op1, const AdaptableUnaryFunction2& op2)
  {
    return UnaryCompose<AdaptableUnaryFunction1,AdaptableUnaryFunction2>(op1, op2);
  }

  /**
   * A unary/binary function adaptor that makes the results of two
   * unary functions the arguments of a binary function. If the binary
   * function is f(x,y) and the two unary functions are g(x) and h(x),
   * the unary composition returns f(g(x),h(x)). If the object is used
   * as a binary function, the composition returns f(g(x), h(y)).
   *
   * ~~~(c++)
   * PiiMatrix<double> mat(1,5, 1.0, 2.0, 3.0, 4.0, 5.0);
   * // Calculate sin(mat) + cos(mat)
   * mat.map(Pii::binaryCompose(std::plus<double>(), std::ptr_fun(sin), std::ptr_fun(cos)));
   *
   * PiiMatrix<double> mat2(1,5, -1.0, 2.0, -3.0, 4.0, -5.0);
   * // Calculate mat - sqrt(mat2)
   * mat.map(Pii::binaryCompose(std::minus<double>(), Pii::Identity<double>, Pii::Sqrt<double>()),
   *         mat2);
   * ~~~
   *
   * @see binaryCompose()
   * @see UnaryCompose
   */
  template <class AdaptableBinaryFunction, class AdaptableUnaryFunction1, class AdaptableUnaryFunction2>
  class BinaryCompose :
    public UnaryFunction<typename AdaptableUnaryFunction1::argument_type,
                         typename AdaptableBinaryFunction::result_type>,
    public BinaryFunction<typename AdaptableUnaryFunction1::argument_type,
                          typename AdaptableUnaryFunction2::argument_type,
                          typename AdaptableBinaryFunction::result_type>

  {
  public:
    /**
     * Construct a %BinaryCompose object. It is seldom necessary to use
     * the constructor directly. Use the [binaryCompose()] function
     * instead.
     */
    BinaryCompose(const AdaptableBinaryFunction& op1,
                  const AdaptableUnaryFunction1& op2,
                  const AdaptableUnaryFunction2& op3) :
      firstOp(op1), secondOp(op2), thirdOp(op3)
    {}

    typename AdaptableBinaryFunction::result_type operator()
      (const typename AdaptableUnaryFunction1::argument_type& value) const
    {
      return firstOp(secondOp(value), thirdOp(value));
    }

    typename AdaptableBinaryFunction::result_type operator()
      (const typename AdaptableUnaryFunction1::argument_type& value1,
       const typename AdaptableUnaryFunction2::argument_type& value2) const
    {
      return firstOp(secondOp(value1), thirdOp(value2));
    }

    /**
     * The first function.
     */
    AdaptableBinaryFunction firstOp;
    /**
     * The second function.
     */
    AdaptableUnaryFunction1 secondOp;
    /**
     * The third function.
     */
    AdaptableUnaryFunction2 thirdOp;
  };


  /**
   * Construct a composed unary function (BinaryCompose) out of an
   * adaptable binary function and two adaptable unary functions.
   *
   * @relates BinaryCompose
   */
  template <class AdaptableBinaryFunction, class AdaptableUnaryFunction1, class AdaptableUnaryFunction2>
  inline BinaryCompose<AdaptableBinaryFunction, AdaptableUnaryFunction1, AdaptableUnaryFunction2>
  binaryCompose(const AdaptableBinaryFunction& op1,
                const AdaptableUnaryFunction1& op2,
                const AdaptableUnaryFunction2& op3)
  {
    return BinaryCompose<AdaptableBinaryFunction, AdaptableUnaryFunction1, AdaptableUnaryFunction2>(op1, op2, op3);
  }

  /**
   * A unary function that increments/decrements its internal counter
   * by a predefined value each time the function is called.
   *
   * ~~~(c++)
   * PiiMatrix<float> matrix(1,3);
   * // Fill matrix
   * Pii::fill(matrix.begin(), matrix.end(), Pii::CountFunction<float>(0,2));
   * // matrix = 0 2 4
   * ~~~
   */
  template <class T> class CountFunction : public Generator<T>
  {
  public:
    /**
     * Create a new count function with the given initial value and
     * increment.
     */
    CountFunction(int initialValue = 0, int increment = 1) : _iCount(initialValue), _iIncrement(increment) {}

    /**
     * Increase the internal counter by the increment.
     */
    T operator() () const
    {
      int iCnt = _iCount;
      _iCount += _iIncrement;
      return iCnt;
    }

    /**
     * Get the current value of the internal counter.
     */
    int count() const { return _iCount; }

    /**
     * Set the value of the internal counter.
     */
    void setCount(int count) { _iCount = count; }

    /**
     * Get value of the increment.
     */
    int increment() const { return _iIncrement; }

    /**
     * Set the value of the increment.
     */
    void setIncrement(int increment) { _iIncrement = increment; }

  private:
    mutable int _iCount;
    int _iIncrement;
  };

  /**
   * A unary function that returns its argument: f(x) = x.
   */
  template <class T> struct Identity : public UnaryFunction<T>
  {
    T operator()(const T& value) const { return value; }
  };

  /**
   * A binary function that returns its first argument: f(x,y) = x.
   */
  template <class T, class U=T> struct SelectFirst : public BinaryFunction<T,U>
  {
    T operator()(const T& value1, const U&) const { return value1; }
  };

  /**
   * A binary function that returns its second argument: f(x,y) = y.
   */
  template <class T, class U=T> struct SelectSecond : public BinaryFunction<T,U,U>
  {
    U operator()(const T&, const U& value2) const { return value2; }
  };

  /**
   * A binary function that wraps another function and passes input
   * arguments to it in reverse order.
   */
  template <class Function>
  struct BinaryReverseArgs :
    public BinaryFunction<typename Function::second_argument_type,
                          typename Function::first_argument_type,
                          typename Function::result_type>
  {
    BinaryReverseArgs(const Function& function) : function(function) {}

    typename Function::result_type operator() (typename Function::second_argument_type a,
                                               typename Function::first_argument_type b) const
    {
      return function(b, a);
    }

    Function function;
  };

  template <class Function>
  inline BinaryReverseArgs<Function> binaryReverseArgs(const Function& function)
  {
    return BinaryReverseArgs<Function>(function);
  }

  /**
   * A unary function (predicate) that always returns `true`.
   */
  template <class T> struct YesFunction : public UnaryFunction<T,bool>
  {
    bool operator()(const T&) const { return true; }
  };

  /**
   * A unary function (predicate) that always returns `false`.
   */
  template <class T> struct NoFunction : public UnaryFunction<T,bool>
  {
    bool operator()(const T&) const { return false; }
  };

#ifdef PII_CXX11
  /// @internal
  template <class ReturnType, class Object, class... Args> struct MemberFunction
  {
    typedef ReturnType (Object::* MemberType)(Args...);

    MemberFunction(Object* object, MemberType member) :
      pObject(object), pMember(member) {}
    MemberFunction(Object& object, MemberType member) :
      pObject(&object), pMember(member) {}

    template <class... Params>
    ReturnType operator() (Params&&... params) const { return (pObject->*pMember)(std::forward<Params>(params)...); }

    Object* pObject;
    MemberType pMember;
  };

  /**
   * Returns a wrapper object that stores a pointer or a reference to
   * object and a pointer to a member function. The wrapper is a
   * function object that binds the *this* argument to a member
   * function and can thus be called as if it was an ordinary
   * function.
   *
   * ~~~(c++)
   * struct A
   * {
   *   int sum(int a, int b) { return a+b; }
   * };
   * // ...
   *
   * A a;
   * auto sum = Pii::memberFunction(a, &A::sum);
   * std::cout << sum(1, 2);
   * // Outputs "3"
   * ~~~
   */
  template <class ReturnType, class Instance, class Object, class... Args>
  MemberFunction<ReturnType, Object, Args...>
  memberFunction(Instance instance,
                 ReturnType (Object::* member)(Args...))
  {
    return MemberFunction<ReturnType, Object, Args...>(instance, member);
  }

  template <class ReturnType, class Instance, class Object, class... Args>
  MemberFunction<ReturnType, const Object, Args...>
  memberFunction(const Instance instance,
                 ReturnType (Object::* member)(Args...) const)
  {
    return MemberFunction<ReturnType, const Object, Args...>(instance, member);
  }

  template <std::size_t... Indices> struct IndexList {};

  template <std::size_t N, std::size_t... Indices>
  struct BuildIndices : BuildIndices<N-1, N-1, Indices...> {};

  template <std::size_t... Indices>
  struct BuildIndices<0, Indices...> : IndexList<Indices...> {};

  template <class... TupleArgs, class... Args>
  BuildIndices<sizeof...(TupleArgs)> buildIndices(const std::tuple<TupleArgs...>&, Args&&...)
  {
    return BuildIndices<sizeof...(TupleArgs)>{};
  }

  template <std::size_t N, std::size_t... Indices>
  struct BuildReverseIndices : BuildReverseIndices<N-1, Indices..., N-1> {};

  template <std::size_t... Indices>
  struct BuildReverseIndices<0, Indices...> : IndexList<Indices...> {};

  template <class... TupleArgs, class... Args>
  BuildReverseIndices<sizeof...(TupleArgs)> buildReverseIndices(const std::tuple<TupleArgs...>&, Args&&...)
  {
    return BuildReverseIndices<sizeof...(TupleArgs)>{};
  }

  template <std::size_t... Indices>
  IndexList<Indices...> indexList(IndexList<Indices...>);

  template <class... TupleArgs>
  auto indexList(const std::tuple<TupleArgs...>& tuple) -> decltype(indexList(buildIndices(tuple)));

  template <class Function, class Tuple, std::size_t... Indices>
  inline auto callWithIndexedTuple(Function function,
                                   Tuple&& tuple,
                                   IndexList<Indices...>)
    -> decltype(function(std::get<Indices>(std::forward<Tuple>(tuple))...))
  {
    return function(std::get<Indices>(std::forward<Tuple>(tuple))...);
  }

  /**
   * Calls *function* with the parameters packed into *tuple*.
   *
   * ~~~(c++)
   * int func(double, int, const char*);
   * // ...
   *
   * int result = Pii::callWithTuple(func, std::make_tuple(1.0, 2, "abc"));
   * // Equivalent to func(1.0, 2, "abc")
   * ~~~
   */
  template <class Function, class Tuple>
  inline auto callWithTuple(Function function,
                            Tuple&& tuple)
    -> decltype(callWithIndexedTuple(function,
                                     std::forward<Tuple>(tuple),
                                     buildIndices(std::forward<Tuple>(tuple))))
  {
    return callWithIndexedTuple(function,
                                std::forward<Tuple>(tuple),
                                buildIndices(std::forward<Tuple>(tuple)));
  }

  template <class ReturnType, class... Args, class Tuple>
  inline ReturnType callWithTuple(ReturnType (*function)(Args...),
                                  Tuple&& tuple)
  {
    return callWithIndexedTuple(std::function<ReturnType(Args...)>(function),
                                std::forward<Tuple>(tuple),
                                buildIndices(std::forward<Tuple>(tuple)));
  }

  /**
   * Calls the *member* of *object* with the given *args* and an
   * unpacked *tuple*.
   */
  template <class Object, class ReturnType, class... Args>
  inline ReturnType callWithTuple(Object object,
                                  ReturnType (ToValue<Object>::Type::* member)(Args...),
                                  std::tuple<Args...>&& tuple)
  {
    return callWithTuple(memberFunction(object, member),
                         std::forward<std::tuple<Args...>>(tuple),
                         BuildIndices<sizeof...(Args)>{});
  }

  template <std::size_t I, class Function, class... Tuples>
  auto callWithIthMember(Function function, Tuples&&... tuples) ->
    decltype(function(std::get<I>(std::forward<Tuples>(tuples))...))
  {
    return function(std::get<I>(std::forward<Tuples>(tuples))...);
  }

  template <class Function, class... Tuples, std::size_t... Indices>
  void callWithIndexedTuples(Function function,
                             IndexList<Indices...>,
                             Tuples&&... tuples)
  {
    PII_FOREACH_TEMPARG(callWithIthMember<Indices>(function,
                                                   std::forward<Tuples>(tuples)...));

  }

  /**
   * Calls *function* as many times as there are elements in the
   * tuples given as parameters. The parameters for the first call are
   * formed by taking the first element of each tuple and so on. Each
   * tuple must have the same number of elements.
   *
   * ~~~(c++)
   * struct Func
   * {
   *   template <class T, class U> void operator() (T t, U u);
   * };
   *
   * auto t1 = std::make_tuple(1, 2.0, "abc");
   * auto t2 = std::make_tuple(0, true, 1.0f);
   *
   * Pii::callWithTuples(Func(), t1, t2);
   * // Calls Func::operator() with (1, 0), (2.0, true) and ("abc", 1.0f)
   *
   * // You can fix any of the parameters using Pii::makeTuple()
   * Pii::callWithTuples(Func(), Pii::makeTuple<3>(1), std::make_tuple(1, 2, 3));
   * // Calls Func::operator() with (1, 1), (1, 2) and (1, 3)
   * ~~~
   *
   * @see makeTuple()
   */
  template <class Function, class... Tuples>
  inline void callWithTuples(Function function,
                             Tuples&&... tuples)
  {
    /* Work around a GCC bug
       http://gcc.gnu.org/bugzilla/show_bug.cgi?id=51253
       TODO: replace with a GCC version check once the bug is fixed
    */
    callWithIndexedTuples(function,
#ifdef __GNUC__
                          buildReverseIndices(std::forward<Tuples>(tuples)...),
#else
                          buildIndices(std::forward<Tuples>(tuples)...),
#endif
                          std::forward<Tuples>(tuples)...);
  }

  template <int N, class Arg1, class... Args> struct MakeTupleHelper;

  template <class Arg1, class... Args>
  struct MakeTupleHelper<1, Arg1, Args...>
  {
    typedef std::tuple<Arg1, Args...> Type;
    static Type create(Arg1 arg1, Args&&... args)
    {
      return Type{std::forward<Arg1>(arg1), std::forward<Args>(args)...};
    }
  };

  template <int N, class Arg1, class... Args>
  struct MakeTupleHelper : MakeTupleHelper<N-1, Arg1, Arg1, Args...>
  {
    typedef MakeTupleHelper<N-1, Arg1, Arg1, Args...> SuperType;
    typedef typename SuperType::Type Type;

    static typename SuperType::Type create(Arg1&& arg1, Args&&... args)
    {
      return SuperType::create(std::forward<Arg1>(arg1),
                               std::forward<Arg1>(arg1),
                               std::forward<Args>(args)...);
    }
  };

  /**
   * Creates a tuple that repeats *value* *N* times.
   *
   * ~~~(c++)
   * auto threeOnes = Pii::makeTuple<3>(1);
   * // threeOnes == std::tuple<int,int,int>{1, 1, 1}
   * ~~~
   */
  template <std::size_t N, class T>
  typename MakeTupleHelper<N,T>::Type makeTuple(T&& value)
  {
    return MakeTupleHelper<N,T>::create(std::forward<T>(value));
  }

#endif
  /// @endgroup
};

#endif //_PIIFUNCTIONAL_H
