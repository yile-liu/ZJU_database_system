#ifndef MINISQL_EXECUTE_ENGINE_H
#define MINISQL_EXECUTE_ENGINE_H

#include <set>
#include <string>
#include <unordered_map>
#include "common/dberr.h"
#include "common/instance.h"
#include "transaction/transaction.h"

extern "C" {
#include "parser/parser.h"
};

const char *LIST_FILE = "list.dblist";
const int MAX_FILE_NAME_LENGTH = 1000;

/**
 * ExecuteContext stores all the context necessary to run in the execute engine
 * This struct is implemented by student self for necessary.
 *
 * eg: transaction info, execute result...
 */
struct ExecuteContext {
  bool flag_quit_{false};
  Transaction *txn_{nullptr};
  string table_name_;
  // 中间节点返回的临时表
  std::set<RowId> internal_result_rid_;
  //select需要输出的列
  std::vector<Column *> select_columns_;

  // 抄来的不知道有什么用
  int print_flag;
  bool dont_print = false;
  struct timeval start;
  int rows_num = 0;
  vector<Column *> columns;
  vector<uint32_t> key_map;
  map<string, IndexInfo *> index_on_one_key;
  string index_name;
};

/**
 * ExecuteEngine
 */
class ExecuteEngine {
 public:
  ExecuteEngine();

  ~ExecuteEngine() {
    fstream list_file;
    list_file.open(LIST_FILE, std::ios::binary | std::ios::out);
    for (auto it : dbs_) {
      list_file << it.first << '\n';
      delete it.second;
    }
  }

  /**
   * executor interface
   */
  dberr_t Execute(pSyntaxNode ast, ExecuteContext *context);

 private:
  dberr_t ExecuteCreateDatabase(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteDropDatabase(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteShowDatabases(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteUseDatabase(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteShowTables(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteCreateTable(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteDropTable(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteShowIndexes(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteCreateIndex(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteDropIndex(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteSelect(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteInsert(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteDelete(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteUpdate(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteTrxBegin(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteTrxCommit(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteTrxRollback(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteExecfile(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteQuit(pSyntaxNode ast, ExecuteContext *context);

 private:
  dberr_t ExecuteInternalNode(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteCompareOperator(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteAllColumn(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteColumnList(pSyntaxNode ast, ExecuteContext *context);

  dberr_t ExecuteConnector(pSyntaxNode ast, ExecuteContext *context);

  std::unordered_map<std::string, DBStorageEngine *> dbs_; /** all opened databases */
  std::string current_db_;                                 /** current database */
};

#endif  // MINISQL_EXECUTE_ENGINE_H
