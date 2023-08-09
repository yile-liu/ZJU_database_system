<template>
  <el-row>
    <el-col :span="3">
      <b>单本入库</b>
    </el-col>
  </el-row>
  <el-form :model="form" label-width="80px" style="margin-top: 20px">
    <el-row>
      <el-col :span="12">
        <el-form-item label="书号">
          <el-input min="0" v-model="form.bno"/>
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
      <el-col :span="8">
        <el-form-item label="年份">
          <el-input min="0" v-model="form.year"/>
        </el-form-item>
      </el-col>
      <el-col :span="8">
        <el-form-item label="价格">
          <el-input min="0" v-model="form.price"/>
        </el-form-item>
      </el-col>
      <el-col :span="8">
        <el-form-item label="类别">
          <el-input v-model="form.category"/>
        </el-form-item>
      </el-col>
    </el-row>
    <el-row>
      <el-col :span="8">
        <el-form-item label="库存">
          <el-input min="0" v-model="form.stock"/>
        </el-form-item>
      </el-col>
      <el-col :span="8">
        <el-form-item label="总藏书量">
          <el-input min="0" v-model="form.total"/>
        </el-form-item>
      </el-col>
      <el-col :span="8">
        <el-form-item>
          <el-button type="primary" @click="onSubmit" :icon="Plus"> 入库</el-button>
          <el-button :icon="Delete" @click="clear"> 清空</el-button>
        </el-form-item>
      </el-col>
    </el-row>
  </el-form>
  <el-row style="margin-top: 20px">
    <el-col :span="3">
      <b>批量入库</b>
    </el-col>
    <el-col :span="5">
      <el-upload
          class="upload-demo"
          action="/api/batch"
          :on-success="handleSuccess"
          multiple
          :limit="3"
          :file-list="fileList"
      >
        <el-button type="primary">批量入库</el-button>
      </el-upload>
    </el-col>
  </el-row>
</template>

<script setup>
import {reactive} from "vue";
import axios from "axios";
import {Delete, Plus} from "@element-plus/icons-vue";
import {loginRouter} from "@/router";

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
      }
    })
    .catch((error) => {
      alert('出错了')
    })

const handleSuccess = (response, file) => {
  if (response['request'] === 'succeeded') {
    alert('入库成功')
  } else {
    alert('出错了')
  }
}
const form = reactive({
  category: '',
  title: '',
  press: '',
  year: '',
  price: '',
  author: '',
  bno: '',
  stock: '',
  total: '',
})
const onSubmit = async () => {
  let flag = 1;
  console.log('submit:', form)
  if (form.bno === '' || form.category === '' || form.title === ''
      || form.press === '' || form.year === '' || form.price === ''
      || form.author === '' || form.stock === '' || form.total === '') {
    alert('不能有项目为空')
    flag = 0
  } else if (form.year !== '' && !/^\d+$/.test(form.year)) {
    alert('年份应该是一个整数')
    flag = 0
  } else if (form.price !== '' && !/^[1-9][0-9]*([.][0-9]{1,2})?$/.test(form.price)) {
    alert('价格应该是一个数字 小数部分不超过两位')
    flag = 0
  } else if (form.stock !== '' && !/^\d+$/.test(form.stock)) {
    alert('库存应该是一个整数')
    flag = 0
  } else if (form.total !== '' && !/^\d+$/.test(form.total)) {
    alert('总量应该是一个整数')
    flag = 0
  }
  if (flag) {
    await axios.post('/api/import', {
      category: form.category,
      title: form.title,
      press: form.press,
      author: form.author,
      year: parseInt(form.year),
      price: parseFloat(form.price),
      bno: parseFloat(form.bno),
      total: parseFloat(form.total),
      stock: parseFloat(form.stock),
    })
        .then((response) => {
          if(response.data.request === 'succeeded') {
            alert('入库成功！')
          } else if(response.data.reason === 'duplicateBno') {
            alert('已存在书号：' + form.bno)
          }
        })
        .catch((error) => {
          alert('出错了')
        })
  }
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
  name: "ImportBook"
}
</script>