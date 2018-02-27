#ifndef CONFLUO_CONTAINER_CURSOR_RECORD_CURSOR_H_
#define CONFLUO_CONTAINER_CURSOR_RECORD_CURSOR_H_

#include "batched_cursor.h"
#include "offset_cursors.h"
#include "schema/record.h"
#include "parser/expression_compiler.h"
#include "data_log.h"

namespace confluo {

typedef batched_cursor<record_t> record_cursor;

class filter_record_cursor : public record_cursor {
 public:
  filter_record_cursor(std::unique_ptr<offset_cursor> o_cursor,
                       const data_log* dlog, const schema_t* schema,
                       const parser::compiled_expression& cexpr,
                       size_t batch_size = 64)
      : record_cursor(batch_size),
        o_cursor_(std::move(o_cursor)),
        dlog_(dlog),
        schema_(schema),
        cexpr_(cexpr) {
    init();
  }

  virtual size_t load_next_batch() override {
    size_t i = 0;
    for (; i < current_batch_.size() && o_cursor_->has_more();
        ++i, o_cursor_->advance()) {
      uint64_t o = o_cursor_->get();
      ro_data_ptr ptr;
      dlog_->cptr(o, ptr);
      if (!cexpr_.test(current_batch_[i] = schema_->apply(o, ptr))) {
        i--;
      }
    }
    return i;
  }

 private:
  std::unique_ptr<offset_cursor> o_cursor_;
  const data_log* dlog_;
  const schema_t* schema_;
  const parser::compiled_expression& cexpr_;
};

}

#endif /* CONFLUO_CONTAINER_CURSOR_RECORD_CURSOR_H_ */
