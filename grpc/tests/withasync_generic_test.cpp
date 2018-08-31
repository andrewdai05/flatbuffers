/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thread>
#include <tuple>
#include <grpc++/grpc++.h>

#include "monster_test.grpc.fb.h"
#include "monster_test_generated.h"

using namespace MyGame::Example;

template <typename GrpcServiceMemberFunctionType>
struct grpc_service_traits;

template <typename Class, typename Req, typename Res>
struct grpc_service_traits<void (Class::*)(::grpc::ServerContext*, flatbuffers::grpc::Message<Req> *, ::grpc::ServerAsyncResponseWriter< flatbuffers::grpc::Message<Res>>*, ::grpc::CompletionQueue *, ::grpc::ServerCompletionQueue *, void *)> {
  using RequestType = Req;
  using ResponseType = Res;
  using RequestMessageType = std::tuple<Req, Res>;
  using ResponseMessageType = std::tuple<Req, Res>;
};

template <typename Class, typename Req, typename Res>
struct grpc_service_traits<void (Class::*)(::grpc::ServerContext*, flatbuffers::grpc::Message<Req> *, ::grpc::ServerAsyncWriter< flatbuffers::grpc::Message<Res>>*, ::grpc::CompletionQueue *, ::grpc::ServerCompletionQueue *, void *)> {
  using RequestType = Req;
  using ResponseType = Res;
  using RequestMessageType = std::tuple<Req, Res>;
  using ResponseMessageType = std::tuple<Req, Res>;
};

template <class T, class Tuple>
struct tuple_cat;

template <class T, class... Args>
struct tuple_cat<T, std::tuple<Args...>> {
  using type = std::tuple<T, Args...>;
};

template <class T>
struct tuple_cat<T, void> {
  using type = std::tuple<T>;
};

template <class T>
struct ProcSet {
  using type = void;
};

template <class NestedMethod, template<class> class WithAsyncMethod>
struct ProcSet<WithAsyncMethod<NestedMethod>> {
  using this_method_traits = grpc_service_traits<decltype(&WithAsyncMethod<NestedMethod>::operator())>;
  using type = typename tuple_cat<this_method_traits,
                                  typename ProcSet<NestedMethod>::type>::type;
};

template <class AsyncService>
struct CallNested {
  static void call(AsyncService &s) {}
};

template <class Nested, template <class> class WithAsyncMethod>
struct CallNested<WithAsyncMethod<Nested>> {
  static void call(WithAsyncMethod<Nested> &s) {
    s(0,0,0,0,0,0);
    CallNested<Nested>::call(s);
  };
};


template <class AsyncService>
void call_nested(AsyncService &s) {
  CallNested<AsyncService>::call(s);
}

template <class B, class C, class HeadTraits, class... TailTraits>
struct MatchingTraits {
  static_assert(sizeof...(TailTraits), "AsyncService overload set resolution failed. Check your method call.");
  using type = typename MatchingTraits<B, C, TailTraits...>::type;
};

template <class B, class C, class Method, class... TailTraits>
struct MatchingTraits<B, C, grpc_service_traits<void (Method::*) (::grpc::ServerContext *, B, C, ::grpc::CompletionQueue *, ::grpc::ServerCompletionQueue *, void *)>, TailTraits...> {
  using type = Method;
};

template <class B, class C, class ProcSet>
struct MatchingProc;

template <class B, class C, class... GrpcServiceTraits>
struct MatchingProc<B, C, std::tuple<GrpcServiceTraits...>> {
  using type = typename MatchingTraits<B, C, GrpcServiceTraits...>::type;
};

template <class AsyncService>
struct OverloadSet : public AsyncService {
  template <class A, class B, class C, class D, class E, class F>
  void operator()(A a, B b, C c, D d, E e, F f) {
    MatchingProc<B, C, typename ProcSet<AsyncService>::type>::type::operator()(a,b,c,d,e,f);
  }
};

int overload_set_test() {
  ::grpc::ServerContext* context = nullptr;
  flatbuffers::grpc::Message<Monster>* request1 = nullptr;
  ::grpc::ServerAsyncResponseWriter< flatbuffers::grpc::Message<Stat>>* response = nullptr;
  ::grpc::CompletionQueue* new_call_cq = nullptr;
  ::grpc::ServerCompletionQueue* notification_cq = nullptr;
  void *tag = nullptr;

  flatbuffers::grpc::Message<Stat>* request2 = nullptr;
  ::grpc::ServerAsyncWriter< flatbuffers::grpc::Message<Monster>>* writer = nullptr;

  OverloadSet<MyGame::Example::MonsterStorage::AsyncService> set;
  set(context, request1, response, new_call_cq, notification_cq, tag);
  set(context, request2, writer, new_call_cq, notification_cq, tag);
  return 0;
}


int withasync_generic() {
  MyGame::Example::MonsterStorage::AsyncService s;
  call_nested(s);
  overload_set_test();
  return 0;
}
