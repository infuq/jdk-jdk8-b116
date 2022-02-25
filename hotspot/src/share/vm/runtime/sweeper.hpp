/*
 * Copyright (c) 1997, 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_RUNTIME_SWEEPER_HPP
#define SHARE_VM_RUNTIME_SWEEPER_HPP

// An NmethodSweeper is an incremental cleaner for:
//    - cleanup inline caches
//    - reclamation of nmethods
// Removing nmethods from the code cache includes two operations
//  1) mark active nmethods
//     Is done in 'mark_active_nmethods()'. This function is called at a
//     safepoint and marks all nmethods that are active on a thread's stack.
//  2) sweep nmethods
//     Is done in sweep_code_cache(). This function is the only place in the
//     sweeper where memory is reclaimed. Note that sweep_code_cache() is not
//     called at a safepoint. However, sweep_code_cache() stops executing if
//     another thread requests a safepoint. Consequently, 'mark_active_nmethods()'
//     and sweep_code_cache() cannot execute at the same time.
//     To reclaim memory, nmethods are first marked as 'not-entrant'. Methods can
//     be made not-entrant by (i) the sweeper, (ii) deoptimization, (iii) dependency
//     invalidation, and (iv) being replaced be a different method version (tiered
//     compilation). Not-entrant nmethod cannot be called by Java threads, but they
//     can still be active on the stack. To ensure that active nmethod are not reclaimed,
//     we have to wait until the next marking phase has completed. If a not-entrant
//     nmethod was NOT marked as active, it can be converted to 'zombie' state. To safely
//     remove the nmethod, all inline caches (IC) that point to the the nmethod must be
//     cleared. After that, the nmethod can be evicted from the code cache. Each nmethod's
//     state change happens during separate sweeps. It may take at least 3 sweeps before an
//     nmethod's space is freed. Sweeping is currently done by compiler threads between
//     compilations or at least each 5 sec (NmethodSweepCheckInterval) when the code cache
//     is full.

class NMethodSweeper : public AllStatic {
  static long      _traversals;      // Stack scan count, also sweep ID.
  static nmethod*  _current;         // Current nmethod
  static int       _seen;            // Nof. nmethod we have currently processed in current pass of CodeCache
  static int       _flushed_count;   // Nof. nmethods flushed in current sweep
  static int       _zombified_count; // Nof. nmethods made zombie in current sweep
  static int       _marked_count;    // Nof. nmethods marked for reclaim in current sweep

  static volatile int  _invocations;   // No. of invocations left until we are completed with this pass
  static volatile int  _sweep_started; // Flag to control conc sweeper

  //The following are reset in mark_active_nmethods and synchronized by the safepoint
  static bool      _request_mark_phase;        // Indicates that a change has happend and we need another mark pahse,
                                               // always checked and reset at a safepoint so memory will be in sync.
  static int       _locked_seen;               // Number of locked nmethods encountered during the scan
  static int       _not_entrant_seen_on_stack; // Number of not entrant nmethod were are still on stack

  // Stat counters
  static int       _total_nof_methods_reclaimed;  // Accumulated nof methods flushed
  static jlong     _total_time_sweeping;          // Accumulated time sweeping
  static jlong     _total_time_this_sweep;        // Total time this sweep
  static jlong     _peak_sweep_time;              // Peak time for a full sweep
  static jlong     _peak_sweep_fraction_time;     // Peak time sweeping one fraction

  static int  process_nmethod(nmethod *nm);
  static void release_nmethod(nmethod* nm);

  static bool sweep_in_progress();
  static void sweep_code_cache();
  static void request_nmethod_marking() { _request_mark_phase = true; }
  static void reset_nmethod_marking()   { _request_mark_phase = false; }
  static bool need_marking_phase()      { return _request_mark_phase; }

  static int _hotness_counter_reset_val;

 public:
  static long traversal_count()              { return _traversals; }
  static int  total_nof_methods_reclaimed()  { return _total_nof_methods_reclaimed; }
  static jlong total_time_sweeping()         { return _total_time_sweeping; }
  static jlong peak_sweep_time()             { return _peak_sweep_time; }
  static jlong peak_sweep_fraction_time()    { return _peak_sweep_fraction_time; }
  static void log_sweep(const char* msg, const char* format = NULL, ...);


#ifdef ASSERT
  static bool is_sweeping(nmethod* which) { return _current == which; }
  // Keep track of sweeper activity in the ring buffer
  static void record_sweep(nmethod* nm, int line);
  static void report_events(int id, address entry);
  static void report_events();
#endif

  static void mark_active_nmethods();      // Invoked at the end of each safepoint
  static void possibly_sweep();            // Compiler threads call this to sweep

  static int sort_nmethods_by_hotness(nmethod** nm1, nmethod** nm2);
  static int hotness_counter_reset_val();

  static void notify() {
    // Request a new sweep of the code cache from the beginning. No
    // need to synchronize the setting of this flag since it only
    // changes to false at safepoint so we can never overwrite it with false.
     request_nmethod_marking();
  }
};

#endif // SHARE_VM_RUNTIME_SWEEPER_HPP