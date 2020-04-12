// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: engine.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_engine_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_engine_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3008000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3008000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_engine_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_engine_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[2]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_engine_2eproto;
namespace engine {
class TemplateRegisterReply;
class TemplateRegisterReplyDefaultTypeInternal;
extern TemplateRegisterReplyDefaultTypeInternal _TemplateRegisterReply_default_instance_;
class TemplateRegisterRequest;
class TemplateRegisterRequestDefaultTypeInternal;
extern TemplateRegisterRequestDefaultTypeInternal _TemplateRegisterRequest_default_instance_;
}  // namespace engine
PROTOBUF_NAMESPACE_OPEN
template<> ::engine::TemplateRegisterReply* Arena::CreateMaybeMessage<::engine::TemplateRegisterReply>(Arena*);
template<> ::engine::TemplateRegisterRequest* Arena::CreateMaybeMessage<::engine::TemplateRegisterRequest>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace engine {

// ===================================================================

class TemplateRegisterRequest :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:engine.TemplateRegisterRequest) */ {
 public:
  TemplateRegisterRequest();
  virtual ~TemplateRegisterRequest();

  TemplateRegisterRequest(const TemplateRegisterRequest& from);
  TemplateRegisterRequest(TemplateRegisterRequest&& from) noexcept
    : TemplateRegisterRequest() {
    *this = ::std::move(from);
  }

  inline TemplateRegisterRequest& operator=(const TemplateRegisterRequest& from) {
    CopyFrom(from);
    return *this;
  }
  inline TemplateRegisterRequest& operator=(TemplateRegisterRequest&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const TemplateRegisterRequest& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const TemplateRegisterRequest* internal_default_instance() {
    return reinterpret_cast<const TemplateRegisterRequest*>(
               &_TemplateRegisterRequest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(TemplateRegisterRequest* other);
  friend void swap(TemplateRegisterRequest& a, TemplateRegisterRequest& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline TemplateRegisterRequest* New() const final {
    return CreateMaybeMessage<TemplateRegisterRequest>(nullptr);
  }

  TemplateRegisterRequest* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<TemplateRegisterRequest>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const TemplateRegisterRequest& from);
  void MergeFrom(const TemplateRegisterRequest& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  #if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  #else
  bool MergePartialFromCodedStream(
      ::PROTOBUF_NAMESPACE_ID::io::CodedInputStream* input) final;
  #endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  void SerializeWithCachedSizes(
      ::PROTOBUF_NAMESPACE_ID::io::CodedOutputStream* output) const final;
  ::PROTOBUF_NAMESPACE_ID::uint8* InternalSerializeWithCachedSizesToArray(
      ::PROTOBUF_NAMESPACE_ID::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(TemplateRegisterRequest* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "engine.TemplateRegisterRequest";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_engine_2eproto);
    return ::descriptor_table_engine_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated string source = 2;
  int source_size() const;
  void clear_source();
  static const int kSourceFieldNumber = 2;
  const std::string& source(int index) const;
  std::string* mutable_source(int index);
  void set_source(int index, const std::string& value);
  void set_source(int index, std::string&& value);
  void set_source(int index, const char* value);
  void set_source(int index, const char* value, size_t size);
  std::string* add_source();
  void add_source(const std::string& value);
  void add_source(std::string&& value);
  void add_source(const char* value);
  void add_source(const char* value, size_t size);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>& source() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>* mutable_source();

  // string name = 1;
  void clear_name();
  static const int kNameFieldNumber = 1;
  const std::string& name() const;
  void set_name(const std::string& value);
  void set_name(std::string&& value);
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  std::string* mutable_name();
  std::string* release_name();
  void set_allocated_name(std::string* name);

  // bytes source_package = 3;
  void clear_source_package();
  static const int kSourcePackageFieldNumber = 3;
  const std::string& source_package() const;
  void set_source_package(const std::string& value);
  void set_source_package(std::string&& value);
  void set_source_package(const char* value);
  void set_source_package(const void* value, size_t size);
  std::string* mutable_source_package();
  std::string* release_source_package();
  void set_allocated_source_package(std::string* source_package);

  // @@protoc_insertion_point(class_scope:engine.TemplateRegisterRequest)
 private:
  class HasBitSetters;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string> source_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr name_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr source_package_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_engine_2eproto;
};
// -------------------------------------------------------------------

class TemplateRegisterReply :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:engine.TemplateRegisterReply) */ {
 public:
  TemplateRegisterReply();
  virtual ~TemplateRegisterReply();

  TemplateRegisterReply(const TemplateRegisterReply& from);
  TemplateRegisterReply(TemplateRegisterReply&& from) noexcept
    : TemplateRegisterReply() {
    *this = ::std::move(from);
  }

  inline TemplateRegisterReply& operator=(const TemplateRegisterReply& from) {
    CopyFrom(from);
    return *this;
  }
  inline TemplateRegisterReply& operator=(TemplateRegisterReply&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const TemplateRegisterReply& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const TemplateRegisterReply* internal_default_instance() {
    return reinterpret_cast<const TemplateRegisterReply*>(
               &_TemplateRegisterReply_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  void Swap(TemplateRegisterReply* other);
  friend void swap(TemplateRegisterReply& a, TemplateRegisterReply& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline TemplateRegisterReply* New() const final {
    return CreateMaybeMessage<TemplateRegisterReply>(nullptr);
  }

  TemplateRegisterReply* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<TemplateRegisterReply>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const TemplateRegisterReply& from);
  void MergeFrom(const TemplateRegisterReply& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  #if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  #else
  bool MergePartialFromCodedStream(
      ::PROTOBUF_NAMESPACE_ID::io::CodedInputStream* input) final;
  #endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  void SerializeWithCachedSizes(
      ::PROTOBUF_NAMESPACE_ID::io::CodedOutputStream* output) const final;
  ::PROTOBUF_NAMESPACE_ID::uint8* InternalSerializeWithCachedSizesToArray(
      ::PROTOBUF_NAMESPACE_ID::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(TemplateRegisterReply* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "engine.TemplateRegisterReply";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_engine_2eproto);
    return ::descriptor_table_engine_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // bool result = 1;
  void clear_result();
  static const int kResultFieldNumber = 1;
  bool result() const;
  void set_result(bool value);

  // @@protoc_insertion_point(class_scope:engine.TemplateRegisterReply)
 private:
  class HasBitSetters;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  bool result_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_engine_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// TemplateRegisterRequest

// string name = 1;
inline void TemplateRegisterRequest::clear_name() {
  name_.ClearToEmptyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline const std::string& TemplateRegisterRequest::name() const {
  // @@protoc_insertion_point(field_get:engine.TemplateRegisterRequest.name)
  return name_.GetNoArena();
}
inline void TemplateRegisterRequest::set_name(const std::string& value) {
  
  name_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:engine.TemplateRegisterRequest.name)
}
inline void TemplateRegisterRequest::set_name(std::string&& value) {
  
  name_.SetNoArena(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:engine.TemplateRegisterRequest.name)
}
inline void TemplateRegisterRequest::set_name(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  name_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:engine.TemplateRegisterRequest.name)
}
inline void TemplateRegisterRequest::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:engine.TemplateRegisterRequest.name)
}
inline std::string* TemplateRegisterRequest::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:engine.TemplateRegisterRequest.name)
  return name_.MutableNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline std::string* TemplateRegisterRequest::release_name() {
  // @@protoc_insertion_point(field_release:engine.TemplateRegisterRequest.name)
  
  return name_.ReleaseNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline void TemplateRegisterRequest::set_allocated_name(std::string* name) {
  if (name != nullptr) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:engine.TemplateRegisterRequest.name)
}

// repeated string source = 2;
inline int TemplateRegisterRequest::source_size() const {
  return source_.size();
}
inline void TemplateRegisterRequest::clear_source() {
  source_.Clear();
}
inline const std::string& TemplateRegisterRequest::source(int index) const {
  // @@protoc_insertion_point(field_get:engine.TemplateRegisterRequest.source)
  return source_.Get(index);
}
inline std::string* TemplateRegisterRequest::mutable_source(int index) {
  // @@protoc_insertion_point(field_mutable:engine.TemplateRegisterRequest.source)
  return source_.Mutable(index);
}
inline void TemplateRegisterRequest::set_source(int index, const std::string& value) {
  // @@protoc_insertion_point(field_set:engine.TemplateRegisterRequest.source)
  source_.Mutable(index)->assign(value);
}
inline void TemplateRegisterRequest::set_source(int index, std::string&& value) {
  // @@protoc_insertion_point(field_set:engine.TemplateRegisterRequest.source)
  source_.Mutable(index)->assign(std::move(value));
}
inline void TemplateRegisterRequest::set_source(int index, const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  source_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:engine.TemplateRegisterRequest.source)
}
inline void TemplateRegisterRequest::set_source(int index, const char* value, size_t size) {
  source_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:engine.TemplateRegisterRequest.source)
}
inline std::string* TemplateRegisterRequest::add_source() {
  // @@protoc_insertion_point(field_add_mutable:engine.TemplateRegisterRequest.source)
  return source_.Add();
}
inline void TemplateRegisterRequest::add_source(const std::string& value) {
  source_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:engine.TemplateRegisterRequest.source)
}
inline void TemplateRegisterRequest::add_source(std::string&& value) {
  source_.Add(std::move(value));
  // @@protoc_insertion_point(field_add:engine.TemplateRegisterRequest.source)
}
inline void TemplateRegisterRequest::add_source(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  source_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:engine.TemplateRegisterRequest.source)
}
inline void TemplateRegisterRequest::add_source(const char* value, size_t size) {
  source_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:engine.TemplateRegisterRequest.source)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>&
TemplateRegisterRequest::source() const {
  // @@protoc_insertion_point(field_list:engine.TemplateRegisterRequest.source)
  return source_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField<std::string>*
TemplateRegisterRequest::mutable_source() {
  // @@protoc_insertion_point(field_mutable_list:engine.TemplateRegisterRequest.source)
  return &source_;
}

// bytes source_package = 3;
inline void TemplateRegisterRequest::clear_source_package() {
  source_package_.ClearToEmptyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline const std::string& TemplateRegisterRequest::source_package() const {
  // @@protoc_insertion_point(field_get:engine.TemplateRegisterRequest.source_package)
  return source_package_.GetNoArena();
}
inline void TemplateRegisterRequest::set_source_package(const std::string& value) {
  
  source_package_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:engine.TemplateRegisterRequest.source_package)
}
inline void TemplateRegisterRequest::set_source_package(std::string&& value) {
  
  source_package_.SetNoArena(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:engine.TemplateRegisterRequest.source_package)
}
inline void TemplateRegisterRequest::set_source_package(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  source_package_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:engine.TemplateRegisterRequest.source_package)
}
inline void TemplateRegisterRequest::set_source_package(const void* value, size_t size) {
  
  source_package_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:engine.TemplateRegisterRequest.source_package)
}
inline std::string* TemplateRegisterRequest::mutable_source_package() {
  
  // @@protoc_insertion_point(field_mutable:engine.TemplateRegisterRequest.source_package)
  return source_package_.MutableNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline std::string* TemplateRegisterRequest::release_source_package() {
  // @@protoc_insertion_point(field_release:engine.TemplateRegisterRequest.source_package)
  
  return source_package_.ReleaseNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline void TemplateRegisterRequest::set_allocated_source_package(std::string* source_package) {
  if (source_package != nullptr) {
    
  } else {
    
  }
  source_package_.SetAllocatedNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), source_package);
  // @@protoc_insertion_point(field_set_allocated:engine.TemplateRegisterRequest.source_package)
}

// -------------------------------------------------------------------

// TemplateRegisterReply

// bool result = 1;
inline void TemplateRegisterReply::clear_result() {
  result_ = false;
}
inline bool TemplateRegisterReply::result() const {
  // @@protoc_insertion_point(field_get:engine.TemplateRegisterReply.result)
  return result_;
}
inline void TemplateRegisterReply::set_result(bool value) {
  
  result_ = value;
  // @@protoc_insertion_point(field_set:engine.TemplateRegisterReply.result)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace engine

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_engine_2eproto
