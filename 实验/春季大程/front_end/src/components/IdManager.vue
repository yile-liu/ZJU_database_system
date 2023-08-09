<template>
  <el-row>
    <el-col :span="3">
      <b>借书卡查询</b>
    </el-col>
  </el-row>
  <el-form :model="manage_form" label-width="80px" style="margin-top: 20px">
    <el-row>
      <el-col :span="12">
        <el-form-item label="卡号">
          <el-input v-model="manage_form.cno" :disabled="Show.is_show"/>
        </el-form-item>
      </el-col>
      <el-col :span="12">
        <el-form-item label="姓名">
          <el-input v-model="manage_form.name" :disabled="Show.is_show"/>
        </el-form-item>
      </el-col>
    </el-row>
    <el-row justify="space-evenly">
      <el-col :span="12">
        <el-form-item label="学院">
          <el-input v-model="manage_form.department" :disabled="Show.is_show"/>
        </el-form-item>
      </el-col>
      <el-col :span="12">
        <el-form-item>
          <el-button type="primary" @click="onSubmit" :icon="Search" :disabled="Show.is_show"> 查询</el-button>
          <el-button :icon="Delete" @click="clear" :disabled="Show.is_show"> 清空</el-button>
          <el-button :icon="Plus" type="success" @click="show" v-if="!Show.is_show">新建</el-button>
          <el-button :icon="Plus" type="success" @click="show" v-else>取消</el-button>
        </el-form-item>
      </el-col>
    </el-row>
  </el-form>

  <el-row v-show="Show.is_show">
    <el-col :span="3">
      <b>新建借书卡</b>
    </el-col>
  </el-row>
  <el-form :model="newCard" label-width="80px" v-show="Show.is_show" ref="newform" style="margin-top: 20px">
    <el-row>
      <el-col :span="12">
        <el-form-item label="卡号" prop="cno">
          <el-input v-model="newCard.cno"/>
        </el-form-item>
      </el-col>
      <el-col :span="12">
        <el-form-item label="姓名" prop="name">
          <el-input v-model="newCard.name"/>
        </el-form-item>
      </el-col>
    </el-row>
    <el-row>
      <el-col :span="12">
        <el-form-item label="学院" prop="department">
          <el-input v-model="newCard.department"/>
        </el-form-item>
      </el-col>
      <el-form-item prop="type">
        <el-select v-model="newCard.type" clearable placeholder="类型" :sapn="6">
          <el-option
              v-for="item in options"
              :key="item.value"
              :label="item.label"
              :value="item.value"
          />
        </el-select>
      </el-form-item>
      <el-col class="text-center" :span="1">
        <span class="text-gray-500"> </span>
      </el-col>
      <el-button type="success" @click="create_card">创建借书卡
      </el-button>
    </el-row>
  </el-form>

  <el-row>
    <el-col :span="3" v-show="!Show.is_show">
      <b>查询结果</b>
    </el-col>
  </el-row>
  <el-table :data="tableData" :key="new Date().getTime()" v-show="!Show.is_show"
            style="width: 100%;margin-top: 20px" stripe border>
    <el-table-column fixed sortable prop="cno" label="卡号" width="100"/>
    <el-table-column sortable prop="name" label="姓名" width="140"/>
    <el-table-column sortable prop="department" label="学院"/>
    <el-table-column sortable prop="type" label="类型" width="120"/>
    <el-table-column fixed="right" label="删除借书证" width="120">
      <template #default="scope">
        <el-button type="danger" @click="delete_card(scope.row.cno,scope.row.name)">删除
        </el-button>
      </template>

    </el-table-column>
  </el-table>
</template>


<script setup>
import {reactive, ref} from "vue";
import {Delete, Search, Plus} from "@element-plus/icons-vue";
import axios from "axios";
import {loginRouter} from "@/router";

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

const manage_form = reactive({
  cno: '',
  name: '',
  department: '',
})

function clear() {
  let index
  for (index in manage_form) {
    manage_form[index] = ''
  }
}

const tableData = reactive([]);

const onSubmit = async () => {
  console.log('submit: ', manage_form)
  await axios.post('/api/manager', {
    cno: manage_form.cno,
    name: manage_form.name,
    department: manage_form.department
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

function delete_card(cno, name) {
  let isdel = confirm("确定要删除" + name + "的借书证吗？删除后信息无法恢复！");
  if (!isdel)
    return;
  axios({
    method: 'post',
    url: '/api/delete_card',
    data: {
      'cno': cno,
      'search_cno': manage_form.cno,
      'name': manage_form.name,
      'department': manage_form.department
    },
    withCredentials: true
  })
      .then((response) => {
        console.log('response:', response)
        if (response.data.request === 'failed')
          alert("出错了，原因:" + response.data.reason)
        else {
          alert("删除成功！")
          tableData.splice(0, tableData.length)
          for (let i = 0; i < response.data.length; ++i)
            tableData.splice(0, 0, response.data[i])
          console.log('tableData:', tableData)
        }
      })
      .catch((error) => {
        alert('出错了')
      })
}

const newCard = reactive({
  cno: '',
  name: '',
  department: '',
  type: '',
})

const options = [
  {
    value: 'O',
    label: '管理员',
  },
  {
    value: 'T',
    label: '教师',
  },
  {
    value: 'G',
    label: '研究生',
  },
  {
    value: 'U',
    label: '本科生',
  },
]

const Show = reactive({
  is_show: false
})
const newform = ref(null);

function show() {
  Show.is_show = !Show.is_show;
  if (!Show.is_show)//clear newCard
    newform.value.resetFields();
}

const create_card = async () => {
  console.log('submit:', newCard);
  if (newCard.cno === '') {
    alert('卡号不能为空');
    return;
  }
  if (newCard.name === '') {
    alert('姓名不能为空');
    return;
  }
  if (newCard.department === '') {
    alert('学院不能为空');
    return;
  }
  if (newCard.type === '') {
    alert('未选择类型');
    return;
  }

  let is_repeat = false;
  await axios.post('/api/check_repeat', {
    cno: newCard.cno,
  })
      .then((response) => {
        console.log('this first');
        if (response.data.request === 'failed') {
          if (response.data.reason === 'repeatCno') {
            alert('已存在卡号:' + newCard.cno);
            is_repeat = true;
          }
          if (response.data.reason === 'databaseError')
            alert('数据库错误');
        }
      })
      .catch((error) => {
        alert('出错了');
      })
  if (is_repeat)
    return;

  let pass = false;
  let password1;
  while (!pass) {
    password1 = prompt('请设置密码:');
    if (password1 == null)
      return;
    let password2 = prompt('请再次输入密码:');
    if (password2 == null)
      return;
    if (password1 !== password2)
      alert('两次密码不一样!');
    else
      pass = true;
      alert('新建借书卡成功！');
      Show.is_show=false;
  }
  await axios.post('/api/create_card', {
    cno: newCard.cno,
    name: newCard.name,
    department: newCard.department,
    type: newCard.type,
    password: password1,
    cur_cno: manage_form.cno,
    cur_name: manage_form.name,
    cur_department: manage_form.department,
  })
      .then((response) => {
        console.log(response);
        if (response.data.request === 'failed') {
          if (response.data.reason === 'databaseError')
            alert('数据库错误');
        } else {
          tableData.splice(0, tableData.length)
          for (let i = 0; i < response.data.length; ++i)
            tableData.splice(0, 0, response.data[i])
        }
      })
      .catch((error) => {
        alert('出错了');
      })
}
</script>

<script>
export default {
  name: "IdManager"
}
</script>