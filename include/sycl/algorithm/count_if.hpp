/* Copyright (c) 2015 The Khronos Group Inc.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and/or associated documentation files (the
   "Materials"), to deal in the Materials without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Materials, and to
   permit persons to whom the Materials are furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Materials.

   MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
   KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
   SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
    https://www.khronos.org/registry/

  THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

*/

#ifndef __SYCL_IMPL_ALGORITHM_COUNT_IF__
#define __SYCL_IMPL_ALGORITHM_COUNT_IF__

#include <type_traits>
#include <algorithm>
#include <iostream>

// SYCL helpers header
#include <sycl/helpers/sycl_buffers.hpp>
#include <sycl/algorithm/algorithm_composite_patterns.hpp>

namespace sycl {
namespace impl {

/* count_if.
* @brief Returns the count_if of one vector across the range [first,
* last) by applying Function p. Implementation of the command group
* that submits a count_if kernel.
*/
template <class ExecutionPolicy, class InputIterator, class UnaryOperation,
          class BinaryOperation>
typename std::iterator_traits<InputIterator>::difference_type count_if(
    ExecutionPolicy& exec, InputIterator first, InputIterator last,
    UnaryOperation unary_op, BinaryOperation binary_op) {
  cl::sycl::queue q(exec.get_queue());
  auto vectorSize = std::distance(first, last);
  typename std::iterator_traits<InputIterator>::difference_type ret = 0;

  if (vectorSize < 1) {
    return ret;
  }

  auto device = q.get_device();
  auto local = device.get_info<cl::sycl::info::device::max_work_group_size>();
  typedef typename std::iterator_traits<InputIterator>::value_type type_;
  auto bufI = sycl::helpers::make_const_buffer(first, last);
  cl::sycl::buffer<int, 1> bufR((cl::sycl::range<1>(vectorSize)));
  size_t length = exec.calculateGlobalSize(vectorSize, local);
  int passes = 0;

  do {
    auto f = [length, local, passes, &bufI, &bufR, unary_op, binary_op](
        cl::sycl::handler& h) mutable {
      cl::sycl::nd_range<1> r{cl::sycl::range<1>{std::max(length, local)},
                              cl::sycl::range<1>{local}};
      auto aI = bufI.template get_access<cl::sycl::access::mode::read>(h);
      auto aR = bufR.template get_access<cl::sycl::access::mode::read_write>(h);
      cl::sycl::accessor<int, 1, cl::sycl::access::mode::read_write,
                         cl::sycl::access::target::local>
          scratch(cl::sycl::range<1>(local), h);

      h.parallel_for<typename ExecutionPolicy::kernelName>(
          r, [aI, aR, scratch, local, length, passes, unary_op, binary_op](
                 cl::sycl::nd_item<1> id) {
            int globalid = id.get_global(0);
            int localid = id.get_local(0);
            auto r = ReductionStrategy<int>(local, length, id, scratch);
            if (passes == 0) {
              r.workitem_get_from(unary_op, aI);
            } else {
              r.workitem_get_from(aR);
            }
            r.combine_threads(binary_op);
            r.workgroup_write_to(aR);
          });  // end kernel
    };         // end command group
    q.submit(f);
    length = length / local;
    passes++;
  } while (length > 1);
  q.wait_and_throw();
  auto hr = bufR.template get_access<cl::sycl::access::mode::read,
                                     cl::sycl::access::target::host_buffer>();
  return hr[0];
}

}  // namespace impl
}  // namespace sycl

#endif  // __SYCL_IMPL_ALGORITHM_COUNT_IF__
