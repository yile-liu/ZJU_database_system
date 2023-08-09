//导入方法
import {createRouter, createWebHashHistory} from "vue-router";

//创建网页内容实例，对应components/LoginView.vue
const login = () => import("../components/LoginView")
const index = () => import("../components/MainView")
//创建路由实例
const loginRoutes = [
    {
        path: "/",
        redirect: "/login"
    },
    {
        path: "/login",
        name: "login",
        component: login
    },
    {
        path: "/index",
        name: 'index',
        component: index,
        children: [
            {
                path: '/index/search',
                name: 'search',
                component: () => import('../components/SearchBook')
            },
            {
                path: '/index/return',
                name: 'return',
                component: () => import('../components/ReturnBook')
            },
            {
                path: '/index/import',
                name: 'import',
                component: () => import('../components/ImportBook')
            },
            {
                path: '/index/manager',
                name: 'manager',
                component: () => import('../components/IdManager')
            }
        ]
    }
]
//用于在main.js导入的默认配置
export const loginRouter = createRouter({
    history: createWebHashHistory(),
    routes: loginRoutes
})
