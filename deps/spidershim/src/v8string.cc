// Copyright Mozilla Foundation. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include <assert.h>

#include "v8.h"
#include "jsapi.h"
#include "jsfriendapi.h"
#include "js/CharacterEncoding.h"
#include "v8isolate.h"
#include "v8local.h"

namespace v8 {

String::Utf8Value::Utf8Value(Handle<v8::Value> obj)
  : str_(nullptr), length_(0) {
  JSContext* cx = JSContextFromIsolate(Isolate::GetCurrent());
  auto jsVal = reinterpret_cast<const JS::Value*>(*obj);
  // JS_BasicObjectToString doesn't deal with undefined and null :(
  if (jsVal->isUndefined()) {
    length_ = sizeof("[object Undefined]");
    str_ = new char[length_ + 1];
    strcpy(str_, "[object Undefined]");
  } else if (jsVal->isNull()) {
    length_ = sizeof("[object Null]");
    str_ = new char[length_ + 1];
    strcpy(str_, "[object Null]");
  } else if (jsVal->isObject()) {
    JS::RootedObject obj(cx, &jsVal->toObject());
    JS::RootedString str(cx,
                         JS_BasicObjectToString(cx, obj));
    if (str) {
      JSFlatString* flat = JS_FlattenString(cx, str);
      if (flat) {
        length_ = JS::GetDeflatedUTF8StringLength(flat);
        str_ = new char[length_ + 1];
        JS::DeflateStringToUTF8Buffer(flat, mozilla::RangedPtr<char>(str_, length_));
        str_[length_] = '\0';
      }
    }
  } else if (jsVal->isString()) {
    JSString* str = jsVal->toString();
    if (str) {
      JSFlatString* flat = JS_FlattenString(cx, str);
      if (flat) {
        length_ = JS::GetDeflatedUTF8StringLength(flat);
        str_ = new char[length_ + 1];
        JS::DeflateStringToUTF8Buffer(flat, mozilla::RangedPtr<char>(str_, length_));
        str_[length_] = '\0';
      }
    }
  } else {
    assert(false && "Not supported yet");
  }
}

String::Utf8Value::~Utf8Value() {
  delete[] str_;
}

Local<String> String::NewFromUtf8(Isolate* isolate, const char* data,
                                  NewStringType type, int length) {
  return NewFromUtf8(isolate, data, static_cast<v8::NewStringType>(type),
                     length).FromMaybe(Local<String>());
}

MaybeLocal<String> String::NewFromUtf8(Isolate* isolate, const char* data,
                                       v8::NewStringType type, int length) {
  assert(type == v8::NewStringType::kNormal); // TODO: Add support for interned strings
  JSContext* cx = JSContextFromIsolate(isolate);
  JS::RootedString str(cx, length >= 0 ?
                        JS_NewStringCopyN(cx, data, length) :
                        JS_NewStringCopyZ(cx, data));
  if (!str) {
    return MaybeLocal<String>();
  }
  JS::Value strVal;
  strVal.setString(str);
  return internal::Local<String>::New(isolate, strVal);
}

}
