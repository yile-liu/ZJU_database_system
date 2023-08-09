#include "common/instance.h"
#include "gtest/gtest.h"
#include "index/b_plus_tree.h"
#include "index/basic_comparator.h"
#include "utils/tree_file_mgr.h"
#include "utils/utils.h"

static const std::string db_name = "bp_tree_insert_test.db";

TEST(BPlusTreeTests, SampleTest) {
  // Init engine
  DBStorageEngine engine(db_name);
  BasicComparator<int> comparator;
  BPlusTree<int, int, BasicComparator<int>> tree(0, engine.bpm_, comparator, 4, 4);
  TreeFileManagers mgr("tree_");
  // Prepare data
  const int n = 30;
  vector<int> keys;
  vector<int> values;
  vector<int> delete_seq;
  map<int, int> kv_map;
  for (int i = 0; i < n; i++) {
    keys.push_back(i);
    values.push_back(i);
    delete_seq.push_back(i);
  }
//   Shuffle data
  ShuffleArray(keys);
  ShuffleArray(values);
  ShuffleArray(delete_seq);
//
//  ifstream in1;
//  in1.open("keys.txt");
//  int num;
//  for (int j = 0; j < n; ++j) {
//    in1>>num;
//    keys.push_back(num);
//  }
//  ifstream in2;
//  in2.open("values.txt");
//  for (int j = 0; j < n; ++j) {
//    in2>>num;
//    values.push_back(num);
//  }
//  ifstream in3;
//  in3.open("delete.txt");
//  for (int j = 0; j < n; ++j) {
//    in3>>num;
//    delete_seq.push_back(num);
//  }
  // Map key value
  for (int i = 0; i < n; i++) {
    kv_map[keys[i]] = values[i];
  }
  // Insert data
  for (int i = 0; i < n; i++) {
    tree.Insert(keys[i], values[i]);
  }
  ASSERT_TRUE(tree.Check());
  // Print tree
  tree.PrintTree(mgr[0]);
  // Search keys
  vector<int> ans;
  for (int i = 0; i < n; i++) {
    tree.GetValue(i, ans);
    ASSERT_EQ(kv_map[i], ans[i]);
  }
  ASSERT_TRUE(tree.Check());
  ofstream out1;
  out1.open("before.txt");
  tree.PrintTree(out1);
  // Delete half keys
  for (int i = 0; i < n / 2; i++) {
    tree.Remove(delete_seq[i]);
  }
  tree.PrintTree(mgr[1]);
  // Check valid
  ans.clear();
  for (int i = 0; i < n / 2; i++) {
    ASSERT_FALSE(tree.GetValue(delete_seq[i], ans));
  }
  for (int i = n / 2; i < n; i++) {
//    if (!tree.GetValue(delete_seq[i], ans)){
//      ofstream out2;
//      out2.open("after.txt");
//      tree.PrintTree(out2);
//      ofstream out3;
//      out3.open("keys.txt");
//      for (int j = 0; j < n; ++j) {
//        out3<<keys[j];
//        out3<<endl;
//      }
//      ofstream out4;
//      out4.open("values.txt");
//      for (int j = 0; j < n; ++j) {
//        out4<<values[j];
//        out4<<endl;
//      }
//      ofstream out5;
//      out5.open("delete.txt");
//      for (int j = 0; j < n; ++j) {
//        out5<<delete_seq[j];
//        out5<<endl;
//      }
//      cout<<"stop"<<endl;
//    }
    ASSERT_TRUE(tree.GetValue(delete_seq[i], ans));
    ASSERT_EQ(kv_map[delete_seq[i]], ans[ans.size() - 1]);
  }
}
