#ifndef MINISQL_TABLE_ITERATOR_H
#define MINISQL_TABLE_ITERATOR_H

#include "buffer/buffer_pool_manager.h"
#include "common/rowid.h"
#include "record/row.h"
#include "transaction/lock_manager.h"
#include "transaction/log_manager.h"
#include "transaction/transaction.h"

class TableHeap;

class TableIterator {
 public:
  // you may define your own constructor based on your member variables

  explicit TableIterator();

  explicit TableIterator(TableHeap *table_heap, const Row &row);

  TableIterator(const TableIterator &other);

  virtual ~TableIterator();

  inline bool operator==(const TableIterator &itr) const{
    return (table_heap_ == itr.table_heap_) && (row_->GetRowId() == itr.row_->GetRowId());
  }

  inline bool operator!=(const TableIterator &itr) const{
    return !(*this == itr);
  }

  const Row &operator*();

  Row *operator->();

  TableIterator &operator++();

  TableIterator operator++(int);

  int64_t GetRowId_64();

 private:
  // add your own private member variables here
  TableHeap *table_heap_;
  Row *row_;
};

#endif  // MINISQL_TABLE_ITERATOR_H
