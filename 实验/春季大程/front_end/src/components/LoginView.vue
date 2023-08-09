<template>
  <el-container>
    <el-header height="100px">
      <h5 style="font-size: 25px;">ZJU图书管理系统</h5>
    </el-header>
    <el-main>
      <el-form :inline="true" :model="formInline" class="demo-form-inline">
        <el-form-item label="用户名">
          <el-input v-model="formInline.user" placeholder="Username" @keydown.enter="login"/>
        </el-form-item>
        <el-form-item label="密码">
          <el-input type="password" v-model="formInline.password" placeholder="Password" @keydown.enter="login"/>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="login">登录</el-button>
        </el-form-item>
      </el-form>
    </el-main>
  </el-container>
</template>

<script setup>
import {reactive} from 'vue'
import axios from "axios";
import {loginRouter} from "@/router";

const formInline = reactive({
  user: '',
  password: '',
})

//if login, redirect to /index
axios({
  method: 'post',
  url: '/api/checkLogin',
  withCredentials: true
})
    .then((response) => {
      console.log(response)
      if (response.data.state === 'login') {
        loginRouter.push('/index/search')
      }
    })
    .catch((error) => {
      console.log(error)
    })

function login() {
  axios({
    method: 'post',
    url: '/api/login',
    data: {'username': formInline.user, 'password': formInline.password},
    withCredentials: true
  })
      .then((response) => {
        console.log(response)
        if (response.data.state === 'login') {
          loginRouter.push('/index/search')
          alert('登录成功')
        } else {
          alert('用户名或密码错误')
          formInline.password = ''
        }
      })
      .catch((error) => {
        console.log(error)
      })
}
</script>


<script>
export default {
  name: 'LoginView'
}
</script>
