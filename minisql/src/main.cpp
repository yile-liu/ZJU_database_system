#include <cstdio>
#include <ctime>
#include "executor/execute_engine.h"
#include "glog/logging.h"
#include "parser/syntax_tree_printer.h"
#include "utils/tree_file_mgr.h"

extern "C" {
int yyparse(void);
FILE *yyin;
#include "parser/minisql_lex.h"
#include "parser/parser.h"
}

// #define ENABLE_PARSER_DEBUG

void InitGoogleLog(char *argv) {
  FLAGS_logtostderr = true;
  FLAGS_colorlogtostderr = true;
  google::InitGoogleLogging(argv);
}

void InputCommand(char *input, const int len) {
  memset(input, 0, len);
  printf("minisql > ");
  int i = 0;
  char ch;
  while ((ch = getchar()) != ';') {
    input[i++] = ch;
  }
  input[i] = ch;  // ;
  getchar();      // remove enter
}

int main(int argc, char **argv) {
  InitGoogleLog(argv[0]);
  // command buffer
  const int buf_size = 1024;
  char cmd[buf_size];
  // execute engine
  ExecuteEngine engine;
  // for print syntax tree
  TreeFileManagers syntax_tree_file_mgr("syntax_tree_");
  [[maybe_unused]] uint32_t syntax_tree_id = 0;

  while (1) {
    // read from buffer
    InputCommand(cmd, buf_size);
    // create buffer for sql input
    YY_BUFFER_STATE bp = yy_scan_string(cmd);
    if (bp == nullptr) {
      LOG(ERROR) << "Failed to create yy buffer state." << std::endl;
      exit(1);
    }
    yy_switch_to_buffer(bp);

    // init parser module
    MinisqlParserInit();

    // parse
    yyparse();

    // parse result handle
    if (MinisqlParserGetError()) {
      // error
      printf("%s\n", MinisqlParserGetErrorMessage());
    } else {
#ifdef ENABLE_PARSER_DEBUG
      printf("[INFO] Sql syntax parse ok!\n");
      SyntaxTreePrinter printer(MinisqlGetParserRootNode());
      printer.PrintTree(syntax_tree_file_mgr[syntax_tree_id++]);
#endif
    }

    ExecuteContext context;

    time_t begin, end;
    double ret;
    begin = clock();
    if (MinisqlGetParserRootNode() != nullptr)
      if (MinisqlGetParserRootNode()->type_ == kNodeExecFile) {
        ifstream in;
        in.open(MinisqlGetParserRootNode()->child_->val_);
        if (in.is_open())
          while (!in.eof()) {
            in.getline(cmd, 1024, ';');
            strcat(cmd, ";\0");

            YY_BUFFER_STATE bp = yy_scan_string(cmd);
            if (bp == nullptr) {
              LOG(ERROR) << "Failed to create yy buffer state." << std::endl;
              exit(1);
            }
            yy_switch_to_buffer(bp);

            // init parser module
            MinisqlParserInit();

            // parse
            yyparse();

            // parse result handle
            if (MinisqlParserGetError()) {
              // error
              printf("%s\n", MinisqlParserGetErrorMessage());
            }

            ExecuteContext context_file;
            engine.Execute(MinisqlGetParserRootNode(), &context_file);
          } else {
          cout << "No such file." << endl;
        }
      } else
        engine.Execute(MinisqlGetParserRootNode(), &context);
    // sleep(1);

    end = clock();
    ret = double(end - begin) / CLOCKS_PER_SEC;
    cout << "runtime:   " << ret << endl;

    // clean memory after parse
    MinisqlParserFinish();
    yy_delete_buffer(bp);
    yylex_destroy();

    // quit condition
    if (context.flag_quit_) {
      printf("bye!\n");
      break;
    }
  }
  return 0;
}