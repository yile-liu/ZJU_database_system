<template>
  <el-row>
    <el-col :span="3">
      <b>图书查询</b>
    </el-col>
  </el-row>
  <el-form :model="form" label-width="80px" style="margin-top: 20px">
    <el-row>
      <el-col :span="12">
        <el-form-item label="类别">
          <el-input v-model="form.category"/>
        </el-form-item>
      </el-col>
      <el-col :span="12">
        <el-form-item label="书名">
          <el-input v-model="form.title"/>
        </el-form-item>
      </el-col>
    </el-row>

    <el-row>
      <el-col :span="12">
        <el-form-item label="出版社">
          <el-input v-model="form.press"/>
        </el-form-item>
      </el-col>
      <el-col :span="12">
        <el-form-item label="作者">
          <el-input v-model="form.author"/>
        </el-form-item>
      </el-col>
    </el-row>

    <el-row>
      <el-col :span="4">
        <el-form-item label="年份">
          <el-input min="0" v-model="form.yearFrom"/>
        </el-form-item>
      </el-col>
      <el-col class="text-center" :span="1">
        <span class="text-gray-500">-</span>
      </el-col>
      <el-col :span="2" class="text-center">
        <el-form-item label-width="0px">
          <el-input min="0" v-model="form.yearTo"/>
        </el-form-item>
      </el-col>
      <el-col class="text-center" :span="1">
        <span class="text-gray-500"> </span>
      </el-col>
      <el-col :span="4">
        <el-form-item label="价格">
          <el-input min="0" v-model="form.priceFrom"/>
        </el-form-item>
      </el-col>
      <el-col class="text-center" :span="1">
        <span class="text-gray-500">-</span>
      </el-col>
      <el-col :span="2" class="text-center">
        <el-form-item label-width="0px">
          <el-input min="0" v-model="form.priceTo"/>
        </el-form-item>
      </el-col>
      <el-col class="text-center" :span="1">
        <span class="text-gray-500"> </span>
      </el-col>
      <el-col :span="8">
        <el-form-item>
          <el-button type="primary" @click="onSubmit" :icon="Search"> 查询</el-button>
          <el-button :icon="Delete" @click="clear"> 清空</el-button>
        </el-form-item>
      </el-col>
    </el-row>
  </el-form>

  <el-row>
    <el-col :span="3" style="margin-top: 20px">
      <b>查询结果</b>
    </el-col>
  </el-row>
  <el-table :data="tableData" style="width: 100%;margin-top: 20px" stripe border>
    <el-table-column fixed sortable prop="bno" label="书号" width="80"/>
    <el-table-column sortable prop="category" label="类别" width="120"/>
    <el-table-column sortable prop="title" label="书名"/>
    <el-table-column sortable prop="press" label="出版社"/>
    <el-table-column sortable prop="year" label="年份" width="80"/>
    <el-table-column sortable prop="author" label="作者"/>
    <el-table-column sortable prop="price" label="价格" width="80"/>
    <el-table-column sortable prop="stock" label="库存" width="80"/>
    <el-table-column fixed="right" label="借书" width="100" v-if="user.isAdmin">
      <template #default="scope">
        <el-button type="primary" :disabled="scope.row.stock === 0" @click="borrow(scope.row.bno,scope.row.stock)">借阅
        </el-button>
      </template>

    </el-table-column>
  </el-table>
</template>


<script setup>
import {reactive} from 'vue'
import {Delete, Search} from "@element-plus/icons-vue";
import axios from "axios"
import {loginRouter} from "@/router";

const user = reactive({
  username: '',
  isAdmin: false
})
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
    })
    .catch((error) => {
      alert('出错了')
    })

// do not use same name with ref
const form = reactive({
  category: '',
  title: '',
  press: '',
  yearFrom: '',
  yearTo: '',
  priceFrom: '',
  priceTo: '',
  author: '',
})

function clear() {
  let index
  for (index in form) {
    form[index] = ''
  }
}

const tableData = reactive([])

const onSubmit = async () => {
  console.log('submit:', form)
  if (form.yearFrom !== '' && !/^\d+$/.test(form.yearFrom))
    alert('年份应该是一个整数')
  else if (form.yearTo !== '' && !/^\d+$/.test(form.yearTo))
    alert('年份应该是一个整数')
  else if (form.priceFrom !== '' && !/^[1-9][0-9]*([.][0-9]{1,2})?$/.test(form.priceFrom))
    alert('价格应该是一个数字 小数部分不超过两位')
  else if (form.priceTo !== '' && !/^[1-9][0-9]*([.][0-9]{1,2})?$/.test(form.priceTo))
    alert('价格应该是一个数字 小数部分不超过两位')

  await axios.post('/api/search', {
    category: form.category,
    title: form.title,
    press: form.press,
    author: form.author,
    yearFrom: parseInt(form.yearFrom),
    yearTo: parseInt(form.yearTo),
    priceFrom: parseFloat(form.priceFrom),
    priceTo: parseFloat(form.priceTo),
  })
      .then((response) => {
        console.log(response.data)
        tableData.splice(0, tableData.length)
        for (let i = 0; i < response.data.length; ++i)
          tableData.splice(0, 0, response.data[i])
      })
      .catch((error) => {
        alert('出错了')
      })
}

function borrow(bno, stock) {
  let borrowcno
  borrowcno = prompt('请输入要借书的卡号:');
  axios({
    method: 'post',
    url: '/api/borrow',
    data: {'bno': bno, 'stock': stock, 'borrowcno': borrowcno},
    withCredentials: true
  })
      .then((response) => {
        console.log(response)
        if (response.data.request === 'succeeded') {
          for (let i = 0; i < tableData.length; ++i) {
            if (tableData[i].bno === bno) {
              tableData[i].stock = stock - 1
            }
          }
          alert("借书成功！")
        } else if (response.data.reason === 'haveBorrowed') {
          alert("您已经借过这本书！")
        } else if (response.data.reason === 'databaseError') {
          alert("出错了！原因：" + response.data.reason)
        }
      })
      .catch((error) => {
        alert('出错了')
      })
}
</script>

<script>
export default {
  name: "SearchBook"
}
</script>
