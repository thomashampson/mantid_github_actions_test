// clang-format off
// automatically generated by the FlatBuffers compiler, do not modify


#pragma once

#include "flatbuffers/flatbuffers.h"

struct RunStart;

struct RunStop;

struct RunInfo;

enum class InfoTypes : uint8_t {
  NONE = 0,
  RunStart = 1,
  RunStop = 2,
  MIN = NONE,
  MAX = RunStop
};

inline const InfoTypes (&EnumValuesInfoTypes())[3] {
  static const InfoTypes values[] = {
    InfoTypes::NONE,
    InfoTypes::RunStart,
    InfoTypes::RunStop
  };
  return values;
}

inline const char * const *EnumNamesInfoTypes() {
  static const char * const names[] = {
    "NONE",
    "RunStart",
    "RunStop",
    nullptr
  };
  return names;
}

inline const char *EnumNameInfoTypes(InfoTypes e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesInfoTypes()[index];
}

template<typename T> struct InfoTypesTraits {
  static const InfoTypes enum_value = InfoTypes::NONE;
};

template<> struct InfoTypesTraits<RunStart> {
  static const InfoTypes enum_value = InfoTypes::RunStart;
};

template<> struct InfoTypesTraits<RunStop> {
  static const InfoTypes enum_value = InfoTypes::RunStop;
};

bool VerifyInfoTypes(flatbuffers::Verifier &verifier, const void *obj, InfoTypes type);
bool VerifyInfoTypesVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

struct RunStart FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "RunStart";
  }
  enum {
    VT_START_TIME = 4,
    VT_RUN_ID = 6,
    VT_INSTRUMENT_NAME = 8,
    VT_N_PERIODS = 10,
    VT_NEXUS_STRUCTURE = 12
  };
  uint64_t start_time() const {
    return GetField<uint64_t>(VT_START_TIME, 0);
  }
  bool mutate_start_time(uint64_t _start_time) {
    return SetField<uint64_t>(VT_START_TIME, _start_time, 0);
  }
  const flatbuffers::String *run_id() const {
    return GetPointer<const flatbuffers::String *>(VT_RUN_ID);
  }
  flatbuffers::String *mutable_run_id() {
    return GetPointer<flatbuffers::String *>(VT_RUN_ID);
  }
  const flatbuffers::String *instrument_name() const {
    return GetPointer<const flatbuffers::String *>(VT_INSTRUMENT_NAME);
  }
  flatbuffers::String *mutable_instrument_name() {
    return GetPointer<flatbuffers::String *>(VT_INSTRUMENT_NAME);
  }
  int32_t n_periods() const {
    return GetField<int32_t>(VT_N_PERIODS, 0);
  }
  bool mutate_n_periods(int32_t _n_periods) {
    return SetField<int32_t>(VT_N_PERIODS, _n_periods, 0);
  }
  const flatbuffers::String *nexus_structure() const {
    return GetPointer<const flatbuffers::String *>(VT_NEXUS_STRUCTURE);
  }
  flatbuffers::String *mutable_nexus_structure() {
    return GetPointer<flatbuffers::String *>(VT_NEXUS_STRUCTURE);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_START_TIME) &&
           VerifyOffset(verifier, VT_RUN_ID) &&
           verifier.VerifyString(run_id()) &&
           VerifyOffset(verifier, VT_INSTRUMENT_NAME) &&
           verifier.VerifyString(instrument_name()) &&
           VerifyField<int32_t>(verifier, VT_N_PERIODS) &&
           VerifyOffset(verifier, VT_NEXUS_STRUCTURE) &&
           verifier.VerifyString(nexus_structure()) &&
           verifier.EndTable();
  }
};

struct RunStartBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_start_time(uint64_t start_time) {
    fbb_.AddElement<uint64_t>(RunStart::VT_START_TIME, start_time, 0);
  }
  void add_run_id(flatbuffers::Offset<flatbuffers::String> run_id) {
    fbb_.AddOffset(RunStart::VT_RUN_ID, run_id);
  }
  void add_instrument_name(flatbuffers::Offset<flatbuffers::String> instrument_name) {
    fbb_.AddOffset(RunStart::VT_INSTRUMENT_NAME, instrument_name);
  }
  void add_n_periods(int32_t n_periods) {
    fbb_.AddElement<int32_t>(RunStart::VT_N_PERIODS, n_periods, 0);
  }
  void add_nexus_structure(flatbuffers::Offset<flatbuffers::String> nexus_structure) {
    fbb_.AddOffset(RunStart::VT_NEXUS_STRUCTURE, nexus_structure);
  }
  explicit RunStartBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RunStartBuilder &operator=(const RunStartBuilder &);
  flatbuffers::Offset<RunStart> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RunStart>(end);
    return o;
  }
};

inline flatbuffers::Offset<RunStart> CreateRunStart(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t start_time = 0,
    flatbuffers::Offset<flatbuffers::String> run_id = 0,
    flatbuffers::Offset<flatbuffers::String> instrument_name = 0,
    int32_t n_periods = 0,
    flatbuffers::Offset<flatbuffers::String> nexus_structure = 0) {
  RunStartBuilder builder_(_fbb);
  builder_.add_start_time(start_time);
  builder_.add_nexus_structure(nexus_structure);
  builder_.add_n_periods(n_periods);
  builder_.add_instrument_name(instrument_name);
  builder_.add_run_id(run_id);
  return builder_.Finish();
}

inline flatbuffers::Offset<RunStart> CreateRunStartDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t start_time = 0,
    const char *run_id = nullptr,
    const char *instrument_name = nullptr,
    int32_t n_periods = 0,
    const char *nexus_structure = nullptr) {
  return CreateRunStart(
      _fbb,
      start_time,
      run_id ? _fbb.CreateString(run_id) : 0,
      instrument_name ? _fbb.CreateString(instrument_name) : 0,
      n_periods,
      nexus_structure ? _fbb.CreateString(nexus_structure) : 0);
}

struct RunStop FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "RunStop";
  }
  enum {
    VT_STOP_TIME = 4,
    VT_RUN_ID = 6
  };
  uint64_t stop_time() const {
    return GetField<uint64_t>(VT_STOP_TIME, 0);
  }
  bool mutate_stop_time(uint64_t _stop_time) {
    return SetField<uint64_t>(VT_STOP_TIME, _stop_time, 0);
  }
  const flatbuffers::String *run_id() const {
    return GetPointer<const flatbuffers::String *>(VT_RUN_ID);
  }
  flatbuffers::String *mutable_run_id() {
    return GetPointer<flatbuffers::String *>(VT_RUN_ID);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_STOP_TIME) &&
           VerifyOffset(verifier, VT_RUN_ID) &&
           verifier.VerifyString(run_id()) &&
           verifier.EndTable();
  }
};

struct RunStopBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_stop_time(uint64_t stop_time) {
    fbb_.AddElement<uint64_t>(RunStop::VT_STOP_TIME, stop_time, 0);
  }
  void add_run_id(flatbuffers::Offset<flatbuffers::String> run_id) {
    fbb_.AddOffset(RunStop::VT_RUN_ID, run_id);
  }
  explicit RunStopBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RunStopBuilder &operator=(const RunStopBuilder &);
  flatbuffers::Offset<RunStop> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RunStop>(end);
    return o;
  }
};

inline flatbuffers::Offset<RunStop> CreateRunStop(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t stop_time = 0,
    flatbuffers::Offset<flatbuffers::String> run_id = 0) {
  RunStopBuilder builder_(_fbb);
  builder_.add_stop_time(stop_time);
  builder_.add_run_id(run_id);
  return builder_.Finish();
}

inline flatbuffers::Offset<RunStop> CreateRunStopDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t stop_time = 0,
    const char *run_id = nullptr) {
  return CreateRunStop(
      _fbb,
      stop_time,
      run_id ? _fbb.CreateString(run_id) : 0);
}

struct RunInfo FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  static FLATBUFFERS_CONSTEXPR const char *GetFullyQualifiedName() {
    return "RunInfo";
  }
  enum {
    VT_INFO_TYPE_TYPE = 4,
    VT_INFO_TYPE = 6
  };
  InfoTypes info_type_type() const {
    return static_cast<InfoTypes>(GetField<uint8_t>(VT_INFO_TYPE_TYPE, 0));
  }
  bool mutate_info_type_type(InfoTypes _info_type_type) {
    return SetField<uint8_t>(VT_INFO_TYPE_TYPE, static_cast<uint8_t>(_info_type_type), 0);
  }
  const void *info_type() const {
    return GetPointer<const void *>(VT_INFO_TYPE);
  }
  template<typename T> const T *info_type_as() const;
  const RunStart *info_type_as_RunStart() const {
    return info_type_type() == InfoTypes::RunStart ? static_cast<const RunStart *>(info_type()) : nullptr;
  }
  const RunStop *info_type_as_RunStop() const {
    return info_type_type() == InfoTypes::RunStop ? static_cast<const RunStop *>(info_type()) : nullptr;
  }
  void *mutable_info_type() {
    return GetPointer<void *>(VT_INFO_TYPE);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_INFO_TYPE_TYPE) &&
           VerifyOffset(verifier, VT_INFO_TYPE) &&
           VerifyInfoTypes(verifier, info_type(), info_type_type()) &&
           verifier.EndTable();
  }
};

template<> inline const RunStart *RunInfo::info_type_as<RunStart>() const {
  return info_type_as_RunStart();
}

template<> inline const RunStop *RunInfo::info_type_as<RunStop>() const {
  return info_type_as_RunStop();
}

struct RunInfoBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_info_type_type(InfoTypes info_type_type) {
    fbb_.AddElement<uint8_t>(RunInfo::VT_INFO_TYPE_TYPE, static_cast<uint8_t>(info_type_type), 0);
  }
  void add_info_type(flatbuffers::Offset<void> info_type) {
    fbb_.AddOffset(RunInfo::VT_INFO_TYPE, info_type);
  }
  explicit RunInfoBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RunInfoBuilder &operator=(const RunInfoBuilder &);
  flatbuffers::Offset<RunInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RunInfo>(end);
    return o;
  }
};

inline flatbuffers::Offset<RunInfo> CreateRunInfo(
    flatbuffers::FlatBufferBuilder &_fbb,
    InfoTypes info_type_type = InfoTypes::NONE,
    flatbuffers::Offset<void> info_type = 0) {
  RunInfoBuilder builder_(_fbb);
  builder_.add_info_type(info_type);
  builder_.add_info_type_type(info_type_type);
  return builder_.Finish();
}

inline bool VerifyInfoTypes(flatbuffers::Verifier &verifier, const void *obj, InfoTypes type) {
  switch (type) {
    case InfoTypes::NONE: {
      return true;
    }
    case InfoTypes::RunStart: {
      auto ptr = reinterpret_cast<const RunStart *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case InfoTypes::RunStop: {
      auto ptr = reinterpret_cast<const RunStop *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return false;
  }
}

inline bool VerifyInfoTypesVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyInfoTypes(
        verifier,  values->Get(i), types->GetEnum<InfoTypes>(i))) {
      return false;
    }
  }
  return true;
}

inline const RunInfo *GetRunInfo(const void *buf) {
  return flatbuffers::GetRoot<RunInfo>(buf);
}

inline const RunInfo *GetSizePrefixedRunInfo(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<RunInfo>(buf);
}

inline RunInfo *GetMutableRunInfo(void *buf) {
  return flatbuffers::GetMutableRoot<RunInfo>(buf);
}

inline const char *RunInfoIdentifier() {
  return "y2gw";
}

inline bool RunInfoBufferHasIdentifier(const void *buf) {
  return flatbuffers::BufferHasIdentifier(
      buf, RunInfoIdentifier());
}

inline bool VerifyRunInfoBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<RunInfo>(RunInfoIdentifier());
}

inline bool VerifySizePrefixedRunInfoBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<RunInfo>(RunInfoIdentifier());
}

inline void FinishRunInfoBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<RunInfo> root) {
  fbb.Finish(root, RunInfoIdentifier());
}

inline void FinishSizePrefixedRunInfoBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<RunInfo> root) {
  fbb.FinishSizePrefixed(root, RunInfoIdentifier());
}
