#pragma once

#include <memory>
#include "cx/common.h"
#include "cx/idioms.h"
#include "cx/unsafe.h"

namespace CX {
 template<typename F, bool = CX::IsFunction<F>::value>
 struct Lambda;

 template<typename R, typename... Args>
 struct Lambda<R (Args...), true> {
 private:
  using lambda_t = Lambda<R (Args...)>;

  enum Type : unsigned char {
   PLAIN,
   ANONYMOUS
  };
  struct BaseData {
   Type type;
   BaseData(Type type) : type(type) {}
  };
  struct PlainData : BaseData {
   void *fn;
   PlainData(void *fn) : BaseData(Type::PLAIN), fn(fn) {}
  };
  struct AnonymousDataBase;
  struct AnyClass;
  using AnonymousFn = void (AnyClass::*)();
  enum class AnonymousDataOperation {
      DESTRUCT, COPY
  };
  using AnonymousDataOperationFn = void (*)(AnonymousDataBase *th, AnonymousDataOperation op, void *param);
  struct AnonymousDataBase : BaseData {
   AnonymousFn fn;
   AnonymousDataOperationFn opFn;

   AnonymousDataBase(AnonymousFn fn, AnonymousDataOperationFn opFn) : BaseData(Type::ANONYMOUS), fn(fn), opFn(opFn) {}

   void *getData() const { return (void *) (this + 1); }
  };
  template <typename T>
  struct AnonymousData : AnonymousDataBase {
   T data;

   explicit AnonymousData(T inst) :
    AnonymousDataBase((AnonymousFn) &T::operator(), (AnonymousDataOperationFn) &opFunction),
    data(std::move(inst))
   {}

   static void opFunction(AnonymousDataBase *th, AnonymousDataOperation op, void *param) {
    if (op == AnonymousDataOperation::DESTRUCT) {
     ((AnonymousData<T> *) th)->~AnonymousData<T>();
    } else if (op == AnonymousDataOperation::COPY) {
     auto &data = ((AnonymousData<T> *) th)->data;
     *(AnonymousData<T> **) param = new AnonymousData(data);
    }
   }
  };

  struct DataDeleter {
   void operator()(BaseData* data) const {
    if (data->type == Type::ANONYMOUS)
     ((AnonymousDataBase *) data)->opFn((AnonymousDataBase *) data, AnonymousDataOperation::DESTRUCT, nullptr);
    delete data;
   }
  };
  using DataPtr = std::unique_ptr<BaseData, DataDeleter>;
  static DataPtr cloneData(BaseData *data) {
   if (data == nullptr)
    return DataPtr();
   switch (data->type) {
    case Type::PLAIN:
     return DataPtr(new PlainData(((PlainData *) data)->fn));
    case Type::ANONYMOUS: {
     AnonymousDataBase *ret;
     ((AnonymousDataBase *) data)->opFn((AnonymousDataBase *) data, AnonymousDataOperation::COPY, &ret);
     return DataPtr(ret);
    }
   }
  }
 public:

  Lambda() noexcept :
   functor(new BaseData(Type::UINIT))
  {}

  Lambda(R (*func)(Args...)) noexcept :
   functor(new PlainData(func))
  {}

  template<typename T>
  Lambda(T inst) noexcept :
   functor(new AnonymousData<T>(inst))
  {}

  Lambda(const lambda_t& other) noexcept :
   functor(cloneData(other.functor.get()))
  {}

  Lambda(lambda_t&& other) noexcept :
   functor(std::move(other.functor))
  {
   other.functor = nullptr;
  }

  lambda_t& operator=(const lambda_t& other) noexcept {
   functor = cloneData(other.functor);
   return *this;
  }

  lambda_t& operator=(lambda_t&& other) noexcept {
   functor = std::move(other.functor);
   other.functor = nullptr;
   return *this;
  }

  lambda_t& operator=(R (* const func)(Args...)) noexcept {
   functor = DataPtr(new BaseData((void *) func));
   return *this;
  }

  template<typename T>
  lambda_t& operator=(T inst) noexcept {
   functor = DataPtr(new AnonymousData(inst));
   return *this;
  }

  [[gnu::always_inline]]
  inline R operator()(Args... args) const {
   switch(functor->type) {
    case PLAIN: {
     return union_cast<R (*)(Args...)>(((PlainData *) functor.get())->fn)(args...);
    }
    case ANONYMOUS: {
     auto pMem = ((AnonymousDataBase *) functor.get())->fn;
     auto pInst = (AnyClass *) ((AnonymousDataBase *) functor.get())->getData();
     return (pInst->*union_cast<R (AnyClass::*)(Args...)>(pMem))(args...);
    }
   }
  }

 private:
  DataPtr functor;
 };
}