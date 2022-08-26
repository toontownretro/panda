/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jobSystem.I
 * @author brian
 * @date 2022-04-30
 */

/**
 * Returns the global JobSystem pointer.
 */
INLINE JobSystem *JobSystem::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new JobSystem;
  }

  return _global_ptr;
}

/**
 *
 */
template<class T, class Pr>
INLINE void
parallel_quicksort(T *data, size_t size, Pr pred, int count_threshold) {
  JobSystem *js = JobSystem::get_global_ptr();
  r_parallel_quicksort(data, pred, 0, (int)size - 1, count_threshold, js);
}

/**
 *
 */
template<class T, class Pr>
INLINE void
r_serial_quicksort(T *data, Pr pred, int left, int right) {
  if ((right - left) < 2) {
    return;
  }

  int i = left;
  int j = right - 1;

  T &pivot = data[i + (j - i) / 2];
  T tmp;

  if (pred(data[i], pivot)) {
    while (pred(data[++i], pivot));
  }
  if (pred(pivot, data[j])) {
    while (pred(pivot, data[--j]));
  }

  while (i < j) {
    tmp = std::move(data[i]);
    data[i] = std::move(data[j]);
    data[j] = std::move(tmp);

    while (pred(data[++i], pivot));
    while (pred(pivot, data[--j]));
  }
  ++j;

  r_serial_quicksort(data, pred, left, j);
  r_serial_quicksort(data, pred, j, right);
}

/**
 *
 */
template<class T, class Pr>
INLINE void
r_parallel_quicksort(T *data, Pr pred, int left, int right, int cutoff, JobSystem *js) {

  if ((right - left) < cutoff) {
    r_serial_quicksort(data, pred, left, right);

  } else {

    int i = left;
    int j = right - 1;
    T &pivot = data[i + (j - i) / 2];
    T tmp;

    if (pred(data[i], pivot)) {
      while (pred(data[++i], pivot));
    }
    if (pred(pivot, data[j])) {
      while (pred(pivot, data[--j]));
    }

    while (i < j) {
      tmp = std::move(data[i]);
      data[i] = std::move(data[j]);
      data[j] = std::move(tmp);

      while (pred(data[++i], pivot));
      while (pred(pivot, data[--j]));
    }
    ++j;

    // Sort both sides in parallel.

    GenericJob left_job([=] () {
      r_parallel_quicksort(data, pred, left, j, cutoff, js);
    });
    left_job.local_object();
    GenericJob right_job([=] () {
      r_parallel_quicksort(data, pred, j, right, cutoff, js);
    });
    right_job.local_object();

    Job *jobs[2] = { &left_job, &right_job };
    js->schedule(jobs, 2, true);
  }
}
