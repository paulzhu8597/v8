// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_COMPILATION_DEPENDENCIES_H_
#define V8_COMPILATION_DEPENDENCIES_H_

#include "src/handles.h"
#include "src/objects.h"
#include "src/objects/map.h"
#include "src/zone/zone-containers.h"

namespace v8 {
namespace internal {

// Collects dependencies for this compilation, e.g. assumptions about
// stable maps, constant globals, etc.
class V8_EXPORT_PRIVATE CompilationDependencies {
 public:
  CompilationDependencies(Isolate* isolate, Zone* zone)
      : isolate_(isolate),
        zone_(zone),
        object_wrapper_(Handle<Foreign>::null()),
        aborted_(false) {
    std::fill_n(groups_, DependentCode::kGroupCount, nullptr);
  }

  void Insert(DependentCode::DependencyGroup group, Handle<HeapObject> handle);

  void AssumeInitialMapCantChange(Handle<Map> map) {
    Insert(DependentCode::kInitialMapChangedGroup, map);
  }
  void AssumeFieldOwner(Handle<Map> map) {
    Insert(DependentCode::kFieldOwnerGroup, map);
  }
  void AssumeMapStable(Handle<Map> map);
  void AssumePrototypeMapsStable(
      Handle<Map> map,
      MaybeHandle<JSReceiver> prototype = MaybeHandle<JSReceiver>());
  void AssumeMapNotDeprecated(Handle<Map> map);
  void AssumePropertyCell(Handle<PropertyCell> cell) {
    Insert(DependentCode::kPropertyCellChangedGroup, cell);
  }
  void AssumeTenuringDecision(Handle<AllocationSite> site) {
    Insert(DependentCode::kAllocationSiteTenuringChangedGroup, site);
  }
  void AssumeTransitionStable(Handle<AllocationSite> site);

  // Adds stability dependencies on all prototypes of every class in
  // {receiver_type} up to (and including) the {holder}.
  void AssumePrototypesStable(Handle<Context> native_context,
                              std::vector<Handle<Map>> const& receiver_maps,
                              Handle<JSObject> holder);

  void Commit(Handle<Code> code);
  void Rollback();
  void Abort() { aborted_ = true; }
  bool HasAborted() const { return aborted_; }

  bool IsEmpty() const {
    for (int i = 0; i < DependentCode::kGroupCount; i++) {
      if (groups_[i]) return false;
    }
    return true;
  }

 private:
  Isolate* isolate_;
  Zone* zone_;
  Handle<Foreign> object_wrapper_;
  bool aborted_;
  ZoneVector<Handle<HeapObject> >* groups_[DependentCode::kGroupCount];

  DependentCode* Get(Handle<Object> object) const;
  void Set(Handle<Object> object, Handle<DependentCode> dep);
};
}  // namespace internal
}  // namespace v8

#endif  // V8_COMPILATION_DEPENDENCIES_H_
