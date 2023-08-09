<template>
  <el-container>
    <el-aside width="200px">
      <el-row class="tac">
        <el-col :span="24">
          <h5 style="font-size: 20px">你好，{{ user.username }}</h5>
          <el-menu
              default-active="/index/search"
              class="el-menu-vertical-demo"
              router
          >
            <el-menu-item index="/index/search">
              <el-icon>
                <search/>
              </el-icon>
              <span>图书借阅</span>
            </el-menu-item>
            <el-menu-item index="/index/return">
              <el-icon>
                <d-arrow-right/>
              </el-icon>
              <span>还书</span>
            </el-menu-item>
            <el-menu-item index="/index/import" v-if="user.isAdmin">
              <el-icon>
                <setting/>
              </el-icon>
              <span>图书入库</span>
            </el-menu-item>
            <el-menu-item index="/index/manager" v-if="user.isAdmin">
              <el-icon>
                <setting/>
              </el-icon>
              <span>借书卡管理</span>
            </el-menu-item>
            <el-menu-item index="#" @click="logout">
              <span>退出系统</span>
            </el-menu-item>
          </el-menu>
        </el-col>
      </el-row>
    </el-aside>
    <el-container>
      <el-header height="100px">
        <h5 style="font-size: 25px;">ZJU图书管理系统</h5>
      </el-header>
      <el-main>
        <router-view :key="new Date().getTime()"></router-view>
      </el-main>
    </el-container>
  </el-container>

</template>

<script setup>
import {
  DArrowRight, Search,
  Setting,
} from '@element-plus/icons-vue'

import axios from "axios";
import {loginRouter} from "@/router";
import {reactive} from "vue";

const user = reactive({
  username: '',
  isAdmin: false
})

//if logout, redirect to /login
//if is not admin, hide register and import
// axios({
//   method: 'post',
//   url: '/api/checkLogin',
//   withCredentials: true
// })
//     .then((response) => {
//       console.log(response)
//       if (response.data.state === 'logout') {
//         loginRouter.push('/login')
//       } else {
//         user.isAdmin = response.data.isAdmin
//         user.username = response.data.username
//       }
//     })
//     .catch((error) => {
//       alert('出错了')
//     })

function logout() {
  axios({
    method: 'post',
    url: '/api/logout',
    withCredentials: true
  })
      .then((response) => {
        console.log(response)
        if (response.data.state === 'logout') {
          loginRouter.push('/login')
          alert('退出成功')
        }
      })
      .catch((error) => {
        alert('出错了')
      })
}

</script>

<script>
export default {
  name: "MainView"
}
</script>

