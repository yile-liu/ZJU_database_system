<template>
  <el-row v-if="user.isAdmin">
    <el-col :span="3">
      <b>未归还查询</b>
    </el-col>
  </el-row>
  <el-row v-if="!user.isAdmin">
    <el-col :span="3">
      <b>未归还书籍</b>
    </el-col>
  </el-row>
  <el-row style="margin-top: 20px" v-if="user.isAdmin">
    <el-form :model="form" label-width="80px">
      <el-row>
        <el-col :span="12">
          <el-form-item label="卡号">
            <el-input v-model="form.cno"/>
          </el-form-item>
        </el-col>
        <el-col :span="12">
          <el-form-item>
            <el-button type="primary" @click="onSubmit" :icon="Search"> 查询</el-button>
            <el-button :icon="Delete" @click="clear"> 清空</el-button>
          </el-form-item>
        </el-col>
      </el-row>
    </el-form>
  </el-row>
  <el-row v-if="user.isAdmin" style="margin-top: 20px">
    <el-col :span="3">
      <b>查询结果</b>
    </el-col>
  </el-row>
  <el-table :data="tableData" :key="new Date().getTime()" style="width: 100%;margin-top: 20px" stripe border>
    <el-table-column fixed sortable prop="bno" label="书号" width="80"/>
    <el-table-column sortable prop="category" label="类别" width="120"/>
    <el-table-column sortable prop="title" label="书名"/>
    <el-table-column sortable prop="press" label="出版社"/>
    <el-table-column sortable prop="year" label="年份" width="80"/>
    <el-table-column sortable prop="author" label="作者"/>
    <el-table-column sortable prop="price" label="价格" width="80"/>
    <el-table-column fixed="right" label="还书" width="100" v-if="user.isAdmin">
      <template #default="scope">
        <el-button type="primary" @click="returnBook(scope.row.bno, scope.row.stock)">
          还书
        </el-button>
      </template>
    </el-table-column>
  </el-table>
</template>
<script setup>
import {reactive} from "vue";
import axios from "axios";
import {loginRouter} from "@/router";

const form = reactive({
  cno: ''
})
const user = reactive({
  username: '',
  isAdmin: false
})
import {Delete, Search} from "@element-plus/icons-vue";

const tableData = reactive([])

//if logout, redirect to /login
axios({
  method: 'post',
  url: '/api/checkLogin',
  withCredentials: true
})
    .then((response) => {
      console.log(response)
      if (response.data.state === 'logout') {
        loginRouter.push('/login')
      } else {
        user.isAdmin = response.data.isAdmin
      }
      if (user.isAdmin === false) {
        axios({
          method: 'post',
          url: '/api/getborrow',
          data: {'cno': form.cno, 'isAdmin': user.isAdmin},
          withCredentials: true
        })
            .then((response) => {
              console.log(response.data)
              tableData.splice(0, tableData.length)
              for (let i = 0; i < response.data.length; ++i)
                tableData.splice(0, 0, response.data[i])
            })
      }
    })
    .catch((error) => {
      alert('出错了')
    })
//get borrowed book


const onSubmit = async () => {
  console.log('submit:', form)
  if (form.cno === '')
    alert('卡号不能为空')

  await axios({
    method: 'post',
    url: '/api/getborrow',
    data: {'cno': form.cno, 'isAdmin': user.isAdmin},
    withCredentials: true
  })
      .then((response) => {
        console.log(response.data)
        tableData.splice(0, tableData.length)
        for (let i = 0; i < response.data.length; ++i)
          tableData.splice(0, 0, response.data[i])
      })
}

function returnBook(bno, stock) {
  axios({
    method: 'post',
    url: '/api/returnbook',
    data: {'bno': bno, 'stock': stock, 'cno': form.cno},
    withCredentials: true
  })
      .then((response) => {
        if (response.data.request === 'failed') {
          alert("出错了！原因：", response.data.reason)
        } else {
          console.log(response.data)
          tableData.splice(0, tableData.length)
          for (let i = 0; i < response.data.length; ++i)
            tableData.splice(0, 0, response.data[i])
          console.log(tableData)
          alert("还书成功！")
        }
      })
}

function clear() {
  let index
  for (index in form) {
    form[index] = ''
  }
}
</script>

<script>
export default {
  name: "ReturnBook"
}
</script>