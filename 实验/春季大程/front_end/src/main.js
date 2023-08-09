import {createApp} from 'vue'
import App from './App.vue'
// 引入element-plus
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'

import {loginRouter} from "@/router"

const app = createApp(App)
app.use(loginRouter)
app.use(ElementPlus)
app.mount('#app')
